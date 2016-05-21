#ifndef cs_doubly_linked_list_h
#define cs_doubly_linked_list_h

typedef struct cs_dl_node cs_dl_node_t;
typedef struct cs_dlist cs_dlist_t;

struct cs_dl_node {
	void* data;
	cs_dl_node_t* prev;
	cs_dl_node_t* next;
};

struct cs_dlist {
	cs_dl_node_t* head;
	cs_dl_node_t* tail;
};

cs_dl_node_t* cs_dl_node_new(void* data);
void cs_dl_node_free(cs_dl_node_t*);

cs_dlist_t* cs_dlist_new();
void cs_dlist_free(cs_dlist_t*);
cs_dl_node_t* cs_dlist_search(cs_dlist_t*, void* data);
void cs_dlist_insert(cs_dlist_t*, cs_dl_node_t* x);
void cs_dlist_remove(cs_dlist_t*, cs_dl_node_t* x);

#endif  /* cs_doubly_linked_list_h */

