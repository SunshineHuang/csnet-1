#include "libcsnet.h"
/*
 * Business header
 */
#include "server-mgr.h"
#include "bmin-register.h"
#include "bmin-get-servers.h"
#include "business-cmd.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

/*
 * These variables define at main.c
 */
csnet_log_t* LOG = NULL;
csnet_ctx_t* CTX = NULL;
cs_lfqueue_t* Q = NULL;
server_mgr_t* mgr;

int
business_init(csnet_conntor_t* conntor, csnet_log_t* log, csnet_ctx_t* ctx, cs_lfqueue_t* q) {
	assert(log);
	assert(ctx);
	assert(q);

	LOG = log;
	CTX = ctx;
	Q = q;

	mgr = server_mgr_new(107);

	LOG_INFO(log, "business init done ...");
	return 0;
}

void
business_entry(csnet_ss_t* ss, csnet_head_t* head, char* data, int data_len) {
	LOG_INFO(LOG, "business_entry cmd: 0x%x, head len: %d, ctxid: %ld, data len: %d",
		head->cmd, head->len, head->ctxid, data_len);

	switch (head->cmd) {
	case CSNET_REGISTER_REQ: {
		struct bmin_register* bm = bmin_register_new(ss, head);
		if (csnet_slow(!bm)) {
			break;
		}

		bmin_register_req(bm, ss, head, data, data_len);
		bmin_register_free(bm);
		break;
	}

	case CSNET_GET_SERVERS_REQ: {
		struct bmin_get_servers* bm = bmin_get_servers_new(ss, head);
		if (csnet_slow(!bm)) {
			break;
		}

		bmin_get_servers_req(bm, ss, head, data, data_len);
		bmin_get_servers_free(bm);
		break;
	}

	default:
		LOG_WARNING(LOG, "unknown cmd: 0x%x", head->cmd);
	}
}

