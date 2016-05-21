#include "cs-lfstack.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

struct threadarg {
	cs_lfstack_t* st;
	int tid;
	int* ops;
};

_Atomic int running;

void* worker(void* arg)
{
	struct threadarg* threadarg = (struct threadarg*) arg;
	cs_lfstack_t* st = threadarg->st;
	int tid = threadarg->tid;
	int* ops = threadarg->ops;
	unsigned int seed = rand();

	while (!atomic_load(&running));
	while (atomic_load(&running)) {
		int cmd = rand_r(&seed) & 1;
		switch (cmd) {
		case 0: {
			cs_lfstack_node_t* node = cs_lfstack_node_new(cmd);
			cs_lfstack_push(st, node);
			(*ops)++;
			break;
		}

		case 1: {
			cs_lfstack_node_t* node = cs_lfstack_pop(st);
			if (node) cs_lfstack_node_free(node);
			(*ops)++;
			break;
		}
		}
	}

	return NULL;
}

double test(int nthreads)
{
	cs_lfstack_t* st = cs_lfstack_new();
	pthread_t threads[32];
	long long ops = 0;
	int* opss = calloc(32, sizeof(int));

	for (int i = 0; i < nthreads; i++) {
		struct threadarg* arg = malloc(sizeof(*arg));
		arg->st = st;
		arg->tid = i;
		arg->ops = &opss[i];
		pthread_create(&threads[i], NULL, worker, arg);
	}

	atomic_store(&running, 1);
	sleep(5);
	atomic_store(&running, 0);

	for (int i = 0; i < nthreads; i++) {
		pthread_join(threads[i], NULL);
		ops += opss[i];
	}

	return (double) ops / 5;
}

int main()
{
	for (int i = 1; i <= 32; i++) {
		double time = test(i);
		printf("%d threads: %ld/sec\n", i, (long)time);
	}

	return 0;
}

