#include "libcsnet.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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

int get_items(int fd, int type) {
	csnet_pack_t pack;
	csnet_pack_init(&pack);
	csnet_pack_puti(&pack, type);

	csnet_head_t h;
	h.cmd = CSNET_GET_SERVERS_REQ;
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
	if (argc != 4) {
		fprintf(stderr, "./get_items_client ip port stype\n");
		return -1;
	}

	char* ip = argv[1];
	int port = atoi(argv[2]);
	int stype = atoi(argv[3]);

	int fd = blocking_connect(ip, port);
	if (fd == -1) {
		printf("can not connect to %s:%d\n", ip, port);
		return -1;
	}

	int nsend = get_items(fd, stype);
	char buff[1024] = {0};
	int nrecv = recv(fd, buff, 1024, 0);
	csnet_head_t* head = (csnet_head_t*)buff;
	printf("send %d bytes, recv %d bytes\n", nsend, nrecv);
	printf("head->status = %d\n", head->status);

	csnet_unpack_t unpack;
	csnet_unpack_init(&unpack, buff + HEAD_LEN, nrecv);
	int num = csnet_unpack_geti(&unpack);
	printf("server items num: %d\n", num);
	for (int i = 0; i < num; i++) {
		int stype = csnet_unpack_geti(&unpack);
		const char* sip = csnet_unpack_getstr(&unpack);
		int sport = csnet_unpack_geti(&unpack);
		printf("items[%d]: stype: %d, sip: %s, sport: %d\n",
			i + 1, stype, sip, sport);
	}
	close(fd);

	return 0;
}

