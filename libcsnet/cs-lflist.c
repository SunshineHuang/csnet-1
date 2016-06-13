#include "cs-lflist.h"
#include "csnet_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define cas __sync_bool_compare_and_swap

int 
is_marked_reference(intptr_t p) {
	return (int) (p & 0x1L);
}

intptr_t
get_marked_reference(intptr_t p) {
	return p | 0x1L;
}

intptr_t
get_unmarked_reference(intptr_t p) {
	return p & ~0x1L;
}

static cs_lflist_node_t* inner_search(cs_lflist_t* l, int64_t key, cs_lflist_node_t** left_node);

cs_lflist_node_t*
cs_lflist_node_new(int64_t key, void* data) {
	cs_lflist_node_t* node = calloc(1, sizeof(*node));
	node->key = key;
	node->data = data;
	node->next = NULL;
	return node;
}

void
cs_lflist_node_free(cs_lflist_node_t* node) {
	free(node);
}

cs_lflist_t*
cs_lflist_new() {
	cs_lflist_t* l = calloc(1, sizeof(*l));
	l->head = cs_lflist_node_new(INT64_MIN, NULL);
	l->tail = cs_lflist_node_new(INT64_MAX, NULL);
	l->head->next = l->tail;
	return l;
}

void
cs_lflist_free(cs_lflist_t* l) {
	cs_lflist_node_t* head = l->head->next;
	cs_lflist_node_t* tmp;
	while (head != l->tail) {
		tmp = head->next;
		cs_lflist_node_free(head);
		head = tmp;
	}
	cs_lflist_node_free(l->head);
	cs_lflist_node_free(l->tail);
	free(l);
}

int
cs_lflist_insert(cs_lflist_t* l, int64_t key, void* data) {
	cs_lflist_node_t* new_node = cs_lflist_node_new(key, data);
	cs_lflist_node_t* right_node = NULL;
	cs_lflist_node_t* left_node = NULL;

	while (1) {
		right_node = inner_search(l, key, &left_node);
		if ((right_node != l->tail) && (right_node->key == key)) {
			free(new_node);
			return -1;
		}
		new_node->next = right_node;
		if (cas(&(left_node->next), right_node, new_node)) {
			return 0;
		}
	}
}

int
cs_lflist_delete(cs_lflist_t* l, int64_t key) {
	cs_lflist_node_t* right_node = NULL;
	cs_lflist_node_t* right_node_next = NULL;
	cs_lflist_node_t* left_node = NULL;

	while (1) {
		right_node = inner_search(l, key, &left_node);
		if ((right_node == l->tail) || right_node->key != key) {
			return -1;
		}

		right_node_next = right_node->next;
		if (!is_marked_reference((intptr_t)right_node_next)) {
			if (cas(&(right_node->next), right_node_next, get_marked_reference((intptr_t)right_node_next))) {
				break;
			}
		}
	}

	if (!cas(&(left_node->next), right_node, right_node_next)) {
		right_node = inner_search(l, right_node->key, &left_node);
	}
	cs_lflist_node_free(right_node);

	return 0;
}

cs_lflist_node_t*
cs_lflist_search(cs_lflist_t* l, int64_t key) {
	cs_lflist_node_t* right_node = NULL;
	cs_lflist_node_t* left_node = NULL;
	right_node = inner_search(l, key, &left_node);

	if ((right_node == l->tail) || (right_node->key != key)) {
		return NULL;
	} else {
		return right_node;
	}
}

static inline cs_lflist_node_t*
inner_search(cs_lflist_t* l, int64_t key, cs_lflist_node_t** left_node) {
	cs_lflist_node_t* left_node_next = NULL;
	cs_lflist_node_t* right_node = NULL;

	while (1) {
		cs_lflist_node_t* t = l->head;
		cs_lflist_node_t* t_next = l->head->next;
		do {
			if (!is_marked_reference((intptr_t)t_next)) {
				(*left_node) = t;
				left_node_next = t_next;
			}
			t = (cs_lflist_node_t*)get_unmarked_reference((intptr_t)t_next);

			if (t == l->tail) {
				break;
			}

			t_next = t->next;
		} while (is_marked_reference((intptr_t)t_next) || (t->key < key));

		right_node = t;

		if (left_node_next == right_node) {
			if ((right_node != l->tail) && is_marked_reference((intptr_t)right_node->next)) {
				continue;
			} else {
				return right_node;
			}
		} else {
			if(cas(&(*left_node)->next, left_node_next, right_node)) {
				if ((right_node != l->tail) && is_marked_reference((intptr_t)right_node->next)) {
					continue;
				} else {
					return right_node;
				}
			}
		}
	}
}

