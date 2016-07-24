#include "libcsnet.h"
/*
 * Business header
 */
#include "bmin-send-msg.h"
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


int
business_init(csnet_conntor_t* conntor, csnet_log_t* log, csnet_ctx_t* ctx, cs_lfqueue_t* q) {
	assert(log);
	assert(ctx);
	assert(q);

	LOG = log;
	CTX = ctx;
	Q = q;

	LOG_INFO(log, "business init done ...");
	return 0;
}

void
business_entry(csnet_ss_t* ss, csnet_head_t* head, char* data, int data_len) {
	LOG_INFO(LOG, "business_entry cmd: 0x%x, head len: %d, ctxid: %ld, data len: %d",
		head->cmd, head->len, head->ctxid, data_len);

	if (head->cmd == csnet_echo_msg_req) {
		struct bmin_send_msg* bm = bmin_send_msg_new(ss, head);
		if (!bm) {
			return;
		}
		long ctxid = bmin_send_msg_req(bm, ss, head, data, data_len);
		if (ctxid == 0) {
			bmin_send_msg_free(bm);
		} else {
			if (csnet_ctx_insert(CTX, ctxid, bm, sizeof(*bm)) == 0) {
				LOG_DEBUG(LOG, "insert bm to CTX with ctxid: %ld", ctxid);
			} else {
				LOG_ERROR(LOG, "could not insert to CTX. ctxid: %ld", ctxid);
				bmin_send_msg_err(bm, ss, head);
				bmin_send_msg_free(bm);
			}
		}
		return;
	}

	LOG_WARNING(LOG, "unknown cmd: 0x%x", head->cmd);
}

