#pragma once

typedef struct cs_sl_node cs_sl_node_t;
typedef struct cs_slist cs_slist_t;

struct cs_sl_node {
	int data;
	cs_sl_node_t* next;
};

struct cs_slist {
	cs_sl_node_t* head;
};

cs_sl_node_t* cs_sl_node_new(int data);
void cs_sl_node_free(cs_sl_node_t* node);

cs_slist_t* cs_slist_new();
void cs_slist_free(cs_slist_t*);

cs_sl_node_t* cs_slist_search(cs_slist_t*, int k);
void cs_slist_insert(cs_slist_t*, cs_sl_node_t* x);
void cs_slist_remove(cs_slist_t*, cs_sl_node_t* x);
void cs_slist_reverse(cs_slist_t*);
void cs_slist_print(const cs_slist_t*);

