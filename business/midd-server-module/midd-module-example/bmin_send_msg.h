#pragma once

#include "libcsnet.h"
#include "business_ops.h"

struct bmin_send_msg {
	business_ops_t ops;
	csnet_ss_t* ss;
	csnet_head_t head;
	int64_t ctxid;
};

struct bmin_send_msg* bmin_send_msg_new(csnet_ss_t* ss, csnet_head_t* head);
int64_t bmin_send_msg_req(void* b, csnet_ss_t* ss, csnet_head_t* head, char* data, int data_len);
int64_t bmin_send_msg_rsp(void* b, csnet_head_t* head, char* data, int data_len);
int64_t bmin_send_msg_timeout(void* b);
void bmin_send_msg_err(void* b, csnet_ss_t* ss, csnet_head_t* head);
void bmin_send_msg_free(struct bmin_send_msg* bm);

