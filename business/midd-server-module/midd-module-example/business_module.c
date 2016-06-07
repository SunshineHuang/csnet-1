#include "business_module.h"
#include "business_ops.h"
#include "business_cmd.h"
#include "csnet_socket_api.h"

#include "bmin_send_msg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define CTX_SIZE 0x100000

/*
 * These variables define at main.c
 */
csnet_log_t* LOG = NULL;
csnet_conntor_t* CONNTOR = NULL;
csnet_ctx_t* CTX = NULL;
cs_lfqueue_t* Q = NULL;

int
business_init(csnet_conntor_t* conntor, csnet_log_t* log, csnet_ctx_t* ctx, cs_lfqueue_t* q) {
	CONNTOR = conntor;
	LOG = log;
	CTX = ctx;
	Q = q;
	LOG_INFO(log, "business init done ...");
	return 0;
}

void
business_entry(csnet_sock_t* sock, csnet_head_t* head, char* data, int data_len) {
	LOG_DEBUG(LOG, "business_entry: cmd: 0x%x, head len: %d, ctxid: %ld, data len: %d",
		head->cmd, head->len, head->ctxid, data_len);

	if (head->cmd == csnet_echo_msg_req) {
		struct bmin_send_msg* bm = bmin_send_msg_new(sock, head);
		if (!bm) {
			LOG_ERROR(LOG, "could not create bmin_send_msg");
			csnet_module_ref_decrement(CONNTOR->module);
			return;
		}
		int64_t ctxid = bm->ctxid;
		if (csnet_ctx_insert(CTX, ctxid, bm, sizeof(*bm)) == 0) {
			LOG_DEBUG(LOG, "insert bm ctxid: %d", ctxid);
			if (bmin_send_msg_req(bm, sock, head, data, data_len) == -1) {
				LOG_ERROR(LOG, "bmin_send_msg_req() failed. ctxid: %d", ctxid);
				csnet_ctx_delete(CTX, ctxid);
				bmin_send_msg_err(bm, sock, head);
				bmin_send_msg_free(bm);
				csnet_module_ref_decrement(CONNTOR->module);
			}
		} else {
			LOG_ERROR(LOG, "could not insert to CTX, ctxid: %ld", ctxid);
			bmin_send_msg_err(bm, sock, head);
			bmin_send_msg_free(bm);
			csnet_module_ref_decrement(CONNTOR->module);
		}
		return;
	}

	if (head->cmd == csnet_echo_msg_rsp) {
		struct bmin_send_msg* bm = csnet_ctx_search(CTX, head->ctxid);
		if (!bm) {
			LOG_ERROR(LOG, "could not find bm by ctxid: %ld", head->ctxid);
			csnet_module_ref_decrement(CONNTOR->module);
			return;
		}
		int ret = bm->ops.rsp(bm, head, data, data_len);
		if (ret == 0) {
			csnet_ctx_delete(CTX, head->ctxid);
		}
		csnet_module_ref_decrement(CONNTOR->module);
		bmin_send_msg_free(bm);
		return;
	}

	LOG_WARNING(LOG, "unknown cmd: 0x%x", head->cmd);
}

