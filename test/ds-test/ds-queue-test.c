#include "cs-queue.h"

#include <stdio.h>
#include <stdlib.h>

int main()
{
	cs_queue_t* q = cs_queue_new();
	for (int i = 0; i < 10; i++) {
		cs_queue_node_t* node = cs_queue_node_new(i);
		cs_queue_enq(q, node);
	}
	for (int i = 0; i < 10; i++) {
		cs_queue_node_t* node = cs_queue_deq(q);
		printf("i: %d\n", node->data);
		cs_queue_node_free(node);
	}
	cs_queue_free(q);
	return 0;
}
