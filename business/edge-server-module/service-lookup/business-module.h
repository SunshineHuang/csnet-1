#pragma once

#include "cs-lfqueue.h"
#include "csnet_ctx.h"
#include "csnet_log.h"
#include "csnet_config.h"
#include "csnet_conntor.h"

int business_init(csnet_conntor_t* conntor, csnet_log_t* log, csnet_ctx_t* ctx, cs_lfqueue_t* q);
void business_entry(csnet_ss_t* ss, csnet_head_t* head, char* data, int data_len);
void business_timeout();
void business_term();

