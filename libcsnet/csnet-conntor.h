#pragma once

#include "csnet-log.h"
#include "csnet-config.h"
#include "csnet-timer.h"
#include "csnet-sockset.h"
#include "csnet-module.h"
#include "csnet-service-type.h"
#include "csnet-epoller.h"
#include "cs-doubly-linked-list.h"
#include "cs-lfqueue.h"
#include "cs-lfhash.h"

struct slot;

typedef struct csnet_conntor {
	int connect_server_type_count;
	int* connect_server_type_list;
	struct slot** slots;
	cs_lfhash_t* hashtbl;
	csnet_epoller_t* epoller;
	csnet_sockset_t* sockset;
	csnet_timer_t* timer;
	csnet_log_t* log;
	csnet_module_t* module;
	csnet_config_t* config;
	cs_lfqueue_t* q;
} csnet_conntor_t;

csnet_conntor_t* csnet_conntor_new(int connect_timeout, csnet_config_t* config, csnet_log_t* log, csnet_module_t* module, cs_lfqueue_t* q);
void csnet_conntor_free(csnet_conntor_t*);
void csnet_conntor_reset_module(csnet_conntor_t*, csnet_module_t* module);
void csnet_conntor_connect_servers(csnet_conntor_t*);
void csnet_conntor_connect(csnet_conntor_t*, int stype, const char* sip, int sport);
void csnet_conntor_reconnect_servers(csnet_conntor_t*);
void csnet_conntor_loop(csnet_conntor_t*);
csnet_ss_t* csnet_conntor_get_ss(csnet_conntor_t*, csnet_server_type_t server_type);

