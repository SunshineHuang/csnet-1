#include "cs-lfqueue.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>

struct threadarg {
	int tid;
	int* ops;
	cs_lfqueue_t* q;
};

_Atomic int running;

void* worker(void* arg)
{
	struct threadarg* threadarg = (struct threadarg*)arg;
	int tid = threadarg->tid;
	cs_lfqueue_t* q = threadarg->q;
	int* ops = threadarg->ops;
	unsigned int seed = rand();
	cs_hp_record_t* private_record = allocate_thread_private_hp_record(q->hp);
	while (!atomic_load(&running));
	while (atomic_load(&running)) {
		int cmd = rand_r(&seed) & 1;
		switch (cmd) {
		case 0: {
			int* data = malloc(sizeof(*data));
			*data = 1;
			cs_lfqueue_enq(q, private_record, data);
			(*ops)++;
			break;
		}
		case 1: {
			void* data;
			int ret = cs_lfqueue_deq(q, private_record, &data);
			if (ret == 1) {
				free(data);
			}
			(*ops)++;
			break;
		}
		};
	}

	return NULL;
}

double test(cs_lfqueue_t* q, int nthreads)
{
	pthread_t threads[32] = {};
	int opss[32] = {};
	long long ops = 0;
	struct threadarg args[32];

	for (int i = 0; i < nthreads; i++) {
		args[i].tid = i;
		args[i].q = q;
		args[i].ops = &opss[i];
		pthread_create(&threads[i], NULL, worker, &args[i]);
	}

	atomic_store(&running, 1);
	sleep(5);
	atomic_store(&running, 0);

	for (int i = 0; i < 32; i++) {
		pthread_join(threads[i], NULL);
		ops += opss[i];
	}

	return (double)ops / 5;
}

int main(int argc, char** argv)
{
	cs_lfqueue_t* q = cs_lfqueue_new();
	for (int i = 1; i <= 32; i++) {
		double time = test(q, i);
		printf("%02d threads: lock-free %ld/sec\n", i, (long)time);
	}
	cs_lfqueue_free(q);
	return 0;
}

