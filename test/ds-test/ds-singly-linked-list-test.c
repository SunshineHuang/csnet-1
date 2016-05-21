#include "cs-singly-linked-list.h"

#include <stdio.h>
#include <stdlib.h>

void t1()
{
	printf("t1() begin ...\n");
	cs_slist_t* l = cs_slist_new();
	for (int i = 0; i < 10; i++) {
		cs_sl_node_t* node = cs_sl_node_new(i);
		cs_slist_insert(l, node);
	}


	for (int i = 0; i < 10; i++) {
		cs_sl_node_t* node = cs_slist_search(l, i);
		printf("i: %d\n", node->data);
		cs_slist_remove(l, node);
	}
	cs_slist_free(l);
	printf("t1() end ...\n");
}

void t2()
{
	printf("t2() begin ...\n");
	cs_slist_t* l = cs_slist_new();
	for (int i = 0; i < 10; i++) {
		cs_sl_node_t* node = cs_sl_node_new(i);
		cs_slist_insert(l, node);
	}


	for (int i = 9; i > -1; i--) {
		cs_sl_node_t* node = cs_slist_search(l, i);
		printf("i: %d\n", node->data);
		cs_slist_remove(l, node);
	}
	cs_slist_free(l);
	printf("t2() end ...\n");
}

void t3()
{
	printf("t3() begin ...\n");
	cs_slist_t* l = cs_slist_new();
	for (int i = 0; i < 10; i++) {
		cs_sl_node_t* node = cs_sl_node_new(i);
		cs_slist_insert(l, node);
	}
	cs_slist_print(l);
	cs_slist_reverse(l);
	cs_slist_print(l);
	cs_slist_free(l);
	printf("t3() end ...\n");
}

int main()
{
	t1();
	t2();
	t3();
	return 0;
}
