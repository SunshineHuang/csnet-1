#include "head.h"
#include "pack.h"
#include "epoller.h"
#include "plog.h"
#include "socket_api.h"
#include "cs_ssl.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <strings.h>

#define MAX_EVENTS 1024
#define BACKLOG    65535

const char* cert_file;
const char* pkey_file;
X509* x = NULL;
EVP_PKEY* pkey = NULL;
int sfd;

struct csssl_ev {
	struct epoller* epoller;
	struct cs_ssl** csssl_list;
};

struct csssl_ev* csssl_ev_create(int count)
{
	struct csssl_ev* sev = malloc(sizeof(*sev));
	sev->epoller = epoller_create(MAX_EVENTS);
	sev->csssl_list = calloc(count, sizeof(struct cs_ssl*));
	return sev;
}

void* wait_thread(void* arg)
{
	struct csssl_ev* csssl_ev = arg;
	struct epoller* epoller = csssl_ev->epoller;
	struct cs_ssl** csssl_list = csssl_ev->csssl_list;
	while (1) {
		int ready = epoller_wait(epoller, 1000);
		if (ready > 0) {
			for (int i = 0; i < ready; i++) {
				epoller_event_t* ev = epoller_get_event(epoller, i);
				int fd = epoller_event_fd(ev);
				struct cs_ssl* csssl = csssl_list[fd % MAX_EVENTS];
				if (epoller_event_is_readable(ev)) {
					char buffer[2048];
					int n = cs_ssl_recv(csssl, buffer, sizeof buffer);
					if (n <= 0) {
						DEBUG("thread... remote socket: %d exits", fd);
						epoller_del(epoller, fd);
						cs_ssl_shutdown(csssl);
						cs_ssl_destroy(csssl);
						continue;
					}
					DEBUG("%s", buffer);
					cs_ssl_shutdown(csssl);
					cs_ssl_destroy(csssl);
				}

				if (epoller_event_is_writeable(ev)) {
					DEBUG("thread... socket: %d writeable", fd);
				}

				if (epoller_event_is_error(ev)) {
					DEBUG("thread... socket: %d occur error", fd);
					epoller_del(epoller, fd);
					cs_ssl_shutdown(csssl);
					cs_ssl_destroy(csssl);
				}
			}
		}
	}

	return NULL;
}

int main(int argc, char** argv)
{
	if (argc != 5) {
		printf("Usage: %s <certifacate> <private key> <port> <thread count>\n", argv[0]);
		exit(-1);
	}

	struct rlimit rlimit;
	rlimit.rlim_cur = 1024 * 10;
	rlimit.rlim_max = 1024 * 10;
	if (setrlimit(RLIMIT_NOFILE, &rlimit)) {
		fprintf(stderr, "setrlimit RLIMIT_NOFILE failed: %s\n", strerror(errno));
		fflush(stderr);
		return -1;
	}

	CS_SSL_ENV_INIT();

	cert_file = argv[1];
	pkey_file = argv[2];
	x =  X509_new();
	pkey = EVP_PKEY_new();
	FILE* f1 = fopen(cert_file, "r");
	FILE* f2 = fopen(pkey_file, "r");
	PEM_read_X509(f1, &x, NULL, NULL);
	PEM_read_PrivateKey(f2, &pkey, NULL, NULL);
	fclose(f1);
	fclose(f2);
	int port = atoi(argv[3]);
	int thread_count = atoi(argv[4]);

	struct csssl_ev** csssl_ev_list = calloc(thread_count, sizeof(struct csssl_ev*));
	for (int i = 0; i < thread_count; i++) {
		csssl_ev_list[i] = csssl_ev_create(MAX_EVENTS);
		pthread_t tid;
		pthread_create(&tid, NULL, wait_thread, csssl_ev_list[i]);
	}

	struct epoller* epoller;

	epoller = epoller_create(MAX_EVENTS);
	if (!epoller) {
		DEBUG("Could not create epoller");
	}

	sfd = listen_port(port);
	if (sfd == -1) {
		DEBUG("Could not listening on port[%d].", port);
		exit(-1);
	}

	set_nonblocking(sfd);
	epoller_add(epoller, sfd);
	DEBUG("listening on port[%d] ...", port);

	while (1) {
		int ready = epoller_wait(epoller, 1000);
		if (ready == -1) {
			if (errno == EINTR)
				continue;
			break;
		}
		for (int i = 0; i < ready; ++i) {
			epoller_event_t* ev = epoller_get_event(epoller, i);
			int fd = epoller_event_fd(ev);
			if (epoller_event_is_readable(ev)) {
				if (fd == sfd) {
					int connfd = accept(fd, 0, 0);
					if (connfd == -1) {
						if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
							continue;
						else
							continue;
					}
					struct cs_ssl* csssl = cs_ssl_create_with_cert_and_pkey(CS_TLSV1, x, pkey);
					if (csssl) {
						close(connfd);
						continue;
					}
					int error; 
					error = cs_ssl_accept(csssl, connfd, true, 1000);
					if (error == 0) {
						int index = connfd % thread_count;
						DEBUG("accept incoming with socket: %d. add it to csssl_ev_list[%d]", connfd, index);
						struct csssl_ev* csssl_ev = csssl_ev_list[index];
						csssl_ev->csssl_list[connfd % MAX_EVENTS] = csssl;
						epoller_add(csssl_ev->epoller, connfd);
					} else {
						cs_ssl_shutdown(csssl);
						cs_ssl_destroy(csssl);
					}
				}
			}
		}
	}

	epoller_destroy(epoller);

	return 0;
}

