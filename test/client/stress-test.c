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

static void
prepare_data(int block_size) {
	send_block = malloc(block_size + 1);
	recv_block = malloc(HEAD_LEN + block_size + 1);

	for (int i = 0; i < block_size; ++i)
		send_block[i] = 'a';
	send_block[block_size] = '\0';;
}

static int
_send(int fd) {
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

static int
_recv(int fd, int block_size) {
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

void*
worker(void* arg) {
	int tid = 0;
	long nrecv = 0;
	long nsend = 0;	
	long qps = 0;
	double tp = 0.0;
	int fd = blocking_connect(ip, port);

	if (fd == -1) {
		tid = __sync_sub_and_fetch(&smp, 1);
		tpsum[tid] = tp;
		qpssum[tid] = qps;
		return NULL;
	}

	while (!atomic_load(&running));
	unsigned long start = csnet_gettime();
	while (atomic_load(&running)) {
		nsend += _send(fd);
		nrecv += _recv(fd, block_size);
		qps++;
	}
	unsigned long end = csnet_gettime();
	unsigned long used = end - start;
	tp = nrecv + nsend;
	tid = __sync_sub_and_fetch(&smp, 1);
	tpsum[tid] = tp;
	qpssum[tid] = qps;
	/*
	printf("send %ld bytes in %d seconds\n", nsend, duration);
	printf("recv %ld bytes in %d seconds\n", nrecv, duration);
	if (tp >= 1024 * 1024)
		printf("thread[%d] through put: %.2fMB/s\n", tid, tp/1024/1024/duration);
	else
		printf("thread[%d] through put: %.2fKB/s\n", tid, tp/1024/duration);
	printf("thread[%d] %ld qps\n", tid, qps);
	*/
	printf("thread[%d] exits. taken %ldus = %.02fms = %.02fs\n",
		tid, used, (double)used / 1000, (double)used / 1000 / 1000);
	close(fd);

	return NULL;
}

int main(int argc, char** argv)
{
	if (argc != 6) {
		fprintf(stderr, "%s <ip> <port> <npthread> <timeout> <block_size>\n", argv[0]);
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

	sleep(5);
	atomic_store(&running, 1);
	sleep(duration);
	atomic_store(&running, 0);

	while (smp) {
		sleep(1);
	}

	double totaltp = 0.0;
	double tp = 0.0;
	long totalqps = 0;
	long qps = 0;
	for (int i = 0; i < nthread; i++) {
		totaltp += tpsum[i];
		totalqps += qpssum[i];
	}
	tp = totaltp/1024/1024/duration;
	qps = totalqps / duration;
	printf("********************************\n");
	printf("through put: %.2fMB/s\n", tp);
	printf("qps: %ld\n", qps);
	printf("%.02fms/request\n", (double)duration * 1000 / qps);
	printf("********************************\n");

	free(tpsum);
	free(qpssum);
	free(send_block);
	free(recv_block);

	return 0;
}

