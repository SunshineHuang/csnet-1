#include "cs-stack.h"

#include <stdlib.h>

cs_stack_node_t*
cs_stack_node_new(int data) {
	cs_stack_node_t* node = calloc(1, sizeof(*node));
	node->data = data;
	node->next = NULL;
	return node;
}

void
cs_stack_node_free(cs_stack_node_t* node) {
	free(node);
}

cs_stack_t*
cs_stack_new() {
	cs_stack_t* s = calloc(1, sizeof(*s));
	s->top = NULL;
	return s;
}

void
cs_stack_free(cs_stack_t* s) {
	cs_stack_node_t* x = s->top;
	while (x) {
		cs_stack_node_t* tmp = x->next;
		cs_stack_node_free(x);
		x = tmp;
	}
	free(s);
}

cs_stack_node_t*
cs_stack_pop(cs_stack_t* s) {
	cs_stack_node_t* node = s->top;
	if (s->top) {
		s->top = s->top->next;
	}
	return node;
}

void
cs_stack_push(cs_stack_t* s, cs_stack_node_t* x) {
	if (!s->top) {
		s->top = x;
	} else {
		x->next = s->top;
		s->top = x;
	}
}

