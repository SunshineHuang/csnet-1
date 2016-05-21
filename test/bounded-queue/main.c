#include "bounded_queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

void* thread1(void* arg)
{
	struct bounded_queue* bq = arg;

	while (1) {
		struct node* node = bounded_queue_deq(bq);
	}

	return NULL;
}

void* thread2(void* arg)
{
	struct bounded_queue* bq = arg;

	while (1) {
		struct node* node = bounded_queue_deq(bq);
	}

	return NULL;
}

void* thread3(void* arg)
{
	struct bounded_queue* bq = arg;

	while (1) {
		struct node* node = bounded_queue_deq(bq);
	}

	return NULL;
}



int main(int argc, char** argv)
{
	int count = atoi(argv[1]);
	int i;
	pthread_t tid;

	struct bounded_queue* bq = bounded_queue_alloc(count);

	pthread_create(&tid, NULL, thread1, bq);
	pthread_create(&tid, NULL, thread2, bq);
	pthread_create(&tid, NULL, thread3, bq);

	struct timeval tv1;
	gettimeofday(&tv1, NULL);
	printf("%ld, %d\n", tv1.tv_sec, tv1.tv_usec);

	for (i = 0; i < count; i++) {
		struct node* node = (struct node*)malloc(sizeof(struct node));
		int len = sprintf(node->data, "0123456789: %d", i);
		node->data[len] = '\0';
		node->next = NULL;

		bounded_queue_enq(bq, node);
	}

	struct timeval tv2;
	gettimeofday(&tv2, NULL);
	printf("%ld, %d\n", tv2.tv_sec, tv2.tv_usec);
/*
	struct node* head = bq->head;
	for (i = 0; i < count; i++) {
		struct node* node = head->next;
		printf("%s\n", node->data);
		head = head->next;
	}
*/

	while (1) {
		printf("%d\n", bq->size);
		sleep(1);
	}

	return 0;
}
