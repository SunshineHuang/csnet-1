#include "lflist.h"

#include <stdio.h>
#include <stdlib.h>

#define R(hp) ((hp->H) / 2)  /* threshold R */
#define L 128

struct hp_list {
	int head;
	int tail;
	void* array[L];
};

struct hp {
	int H;
	struct hp_record* head_hp_record;
};

static struct hp_list* list_new()
{
	struct hp_list* l = malloc(sizeof(*l));
	l->head = l->tail = 0;
	for (int i = 0; i < L; i++) {
		l->array[i] = NULL;
	}
	return l;
}

static void list_free(struct hp_list* l)
{
	free(l);
}

static int list_insert(struct hp_list* l, void* node)
{
	int pos = (l->head + 1) & (L - 1);  /* Prevent array overflow */
	if (pos != l->tail) {               /* Prevent overwrite tail */
		l->array[l->head] = node;
		l->head = pos;
		return pos;
	} else {
		return -1;
	}
}

static void* list_pop(struct hp_list* l)
{
	void* tmpval = l->array[l->tail];
	l->array[l->tail] = NULL;

	if (l->head != l->tail) {
		l->tail = (l->tail + 1) & (L - 1);  /* Prevent array overflow */
	}
	return tmpval;
}

static int list_lookup(struct hp_list* l, void* node)
{
	for (int i = 0; i < L; i++) {
		if (l->array[i] == node) {
			return 1;
		}
	}
	return 0;
}

static int list_popall(struct hp_list* l, void** output)
{
	int length = (l->tail <= l->head)
		     ? (l->head - l->tail)
		     : (L - l->tail + l->head);
	for (int i = l->tail; i < l->head; /*(i++) & (L-1)*/ i++) {
		output[i] = l->array[i];
	}
	l->head = 0;
	l->tail = 0;
	return length;
}

static void scan(struct hp* hp, struct hp_record* private_record)
{
	/* Stage 1: Scan HP lists and insert non-null values in plist */
	struct hp_list* plist = list_new();
	struct hp_record* head = hp->head_hp_record;
	while (head) {
		for (int i = 0; i < K; i++) {
			if (head->hp[i]) {
				list_insert(plist, head->hp[i]);
			}
		}
		head = head->hp_next;
	}

	/* Stage 2: Search plist */
	void** tmplist = (void**)malloc(L * sizeof(void*));
	int length = list_popall(private_record->rlist, tmplist);
	private_record->rcount = 0;
	for (int i = 0; i < length; i++) {
		if (list_lookup(plist, tmplist[i])) {
			list_insert(private_record->rlist, tmplist[i]);
			private_record->rcount++;
		} else {
			free(tmplist[i]);
		}
	}
	list_free(plist);
	free(tmplist);
}

static void help_scan(struct hp* hp, struct hp_record* private_record)
{
	struct hp_record* head_record = hp->head_hp_record;
	for (; head_record; head_record = head_record->hp_next) {
		if (head_record->active || !__sync_bool_compare_and_swap(&head_record->active, 0, 1)) {
			continue;
		}

		while (head_record->rcount > 0) {
			void* node = list_pop(head_record->rlist);
			head_record->rcount--;
			list_insert(private_record->rlist, node);
			private_record->rcount++;
			if (private_record->rcount >= R(hp)) {
				scan(hp, private_record);
			}
		}
		head_record->active = 0;
  	}
}

static struct hp_record* hp_record_new()
{
	struct hp_record* record = malloc(sizeof(*record));
	record->active = 1;
	record->rcount = 0;
	record->rlist = list_new();
	record->hp_next = NULL;
	for (int i = 0; i < K; i++) {
		record->hp[i] = NULL;
	}
	record->prev = NULL;
	record->curr = NULL;
	record->next = NULL;
	return record;
}

static void hp_record_free(struct hp_record* record)
{
	list_free(record->rlist);
	free(record);
}

/* Each thread call this to allocate a private hp record */
struct hp_record* allocate_thread_private_hp_record(struct hp* hp)
{
	/* First try to reuse a retired HP record */
	for (struct hp_record* record = hp->head_hp_record; record; record = record->hp_next) {
		if (record->active || !__sync_bool_compare_and_swap(&record->active, 0, 1)) {
			continue;
		}

		return record;
	}

