#include "cs-singly-linked-list.h"

#include <stdio.h>
#include <stdlib.h>

cs_sl_node_t*
cs_sl_node_new(int data) {
	cs_sl_node_t* node = calloc(1, sizeof(*node));
	node->data = data;
	node->next = NULL;
	return node;
}

void
cs_sl_node_free(cs_sl_node_t* node) {
	free(node);
}

cs_slist_t*
cs_slist_new() {
	cs_slist_t* l = calloc(1, sizeof(*l));
	l->head = NULL;
	return l;
}

void
cs_slist_free(cs_slist_t* l) {
	cs_sl_node_t* head = l->head;
	while (head) {
		cs_sl_node_t* tmp = head->next;
		cs_sl_node_free(head);
		head = tmp;
	}
	free(l);
}

cs_sl_node_t*
cs_slist_search(cs_slist_t* l, int data) {
	cs_sl_node_t* head = l->head;
	while (head && head->data != data) {
		head = head->next;
	}
	return head;
}

void
cs_slist_insert(cs_slist_t* l, cs_sl_node_t* node) {
	node->next = l->head;
	l->head = node;
}

void
cs_slist_remove(cs_slist_t* l, cs_sl_node_t* node) {
	cs_sl_node_t* head = l->head;
	cs_sl_node_t* curr = head;
	while (head && head->data != node->data) {
		curr = head;
		head = head->next;
	}
	if (curr) {
		if (curr->data == node->data) {
			l->head = node->next;
		} else {
			curr->next = node->next;
		}

		cs_sl_node_free(node);
	}
}

void
cs_slist_reverse(cs_slist_t* l) {
	cs_sl_node_t* prev = NULL;
	cs_sl_node_t* curr = l->head;
	while (curr) {
		cs_sl_node_t* next = curr->next;
		curr->next = prev;
		prev = curr;
		curr = next;
	}
	l->head = prev;
}

/* debug */
void
cs_slist_print(const cs_slist_t* l) {
	cs_sl_node_t* node = l->head;
	while (node) {
		printf("%d\n", node->data);
		node = node->next;
	}
}

