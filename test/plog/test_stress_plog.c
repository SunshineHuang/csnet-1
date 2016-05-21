#include "plog.h"

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int nloop = 0;
struct plog* plog;

void* log_thread(void* arg)
{
	pthread_t tid = pthread_self();
	int i = 0;
	for (; i < nloop; i++) {
		PLOG_DEBUG(plog, "this is thread[%ld]: %d", tid, i);
	}

	printf("thread[%ld] exit\n", (long)tid);
	return NULL;
}

int main(int argc, char** argv)
{
	if (argc != 4) {
		printf("usage:\n");
		printf("\t%s logname nthreads nlogperthread\n", argv[0]);
		return -1;
	}

	plog = plog_create(argv[1], PLOG_LEVEL_DEBUG, 1024 * 1024 * 5);
	int n = atoi(argv[2]);
	nloop = atoi(argv[3]);

	int i;
	for (i = 0; i < n; i++) {
		pthread_t tid;
		pthread_create(&tid, NULL, log_thread, NULL);
	}

	while (1) {
		sleep(1);
	}

	plog_destroy(plog);

	return 0;
}

