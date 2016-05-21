#ifndef csnet_module_h
#define csnet_module_h

#include "cs-lfqueue.h"
#include "csnet_sock.h"
#include "csnet_log.h"
#include "csnet_head.h"
#include "csnet_config.h"

typedef int (*module_init_cb) (void* conntor, csnet_log_t* log, csnet_config_t* config, cs_lfqueue_t* q);
typedef void (*module_entry_cb) (csnet_sock_t* sock, csnet_head_t* head, char* data, int data_len, cs_hp_record_t* record);

typedef struct csnet_module {
	void* module;
	void* conntor;
	csnet_log_t* log;
	csnet_config_t* config;
	cs_lfqueue_t* q;
	module_init_cb business_init;
	module_entry_cb business_entry;
} csnet_module_t;

csnet_module_t* csnet_module_new();
void csnet_module_init(csnet_module_t*, void* conntor, csnet_log_t* log, csnet_config_t* config, cs_lfqueue_t* q);
void csnet_module_load(csnet_module_t*, const char* module);
void csnet_module_entry(csnet_module_t*, csnet_sock_t* sock, csnet_head_t* head, char* data, int data_len, cs_hp_record_t* record);
void csnet_module_free(csnet_module_t*);

#endif  /* csnet_module_h */

