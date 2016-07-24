#include "libcsnet.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
	int  port = atoi(argv[2]);
	char* msg = argv[3];
	int loop = atoi(argv[4]);

	csnet_sock_t* sock = csnet_sock_new(1024);
	int fd = blocking_connect(ip, port);

	if (fd == -1) {
		printf("can not connect to %s:%d\n", ip, port);
		return -1;
	}

	sock->fd = fd;
	unsigned long start = csnet_gettime();
	for (int i = 0; i < loop; ++i) {
		int nsend = echo(sock, msg);
		int nrecv = csnet_sock_recv(sock);
		char* data = csnet_rb_data(sock->rb);
		csnet_head_t* head = (csnet_head_t*)data;
		csnet_unpack_t unpack;
		csnet_unpack_init(&unpack, data + HEAD_LEN, head->len - HEAD_LEN);
		const char* m = csnet_unpack_getstr(&unpack);
		csnet_rb_seek(sock->rb, nrecv);
	}
	unsigned long end = csnet_gettime();
	unsigned long us = end - start;
	printf("%d requests within %ld us, %.04fms/request\n", loop, us, (double) us / 1000 / loop);
	close(fd);
	csnet_sock_free(sock);
	return 0;
}

