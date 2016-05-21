#include "slist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int match1(void* a, void* b)
{
	if (a == b) {
		return 0;
	}
	return -1;
}

int match2(void*a, void* b)
{
	return strcmp(a, b);
}

void test1()
{
	struct slist* sl = slist_create(match1);

	slist_destroy(sl);
}

void test2()
{
	struct slist* sl = slist_create(match1);
	int* a = malloc(sizeof(int));
	*a = 10;
	int* b = malloc(sizeof(int));
	*b = 9;
	slist_insert(sl, a);
	slist_insert(sl, b);

	struct slnode* t1 = slist_lookup(sl, a);
	struct slnode* t2 = slist_lookup(sl, b);
	printf("t1->value: %d\n", *(int*)t1->value);
	printf("t2->value: %d\n", *(int*)t2->value);

	slist_remove(sl, t1);
	slist_remove(sl, t2);

	int* c = malloc(sizeof(int));
	*c = 1123;

	struct slnode* t3 = slist_lookup(sl, c);
	if (t3 == NULL)
		printf("can not found\n");
	free(c);

	slist_destroy(sl);
}

void test3()
{
	struct slist* sl = slist_create(match2);
	char* p = malloc(10);
	char* p1 = malloc(10);
	strcpy(p, "hello");
	strcpy(p1, "world");
	slist_insert(sl, p);
	slist_insert(sl, p1);

	struct slnode* t1 = slist_lookup(sl, p);
	struct slnode* t2 = slist_lookup(sl, p1);
	printf("t1->value: %s\n", t1->value);
	printf("t2->value: %s\n", t2->value);
	slist_remove(sl, t1);
	slist_remove(sl, t2);

	slist_destroy(sl);
}

int main()
{
	test1();
	test2();
	test3();

	return 0;
}

