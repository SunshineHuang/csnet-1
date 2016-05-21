#include "cs-queue.h"

#include <stdlib.h>

cs_queue_node_t*
cs_queue_node_new(int data) {
	cs_queue_node_t* node = calloc(1, sizeof(*node));
	node->data = data;
	node->next = NULL;
	return node;
}

void
cs_queue_node_free(cs_queue_node_t* node) {
	free(node);
}

cs_queue_t*
cs_queue_new() {
	cs_queue_t* q = calloc(1, sizeof(*q));
	q->head = NULL;
	q->tail = NULL;
	return q;
}

void
cs_queue_free(cs_queue_t* q) {
	cs_queue_node_t* head = q->head;
	while (head) {
		cs_queue_node_t* tmp = head->next;
		free(head);
		head = tmp;
	}
	free(q);
}

void
cs_queue_enq(cs_queue_t* q, cs_queue_node_t* node) {
	if (!q->head) {
		q->head = node;
	}
	if (q->tail) {
		q->tail->next = node;
	}
	q->tail = node;
}

cs_queue_node_t*
cs_queue_deq(cs_queue_t* q) {
	cs_queue_node_t* result;
	if (!q->head) {
		return NULL;
	}
	result = q->head;
	q->head = result->next;
	if (!q->head) {
		q->tail = NULL;
	}
	return result;
}

