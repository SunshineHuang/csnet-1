#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#define CALLING_CONVENTION  

unsigned int abstraction_cpu_count(void);
int thread_start(pthread_t* thread_state, unsigned int cpu, void* thread_function, void* thread_user_state);
