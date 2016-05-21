#include "cs-hashtable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test1() {
	cs_ht_t* table = cs_ht_new();

	char* key1 = malloc(5);
	strcpy(key1, "key1");
	char* value1 = malloc(7);
	strcpy(value1, "value1");
	cs_ht_insert(table, key1, 4, value1, 6);

	cs_htnode_t* node = cs_ht_search(table, key1, 4);
	if (node) {
		printf("node->key: %s, node->value: %s\n", node->key, node->value);
		cs_ht_delete(table, node);
		free(node->key);
		free(node->value);
		free(node);
	}

	cs_ht_free(table);
}

int main()
{
	test1();
	return 0;
}
