#include "csnet_module.h"
#include "csnet_utils.h"
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
	return m;
}

void
csnet_module_init(csnet_module_t* m, void* conntor, csnet_log_t* log, csnet_config_t* config, cs_lfqueue_t* q) {
	m->conntor = conntor;
	m->log = log;
	m->config = config;
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

	m->business_init(m->conntor, m->log, m->config, m->q);
}

void
csnet_module_entry(csnet_module_t* m, csnet_sock_t* sock, csnet_head_t* head, char* data, int data_len) {
	m->business_entry(sock, head, data, data_len);
}

void
csnet_module_free(csnet_module_t* m) {
	dlclose(m->module);
	free(m);
}

