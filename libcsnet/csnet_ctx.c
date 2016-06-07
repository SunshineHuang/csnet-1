#include "csnet_ctx.h"
#include "business_ops.h"
#include "libcsnet.h"

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

static inline void* thread_check_timeout(void* arg);

void
business_timeout(csnet_ctx_t* ctx) {
	int expired_wheel = csnet_ctx_book_keeping(ctx);
	if (expired_wheel > -1) {
		linked_list_t* keys = ht_get_all_keys(ctx->wheels_tbl[expired_wheel]);
		int count = list_count(keys);
		for (int i = 0; i < count; i++) {
			hashtable_key_t* key = list_pick_value(keys, i);
			if (!key) continue;
			long* ctxid = (long *)key->data;
			void* b = csnet_ctx_search(ctx, *ctxid);
			if (!b) continue;
			business_ops_t* ops = (business_ops_t *)b;
			if (ops->timeout(b) == 0) {
				free(b);
				csnet_ctx_delete(ctx, *ctxid);
			} else {
				csnet_ctx_update(ctx, *ctxid);
			}
		}
		list_destroy(keys);
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

inline static unsigned long
gettime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

csnet_ctx_t*
csnet_ctx_new(int size, int timeout, cs_lfqueue_t* q) {
	pthread_t tid;
	csnet_ctx_t* ctx = calloc(1, sizeof(*ctx) + (timeout + 1) * sizeof(hashtable_t*));
	if (!ctx) {
		csnet_oom(sizeof(*ctx));
	}

	ctx->ctxid = 1;
	ctx->timeout = timeout;
	ctx->prev_wheel = timeout;
	ctx->curr_wheel = 0;
	ctx->curr_time = gettime();
	ctx->q = q;
	ctx->which_wheel_tbl = ht_create(timeout, timeout * size, NULL);
	for (int i = 0; i < timeout + 1; i++) {
		ctx->wheels_tbl[i] = ht_create(size, size, NULL);
	}
	pthread_create(&tid, NULL, thread_check_timeout, ctx);
	return ctx;
}

void
csnet_ctx_free(csnet_ctx_t* ctx) {
	ht_destroy(ctx->which_wheel_tbl);
	for (int i = 0; i < ctx->timeout + 1; i++) {
		ht_destroy(ctx->wheels_tbl[i]);
	}
	free(ctx);
}

int64_t
csnet_ctx_ctxid(csnet_ctx_t* ctx) {
	int64_t ctxid = __sync_fetch_and_add(&(ctx->ctxid), 1);
	if (ctxid == 0xcafebabeface) {
		ctx->ctxid = 1;
		return __sync_fetch_and_add(&(ctx->ctxid), 1);
	}
	return ctxid;
}

int
csnet_ctx_insert(csnet_ctx_t* ctx, int64_t ctxid, void* business, int bsize) {
	int* which_wheel = malloc(sizeof(int));
	if (!which_wheel) {
		csnet_oom(sizeof(int));
	}
	*which_wheel = ctx->prev_wheel;
	if (ht_set(ctx->which_wheel_tbl, &ctxid, sizeof(int64_t), which_wheel, sizeof(int*)) == -1) {
		free(which_wheel);
		return -1;
	}
	return ht_set(ctx->wheels_tbl[*which_wheel], &ctxid, sizeof(int64_t), business, bsize);
}

void
csnet_ctx_update(csnet_ctx_t* ctx, int64_t ctxid) {
	int prev_wheel = ctx->prev_wheel;
	int curr_wheel = ctx->curr_wheel;
	int* which_wheel = ht_get(ctx->which_wheel_tbl, &ctxid, sizeof(int64_t), NULL);

	if (which_wheel) {
		if (*which_wheel != prev_wheel) {
			size_t bsize = 0;
			void* b = ht_get(ctx->wheels_tbl[*which_wheel], &ctxid, sizeof(int64_t), &bsize);

			if (b) {
				ht_delete(ctx->wheels_tbl[*which_wheel], &ctxid, sizeof(int64_t), NULL, NULL);
				*which_wheel = curr_wheel;
				ht_set(ctx->wheels_tbl[*which_wheel], &ctxid, sizeof(int64_t), b, bsize);
			}
		}
	}
}

void*
csnet_ctx_search(csnet_ctx_t* ctx, int64_t ctxid) {
	int* which_wheel = ht_get(ctx->which_wheel_tbl, &ctxid, sizeof(int64_t), NULL);
	if (which_wheel) {
		return ht_get(ctx->wheels_tbl[*which_wheel], &ctxid, sizeof(int64_t), NULL);
	}
	return NULL;
}

/*
 * We release the node memory, the node's value should be free by the
 * caller if the caller wanna release it.
 */
void
csnet_ctx_delete(csnet_ctx_t* ctx, int64_t ctxid) {
	int* which_wheel = ht_get(ctx->which_wheel_tbl, &ctxid, sizeof(int64_t), NULL);
	if (which_wheel) {
		ht_delete(ctx->wheels_tbl[*which_wheel], &ctxid, sizeof(int64_t), NULL, NULL);
		ht_delete(ctx->which_wheel_tbl, &ctxid, sizeof(int64_t), NULL, NULL);
		free(which_wheel);
	}
}

int
csnet_ctx_book_keeping(csnet_ctx_t* ctx) {
	unsigned long now = gettime();
	if (now - ctx->curr_time < 1000000) {
		return -1;
	}

	ctx->curr_time = now;
	ctx->prev_wheel = ctx->curr_wheel;
	__sync_fetch_and_add(&ctx->curr_wheel, 1);
	if (ctx->curr_wheel > ctx->timeout) {
		ctx->curr_wheel = 0;
	}

	int count = ht_count(ctx->wheels_tbl[ctx->curr_wheel]);
	if (count <= 0) {
		return -1;
	}
	return ctx->curr_wheel;
}

