#include "libcsnet.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdatomic.h>

char* ip;
int port;
int block_size;
_Atomic int running = 0;
int duration;
char* send_block;
char* recv_block;
int smp = 0;
long* tpsum;
long* qpssum;

void prepare_data(int block_size)
{
	send_block = malloc(block_size + 1);
	recv_block = malloc(HEAD_LEN + block_size + 1);

	for (int i = 0; i < block_size; ++i)
		send_block[i] = 'a';
	send_block[block_size] = '\0';;
}

int _send(int fd)
{
	csnet_pack_t pack;
	csnet_pack_init(&pack);
	csnet_pack_putstr(&pack, send_block);

	csnet_head_t h;
	h.cmd = 0x1001;
	h.status = 0x00;
	h.version = 0x01;
	h.compress = 0x00;
	h.ctxid = 0x00;
	h.sid = 0;
	h.len = HEAD_LEN + pack.len;

	char buf[128 * 1024];
	memcpy(buf, &h, HEAD_LEN);
	memcpy(buf + HEAD_LEN, pack.data, pack.len);

	int nsend = 0;
	int len = h.len;
	int remain = len;
	while (remain > 0) {
		nsend = send(fd, buf + len - remain, remain, 0);
		if (nsend < 0)
			return -1;
		remain -= nsend;
	}
	return len;
}

int _recv(int fd, int block_size)
{
	int nrecv = 0;
	int len = HEAD_LEN + block_size + 1;
	int remain = len;
	while (remain > 0) {
		nrecv = recv(fd, recv_block + len - remain, remain, 0);
		if (nrecv < 0)
			return -1;
		remain -= nrecv;
	}
	return len;
}

void* worker(void* arg)
{
	long nrecv = 0;
	long nsend = 0;	
	long qps = 0;
	int fd = blocking_connect(ip, port);
	if (fd == -1)
		exit(1);
	while (!atomic_load(&running));
	while (atomic_load(&running)) {
		nsend += _send(fd);
		nrecv += _recv(fd, block_size);
		qps++;
	}

	double tp = nrecv + nsend;
	int tmp = __sync_sub_and_fetch(&smp, 1);
	tpsum[tmp] = tp;
	qpssum[tmp] = qps;
	/*
	printf("send %ld bytes in %d seconds\n", nsend, duration);
	printf("recv %ld bytes in %d seconds\n", nrecv, duration);
	if (tp >= 1024 * 1024)
		printf("thread[%d] through put: %.2fMB/s\n", tmp, tp/1024/1024/duration);
	else
		printf("thread[%d] through put: %.2fKB/s\n", tmp, tp/1024/duration);
	printf("thread[%d] %ld qps\n", tmp, qps);
	*/

	return NULL;
}

int main(int argc, char** argv)
{
	if (argc != 6) {
		fprintf(stderr, "./pingpong <ip> <port> <npthread> <timeout> <block_size>\n");
		return -1;
	}

	ip = argv[1];
	port = atoi(argv[2]);
	int nthread = atoi(argv[3]);
	duration = atoi(argv[4]);
	block_size = atoi(argv[5]);
	tpsum = calloc(nthread, sizeof(long));
	qpssum = calloc(nthread, sizeof(long));

	printf("============================\n");
	printf("server: ip: %s, port: %d\n", ip, port);
	printf("client thread count: %d\n", nthread);
	printf("client timeout: %d seconds\n", duration);
	printf("send block size: %d\n", block_size);
	printf("============================\n\n");

	prepare_data(block_size);

	atomic_store(&running, 0);
	for (int i = 0; i < nthread; i++) {
		pthread_t t;
		pthread_create(&t, NULL, worker, NULL);
		smp++;
	}

	atomic_store(&running, 1);
	sleep(duration);
	atomic_store(&running, 0);

	while (smp) {
		sleep(1);
	}

	double totaltp = 0;
	long totalqps = 0;
	for (int i = 0; i < nthread; i++) {
		totaltp += tpsum[i];
		totalqps += qpssum[i];
	}
	printf("=======\n");
	printf("through put: %.2fMB/s\n", totaltp/1024/1024/duration);
	printf("qps: %ld\n", totalqps/duration);
	printf("=======\n");
	free(tpsum);
	free(qpssum);
	free(send_block);
	free(recv_block);
	return 0;
}

