#include "cs-binary-search-tree.h"

#include <stdio.h>
#include <stdlib.h>


int test1() {
	cs_bstree_t* t = cs_bstree_new();
	int arr[10] = {8, 4, 3, 5, 6, 7, 2, 9, 10, 11};
	for (int i = 0; i < 10; i++) {
		cs_bstree_insert(t, arr[i], arr[i]);
	}

	printf("root->key: %d\n", t->root->key);
	cs_bstree_inorder_walk(t);
	for (int i = 0; i < 10; i++) {
		cs_bsnode_t* node = cs_bstree_search(t, arr[i]);
		printf("node->key: %d\n", node->key);
		cs_bstree_delete(t, node);
	}

	cs_bsnode_t* min = cs_bstree_minimum(t);
	cs_bsnode_t* max = cs_bstree_maximum(t);
	if (min) {
		printf("min->key: %d\n", min->key);
		cs_bstree_delete(t, min);
	}
	if (max) {
		printf("max->key: %d\n", max->key);
	}

	cs_bstree_inorder_walk(t);
	cs_bstree_free(t);
	return 0;
}

void test2() {
	cs_bstree_t* t = cs_bstree_new();
	int arr[10] = {8, 4, 3, 5, 6, 7, 2, 9, 10, 11};
	for (int i = 0; i < 10; i++) {
		cs_bstree_insert(t, arr[i], arr[i]);
	}

	printf("befort invert\n");
	cs_bstree_inorder_walk(t);
	cs_bstree_invert(t);
	printf("after invert\n");
	cs_bstree_inorder_walk(t);
	cs_bstree_free(t);
}

int main()
{
	test1();
	test2();
	return 0;
}

