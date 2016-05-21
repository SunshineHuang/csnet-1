#include "csnet_ctx.h"
#include "libcsnet.h"

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

inline static unsigned long
gettime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

csnet_ctx_t*
csnet_ctx_new(int size, int timeout) {
	csnet_ctx_t* ctx = calloc(1, sizeof(*ctx));
	if (!ctx) {
		csnet_oom(sizeof(*ctx));
	}

	ctx->ctxid = 1;
	ctx->timeout = timeout;
	ctx->prev_slot = timeout;
	ctx->curr_slot = 0;
	ctx->curr_time = gettime();
	ctx->timerid_hashtbl = ht_create(timeout, timeout * size, NULL);
	ctx->hashtbls = (hashtable_t **)calloc(timeout + 1, sizeof(hashtable_t*));

	for (int i = 0; i < timeout + 1; i++) {
		ctx->hashtbls[i] = ht_create(size, size, NULL);
	}

	return ctx;
}

void
csnet_ctx_free(csnet_ctx_t* ctx) {
	ht_destroy(ctx->timerid_hashtbl);
	for (int i = 0; i < ctx->timeout + 1; i++) {
		ht_destroy(ctx->hashtbls[i]);
	}
	free(ctx->hashtbls);
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
	int* which_slot = malloc(sizeof(int));
	if (!which_slot) {
		csnet_oom(sizeof(int));
	}
	*which_slot = ctx->prev_slot;
	if (ht_set(ctx->timerid_hashtbl, &ctxid, sizeof(int64_t), which_slot, sizeof(int*)) == -1) {
		free(which_slot);
		return -1;
	}
	return ht_set(ctx->hashtbls[*which_slot], &ctxid, sizeof(int64_t), business, bsize);
}

void
csnet_ctx_update(csnet_ctx_t* ctx, int64_t ctxid) {
	int prev_slot = ctx->prev_slot;
	int curr_slot = ctx->curr_slot;
	int* which_slot = ht_get(ctx->timerid_hashtbl, &ctxid, sizeof(int64_t), NULL);

	if (which_slot) {
		if (*which_slot != prev_slot) {
			size_t bsize = 0;
			void* b = ht_get(ctx->hashtbls[*which_slot], &ctxid, sizeof(int64_t), &bsize);

			if (b) {
				ht_delete(ctx->hashtbls[*which_slot], &ctxid, sizeof(int64_t), NULL, NULL);
				*which_slot = curr_slot;
				ht_set(ctx->hashtbls[*which_slot], &ctxid, sizeof(int64_t), b, bsize);
			}
		}
	}
}

void*
csnet_ctx_search(csnet_ctx_t* ctx, int64_t ctxid) {
	int* which_slot = ht_get(ctx->timerid_hashtbl, &ctxid, sizeof(int64_t), NULL);
	if (which_slot) {
		return ht_get(ctx->hashtbls[*which_slot], &ctxid, sizeof(int64_t), NULL);
	}
	return NULL;
}

/*
 * We release the node memory, the node's value should be free by the
 * caller if the caller wanna release it.
 */
void
csnet_ctx_delete(csnet_ctx_t* ctx, int64_t ctxid) {
	int* which_slot = ht_get(ctx->timerid_hashtbl, &ctxid, sizeof(int64_t), NULL);
	if (which_slot) {
		ht_delete(ctx->hashtbls[*which_slot], &ctxid, sizeof(int64_t), NULL, NULL);
		ht_delete(ctx->timerid_hashtbl, &ctxid, sizeof(int64_t), NULL, NULL);
		free(which_slot);
	}
}

int
csnet_ctx_book_keeping(csnet_ctx_t* ctx) {
	unsigned long now = gettime();
	if (now - ctx->curr_time < 1000000) {
		return -1;
	}

	ctx->curr_time = now;
	ctx->prev_slot = ctx->curr_slot;
	__sync_fetch_and_add(&ctx->curr_slot, 1);
	if (ctx->curr_slot > ctx->timeout) {
		ctx->curr_slot = 0;
	}

	int count = ht_count(ctx->hashtbls[ctx->curr_slot]);
	if (count <= 0) {
		return -1;
	}
	return ctx->curr_slot;
}

