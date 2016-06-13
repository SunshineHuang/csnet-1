#pragma once

#include <stdint.h>

typedef struct cs_htnode cs_htnode_t;
typedef struct cs_ht cs_ht_t;
typedef struct cs_htlist cs_htlist_t;

struct cs_htlist {
	cs_htnode_t* head;
	cs_htnode_t* tail;
};

struct cs_htnode {
	int key_len;
	int value_len;
	void* key;
	void* value;
	cs_htnode_t* prev;
	cs_htnode_t* next;
};

struct cs_ht {
	cs_htlist_t** lists;
	uint32_t seed;
	int key_count;
	int size;
};

cs_ht_t* cs_ht_new();
void cs_ht_free(cs_ht_t*);
int cs_ht_insert(cs_ht_t*, void* key, int key_len, void* value, int value_len);
cs_htnode_t* cs_ht_search(cs_ht_t*, void* key, int key_len);

/*
 * cs_ht_delete() does not free the memory of node, do it yourself.
 * Both key and vlaue of the node.
 */
int cs_ht_delete(cs_ht_t*, cs_htnode_t* node);

