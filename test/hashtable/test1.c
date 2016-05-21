#include "int_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
	if (argc != 2) {
		printf("%s count\n", argv[0]);
		return -1;
	}

	int i;
	int n = atoi(argv[1]);
	int_hash_t* int_hashtbl = int_hash_create(701);

	for (i = 0; i < n; i++) {
		char* p = malloc(10);
		memcpy(p, "helloworld", 10);
		int ret = int_hash_insert(int_hashtbl, i, p);
		printf("ret: %d\n", ret);
	}

	printf("-------\n");
	for (i = 0; i < n; i++) {
		char* p = malloc(10);
		memcpy(p, "helloworld", 10);
		int ret = int_hash_insert(int_hashtbl, i, p);
		printf("ret: %d\n", ret);
		free(p);
	}

	printf("-------\n");
	for (i = 0; i < n; i++) {
		int_hash_node_t* v = int_hash_lookup(int_hashtbl, i);
		printf("v: %s\n", v->value);
	}

	printf("-------\n");
	for (i = 0; i < n + 10; i++) {
		int ret = int_hash_has_key(int_hashtbl, i);
		printf("ret: %d\n", ret);
	}

	printf("-------\n");
	for (i = 0; i < n; i++) {
		int ret = int_hash_remove(int_hashtbl, i);
		printf("ret: %d\n", ret);
	}

	int_hash_destroy(int_hashtbl);
	return 0;
}
