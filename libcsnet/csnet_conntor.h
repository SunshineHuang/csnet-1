#pragma once

#include "csnet_log.h"
#include "csnet_timer.h"
#include "csnet_sockset.h"
#include "csnet_module.h"
#include "server_type.h"
#include "cs-doubly-linked-list.h"
#include "csnet_epoller.h"
#include "cs-lfqueue.h"
#include "cs-lfhash.h"

struct slot;

typedef struct csnet_conntor {
	struct slot** slots;
	cs_lfhash_t* hashtbl;
	csnet_epoller_t* epoller;
	csnet_sockset_t* sockset;
	csnet_timer_t* timer;
	csnet_log_t* log;
	csnet_module_t* module;
	cs_lfqueue_t* q;
} csnet_conntor_t;

csnet_conntor_t* csnet_conntor_new(int connect_timeout, const char* config,
	csnet_log_t* log, csnet_module_t* module, cs_lfqueue_t* q);
void csnet_conntor_free(csnet_conntor_t*);
void csnet_conntor_reset_module(csnet_conntor_t*, csnet_module_t* module);
void csnet_conntor_connect_servers(csnet_conntor_t*);
void csnet_conntor_reconnect_servers(csnet_conntor_t*);
void csnet_conntor_loop(csnet_conntor_t*);
csnet_sock_t* csnet_conntor_get_sock(csnet_conntor_t*, csnet_server_type_t server_type);

