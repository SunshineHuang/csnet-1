#include "cs-stack.h"

#include <stdio.h>
#include <stdlib.h>

int main()
{
	cs_stack_t* s = cs_stack_new();
	for (int i = 0; i < 10; i++) {
		cs_stack_node_t* node = cs_stack_node_new(i);
		cs_stack_push(s, node);
	}
	for (int i = 0; i < 10; i++) {
		cs_stack_node_t* node = cs_stack_pop(s);
		printf("[%d]: %d\n", i, node->data);
		cs_stack_node_free(node);
	}
	cs_stack_free(s);
	return 0;
}
