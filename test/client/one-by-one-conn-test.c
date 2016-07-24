#include "libcsnet.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

static inline int
echo(csnet_sock_t* sock, const char* msg) {
	csnet_pack_t pack;
	csnet_pack_init(&pack);
	csnet_pack_putstr(&pack, msg);

	csnet_head_t h;
	h.cmd = 0x1001;
	h.status = 0x00;
	h.version = 0x01;
	h.compress = 0x00;
	h.ctxid = 0x00;
	h.sid = 0;
	h.len = HEAD_LEN + pack.len;

	char buf[1024];
	memcpy(buf, &h, HEAD_LEN);
	memcpy(buf + HEAD_LEN, pack.data, pack.len);

	return csnet_sock_send(sock, buf, h.len);
}

int main(int argc, char** argv)
{
	if (argc != 5) {
		fprintf(stderr, "%s ip port msg package_count\n", argv[0]);
		return -1;
	}

	char* ip = argv[1];
	int port = atoi(argv[2]);
	char* msg = argv[3];
	int loop = atoi(argv[4]);
	for (int i = 0; i < loop; ++i) {
		int fd = blocking_connect(ip, port);
		if (fd == -1) {
			printf("can not connect to %s:%d\n", ip, port);
			return -1;
		}
		csnet_sock_t* sock = csnet_sock_new(1024);
		sock->fd = fd;
		int nsend = echo(sock, msg);
		int nrecv = csnet_sock_recv(sock);
		printf("send %d bytes, recv %d bytes\n", nsend, nrecv);

		close(fd);
		csnet_sock_free(sock);
	}
	return 0;
}

