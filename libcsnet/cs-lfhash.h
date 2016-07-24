#pragma once

#include "cs-lflist.h"
#include "csnet-spinlock.h"

#include <stdint.h>

typedef struct cs_lfhash cs_lfhash_t;

struct cs_lfhash {
	unsigned int locked;
	csnet_spinlock_t lock;
	int size;
	uint32_t seed;
	unsigned long count;
	cs_lflist_t* table[0];
};

cs_lfhash_t* cs_lfhash_new(int size);
void cs_lfhash_free(cs_lfhash_t*);
int cs_lfhash_insert(cs_lfhash_t*, int64_t key, void* data);
void* cs_lfhash_search(cs_lfhash_t*, int64_t key);
int cs_lfhash_delete(cs_lfhash_t*, int64_t key);
unsigned long cs_lfhash_count(cs_lfhash_t*);
cs_lflist_t* cs_lfhash_get_all_keys(cs_lfhash_t*);
cs_lflist_t* cs_lfhash_getlist(cs_lfhash_t* ht, int64_t key);
