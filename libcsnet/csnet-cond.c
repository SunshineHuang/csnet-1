#include "csnet-cond.h"

#include <sys/time.h>

int
csnet_cond_init(csnet_cond_t* cond) {
	if (pthread_mutex_init(&cond->mutex, NULL) < 0) {
		return -1;
	}

	if (pthread_cond_init(&cond->cond, NULL) < 0) {
		pthread_mutex_destroy(&cond->mutex);
		return -1;
	}

	return 0;
}

void
csnet_cond_destroy(csnet_cond_t* cond) {
	pthread_mutex_destroy(&cond->mutex);
	pthread_cond_destroy(&cond->cond);
}

void
csnet_cond_wait(csnet_cond_t* cond) {
	pthread_mutex_lock(&cond->mutex);
	pthread_cond_wait(&cond->cond, &cond->mutex);
	pthread_mutex_unlock(&cond->mutex);
}

void
csnet_cond_wait_sec(csnet_cond_t* cond, int second) {
	struct timeval now;
	struct timespec timeout;

	gettimeofday(&now, NULL);
	timeout.tv_sec = now.tv_sec + second;
	timeout.tv_nsec = 0;

	pthread_mutex_lock(&cond->mutex);
	pthread_cond_timedwait(&cond->cond, &cond->mutex, &timeout);
	pthread_mutex_unlock(&cond->mutex);
}

void
csnet_cond_signal_one(csnet_cond_t* cond) {
	pthread_cond_signal(&cond->cond);
}

void
csnet_cond_signal_all(csnet_cond_t* cond) {
	pthread_cond_broadcast(&cond->cond);
}

