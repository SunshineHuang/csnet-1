#include "csnet_el.h"
#include "csnet_msg.h"
#include "csnet_cmd.h"
#include "csnet_fast.h"
#include "csnet_head.h"
#include "csnet_utils.h"
#include "csnet_sockset.h"
#include "csnet_socket_api.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <strings.h>
#include <netinet/in.h>

#ifdef JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

static void csnet_el_check_timeout(csnet_el_t* el);

csnet_el_t*
csnet_el_new(int max_conn, int connect_timeout, csnet_log_t* log, csnet_module_t* module, cs_lfqueue_t* q) {
	csnet_el_t* el = calloc(1, sizeof(*el));
	if (!el) {
		csnet_oom(sizeof(*el));
	}

	el->max_conn = max_conn;
	el->cur_conn = 0;
	el->epoller = csnet_epoller_new(max_conn);
	el->sockset = csnet_sockset_new(max_conn, 0xab);
	el->timer = csnet_timer_new(connect_timeout, 709);
	el->log = log;
	el->module = module;
	el->q = q;

	return el;
}

int
csnet_el_add_connection(csnet_el_t* el, int fd) {
	unsigned int sid;
	if (csnet_slow(el->cur_conn++ > el->max_conn)) {
		LOG_WARNING(el->log, "Too much connections, closing socket %d", fd);
		return -1;
	}

	sid = csnet_sockset_put(el->sockset, fd);
	csnet_epoller_add(el->epoller, fd, sid);
	csnet_timer_insert(el->timer, fd, sid);

	return 0;
}

void*
csnet_el_out_loop(void* arg) {
	cs_lfqueue_t* q = (cs_lfqueue_t*)arg;
	cs_lfqueue_register_thread(q);

	while (1) {
		csnet_msg_t* msg = NULL;
		int ret = cs_lfqueue_deq(q, (void*)&msg);
		if (csnet_fast(ret == 1)) {
			int nsend = csnet_sock_send(msg->sock, msg->data, msg->size);
			csnet_msg_free(msg);
		} else {
			usleep(10);
		}
	}
}

void*
csnet_el_in_loop(void* arg) {
	csnet_el_t* el = (csnet_el_t*)arg;
	cs_lfqueue_register_thread(el->q);

	while (1) {
		int ready = csnet_epoller_wait(el->epoller, 1000);
		for (int i = 0; i < ready; ++i) {
			csnet_epoller_event_t* ee;
			csnet_sock_t* sock;
			unsigned int sid;
			int fd;

			ee = csnet_epoller_get_event(el->epoller, i);
			sid = csnet_epoller_event_sid(ee);
			sock = csnet_sockset_get(el->sockset, sid);
			fd = sock->fd;

			if (csnet_fast(csnet_epoller_event_is_readable(ee))) {
				int nrecv = csnet_sock_recv(sock);
				if (csnet_fast(nrecv > 0)) {
					while (1) {
						char* data = csnet_rb_data(sock->rb);
						int data_len = sock->rb->data_len;
						csnet_head_t* head = (csnet_head_t*)data;

						if (data_len < HEAD_LEN) {
						        break;
						}

						if (head->len < HEAD_LEN) {
							LOG_ERROR(el->log, "incorrect package from %d. head len: %d", fd,
								  head->len);
							csnet_epoller_del(el->epoller, fd, sid);
							csnet_sockset_reset_sock(el->sockset, sid);
							csnet_timer_remove(el->timer, sid);
							el->cur_conn--;
							break;
						}

						if (data_len < head->len) {
							break;
						}

						if (csnet_fast(head->cmd != CSNET_HEARTBEAT_SYN)) {
							csnet_module_ref_increment(el->module);
							csnet_module_entry(el->module, sock, head, data + HEAD_LEN,
									head->len - HEAD_LEN);
						} else {
							LOG_INFO(el->log, "recv heartbeat syn from socket[%d], sid[%d]", fd, sid);
							csnet_head_t h = {
								.version = head->version,
								.compress = head->compress,
								.cmd = CSNET_HEARTBEAT_ACK,
								.status = head->status,
								.ctxid = head->ctxid,
								.sid = head->sid,
								.len = HEAD_LEN
							};

							csnet_msg_t* msg = csnet_msg_new(h.len, sock);
							csnet_msg_append(msg, (char*)&h, h.len);
							cs_lfqueue_enq(el->q, msg);
							csnet_timer_update(el->timer, sid);
						}

						if (csnet_rb_seek(sock->rb, head->len) == 0) {
							break;
						}
					}

					csnet_epoller_mod_rw(el->epoller, fd, sid);
				} else {
					/* We clear a socket here. Because of if remote peer close immediately
					 * after remote peer send a large amout of data, we can still read() data
					 * from this socket. */
					csnet_epoller_del(el->epoller, fd, sid);
					csnet_sockset_reset_sock(el->sockset, sid);
					csnet_timer_remove(el->timer, sid);
					el->cur_conn--;
				}
			}

			if (csnet_epoller_event_is_error(ee)) {
				/* EPOLLERR and EPOLLHUP events can occur if the remote peer
				 * was colsed or a terminal hangup occured. We do nothing
				 * here but LOGGING. */
				LOG_WARNING(el->log, "EPOLLERR on socket: %d. ", fd);
			}
		} /* end of for (ready) */

		/* Check the connection which has timeout. */
		csnet_el_check_timeout(el);

		if (ready == -1) {
			if (errno == EINTR) {
				/* Stopped by a signal */
				continue;
			} else {
				LOG_ERROR(el->log, "epoll_wait(): %s", strerror(errno));
				return NULL;
			}
		}
	} /* end of while (1) */

	return NULL;
}

void
csnet_el_free(csnet_el_t* el) {
	csnet_epoller_free(el->epoller);
	csnet_sockset_free(el->sockset);
	csnet_timer_free(el->timer);
}

static inline void
csnet_el_check_timeout(csnet_el_t* el) {
	int expired_wheel = csnet_timer_book_keeping(el->timer);
	if (expired_wheel > -1) {
		cs_lfhash_t* hashtbl = el->timer->wheels_tbl[expired_wheel];
		cs_lflist_t* keys = cs_lfhash_get_all_keys(hashtbl);
		cs_lflist_node_t* head = keys->head->next;
		while (head != keys->tail) {
			/* We just close this connection simply here.
			 * TODO: Shoud send a timeout package to this connection?
			 * If we do this, what's the right timing to close this connection? */

			unsigned int sid = head->key;
			csnet_sock_t* sock = csnet_sockset_get(el->sockset, sid);
			LOG_WARNING(el->log, "sid[%d] timeout, closing socket[%d]", sid, sock->fd);
			csnet_epoller_del(el->epoller, sock->fd, sid);
			csnet_sockset_reset_sock(el->sockset, sid);
			el->cur_conn--;
			csnet_timer_remove(el->timer, sid);
			head = head->next;
		}
		cs_lflist_free(keys);
	}
}

