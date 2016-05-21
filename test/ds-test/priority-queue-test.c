#include "cs-priority-queue.h"

#include <stdio.h>
#include <stdlib.h>


int test1() {
	cs_pqueue_t* t = cs_pqueue_new(CS_PQ_LOWEST_PRIORITY);
	int arr[10] = {1, 8, 4, 3, 5, 6, 7, 2, 9, 10};
	for (int i = 0; i < 10; i++) {
		cs_pqueue_push(t, arr[i], arr[i]);
	}

	for (int i = 0; i < 10; i++) {
		cs_pqnode_t* max = cs_pqueue_pop(t);
		if (max) {
			printf("max->key: %d\n", max->value);
			cs_pqueue_delete(t, max);
		}
	}

	cs_pqueue_free(t);
	return 0;
}

int main()
{
	test1();
	return 0;
}

