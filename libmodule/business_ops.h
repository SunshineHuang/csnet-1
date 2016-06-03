#ifndef business_ops_h
#define business_ops_h

#include <stdint.h>

typedef int64_t (*rsp_cb) (void* b, csnet_head_t* head, char* body, int body_len);
typedef void (*err_cb) (void* b, csnet_sock_t* sock, csnet_head_t* head);
typedef int64_t (*timeout_cb) (void* b);

typedef struct business_ops {
	rsp_cb rsp;
	err_cb err;
	timeout_cb timeout;
} business_ops_t;

#endif  /* business_ops_h */

