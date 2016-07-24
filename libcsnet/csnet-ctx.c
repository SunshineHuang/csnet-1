#include "csnet-ctx.h"
#include "csnet-atomic.h"
#include "csnet-utils.h"
#include "csnet-socket-api.h"
#include "csnet-business-ops.h"
#include "csnet-fast.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static void* thread_check_timeout(void* arg);
static void business_timeout(csnet_ctx_t*);

csnet_ctx_t*
csnet_ctx_new(int size, int timeout, cs_lfqueue_t* q) {
	pthread_t tid;
	csnet_ctx_t* ctx = calloc(1, sizeof(*ctx) + (timeout + 1) * sizeof(cs_lfhash_t*));
	if (!ctx) {
		csnet_oom(sizeof(*ctx));
	}

	ctx->ctxid = 1;
	ctx->timeout = timeout;
	ctx->prev_wheel = timeout;
	ctx->curr_wheel = 0;
	ctx->curr_time = csnet_gettime();
	ctx->q = q;
	ctx->which_wheel_tbl = cs_lfhash_new(size);
	for (int i = 0; i < timeout + 1; i++) {
		ctx->wheels_tbl[i] = cs_lfhash_new(size);
	}
	pthread_create(&tid, NULL, thread_check_timeout, ctx);
	return ctx;
}

void
csnet_ctx_free(csnet_ctx_t* ctx) {
	cs_lfhash_free(ctx->which_wheel_tbl);
	for (int i = 0; i < ctx->timeout + 1; i++) {
		cs_lfhash_free(ctx->wheels_tbl[i]);
	}
	free(ctx);
}

int64_t
csnet_ctx_ctxid(csnet_ctx_t* ctx) {
	int64_t ctxid = INC_ONE_ATOMIC(&ctx->ctxid);
	if (csnet_slow(ctxid == 0xcafebabeface)) {
		ctx->ctxid = 1;
		return INC_ONE_ATOMIC(&ctx->ctxid);
	}
	return ctxid;
}

int
csnet_ctx_insert(csnet_ctx_t* ctx, int64_t ctxid, void* business, int bsize) {
	int* which_wheel = malloc(sizeof(int));
	if (csnet_slow(!which_wheel)) {
		csnet_oom(sizeof(int));
	}
	*which_wheel = ctx->prev_wheel;
	if (cs_lfhash_insert(ctx->which_wheel_tbl, ctxid, which_wheel) == -1) {
		free(which_wheel);
		return -1;
	}
	return cs_lfhash_insert(ctx->wheels_tbl[*which_wheel], ctxid, business);
}

void
csnet_ctx_update(csnet_ctx_t* ctx, int64_t ctxid) {
	int prev_wheel = ctx->prev_wheel;
	int curr_wheel = ctx->curr_wheel;
	int* which_wheel = cs_lfhash_search(ctx->which_wheel_tbl, ctxid);

	if (csnet_fast(which_wheel)) {
		if (*which_wheel != prev_wheel) {
			void* b = cs_lfhash_search(ctx->wheels_tbl[*which_wheel], ctxid);
			if (b) {
				cs_lfhash_delete(ctx->wheels_tbl[*which_wheel], ctxid);
				*which_wheel = curr_wheel;
				cs_lfhash_insert(ctx->wheels_tbl[*which_wheel], ctxid, b);
			}
		}
	}
}

void*
csnet_ctx_search(csnet_ctx_t* ctx, int64_t ctxid) {
	int* which_wheel = cs_lfhash_search(ctx->which_wheel_tbl, ctxid);
	if (csnet_fast(which_wheel)) {
		return cs_lfhash_search(ctx->wheels_tbl[*which_wheel], ctxid);
	}
	return NULL;
}

/*
 * We release the node memory, the node's value should be free by the
 * caller if the caller wanna release it.
 */
void
csnet_ctx_delete(csnet_ctx_t* ctx, int64_t ctxid) {
	int* which_wheel = cs_lfhash_search(ctx->which_wheel_tbl, ctxid);
	if (csnet_fast(which_wheel)) {
		cs_lfhash_delete(ctx->wheels_tbl[*which_wheel], ctxid);
		cs_lfhash_delete(ctx->which_wheel_tbl, ctxid);
		free(which_wheel);
	}
}

int
csnet_ctx_book_keeping(csnet_ctx_t* ctx) {
	unsigned long now = csnet_gettime();
	if (now - ctx->curr_time < 1000000) {
		return -1;
	}
	ctx->curr_time = now;
	ctx->prev_wheel = ctx->curr_wheel;
	INC_ONE_ATOMIC(&ctx->curr_wheel);
	if (ctx->curr_wheel > ctx->timeout) {
		ctx->curr_wheel = 0;
	}
	int count = cs_lfhash_count(ctx->wheels_tbl[ctx->curr_wheel]);
	if (count <= 0) {
		return -1;
	}
	return ctx->curr_wheel;
}

static void
business_timeout(csnet_ctx_t* ctx) {
	int expired_wheel = csnet_ctx_book_keeping(ctx);
	if (expired_wheel > -1) {
		cs_lflist_t* keys = cs_lfhash_get_all_keys(ctx->wheels_tbl[expired_wheel]);
		cs_lflist_node_t* head = keys->head->next;
		while (head != keys->tail) {
			int64_t ctxid = head->key;
			void* b = csnet_ctx_search(ctx, ctxid);
			if (b) {
				business_ops_t* ops = (business_ops_t *)b;
				if (ops->timeout(b) == 0) {
					free(b);
					csnet_ctx_delete(ctx, ctxid);
				} else {
					csnet_ctx_update(ctx, ctxid);
				}
			}
			head = head->next;
		}
		cs_lflist_free(keys);
	}
}

static inline void*
thread_check_timeout(void* arg) {
	csnet_ctx_t* ctx = arg;
	cs_lfqueue_t* q = ctx->q;
	cs_lfqueue_register_thread(q);
	while (1) {
		business_timeout(ctx);
		wait_milliseconds(1000);
	}
	return NULL;
}

