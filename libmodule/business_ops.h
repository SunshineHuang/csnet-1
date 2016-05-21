#ifndef business_ops_h
#define business_ops_h

#include "cs-lfqueue.h"

#include <stdint.h>

typedef int64_t (*rsp_cb) (void* b, csnet_head_t* head, char* body, int body_len, cs_hp_record_t* record);
typedef void (*err_cb) (void* b, csnet_sock_t* sock, csnet_head_t* head, cs_hp_record_t* record);
typedef int64_t (*timeout_cb) (void* b, cs_hp_record_t* record);

typedef struct business_ops {
	rsp_cb rsp;
	err_cb err;
	timeout_cb timeout;
} business_ops_t;

#endif  /* business_ops_h */

