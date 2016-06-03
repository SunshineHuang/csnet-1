#ifndef business_module_h
#define business_module_h

#include "cs-lfqueue.h"
#include "csnet_log.h"
#include "csnet_config.h"
#include "csnet_conntor.h"

int business_init(csnet_conntor_t* conntor, csnet_log_t* log, csnet_config_t* config, cs_lfqueue_t* q);
void business_entry(csnet_sock_t* sock, csnet_head_t* head, char* data, int data_len);
void business_timeout();

#endif  /* business_module_h */

