#include "csnet-hotpatch.h"
#include "csnet-utils.h"
#include "csnet-log.h"
#include "cs-priority-queue.h"
#include "csnet-socket-api.h"

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>

static int hotpatching(void* target, void* replacement);

void*
hotpatching_thread(void* arg) {
	csnet_hotpatch_t* hp = arg;
	while (1) {
		csnet_hotpatch_do_patching(hp);
		sleep(10);
	}

	return NULL;
}

csnet_hotpatch_t*
csnet_hotpatch_new(cs_lfqueue_t* q, csnet_t* csnet, csnet_log_t* log, csnet_ctx_t* ctx, csnet_conntor_t* conntor, csnet_module_t* module) {
	pthread_t tid;
	csnet_hotpatch_t* hp = calloc(1, sizeof(*hp));
	hp->q = q;
	hp->csnet = csnet;
	hp->log = log;
	hp->ctx = ctx;
	hp->conntor = conntor;
	hp->module = module;
	pthread_create(&tid, NULL, hotpatching_thread, hp);
	return hp;
}

void
csnet_hotpatch_free(csnet_hotpatch_t* hp) {
	free(hp);
}

int csnet_hotpatch_do_patching(csnet_hotpatch_t* hp) {
	int ret = -1;
	int len = strlen("business_module.so");
	cs_pqueue_t* pq = cs_pqueue_new(CS_PQ_HIGHEST_PRIORITY);
	DIR* d;
	struct dirent* dir;
	d = opendir(".");

	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if (strncmp(dir->d_name, "business_module.so", len) == 0) {
				struct stat stat_buff;
				stat(dir->d_name, &stat_buff);
				char buff[32] = {0};
				strcat(buff, "./");
				strcat(buff, dir->d_name);
				char* value = strdup(buff);
				cs_pqueue_push(pq, stat_buff.st_mtime, value);
			}
		}
		closedir(d);
	}

	cs_pqnode_t* highest = cs_pqueue_pop(pq);
	if (highest) {
		unsigned char new_md5[17];
		csnet_md5sum(highest->value, new_md5);
		new_md5[16] = '\0';

		if (strcmp((char*)new_md5, (char*)hp->module->md5) == 0) {
			ret = -1;
			goto out;
		}

		csnet_module_t* old_module = hp->module;
		hp->module = csnet_module_new();
		csnet_module_init(hp->module, hp->conntor, hp->log, hp->ctx, hp->q);
		csnet_module_load(hp->module, highest->value);

		if (hotpatching(old_module->business_entry, hp->module->business_entry) == 0) {
			csnet_reset_module(hp->csnet, hp->module);
			if (hp->conntor) {
				csnet_conntor_reset_module(hp->conntor, hp->module);
			}
			while (hp->conntor || old_module->ref_count != 0) {
				wait_milliseconds(10);
			}
			csnet_module_free(old_module);
			LOG_INFO(hp->log, "hotpatch done");
			ret = 0;
			goto out;
		}

		LOG_ERROR(hp->log, "hotpatch failed");
		ret = -1;
	}

out:
	if (highest) {
		free(highest->value);
		cs_pqueue_delete(pq, highest);
	}
	cs_pqueue_free(pq);
	return ret;
}

/*
 * This function was obtained from http://nullprogram.com/blog/2016/03/31/.
 * Copyright (c) 2016 wellons <wellons@nullprogram.com>
 */

static int
hotpatching(void* target, void* replacement) {
	int ret;
	assert(((uintptr_t)target & 0x07) == 0);
	void* page = (void *)((uintptr_t)target & ~0xfff);
	ret = mprotect(page, 4096, PROT_WRITE | PROT_EXEC);

	if (ret == -1) {
		return -1;
	}

	uint32_t rel = (char *)replacement - (char *)target - 5;
	union {
		uint8_t bytes[8];
		uint64_t value;
	} instruction = {{0xe9, rel >> 0, rel >> 8, rel >> 16, rel >> 24}};

	*(uint64_t *)target = instruction.value;
	mprotect(page, 4096, PROT_EXEC);

	return 0;
}

