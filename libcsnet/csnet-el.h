#pragma once

#include "cs-lfqueue.h"
#include "csnet-log.h"
#include "csnet-timer.h"
#include "csnet-module.h"
#include "csnet-epoller.h"
#include "csnet-sockset.h"

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

csnet_el_t* csnet_el_new(int max_conn, int connect_timeout, csnet_log_t* log, csnet_module_t* module, cs_lfqueue_t* q, int type, X509* x, EVP_PKEY* pkey);
void csnet_el_free(csnet_el_t*);
int csnet_el_add_connection(csnet_el_t*, int fd);
int csnet_el_s_add_connection(csnet_el_t*, int fd);
void* csnet_el_io_loop(void* arg);

