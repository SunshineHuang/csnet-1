#include "cs-doubly-linked-list.h"

#include <stdlib.h>

cs_dl_node_t*
cs_dl_node_new(void* data) {
	cs_dl_node_t* node = calloc(1, sizeof(*node));
	node->data = data;
	node->prev = NULL;
	node->next = NULL;
	return node;
}

void
cs_dl_node_free(cs_dl_node_t* node) {
	free(node);
}

cs_dlist_t*
cs_dlist_new() {
	cs_dlist_t* l = calloc(1, sizeof(*l));
	l->head = NULL;
	l->tail = NULL;
	return l;
}

void
cs_dlist_free(cs_dlist_t* l) {
	cs_dl_node_t* x = l->head;
	while (x) {
		cs_dl_node_t* tmp = x->next;
		cs_dl_node_free(x);
		x = tmp;
	}
	free(l);
}

cs_dl_node_t*
cs_dlist_search(cs_dlist_t* l, void* data) {
	cs_dl_node_t* head = l->head;
	while (head && head->data != data) {
		head = head->next;
	}
	return head;
}

void
cs_dlist_insert(cs_dlist_t* l, cs_dl_node_t* node) {
	node->next = l->head;
	if (l->head) {
		l->head->prev = node;
	}
	l->head = node;
	node->prev = NULL;
}

void
cs_dlist_remove(cs_dlist_t* l, cs_dl_node_t* node) {
	if (node->prev) {
		node->prev->next = node->next;
	} else {
		l->head = node->next;
	}
	if (node->next) {
		node->next->prev = node->prev;
	}
}

