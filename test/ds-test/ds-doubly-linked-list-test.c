#include "cs-doubly-linked-list.h"

#include <stdio.h>
#include <stdlib.h>

void test1()
{
	cs_dlist_t* l = cs_dlist_new();
	for (int i = 0; i < 10; i++) {
		cs_dl_node_t* node = cs_dl_node_new(i);
		cs_dlist_insert(l, node);
	}

	for (int i = 0; i < 10; i++) {
		cs_dl_node_t* node = cs_dlist_search(l, i);
		printf("i: %d\n", node->data);
		cs_dlist_remove(l, node);
	}
	cs_dlist_free(l);
}

void test2()
{
	cs_dlist_t* l = cs_dlist_new();
	for (int i = 0; i < 10; i++) {
		cs_dl_node_t* node = cs_dl_node_new(i);
		cs_dlist_insert(l, node);
	}

	for (int i = 9; i > -1; i--) {
		cs_dl_node_t* node = cs_dlist_search(l, i);
		printf("i: %d\n", node->data);
		cs_dlist_remove(l, node);
	}
	cs_dlist_free(l);
}

int main()
{
	test1();
	test2();

	return 0;
}
