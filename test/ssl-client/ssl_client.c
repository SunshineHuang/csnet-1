#include "libcsnet.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int echo(csnet_ssock_t* ssock, const char* msg)
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
	return csnet_ssock_send(ssock, buf, h.len);
}

int main(int argc, char** argv)
{
	if (argc != 5) {
		printf("Usage: %s <host> <port> <msg> <count>\n", argv[0]);
		exit(-1);
	}
	
	char* host = argv[1];
	int port = atoi(argv[2]);
	char* msg = argv[3];
	int loops = atoi(argv[4]);

	csnet_ssock_env_init();

	for (int i = 0; i < loops; i++)	{
		csnet_ssock_t* ssock = csnet_ssock_new(CSNET_TLSV1);
		if (ssock) {
			int ret = csnet_ssock_connect(ssock, host, port);
			printf("ret: %d\n", ret);
			if (ret == 0) {
				int nsend = echo(ssock, msg);
				printf("send %d bytes\n", nsend);
				char buffer[128];
				int nrecv = csnet_ssock_recv_buff(ssock, buffer, 128);
				printf("recv %d bytes\n", nrecv);
				csnet_ssock_free(ssock);
			}
		} else {
			printf("could not create ssock\n");
		}
	}

	return 0;
}

