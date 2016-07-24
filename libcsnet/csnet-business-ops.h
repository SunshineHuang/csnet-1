#pragma once

#include "csnet-head.h"
#include "csnet-ss.h"

#include <stdint.h>

typedef int64_t (*rsp_cb) (void* b, csnet_head_t* head, char* body, int body_len);
typedef void (*err_cb) (void* b, csnet_ss_t* ss, csnet_head_t* head);
typedef int64_t (*timeout_cb) (void* b);

typedef struct business_ops {
	rsp_cb rsp;
	err_cb err;
	timeout_cb timeout;
} business_ops_t;

