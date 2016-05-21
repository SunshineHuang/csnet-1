#ifndef csnet_el_h
#define csnet_el_h

#include "cs-lfqueue.h"
#include "csnet_log.h"
#include "csnet_timer.h"
#include "csnet_module.h"
#include "csnet_epoller.h"
#include "csnet_sockset.h"

typedef struct csnet_el {
	int max_conn;
	int cur_conn;
	csnet_epoller_t* epoller;
	csnet_sockset_t* sockset;
	csnet_timer_t* timer;
	csnet_log_t* log;
	csnet_module_t* module;
	cs_lfqueue_t* q;
} csnet_el_t;

csnet_el_t* csnet_el_new(int max_conn, int connect_timeout, csnet_log_t* log, csnet_module_t* module, cs_lfqueue_t* q);
void csnet_el_free(csnet_el_t*);
int csnet_el_add_connection(csnet_el_t*, int fd);
void* csnet_el_in_loop(void* arg);
void* csnet_el_out_loop(void* arg);

#endif  /* csnet_el_h */

