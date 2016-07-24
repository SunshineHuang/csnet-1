#pragma once

#include "cs-lfqueue.h"
#include "csnet-ctx.h"
#include "csnet-log.h"
#include "csnet-config.h"
#include "csnet-conntor.h"

int business_init(csnet_conntor_t* conntor, csnet_log_t* log, csnet_ctx_t* ctx, cs_lfqueue_t* q);
void business_entry(csnet_ss_t* ss, csnet_head_t* head, char* data, int data_len);
void business_timeout();

