#pragma once

#include "libcsnet.h"

struct bmin_get_servers {
	business_ops_t ops;
	csnet_head_t head;
	csnet_ss_t* ss;
};

struct bmin_get_servers* bmin_get_servers_new(csnet_ss_t* ss, csnet_head_t* head);
int64_t bmin_get_servers_req(void* b, csnet_ss_t* ss, csnet_head_t* head, char* data, int data_len);
int64_t bmin_get_servers_rsp(void* b, csnet_head_t* head, char* data, int data_len);
int64_t bmin_get_servers_timeout(void* b);
void bmin_get_servers_err(void* b, csnet_ss_t* ss, csnet_head_t* head);
void bmin_get_servers_free(struct bmin_get_servers* bm);

