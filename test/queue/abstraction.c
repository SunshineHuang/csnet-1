#include "internal.h"

unsigned int abstraction_cpu_count()
{
	long int cpu_count;
	cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
	return (unsigned int)cpu_count;
}

#ifdef __APPLE__
int thread_start(pthread_t* thread_state, unsigned int cpu, void* thread_function, void* thread_user_state)
{
	return pthread_create(thread_state, NULL, thread_function, thread_user_state);	
}
#else
int thread_start(pthread_t* thread_state, unsigned int cpu, void* thread_function, void* thread_user_state)
{
	int rv = 0, rv_create;
	pthread_attr_t attr;
	cpu_set_t cpuset;

	assert(thread_state != NULL);
	assert(thread_function != NULL);

	pthread_attr_init(&attr);

	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	pthread_attr_setaffinity_np(&attr, sizeof(cpuset), &cpuset);

	rv_create = pthread_create(thread_state, &attr, thread_function, thread_user_state);

	if (rv_create == 0)
		rv = 1;

	pthread_attr_destroy( &attr );

	return rv;
}
#endif
