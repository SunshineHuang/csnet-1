#ifndef csnet_ctx_h
#define csnet_ctx_h

#include "hlhashtable.h"

#include <pthread.h>
#include <stdint.h>

typedef struct csnet_ctx {
	int64_t ctxid;
	unsigned long curr_time;
	int timeout;
	int prev_wheel;
	int curr_wheel;
	hashtable_t* which_wheel_tbl;
	hashtable_t** wheels_tbl;
} csnet_ctx_t;

csnet_ctx_t* csnet_ctx_new(int size, int timeout);
void csnet_ctx_free(csnet_ctx_t*);
int csnet_ctx_insert(csnet_ctx_t*, int64_t ctxid, void* business, int bsize);
int64_t csnet_ctx_ctxid(csnet_ctx_t*);
void csnet_ctx_update(csnet_ctx_t*, int64_t ctxid);
void* csnet_ctx_search(csnet_ctx_t*, int64_t ctxid);
void csnet_ctx_delete(csnet_ctx_t*, int64_t ctxid);
int csnet_ctx_book_keeping(csnet_ctx_t*);

#endif  /* csnet_ctx_h */

