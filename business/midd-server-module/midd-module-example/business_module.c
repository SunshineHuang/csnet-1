#include "business_module.h"
#include "business_ops.h"
#include "business_cmd.h"
#include "csnet_ctx.h"
#include "csnet_config.h"
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
csnet_config_t* CONFIG = NULL;

/*
 * Global variables
 */
csnet_ctx_t* CTX = NULL;
cs_lfqueue_t* Q = NULL;

static void* thread_check_timeout(void* arg);

int
business_init(csnet_conntor_t* conntor, csnet_log_t* log, csnet_config_t* config, cs_lfqueue_t* q) {
	CONNTOR = conntor;
	LOG = log;
	CONFIG = config;
	Q = q;

	/*
	 * Global variables initialization once.
	 */

	char* business_timeout = csnet_config_find(CONFIG, "business_timeout", strlen("business_timeout"));

	if (!business_timeout) {
		LOG_FATAL(LOG, "could not find `business_timeout`!");
	}

	CTX = csnet_ctx_new(CTX_SIZE, atoi(business_timeout));
	pthread_t tid;
	pthread_create(&tid, NULL, thread_check_timeout, q);
	LOG_INFO(LOG, "business init done ...");

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
			}
		} else {
			LOG_ERROR(LOG, "could not insert to CTX, ctxid: %ld", ctxid);
			bmin_send_msg_err(bm, sock, head);
			bmin_send_msg_free(bm);
		}
		return;
	}

	if (head->cmd == csnet_echo_msg_rsp) {
		struct bmin_send_msg* bm = csnet_ctx_search(CTX, head->ctxid);
		if (!bm) {
			LOG_ERROR(LOG, "could not find bm by ctxid: %ld", head->ctxid);
			return;
		}
		int ret = bm->ops.rsp(bm, head, data, data_len);
		if (ret == 0) {
			csnet_ctx_delete(CTX, head->ctxid);
		}
		bmin_send_msg_free(bm);
		return;
	}

	LOG_WARNING(LOG, "unknown cmd: 0x%x", head->cmd);
}

void business_timeout() {
	/*
	int expired_wheel = csnet_ctx_book_keeping(CTX);
	if (expired_wheel > -1) {
		linked_list_t* keys = ht_get_all_keys(CTX->wheels_tbl[expired_wheel]);
		int count = list_count(keys);
		for (int i = 0; i < count; i++) {
			hashtable_key_t* key = list_pick_value(keys, i);
			if (!key) continue;
			long* ctxid = (long *)key->data;
			void* b = csnet_ctx_search(CTX, *ctxid);
			if (!b) continue;
			business_ops_t* ops = (business_ops_t*)b;
			if (ops->timeout(b) == 0) {
				free(b);
				csnet_ctx_delete(CTX, *ctxid);
			} else {
				csnet_ctx_update(CTX, *ctxid);
			}
		}
		list_destroy(keys);
	}
	*/
}

static inline void*
thread_check_timeout(void* arg) {
	cs_lfqueue_t* q = (cs_lfqueue_t*)arg;
	cs_lfqueue_register_thread(q);
	while (1) {
		business_timeout();
		wait_milliseconds(1000);
	}
	return NULL;

}

