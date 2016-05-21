#ifndef csnet_cond_h
#define csnet_cond_h

#include <pthread.h>

typedef struct csnet_cond {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} csnet_cond_t;


int csnet_cond_init(csnet_cond_t*);
void csnet_cond_destroy(csnet_cond_t*);
void csnet_cond_wait(csnet_cond_t*);
void csnet_cond_wait_sec(csnet_cond_t*, int second);
void csnet_cond_signal_one(csnet_cond_t*);
void csnet_cond_signal_all(csnet_cond_t*);

#endif  /* csnet_cond_h */

