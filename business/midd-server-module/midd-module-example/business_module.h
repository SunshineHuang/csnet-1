#ifndef business_module_h
#define business_module_h

#include "libcsnet.h"
#include "csnet_ctx.h"

int business_init(csnet_conntor_t* conntor, csnet_log_t* log, csnet_ctx_t* ctx, cs_lfqueue_t* q);
void business_entry(csnet_sock_t* sock, csnet_head_t* head, char* data, int data_len);
void business_timeout();

#endif  /* business_module_h */

