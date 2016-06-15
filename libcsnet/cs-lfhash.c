#include "cs-lfhash.h"
#include "csnet_fast.h"
#include "csnet_atomic.h"

#include <stdlib.h>

static uint32_t do_hash(uint32_t seed, const unsigned char* str, const ssize_t len);

cs_lfhash_t*
cs_lfhash_new(int size) {
	cs_lfhash_t* ht = calloc(1, sizeof(*ht) + size * sizeof(cs_lflist_t*));
	ht->locked = 0;
	csnet_spinlock_init(&ht->lock);
	ht->size = size;
	ht->seed = random() % UINT32_MAX;
	ht->count = 0;
	for (int i = 0; i < size; i++) {
		ht->table[i] = cs_lflist_new();
	}
	return ht;
}

void
cs_lfhash_free(cs_lfhash_t* ht) {
	for (int i = 0; i < ht->size; i++) {
		cs_lflist_free(ht->table[i]);
	}
	free(ht);
}

int
cs_lfhash_insert(cs_lfhash_t* ht, int64_t key, void* data) {
	if (csnet_slow(ht->locked)) {
		csnet_spinlock_lock(&ht->lock);
	}
	uint32_t hash = do_hash(ht->seed, (void*)&key, sizeof(int64_t));
	int index = hash % ht->size;
	int ret = cs_lflist_insert(ht->table[index], key, data);
	if (ret == 0) {
		INC_ONE_ATOMIC(&ht->count);
	}
	if (csnet_slow(ht->locked)) {
		csnet_spinlock_unlock(&ht->lock);
	}
	return ret;
}

void*
cs_lfhash_search(cs_lfhash_t* ht, int64_t key) {
	void* data = NULL;
	if (csnet_slow(ht->locked)) {
		csnet_spinlock_lock(&ht->lock);
	}
	uint32_t hash = do_hash(ht->seed, (void*)&key, sizeof(int64_t));
	int index = hash % ht->size;
	cs_lflist_node_t* node = cs_lflist_search(ht->table[index], key);
	if (node) {
		data = node->data;
	}
	if (csnet_slow(ht->locked)) {
		csnet_spinlock_unlock(&ht->lock);
	}
	return data;
}

int cs_lfhash_delete(cs_lfhash_t* ht, int64_t key) {
	if (csnet_slow(ht->locked)) {
		csnet_spinlock_lock(&ht->lock);
	}
	uint32_t hash = do_hash(ht->seed, (void*)&key, sizeof(int64_t));
	int index = hash % ht->size;
	int ret = cs_lflist_delete(ht->table[index], key);
	if (ret == 0) {
		DEC_ONE_ATOMIC(&ht->count);
	}
	if (csnet_slow(ht->locked)) {
		csnet_spinlock_unlock(&ht->lock);
	}
	return ret;
}

unsigned long
cs_lfhash_count(cs_lfhash_t* ht) {
	return INC_N_ATOMIC(&ht->count, 0);
}

cs_lflist_t*
cs_lfhash_get_all_keys(cs_lfhash_t* ht) {
	ht->locked = 1;
	csnet_spinlock_lock(&ht->lock);
	cs_lflist_t* new_list = cs_lflist_new();
	for (int i = 0; i < ht->size; i++) {
		cs_lflist_t* tmp_list = ht->table[i];
		cs_lflist_node_t* tmp_node = tmp_list->head->next;
		for (; tmp_node != tmp_list->tail; tmp_node = tmp_node->next) {
			cs_lflist_insert(new_list, tmp_node->key, NULL);
		}
	}
	csnet_spinlock_unlock(&ht->lock);
	ht->locked = 0;
	return new_list;
}

static inline uint32_t
do_hash(uint32_t seed, const unsigned char* str, const ssize_t len) {
	uint32_t hash = seed + len;
	ssize_t slen = len;

	while (slen--) {
		hash += *str++;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	return (hash + (hash << 15));
}

