#include "csnet_conntor.h"
#include "csnet_atomic.h"
#include "csnet_head.h"
#include "csnet_cmd.h"
#include "csnet_socket_api.h"
#include "csnet_utils.h"
#include "csnet_log.h"
#include "csnet_msg.h"
#include "csnet_fast.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <strings.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#ifdef JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

#define MAGIC_NUMBER 1024
#define SLOT_SIZE 32

struct server_node {
	csnet_server_type_t server_type;
	int index;
	int port;
	char ip[16];
	char name[64];
	unsigned int sids[4];
};

struct slot {
	cs_dl_node_t* curr_node;
	cs_dlist_t* list;
};

static inline void
_heartbeat(csnet_conntor_t* conntor) {
	int expired_wheel = csnet_timer_book_keeping(conntor->timer);
	if (expired_wheel > -1) {
		cs_lfhash_t* hashtable = conntor->timer->wheels_tbl[expired_wheel];
		cs_lflist_t* keys = cs_lfhash_get_all_keys(hashtable);
		cs_lflist_node_t* head = keys->head->next;
		while (head != keys->tail) {
			unsigned int sid = head->key;
			LOG_INFO(conntor->log, "sid[%d] timeout, heartbeating", sid);

			csnet_head_t h = {
				.version = 0x0,
				.compress = 0,
				.cmd = CSNET_HEARTBEAT_SYN,
				.status = 0x0,
				.ctxid = 0x0,
				.sid = sid,
				.len = HEAD_LEN
			};

			csnet_sock_t* sock = csnet_sockset_get(conntor->sockset, sid);
			csnet_msg_t* msg = csnet_msg_new(h.len, sock);
			csnet_msg_append(msg, (char*)&h, h.len);
			cs_lfqueue_enq(conntor->q, msg);
			head = head->next;
		}
		cs_lflist_free(keys);
	}
}

static void*
_conntor_thread(void* arg) {
	csnet_conntor_t* conntor = (csnet_conntor_t *)arg;
	cs_lfqueue_t* q = conntor->q;
	cs_lfqueue_register_thread(q);

	while (1) {
		int ready = csnet_epoller_wait(conntor->epoller, 1000);
		for (int i = 0; i < ready; ++i) {
			csnet_epoller_event_t* ee;
			csnet_sock_t* sock;
			unsigned int sid;
			int fd;

			ee = csnet_epoller_get_event(conntor->epoller, i);
			sid = csnet_epoller_event_sid(ee);
			sock = csnet_sockset_get(conntor->sockset, sid);
			fd = sock->fd;

			if (csnet_fast(csnet_epoller_event_is_readable(ee))) {
				int nrecv = csnet_sock_recv(sock);
				if (csnet_fast(nrecv > 0)) {
					while (1) {
						char* data = csnet_rb_data(sock->rb);
						unsigned int data_len = sock->rb->data_len;
						csnet_head_t* head = (csnet_head_t*)data;

						if (data_len < HEAD_LEN) {
							break;
						}

						if (head->len < HEAD_LEN || head->len > MAX_LEN) {
							LOG_ERROR(conntor->log, "Incorrect package from %d. head len: %d", fd, head->len);
							csnet_epoller_del(conntor->epoller, fd, sid);
							csnet_sockset_reset_sock(conntor->sockset, sid);
							csnet_timer_remove(conntor->timer, sid);
							struct server_node* server_node = cs_lfhash_search(conntor->hashtbl, sid);

							if (server_node) {
								cs_lfhash_delete(conntor->hashtbl, sid);
								for (int j = 0; j < 4; j++) {
									if (server_node->sids[j] == sid) {
										server_node->sids[j] = 0;
									}
								}
							}
							break;
						}

						if (data_len < head->len) {
							break;
						}

						if (csnet_fast(head->cmd != CSNET_HEARTBEAT_ACK)) {
							csnet_module_ref_increment(conntor->module);
							csnet_module_entry(conntor->module, sock, head, data + HEAD_LEN,
									head->len - HEAD_LEN);
						} else {
							/* Do nothing when recv CSNET_HEARTBEAT_ACK */
						}

						if (csnet_rb_seek(sock->rb, head->len) == 0) {
							break;
						}
					}

					csnet_epoller_mod_read(conntor->epoller, fd, sid);
				} else if (nrecv == 0) {
					csnet_epoller_mod_read(conntor->epoller, fd, sid);
				} else {
					csnet_epoller_del(conntor->epoller, fd, sid);
					csnet_sockset_reset_sock(conntor->sockset, sid);
					csnet_timer_remove(conntor->timer, sid);
					struct server_node* server_node = cs_lfhash_search(conntor->hashtbl, sid);

					if (server_node) {
						cs_lfhash_delete(conntor->hashtbl, sid);
						for (int j = 0; j < 4; j++) {
							if (server_node->sids[j] == sid) {
								server_node->sids[j] = 0;
							}
						}
					}
				}
			}

			if (csnet_epoller_event_is_error(ee)) {
				/* EPOLLERR and EPOLLHUP events can occur if the remote peer
				 * was colsed or a terminal hangup occured. We do nothing
				 * here but LOGGING. */
				LOG_WARNING(conntor->log, "EPOLLERR on socket: %d", fd);
			}
		} /* end of for (ready) */

		csnet_conntor_reconnect_servers(conntor);
		_heartbeat(conntor);

		if (ready == -1) {
			if (errno == EINTR) {
				/* Stopped by a signal */
				continue;
			} else {
				LOG_ERROR(conntor->log, "epoll_wait(): %s", strerror(errno));
				return NULL;
			}
		}
	} /* end of while (1) */

	return NULL;
}

