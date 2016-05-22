#include "libcsnet.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

csnet_timer_t* timer = NULL;
char* ip;
int port;
int block_size;
int running = 1;
int interval;
char* send_block;
char* recv_block;
int smp = 0;
long* sum;

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
	int fd = blocking_connect(ip, port);
	if (fd == -1)
		exit(1);
	while (running) {
		nsend += _send(fd);
		nrecv += _recv(fd, block_size);
	}

	double tp = nrecv + nsend;
	int tmp = __sync_sub_and_fetch(&smp, 1);
	sum[tmp] = tp;
	printf("send %ld bytes in %d seconds\n", nsend, interval);
	printf("recv %ld bytes in %d seconds\n", nrecv, interval);
	if (tp >= 1024 * 1024)
		printf("thread[%d] through put: %.2fMB/s\n", tmp, tp/1024/1024/interval);
	else
		printf("thread[%d] through put: %.2fKB/s\n", tmp, tp/1024/interval);

	return NULL;
}

void* timeout(void* arg)
{
	while (1) {
		int which_slot = csnet_timer_book_keeping(timer);
		if (which_slot > -1) {
			running = 0;
			break;
		}
	}

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
	interval = atoi(argv[4]);
	block_size = atoi(argv[5]);
	sum = calloc(nthread, sizeof(long));

	printf("============================\n");
	printf("server: ip: %s, port: %d\n", ip, port);
	printf("client thread count: %d\n", nthread);
	printf("client timeout: %d seconds\n", interval);
	printf("send block size: %d\n", block_size);
	printf("============================\n\n");

	prepare_data(block_size);

	timer = csnet_timer_new(interval, 109);
	csnet_timer_insert(timer, 1, 0);

	pthread_t tid;
	if (pthread_create(&tid, NULL, timeout, NULL) < 0) {
		printf("pthread_create() error\n");
		return -1;
	}

	for (int i = 0; i < nthread; i++) {
		pthread_t t;
		pthread_create(&t, NULL, worker, NULL);
		smp++;
	}

	pthread_join(tid, NULL);
	csnet_timer_free(timer);
	while (smp) {
		sleep(1);
	}
	double total = 0;
	for (int i = 0; i < nthread; i++) {
		total += sum[i];
	}
	printf("=======\n");
	printf("total through put: %.2fMB/s\n", total/1024/1024/interval);
	printf("=======\n");
	free(sum);
	free(send_block);
	free(recv_block);
	return 0;
}

