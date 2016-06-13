#ifndef cs_lflist_h
#define cs_lflist_h

#include <stdint.h>

typedef struct cs_lflist cs_lflist_t;
typedef struct cs_lflist_node cs_lflist_node_t;

struct cs_lflist_node {
	int64_t key;
	void* data;
	cs_lflist_node_t* next;
};

struct cs_lflist {
	cs_lflist_node_t* head;
	cs_lflist_node_t* tail;
};

cs_lflist_node_t* cs_lflist_node_new(int64_t key, void* data);
void cs_lflist_node_free(cs_lflist_node_t* node);

cs_lflist_t* cs_lflist_new();
void cs_lflist_free(cs_lflist_t*);
int cs_lflist_insert(cs_lflist_t*, int64_t key, void* data);
int cs_lflist_delete(cs_lflist_t*, int64_t key);
cs_lflist_node_t* cs_lflist_search(cs_lflist_t*, int64_t key);

#endif  /* cs_lflist_h */