	/* No HP records avaliable for resue.
	   Increment H, then allocate a new HP and push it */
	__sync_fetch_and_add(&hp->H, K);
	struct hp_record* new_record = hp_record_new();
	struct hp_record* old_record;
	do {
		old_record = hp->head_hp_record;
		new_record->hp_next = old_record;
	} while (!__sync_bool_compare_and_swap(&hp->head_hp_record, old_record, new_record));

	return new_record;
}

static struct hp* hp_new()
{
	struct hp* hp = malloc(sizeof(*hp));
	hp->H = 0;
	hp->head_hp_record = NULL;
	return hp;
}

static void hp_free(struct hp* hp)
{
	struct hp_record* head = hp->head_hp_record;
	while (head) {
		struct hp_record* tmp = head->hp_next;
		hp_record_free(head);
		head = tmp;
	}
	free(hp);
}

void retire_node(struct hp* hp, struct hp_record* private_record, void* node)
{
	for (int i = 0; i < K; i++) {
		if (private_record->hp[i] == node) {
			list_insert(private_record->rlist, node);
			private_record->rcount++;
			private_record->hp[i] = NULL;
			if (private_record->rcount >= R(hp)) {
				scan(hp, private_record);
				help_scan(hp, private_record);
			}
			break;
		}
	}
}

static struct lflnode* new_lnode(int data)
{
	struct lflnode* node = malloc(sizeof(*node));
	node->data = data;
	node->next = NULL;
	return node;
}

static int lflist_find(struct lflist* l, struct hp_record* private_record, int data)
{
try_again:
	private_record->prev = &l->head;
	private_record->curr = *(private_record->prev);
	while (private_record->curr) {
		private_record->hp[0] = private_record->curr;
		if (*(private_record->prev) != private_record->curr) {
			goto try_again;
		}
		private_record->next = private_record->curr->next;
		if ((uintptr_t)private_record->next & 1) {
			if (!__sync_bool_compare_and_swap(private_record->prev, private_record->curr, private_record->next - 1)) {
				goto try_again;
			}
			private_record->curr = private_record->next - 1;
		} else {
			int cdata = private_record->curr->data;
			if (*private_record->prev != private_record->curr) {
				goto try_again;
			}
			if (cdata >= data) {
				return cdata == data;
			}
			private_record->prev = &private_record->curr->next;
			void* tmp = private_record->hp[0];
			private_record->hp[0] = private_record->hp[1];
			private_record->hp[1] = tmp;
			private_record->curr = private_record->next;
		}
	}
	return 0;
}

struct lflist* lflist_new()
{
	struct lflist* l = malloc(sizeof(*l));
	l->head = new_lnode(-1);
	l->hp = hp_new();
	return l;
}

void lflist_free(struct lflist* l)
{
	struct lflnode* head = l->head;
	while (head) {
		struct lflnode* tmp = head->next;
		free(head);
		head = tmp;
	}
	hp_free(l->hp);
	free(l);
}

int lflist_add(struct lflist* l, struct hp_record* private_record, int data)
{
	struct lflnode* node = new_lnode(data);
	while (1) {
		if (lflist_find(l, private_record, data)) {
			return -1;
		}
		private_record->curr = l->head;
		node->next = private_record->curr;
		if (__sync_bool_compare_and_swap(&l->head, private_record->curr, node)) {
			return 0;
		}
	}
}

int lflist_del(struct lflist* l, struct hp_record* private_record, int data)
{
	while (1) {
		if (!lflist_find(l, private_record, data)) {
			return -1;
		}
		if (!__sync_bool_compare_and_swap(&private_record->curr->next, private_record->next, private_record->next + 1)) {
			continue;
		}
		if (__sync_bool_compare_and_swap(private_record->prev, private_record->curr, private_record->next)) {
			retire_node(l->hp, private_record, private_record->curr);
		} else {
			lflist_find(l, private_record, data);
		}
		return 0;
	}
}

int lflist_search(struct lflist* l, struct hp_record* private_record, int data)
{
	return lflist_find(l, private_record, data);
}

