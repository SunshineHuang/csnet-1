#ifndef csnet_log_h
#define csnet_log_h

#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define LL_ERROR   0
#define LL_WARNING 1
#define LL_INFO    2
#define LL_DEBUG   3

#define DEBUG(fmt, args ...) do { \
	fprintf(stderr, "[%s:%u:%s()] %ld " fmt "\n", __FILE__, __LINE__, __FUNCTION__, syscall(__NR_gettid), ##args); \
	fflush(stderr); \
} while (0)

#define LOG_DEBUG(log, args...) do { \
        csnet_log_log(log, LL_DEBUG, __FILE__, __LINE__, args); \
} while (0)

#define LOG_INFO(log, args...) do { \
        csnet_log_log(log, LL_INFO, __FILE__, __LINE__, args); \
} while (0)

#define LOG_WARNING(log, args...) do { \
        csnet_log_log(log, LL_WARNING, __FILE__, __LINE__, args); \
} while (0)

#define LOG_ERROR(log, args...) do { \
        csnet_log_log(log, LL_ERROR, __FILE__, __LINE__, args); \
} while (0)

#define LOG_FATAL(log, args...) do { \
	csnet_log_fatal(log, __FILE__, __LINE__, args); \
} while (0)

typedef struct csnet_log csnet_log_t;

csnet_log_t* csnet_log_new(const char* logname, int level, long rotate_size);
void csnet_log_log(csnet_log_t*, int level, const char* filename, int lineno, const char* fmt, ...);
void csnet_log_fatal(csnet_log_t*, const char* filename, int lineno, const char* fmt, ...);
void csnet_log_free(csnet_log_t*);

#endif  /* csnet_log_h */

