#include "csnet-utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <sys/time.h>
#include <openssl/md5.h>

int
csnet_md5sum(const char* path, unsigned char* buff) {
	FILE* file = fopen(path, "rb");
	if (!file) {
		return -1;
	}

	MD5_CTX md5_ctx;
	int bytes;
	unsigned char data[1024];

	MD5_Init(&md5_ctx);
	while ((bytes = fread(data, 1, 1024, file)) != 0) {
		MD5_Update(&md5_ctx, data, bytes);
	}
	MD5_Final(buff, &md5_ctx);
	fclose(file);
	return 0;
}

char*
csnet_trim(char* name) {
	char* start_pos = name;
	while ((*start_pos == ' ') || (*start_pos == '\t')) {
		start_pos++;
	}
	if (strlen(start_pos) == 0) {
		return NULL; 
	}
	char* end_pos = name + strlen(name) - 1;
	while ((*end_pos == ' ') || (*end_pos == '\t')) {
		*end_pos = 0;
		end_pos--;
	}
	int len = (int)(end_pos - start_pos) + 1;
	if (len <= 0) {
		return NULL; 
	}
	return start_pos;
}

void
csnet_oom(unsigned int size) {
	fprintf(stderr, "out of memory trying to allocate %u bytes\n", size);
	fflush(stderr);
	abort();
}

inline int
csnet_cpu_cores(void) {
	int cores = sysconf(_SC_NPROCESSORS_ONLN);
	return cores > 0 ? cores : 1;
}

inline int
csnet_bind_to_cpu(pthread_t tid, int cpuid) {
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(cpuid, &mask);
	return pthread_setaffinity_np(tid, sizeof(mask), &mask);
}

inline int
csnet_bound_cpuid(pthread_t tid) {
	cpu_set_t mask;
	CPU_ZERO(&mask);
	pthread_getaffinity_np(tid, sizeof(mask), &mask);
	int cpu_cores = csnet_cpu_cores();
	for (int i = 0; i < cpu_cores; i++) {
		if (CPU_ISSET(i, &mask)) {
			return i;
		}
	}
	return -1;
}

inline unsigned long
csnet_gettime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

