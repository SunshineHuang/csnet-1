#pragma once

#include "cs-lfqueue.h"
#include "csnet-el.h"
#include "csnet-log.h"
#include "csnet-msg.h"
#include "csnet-module.h"
#include "csnet-epoller.h"

typedef struct csnet {
	int type;
	int listenfd;
	int cpu_cores;
	int thread_count;
	int max_conn;
	X509* x;
	EVP_PKEY* pkey;
	csnet_epoller_t* epoller;
	cs_lfqueue_t* q;
	csnet_log_t* log;
	csnet_el_t* el_list[0];
} csnet_t;

csnet_t* csnet_new(int port, int thread_count, int max_conn, int connect_timeout,
	csnet_log_t* log, csnet_module_t* module, cs_lfqueue_t* q,
	int type, const char*, const char*);

void csnet_free(csnet_t*);
void csnet_reset_module(csnet_t*, csnet_module_t* module);
void csnet_loop(csnet_t*, int timeout);
int csnet_sendto(cs_lfqueue_t* q, csnet_msg_t* msg);

