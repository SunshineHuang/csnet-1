#include "csnet_sock.h"
#include "csnet_log.h"
#include "csnet_fast.h"
#include "csnet_utils.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

#ifdef JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

#define READ_BUFFER_SIZE  64 * 1024

csnet_sock_t*
csnet_sock_new(int rsize) {
	csnet_sock_t* sock = calloc(1, sizeof(*sock));
	if (csnet_slow(!sock)) {
		csnet_oom(sizeof(*sock));
	}
	sock->fd = 0;
	sock->sid = 0;
	sock->rb = csnet_rb_new(rsize);
	return sock;
}

void
csnet_sock_free(csnet_sock_t* sock) {
	csnet_rb_free(sock->rb);
	free(sock);
}

int
csnet_sock_recv(csnet_sock_t* sock) {
	/* TODO: reduce data copy */
	char recvbuf[READ_BUFFER_SIZE] = {0};
	int nrecv;

tryagain:
	nrecv = recv(sock->fd, recvbuf, READ_BUFFER_SIZE, 0);
	if (csnet_fast(nrecv > 0)) {
		csnet_rb_append(sock->rb, recvbuf, nrecv);
		return nrecv;
	}

	if (nrecv < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			goto tryagain;
		}
		DEBUG("recv error from socket[%d]: %s", sock->fd, strerror(errno));
		return -1; /* error */
	}

	return -1; /* peer closed */
}

int
csnet_sock_send(csnet_sock_t* sock, char* buff, int len) {
	int nsend = 0;
	int remain = len;
	while (remain > 0) {
		nsend = send(sock->fd, buff + len - remain, remain, 0);
		if (csnet_slow(nsend < 0)) {
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
				continue;
			}
			DEBUG("send(%d) error: %s", sock->fd, strerror(errno));
			return -1;
		}
		remain -= nsend;
	}
	return len;
}

