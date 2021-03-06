#pragma once

#include "csnet.h"
#include "csnet-log.h"
#include "csnet-ctx.h"
#include "csnet-module.h"
#include "csnet-conntor.h"
#include "cs-lfqueue.h"

typedef struct csnet_hotpatch {
	cs_lfqueue_t* q;
	csnet_t* csnet;
	csnet_log_t* log;
	csnet_ctx_t* ctx;
	csnet_conntor_t* conntor;
	csnet_module_t* module;
} csnet_hotpatch_t;

csnet_hotpatch_t* csnet_hotpatch_new(cs_lfqueue_t*, csnet_t*, csnet_log_t*, csnet_ctx_t*, csnet_conntor_t*, csnet_module_t*);
void csnet_hotpatch_free(csnet_hotpatch_t*);
int csnet_hotpatch_do_patching(csnet_hotpatch_t*);

