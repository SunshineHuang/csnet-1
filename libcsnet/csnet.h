#ifndef csnet_h
#define csnet_h

#include "cs-lfqueue.h"
#include "csnet_el.h"
#include "csnet_log.h"
#include "csnet_module.h"
#include "csnet_epoller.h"

typedef struct csnet {
	int listenfd;
	int cpu_cores;
	int thread_count;
	int max_conn;
	csnet_epoller_t* epoller;
	csnet_el_t** el_list;
	csnet_log_t* log;
	cs_lfqueue_t* q;
} csnet_t;

csnet_t* csnet_new(int port, int thread_count, int max_conn, int connect_timeout, csnet_log_t* log, csnet_module_t* module, cs_lfqueue_t* q);
void csnet_loop(csnet_t*, int timeout);
void csnet_free(csnet_t*);

#endif  /* csnet_h */
