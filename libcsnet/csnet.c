#include "csnet.h"
#include "csnet_utils.h"
#include "csnet_socket_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

#define MAGIC_NUMBER 1024

static void _do_accept(csnet_t* csnet, int* listenfd);

csnet_t*
csnet_new(int port, int thread_count, int max_conn, int connect_timeout, csnet_log_t* log, csnet_module_t* module, cs_lfqueue_t* q) {
	csnet_t* csnet = calloc(1, sizeof(*csnet) + thread_count * sizeof(csnet_el_t*));
	if (!csnet) {
		csnet_oom(sizeof(*csnet));
	}

	csnet->listenfd = listen_port(port);
	if (csnet->listenfd == -1) {
		LOG_FATAL(log, "epoll_create(): %s", strerror(errno));
	}

	if (set_nonblocking(csnet->listenfd) == -1) {
		LOG_FATAL(log, "cannot set socket: %d to nonblock", csnet->listenfd);
	}

	csnet->cpu_cores = csnet_cpu_cores();
	csnet->thread_count = thread_count;
	csnet->max_conn = max_conn;
	csnet->epoller = csnet_epoller_new(MAGIC_NUMBER);

	if (!csnet->epoller) {
		LOG_FATAL(log, "epoll_create(): %s", strerror(errno));
	}

	if (csnet_epoller_add(csnet->epoller, csnet->listenfd, 0) == -1) {
		LOG_FATAL(log, "epoll_ctl(): %s", strerror(errno));
	}

	for (int i = 0; i < thread_count; i++) {
		int count = max_conn / thread_count + 1;
		csnet->el_list[i] = csnet_el_new(count, connect_timeout, log, module, q);
	}

	csnet->log = log;
	csnet->q = q;
	return csnet;
}

void
csnet_loop(csnet_t* csnet, int timeout) {
	for (int i = 0; i < csnet->thread_count; i++) {
		int cpuid;
		pthread_t in_tid;

		if (pthread_create(&in_tid, NULL, csnet_el_in_loop, csnet->el_list[i]) < 0) {
			LOG_FATAL(csnet->log, "create csnet_el_in_loop() error. pthread_create(): %s", strerror(errno));
		}

		if (csnet->cpu_cores > 4) {
			/* Skip CPU0 and CPU1 */
			cpuid = i + 2 % csnet->cpu_cores;
			if (i >= csnet->cpu_cores - 2) {
				cpuid = i + 4 - csnet->cpu_cores;
			}

			csnet_bind_to_cpu(in_tid, cpuid);
		} else {
			cpuid = i % csnet->cpu_cores;
			csnet_bind_to_cpu(in_tid, cpuid);
		}
	}

	pthread_t out_tid;
	if (pthread_create(&out_tid, NULL, csnet_el_out_loop, csnet->q) < 0) {
		LOG_FATAL(csnet->log, "create csnet_el_out_loop() error. pthread_create(): %s", strerror(errno));
	}
	if (csnet->cpu_cores > 4) {
		csnet_bind_to_cpu(out_tid, csnet->cpu_cores - 2);
	}

	while (1) {
		int ready = csnet_epoller_wait(csnet->epoller, timeout);
		for (int i = 0; i < ready; ++i) {
			csnet_epoller_event_t* ee = csnet_epoller_get_event(csnet->epoller, i);
			int fd = csnet_epoller_event_fd(ee);

			if (csnet_epoller_event_is_readable(ee)) {
				if (fd == csnet->listenfd) {
					/* Have a notification on the listening socket,
					   which means one or more new incoming connecttions */
					_do_accept(csnet, &csnet->listenfd);
				}
			}

			if (csnet_epoller_event_is_error(ee)) {
				LOG_ERROR(csnet->log, "epoll event error");
				close(fd);
				continue;
			}
		}

		if (ready == -1) {
			if (errno == EINTR) {  /* Stopped by a signal */
				continue;
			} else {
				LOG_ERROR(csnet->log, "epoll_wait(): %s", strerror(errno));
				return;
			}
		}
	}
}

void
csnet_free(csnet_t* csnet) {
	close(csnet->listenfd);
	csnet_epoller_free(csnet->epoller);
	for (int i = 0; i < csnet->thread_count; i++) {
		csnet_el_free(csnet->el_list[i]);
	}
	free(csnet);
}

static inline void
_do_accept(csnet_t* csnet, int* listenfd) {
	while (1) {
		int fd;
		struct sockaddr_in sin;
		socklen_t len = sizeof(struct sockaddr_in);
		bzero(&sin, len);
		fd = accept(*listenfd, (struct sockaddr*)&sin, &len);

		if (fd > 0) {
			LOG_INFO(csnet->log, "accept incoming [%s:%d] with socket: %d.",
				inet_ntoa(sin.sin_addr), ntohs(sin.sin_port), fd);

			if (set_nonblocking(fd) == -1) {
				LOG_ERROR(csnet->log, "can not set socket: %d to nonblock", fd);
				close(fd);
				continue;
			}

			if (csnet_el_add_connection(csnet->el_list[fd % csnet->thread_count], fd) == -1) {
				close(fd);
				return;
			}
		} else {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				/* We have processed all incoming connections. */
				return;
			} else {
				LOG_ERROR(csnet->log, "accept(): %s", strerror(errno));
				return;
			}
		}
	}
}

