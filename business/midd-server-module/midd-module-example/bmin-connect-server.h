#pragma once

#include "libcsnet.h"
#include "business_ops.h"

struct bmin_connect_server {
	business_ops_t ops;
	csnet_ss_t* ss;
	csnet_head_t head;
	int64_t ctxid;
};

struct bmin_connect_server* bmin_connect_server_new(csnet_ss_t* ss, csnet_head_t* head);
int64_t bmin_connect_server_req(void* b, csnet_ss_t* ss, csnet_head_t* head, char* data, int data_len);
int64_t bmin_connect_server_rsp(void* b, csnet_head_t* head, char* data, int data_len);
int64_t bmin_connect_server_timeout(void* b);
void bmin_connect_server_err(void* b, csnet_ss_t* ss, csnet_head_t* head);
void bmin_connect_server_free(struct bmin_connect_server* bm);

