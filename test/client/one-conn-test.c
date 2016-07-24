#include "libcsnet.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

int echo(int fd, const char* msg)
{
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

	char buf[128];
	memcpy(buf, &h, HEAD_LEN);
	memcpy(buf + HEAD_LEN, pack.data, pack.len);

	return send(fd, buf, h.len, 0);
}

int main(int argc, char** argv)
{
	if (argc != 5) {
		fprintf(stderr, "./client ip port msg package_count\n");
		return -1;
	}

	int fd = blocking_connect(argv[1], atoi(argv[2]));
	if (fd == -1) {
		printf("can not connect to %s:%s\n", argv[1], argv[2]);
		return -1;
	}
	for (int i = 0; i < atoi(argv[4]); ++i) {
		int nsend = echo(fd, argv[3]);
		char buff[1024] = {0};
		int nrecv = recv(fd, buff, 1024, 0);
		printf("send %d bytes, recv %d bytes\n", nsend, nrecv);

	}
	close(fd);
	return 0;
}

