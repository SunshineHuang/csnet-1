#ifndef csnet_ctx_h
#define csnet_ctx_h

#include "cs-lfhash.h"
#include "csnet_ctx.h"
#include "cs-lfqueue.h"

#include <pthread.h>
#include <stdint.h>

#define CTX_SIZE 0x1000

typedef struct csnet_ctx {
	int64_t ctxid;
	int timeout;
	int prev_wheel;
	int curr_wheel;
	unsigned long curr_time;
	cs_lfqueue_t* q;
	cs_lfhash_t* which_wheel_tbl;
	cs_lfhash_t* wheels_tbl[0];
} csnet_ctx_t;

csnet_ctx_t* csnet_ctx_new(int size, int timeout, cs_lfqueue_t* q);
void csnet_ctx_free(csnet_ctx_t*);
int csnet_ctx_insert(csnet_ctx_t*, int64_t ctxid, void* business, int bsize);
int64_t csnet_ctx_ctxid(csnet_ctx_t*);
void csnet_ctx_update(csnet_ctx_t*, int64_t ctxid);
void* csnet_ctx_search(csnet_ctx_t*, int64_t ctxid);
void csnet_ctx_delete(csnet_ctx_t*, int64_t ctxid);
int csnet_ctx_book_keeping(csnet_ctx_t*);

#endif  /* csnet_ctx_h */

