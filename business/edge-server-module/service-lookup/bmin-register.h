#pragma once

#include "libcsnet.h"

struct bmin_register {
	business_ops_t ops;
	csnet_head_t head;
	csnet_ss_t* ss;
};

struct bmin_register* bmin_register_new(csnet_ss_t* ss, csnet_head_t* head);
int64_t bmin_register_req(void* b, csnet_ss_t* ss, csnet_head_t* head, char* data, int data_len);
int64_t bmin_register_rsp(void* b, csnet_head_t* head, char* data, int data_len);
int64_t bmin_register_timeout(void* b);
void bmin_register_err(void* b, csnet_ss_t* ss, csnet_head_t* head);
void bmin_register_free(struct bmin_register* bm);

