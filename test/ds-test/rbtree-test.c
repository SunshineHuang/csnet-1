#include "cs-rbtree.h"

#include <stdio.h>
#include <stdlib.h>

void test1() {
	cs_rbtree_t* t = cs_rbtree_new();
	int arr[10] = {1, 8, 4, 3, 5, 6, 7, 2, 9, 10};
	for (int i = 0; i < 10; i++) {
		cs_rbnode_t* node = cs_rbnode_new(arr[i], arr[i]);
		cs_rbtree_insert(t, node);
	}

	printf("root->key: %d\n", t->root->key);

	cs_rbtree_inorder_walk(t);

	for (int i = 0; i < 10; i++) {
		cs_rbnode_t* node = cs_rbtree_search(t, arr[i]);
		if (node) {
			printf("found: node->key: %d\n", node->key);
			cs_rbtree_delete(t, node);
			free(node);
		} else {
			printf("not found\n");
		}
	}

	cs_rbtree_inorder_walk(t);
	cs_rbtree_free(t);
}

int main()
{
	test1();
	return 0;
}