static inline int
_load_nodes(csnet_conntor_t* conntor, const char* config) {
	FILE* f = fopen(config, "r");
	assert(f != NULL);

	int has_read_a_completed_node = 0;  /* when this variable's value equal to 3 means we
					       have read a completed node config, so we can
					       add it to the hash table */
	struct server_node* server_node;
	char line[512] = {0};

	while (fgets(line, 512, f)) {
		if (line[0] == '#' || line[0] == '\n') {
			continue;
		}

		line[strlen(line) - 1] = '\0';
		char* p = strchr(line, '=');

		if (!p) {
			continue;
		}

		if (has_read_a_completed_node == 0) {
			server_node = calloc(1, sizeof(struct server_node));
			if (!server_node) {
				csnet_oom(sizeof(struct server_node));
			}
		}

		*p = '\0';

		char* key = csnet_trim(line);
		char* value = csnet_trim(p + 1);

		if (strcmp(key, "name") == 0) {
			strncpy(server_node->name, value, 64);
			has_read_a_completed_node++;
		} else if (strcmp(key, "sip") == 0) {
			strncpy(server_node->ip, value, 16);
			has_read_a_completed_node++;
		} else if (strcmp(key, "sport") == 0) {
			server_node->port = atoi(value);
			has_read_a_completed_node++;
		} else if (strcmp(key, "server_type") == 0) {
			server_node->server_type = atoi(value);
			has_read_a_completed_node++;
		}

		if (has_read_a_completed_node == 4) {
			LOG_INFO(conntor->log, "server node info: name: %s, ip: %s, port: %d, server type: %d",
				  server_node->name, server_node->ip, server_node->port, server_node->server_type);
			struct slot* slot = conntor->slots[server_node->server_type % SLOT_SIZE];
			cs_dl_node_t* dlnode = cs_dl_node_new(server_node);
			cs_dlist_insert(slot->list, dlnode);
			slot->curr_node = slot->list->head;
			has_read_a_completed_node = 0;
		}
	}

	fclose(f);
	return 0;
}

csnet_conntor_t*
csnet_conntor_new(int connect_timeout, const char* config, csnet_log_t* log, csnet_module_t* module, cs_lfqueue_t* q) {
	csnet_conntor_t* conntor = calloc(1, sizeof(csnet_conntor_t));
	if (!conntor) {
		csnet_oom(sizeof(csnet_conntor_t));
	}

	conntor->epoller = csnet_epoller_new(MAGIC_NUMBER);
	if (!conntor->epoller) {
		LOG_ERROR(log, "epoll_create(): %s", strerror(errno));
		free(conntor);
		return NULL;
	}

	conntor->slots = calloc(SLOT_SIZE, sizeof(struct slot*));
	for (int i = 0; i < SLOT_SIZE; i++) {
		conntor->slots[i] = calloc(1, sizeof(struct slot));
		conntor->slots[i]->list = cs_dlist_new();
	}

	conntor->hashtbl = cs_lfhash_new(71);
	conntor->log = log;
	_load_nodes(conntor, config);
	conntor->sockset = csnet_sockset_new(1024, 0x0);
	conntor->timer = csnet_timer_new(connect_timeout, 709);
	conntor->module = module;
	conntor->q = q;

	return conntor;
}

