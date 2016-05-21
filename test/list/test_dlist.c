#include "dlist.h"

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
	struct dlist* dl = dlist_create(match1);

	dlist_destroy(dl);
}

void test2()
{
	struct dlist* dl = dlist_create(match1);
	int* a = malloc(sizeof(int));
	*a = 10;
	int* b = malloc(sizeof(int));
	*b = 9;
	int* c = malloc(sizeof(int));
	*c = 1123;

	dlist_linsert(dl, a);
	dlist_linsert(dl, b);
	dlist_rinsert(dl, c);

	struct dlnode* t1 = dlist_lookup(dl, a);
	struct dlnode* t2 = dlist_lookup(dl, b);
	struct dlnode* t3 = dlist_lookup(dl, c);
	printf("t1->value: %d\n", *(int*)t1->value);
	printf("t2->value: %d\n", *(int*)t2->value);
	printf("t3->value: %d\n", *(int*)t3->value);

	dlist_remove(dl, t2);
	dlist_remove(dl, t3);
	dlist_remove(dl, t1);

	dlist_destroy(dl);
}

void test3()
{
	struct dlist* dl = dlist_create(match2);
	char* p = malloc(10);
	char* p1 = malloc(10);
	strcpy(p, "hello");
	strcpy(p1, "world");
	dlist_linsert(dl, p);
	dlist_linsert(dl, p1);

	struct dlnode* t1 = dlist_lookup(dl, p);
	struct dlnode* t2 = dlist_lookup(dl, p1);
	printf("t1->value: %s\n", t1->value);
	printf("t2->value: %s\n", t2->value);
	dlist_remove(dl, t1);
	dlist_remove(dl, t2);

	dlist_destroy(dl);
}

int main()
{
	test1();
	test2();
	test3();

	return 0;
}

