#ifndef csnet_utils_h
#define csnet_utils_h

#include <pthread.h>

char* csnet_trim(char* name);
void csnet_oom(unsigned int size);
int csnet_cpu_cores();
int csnet_bind_to_cpu(pthread_t tid, int cpuid);
int csnet_bound_cpuid(pthread_t tid);
int csnet_md5sum(const char* path, unsigned char* buff);
unsigned long csnet_gettime();

#endif  /* csnet_utils_h */