void
csnet_conntor_free(csnet_conntor_t* conntor) {
	csnet_epoller_free(conntor->epoller);
	for (int i = 0; i < SLOT_SIZE; i++) {
		cs_dlist_free(conntor->slots[i]->list);
		free(conntor->slots[i]);
	}

	free(conntor->slots);
	cs_lfhash_free(conntor->hashtbl);
	csnet_sockset_free(conntor->sockset);
	csnet_timer_free(conntor->timer);
	free(conntor);
}

void
csnet_conntor_reset_module(csnet_conntor_t* conntor, csnet_module_t* module) {
	conntor->module = module;
}

void
csnet_conntor_connect_servers(csnet_conntor_t* conntor) {
	for (int i = 0; i < SLOT_SIZE; i++) {
		struct slot* slot = conntor->slots[i];
		cs_dl_node_t* dnode = slot->list->head;

		while (dnode) {
			struct server_node* server_node = dnode->data;
			for (int i = 0; i < 4; i++) {
				int fd = nonblocking_connect(server_node->ip, server_node->port, 500);
				if (fd != -1) {
					set_nonblocking(fd);
					server_node->sids[i] = csnet_sockset_put(conntor->sockset, fd);
					csnet_epoller_add(conntor->epoller, fd, server_node->sids[i]);
					cs_lfhash_insert(conntor->hashtbl, server_node->sids[i], server_node);
					csnet_timer_insert(conntor->timer, fd, server_node->sids[i]);
					LOG_INFO(conntor->log, "connected to [%s:%d] with socket: %d, sid: %d",
						  server_node->ip, server_node->port, fd, server_node->sids[i]);
				}
			}

			dnode = dnode->next;
		}
	}
}

void
csnet_conntor_reconnect_servers(csnet_conntor_t* conntor) {
	for (int i = 0; i < SLOT_SIZE; i++) {
		struct slot* slot = conntor->slots[i];
		cs_dl_node_t* node = slot->list->head;

		while (node) {
			struct server_node* server_node = node->data;
			for (int i = 0; i < 4; i++) {
				if (server_node->sids[i] == 0) {
					int fd = nonblocking_connect(server_node->ip, server_node->port, 500);
					if (fd != -1) {
						set_nonblocking(fd);
						server_node->sids[i] = csnet_sockset_put(conntor->sockset, fd);
						csnet_epoller_add(conntor->epoller, fd, server_node->sids[i]);
						cs_lfhash_insert(conntor->hashtbl, server_node->sids[i], server_node);
						csnet_timer_insert(conntor->timer, fd, server_node->sids[i]);
						LOG_INFO(conntor->log, "re-connected to [%s:%d] with sockect: %d, sid: %d",
							  server_node->ip, server_node->port, fd, server_node->sids[i]);
					}
				}
			}

			node = node->next;
		}
	}
}

csnet_sock_t*
csnet_conntor_get_sock(csnet_conntor_t* conntor, csnet_server_type_t server_type) {
	struct slot* slot = conntor->slots[server_type % SLOT_SIZE];
	struct server_node* server_node = slot->curr_node->data;

	if (server_node) {
		int idx = INC_ONE_ATOMIC(&server_node->index);
		if (idx >= 4) {
			server_node->index = 0;
			idx = 0;
		}

		return csnet_sockset_get(conntor->sockset, server_node->sids[idx]);
	}

	return NULL;
}

void
csnet_conntor_loop(csnet_conntor_t* conntor) {
	pthread_t tid;
	if (pthread_create(&tid, NULL, _conntor_thread, conntor) < 0) {
		LOG_FATAL(conntor->log, "create _conntor_thread() error. pthread_create(): %s", strerror(errno));
	}
	int cpu_cores = csnet_cpu_cores();
	if (cpu_cores > 4) {
		csnet_bind_to_cpu(tid, cpu_cores - 3);
	}
}

