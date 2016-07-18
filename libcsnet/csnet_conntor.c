#include "csnet_conntor.h"
#include "csnet_atomic.h"
#include "csnet_head.h"
#include "csnet_cmd.h"
#include "csnet.h"
#include "csnet_sb.h"
#include "csnet_socket_api.h"
#include "csnet_utils.h"
#include "csnet_log.h"
#include "csnet_msg.h"
#include "csnet_fast.h"
#include "csnet_pack.h"
#include "csnet_unpack.h"
#include "csnet-register.h"
#include "csnet_spinlock.h"

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
	csnet_server_type_t stype;
	unsigned int sid;
	int port;
	char ip[16];
};

struct slot {
	csnet_spinlock_t lock;
	cs_dl_node_t* curr_node;
	cs_dlist_t* dlist;
};

static void heartbeating(csnet_conntor_t* conntor);
static void readable_event(csnet_conntor_t* conntor, csnet_ss_t* ss, unsigned int sid, int fd);

static void*
csnet_conntor_io_loop(void* arg) {
	csnet_conntor_t* conntor = (csnet_conntor_t *)arg;
	cs_lfqueue_t* q = conntor->q;
	cs_lfqueue_register_thread(q);

	while (1) {
		int ready = csnet_epoller_wait(conntor->epoller, 1000);
		for (int i = 0; i < ready; ++i) {
			csnet_epoller_event_t* ee;
			csnet_ss_t* ss;
			unsigned int sid;
			int fd;

			ee = csnet_epoller_get_event(conntor->epoller, i);
			sid = csnet_epoller_event_sid(ee);
			ss = csnet_sockset_get(conntor->sockset, sid);
			fd = ss->ss.sock->fd;

			if (csnet_fast(csnet_epoller_event_is_readable(ee))) {
				readable_event(conntor, ss, sid, fd);	
			}

			if (csnet_epoller_event_is_error(ee)) {
				/* EPOLLERR and EPOLLHUP events can occur if the remote peer
				 * was colsed or a terminal hangup occured. We do nothing
				 * here but LOGGING. */
				LOG_WARNING(conntor->log, "EPOLLERR on socket: %d", fd);
			}
		} /* end of for (ready) */

		heartbeating(conntor);

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

csnet_conntor_t*
csnet_conntor_new(int connect_timeout, csnet_config_t* config, csnet_log_t* log, csnet_module_t* module, cs_lfqueue_t* q) {
	csnet_conntor_t* conntor = calloc(1, sizeof(csnet_conntor_t));
	if (!conntor) {
		csnet_oom(sizeof(csnet_conntor_t));
	}

	conntor->epoller = csnet_epoller_new(MAGIC_NUMBER);
	if (!conntor->epoller) {
		LOG_FATAL(log, "epoll_create(): %s", strerror(errno));
	}

	conntor->slots = calloc(SLOT_SIZE, sizeof(struct slot*));
	for (int i = 0; i < SLOT_SIZE; i++) {
		conntor->slots[i] = calloc(1, sizeof(struct slot));
		csnet_spinlock_init(&conntor->slots[i]->lock);
		conntor->slots[i]->dlist = cs_dlist_new();
	}

	conntor->hashtbl = cs_lfhash_new(71);
	conntor->log = log;
	conntor->sockset = csnet_sockset_new(1024, 0x0, sock_type, NULL, NULL);
	conntor->timer = csnet_timer_new(connect_timeout, 709);
	conntor->module = module;
	conntor->config = config;
	conntor->q = q;

	return conntor;
}

void
csnet_conntor_free(csnet_conntor_t* conntor) {
	csnet_epoller_free(conntor->epoller);
	for (int i = 0; i < SLOT_SIZE; i++) {
		cs_dlist_free(conntor->slots[i]->dlist);
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
csnet_conntor_connect(csnet_conntor_t* conntor, int stype, const char* sip, int sport) {
	struct server_node* server_node = calloc(1, sizeof(struct server_node));
	server_node->stype = stype;
	strncpy(server_node->ip, sip, 16);
	server_node->port = sport;

	int fd = nonblocking_connect(server_node->ip, server_node->port, 500);
	if (fd != -1) {
		struct slot* slot = conntor->slots[server_node->stype % SLOT_SIZE];
		csnet_spinlock_lock(&slot->lock);
		cs_dl_node_t* dlnode = cs_dl_node_new(server_node);
		cs_dlist_insert(slot->dlist, dlnode);
		set_nonblocking(fd);
		server_node->sid = csnet_sockset_put(conntor->sockset, fd);
		csnet_epoller_add(conntor->epoller, fd, server_node->sid);
		cs_lfhash_insert(conntor->hashtbl, server_node->sid, server_node);
		csnet_timer_insert(conntor->timer, fd, server_node->sid);
		LOG_INFO(conntor->log, "connected to [%s:%d] with socket: %d, sid: %d",
			server_node->ip, server_node->port, fd, server_node->sid);
		csnet_spinlock_unlock(&slot->lock);
	} else {
		LOG_WARNING(conntor->log, "could not connect to [%s:%d]",
			server_node->ip, server_node->port);
		free(server_node);
	}
}

void
csnet_conntor_connect_servers(csnet_conntor_t* conntor) {
	/* First register myself to lookup server */

	csnet_register_t* reg = csnet_register_new(conntor->config, conntor->log);
	csnet_register_myself(reg);
	csnet_sock_t* sock = csnet_sock_new(1024);
	sock->fd = reg->lookup_server_fd;

	csnet_sb_t* send_buffer = csnet_sb_new(1024);

	/* Get server items from lookup server */

	for (int i = 0; i < reg->connect_server_type_count; i++) {
		LOG_INFO(conntor->log, "getting stype[%d] server items ...", reg->connect_server_type_list[i]);
		csnet_pack_t pack;
		csnet_pack_init(&pack);
		csnet_pack_puti(&pack, reg->connect_server_type_list[i]);

		csnet_head_t h = {
			.cmd = CSNET_GET_SERVERS_REQ,
			.status = 0x00,
			.version = VERSION,
			.compress = COMPRESS_NON,
			.ctxid = 0x00,
			.sid = 0,
			.len = HEAD_LEN + pack.len,
		};

		csnet_sb_append(send_buffer, (const char*)&h, HEAD_LEN);
		csnet_sb_append(send_buffer, pack.data, pack.len);
		csnet_sock_send(sock, csnet_sb_data(send_buffer), csnet_sb_data_len(send_buffer));

		int nrecv = csnet_sock_recv(sock);
		char* data = csnet_rb_data(sock->rb);
		csnet_head_t* head = (csnet_head_t*)data;

		csnet_unpack_t unpack;
		csnet_unpack_init(&unpack, data + HEAD_LEN, head->len - HEAD_LEN);
		int num = csnet_unpack_geti(&unpack);
		LOG_INFO(conntor->log, "stype[%d] has %d nodes", reg->connect_server_type_list[i], num);

		for (int j = 0; j < num; j++) {
			struct server_node* server_node = calloc(1, sizeof(struct server_node));
			server_node->stype = csnet_unpack_geti(&unpack);
			const char* sip = csnet_unpack_getstr(&unpack);
			strncpy(server_node->ip, sip, 16);
			server_node->port = csnet_unpack_geti(&unpack);

			struct slot* slot = conntor->slots[server_node->stype % SLOT_SIZE];
			csnet_spinlock_lock(&slot->lock);
			cs_dl_node_t* dlnode = cs_dl_node_new(server_node);
			cs_dlist_insert(slot->dlist, dlnode);
			slot->curr_node = slot->dlist->head;
			csnet_spinlock_unlock(&slot->lock);
		}
		csnet_rb_seek(sock->rb, nrecv);
		csnet_sb_reset(send_buffer);
	}
	csnet_sock_free(sock);
	csnet_sb_free(send_buffer);

	for (int i = 0; i < SLOT_SIZE; i++) {
		struct slot* slot = conntor->slots[i];
		csnet_spinlock_lock(&slot->lock);
		cs_dl_node_t* dlnode = slot->dlist->head;
		while (dlnode) {
			cs_dl_node_t* next = dlnode->next;
			struct server_node* server_node = dlnode->data;
			int fd = nonblocking_connect(server_node->ip, server_node->port, 1000);
			if (fd > 0) {
				set_nonblocking(fd);
				server_node->sid = csnet_sockset_put(conntor->sockset, fd);
				csnet_epoller_add(conntor->epoller, fd, server_node->sid);
				cs_lfhash_insert(conntor->hashtbl, server_node->sid, server_node);
				csnet_timer_insert(conntor->timer, fd, server_node->sid);
				LOG_INFO(conntor->log, "connected to [%s:%d] with socket: %d, sid: %d",
					  server_node->ip, server_node->port, fd, server_node->sid);
			} else {
				LOG_WARNING(conntor->log, "could not connect to [%s:%d]",
					server_node->ip, server_node->port);
				cs_dlist_remove(slot->dlist, dlnode);
				free(server_node);
				cs_dl_node_free(dlnode);
				slot->curr_node = slot->dlist->head;
			}

			dlnode = next;
		}
		csnet_spinlock_unlock(&slot->lock);
	}
	csnet_register_free(reg);
}

csnet_ss_t*
csnet_conntor_get_ss(csnet_conntor_t* conntor, csnet_server_type_t stype) {
	struct slot* slot = conntor->slots[stype % SLOT_SIZE];
	csnet_spinlock_lock(&slot->lock);
	struct server_node* server_node = slot->curr_node->data;

	if (server_node) {
		slot->curr_node = slot->curr_node->next;
		if (slot->curr_node == slot->dlist->tail) {
			slot->curr_node = slot->dlist->head;
		}
		csnet_spinlock_unlock(&slot->lock);
		return csnet_sockset_get(conntor->sockset, server_node->sid);
	}

	csnet_spinlock_unlock(&slot->lock);
	return NULL;
}

void
csnet_conntor_loop(csnet_conntor_t* conntor) {
	pthread_t tid;
	if (pthread_create(&tid, NULL, csnet_conntor_io_loop, conntor) < 0) {
		LOG_FATAL(conntor->log, "create csnet_conntor_io_loop() error: %s", strerror(errno));
	}
	int cpu_cores = csnet_cpu_cores();
	if (cpu_cores > 4) {
		csnet_bind_to_cpu(tid, cpu_cores - 3);
	}
}

static void
readable_event(csnet_conntor_t* conntor, csnet_ss_t* ss, unsigned int sid, int fd) {
	int nrecv = csnet_ss_recv(ss);
	if (csnet_fast(nrecv > 0)) {
		while (1) {
			char* data = csnet_rb_data(ss->ss.sock->rb);
			unsigned int data_len = ss->ss.sock->rb->data_len;
			csnet_head_t* head = (csnet_head_t*)data;

			if (data_len < HEAD_LEN) {
				break;
			}

			if (head->len < HEAD_LEN || head->len > MAX_LEN) {
				LOG_ERROR(conntor->log, "Incorrect package from %d. head len: %d", fd, head->len);
				csnet_epoller_del(conntor->epoller, fd, sid);
				csnet_sockset_reset_ss(conntor->sockset, sid);
				csnet_timer_remove(conntor->timer, sid);
				struct server_node* server_node = cs_lfhash_search(conntor->hashtbl, sid);

				if (server_node) {
					cs_lfhash_delete(conntor->hashtbl, sid);
					if (server_node->sid == sid) {
						server_node->sid = 0;
					}
				}
				break;
			}

			if (data_len < head->len) {
				break;
			}

			if (csnet_fast(head->cmd != CSNET_HEARTBEAT_ACK)) {
				csnet_module_ref_increment(conntor->module);
				csnet_module_entry(conntor->module, ss, head, data + HEAD_LEN, head->len - HEAD_LEN);
			} else {
				/* Do nothing when recv CSNET_HEARTBEAT_ACK */
			}

			if (csnet_rb_seek(ss->ss.sock->rb, head->len) == 0) {
				break;
			}
		}

		csnet_epoller_mod_read(conntor->epoller, fd, sid);
	} else if (nrecv == 0) {
		csnet_epoller_mod_read(conntor->epoller, fd, sid);
	} else {
		LOG_WARNING(conntor->log, "remote peer socket: %d, sid: %u closed.", fd, sid);
		csnet_epoller_del(conntor->epoller, fd, sid);
		csnet_sockset_reset_ss(conntor->sockset, sid);
		csnet_timer_remove(conntor->timer, sid);

		struct server_node* server_node = cs_lfhash_search(conntor->hashtbl, sid);
		if (server_node) {
			cs_lfhash_delete(conntor->hashtbl, sid);
			if (server_node->sid == sid) {
				server_node->sid = 0;
			}
			struct slot* slot = conntor->slots[server_node->stype % SLOT_SIZE];
			csnet_spinlock_lock(&slot->lock);
			cs_dl_node_t* dlnode = cs_dlist_search(slot->dlist, server_node);
			cs_dlist_remove(slot->dlist, dlnode);
			slot->curr_node = slot->dlist->head;
			csnet_spinlock_unlock(&slot->lock);
			free(server_node);
			free(dlnode);
		}
	}
}

static inline void
heartbeating(csnet_conntor_t* conntor) {
	int expired_wheel = csnet_timer_book_keeping(conntor->timer);
	if (expired_wheel > -1) {
		cs_lfhash_t* hashtable = conntor->timer->wheels_tbl[expired_wheel];
		cs_lflist_t* keys = cs_lfhash_get_all_keys(hashtable);
		cs_lflist_node_t* head = keys->head->next;
		while (head != keys->tail) {
			unsigned int sid = head->key;
			LOG_INFO(conntor->log, "sid[%d] timeout, heartbeating", sid);

			csnet_head_t h = {
				.version = VERSION,
				.compress = COMPRESS_NON,
				.cmd = CSNET_HEARTBEAT_SYN,
				.status = 0x0,
				.ctxid = 0x0,
				.sid = sid,
				.len = HEAD_LEN
			};

			csnet_ss_t* ss = csnet_sockset_get(conntor->sockset, sid);
			csnet_msg_t* msg = csnet_msg_new(h.len, ss);
			csnet_msg_append(msg, (char*)&h, h.len);
			csnet_sendto(conntor->q, msg);
			head = head->next;
		}
		cs_lflist_free(keys);
	}
}

