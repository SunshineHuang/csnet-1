#include "csnet-module.h"
#include "csnet-atomic.h"
#include "csnet-utils.h"
#include "cs-lfqueue.h"

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

csnet_module_t*
csnet_module_new() {
	csnet_module_t* m = calloc(1, sizeof(*m));
	if (!m) {
		csnet_oom(sizeof(*m));
	}
	m->ref_count = 0;
	return m;
}

void
csnet_module_init(csnet_module_t* m, void* conntor, csnet_log_t* log, csnet_ctx_t* ctx, cs_lfqueue_t* q) {
	m->conntor = conntor;
	m->log = log;
	m->ctx = ctx;
	m->q = q;
}

void
csnet_module_load(csnet_module_t* m, const char* module) {
	m->module = dlopen(module, RTLD_NOW);
	if (!m->module) {
		LOG_FATAL(m->log, "%s", dlerror());
	}

	m->business_init = dlsym(m->module, "business_init");
	if (!m->business_init) {
		LOG_FATAL(m->log, "%s", dlerror());
	}

	m->business_entry = dlsym(m->module, "business_entry");
	if (!m->business_entry) {
		LOG_FATAL(m->log, "%s", dlerror());
	}
	csnet_md5sum(module, m->md5);
	m->md5[16] = '\0';
	m->business_init(m->conntor, m->log, m->ctx, m->q);
}

void
csnet_module_ref_increment(csnet_module_t* m) {
	INC_ONE_ATOMIC(&m->ref_count);
}

void
csnet_module_ref_decrement(csnet_module_t* m) {
	DEC_ONE_ATOMIC(&m->ref_count);
}

void
csnet_module_entry(csnet_module_t* m, csnet_ss_t* ss, csnet_head_t* head, char* data, int data_len) {
	m->business_entry(ss, head, data, data_len);
}

void
csnet_module_free(csnet_module_t* m) {
	dlclose(m->module);
	free(m);
}

