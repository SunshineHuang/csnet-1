#include "cs-lfqueue.h"

#include <stdio.h>
#include <stdlib.h>

#define cas __sync_bool_compare_and_swap
#define R(hp) ((hp->H) / 2)  /* threshold R */
#define L 128

struct hp_list {
	int head;
	int tail;
	void* array[L];
};

struct cs_hp {
	int H;
	cs_hp_record_t* head_hp_record;
};

static struct hp_list* list_new();
static void list_free(struct hp_list* l);
static int list_insert(struct hp_list* l, void* node);
static void* list_pop(struct hp_list* l);
static int list_lookup(struct hp_list* l, void* node);
static int list_popall(struct hp_list* l, void** output);
static void scan(cs_hp_t* hp, cs_hp_record_t* private_record);
static void help_scan(cs_hp_t* hp, cs_hp_record_t* private_record);
static cs_hp_record_t* hp_record_new();
static void hp_record_free(cs_hp_record_t* record);
static cs_hp_t* hp_new();
static void hp_free(cs_hp_t* hp);
static void retire_node(cs_hp_t* hp, cs_hp_record_t* private_record, void* node);
static cs_lfqnode_t* new_qnode(void* data);

/* Each thread call this to allocate a private hp record */
cs_hp_record_t*
allocate_thread_private_hp_record(cs_hp_t* hp) {
	/* First try to reuse a retired HP record */
	for (cs_hp_record_t* record = hp->head_hp_record; record; record = record->next) {
		if (record->active || !cas(&record->active, 0, 1)) {
			continue;
		}
		return record;
	}

	/* No HP records avaliable for resue.
	   Increment H, then allocate a new HP and push it */
	__sync_fetch_and_add(&hp->H, K);
	cs_hp_record_t* new_record = hp_record_new();
	cs_hp_record_t* old_record;

	do {
		old_record = hp->head_hp_record;
		new_record->next = old_record;
	} while (!cas(&hp->head_hp_record, old_record, new_record));

	return new_record;
}

cs_lfqueue_t*
cs_lfqueue_new() {
	cs_lfqueue_t* q = malloc(sizeof(*q));
	q->head = q->tail = new_qnode(NULL);
	q->hp = hp_new();
	return q;
}

void
cs_lfqueue_free(cs_lfqueue_t* q) {
	cs_lfqnode_t* head = q->head;
	while (head) {
		cs_lfqnode_t* tmp = head->next;
		free(head);
		head = tmp;
	}
	hp_free(q->hp);
	free(q);
}

int
cs_lfqueue_enq(cs_lfqueue_t* q, cs_hp_record_t* private_record, void* data) {
	cs_lfqnode_t* node = new_qnode(data);
	cs_lfqnode_t* tail;
	cs_lfqnode_t* next;

	while (1) {
		tail = q->tail;
		private_record->hp[0] = tail;

		if (q->tail != tail) {
			continue;
		}

		next = tail->next;
		if (q->tail != tail) {
			continue;
		}

		if (next) {
			cas(&q->tail, tail, next);
			continue;
		}

		if (cas(&tail->next, NULL, node)) {
			break;
		}
	}

	cas(&q->tail, tail, node);
	private_record->hp[0] = NULL;
	return 1;
}

int
cs_lfqueue_deq(cs_lfqueue_t* q, cs_hp_record_t* private_record, void** data) {
	cs_lfqnode_t* head;
	cs_lfqnode_t* tail;
	cs_lfqnode_t* next;

	while (1) {
		head = q->head;
		private_record->hp[0] = head;

		if (q->head != head) {
			continue;
		}

		tail = q->tail;
		next = head->next;
		private_record->hp[1] = next;

		if (q->head != head) {
			continue;
		}

		if (!next) {
			return -1;
		}

		if (head == tail) {
			cas(&q->tail, tail, next);
			continue;
		}

		*data = next->data;
		if (cas(&q->head, head, next)) {
			break;
		}
	}

	retire_node(q->hp, private_record, (void*)head);
	private_record->hp[0] = NULL;
	private_record->hp[1] = NULL;

	return 1;
}

static struct
hp_list* list_new() {
	struct hp_list* l = malloc(sizeof(*l));
	l->head = l->tail = 0;
	for (int i = 0; i < L; i++) {
		l->array[i] = NULL;
	}
	return l;
}

static void
list_free(struct hp_list* l) {
	free(l);
}

static int
list_insert(struct hp_list* l, void* node) {
	int pos = (l->head + 1) & (L - 1);  /* Prevent array overflow */
	if (pos != l->tail) {               /* Prevent overwrite tail */
		l->array[l->head] = node;
		l->head = pos;
		return pos;
	} else {
		return -1;
	}
}

static void*
list_pop(struct hp_list* l) {
	void* tmpval = l->array[l->tail];
	l->array[l->tail] = NULL;
	if (l->head != l->tail) {
		l->tail = (l->tail + 1) & (L - 1);  /* Prevent array overflow */
	}
	return tmpval;
}

static int
list_lookup(struct hp_list* l, void* node) {
	for (int i = 0; i < L; i++) {
		if (l->array[i] == node) {
			return 1;
		}
	}
	return 0;
}

static int
list_popall(struct hp_list* l, void** output) {
	int length = (l->tail <= l->head)
		     ? (l->head - l->tail)
		     : (L - l->tail + l->head);
	for (int i = l->tail; i < l->head; i++) {
		output[i] = l->array[i];
	}
	l->head = 0;
	l->tail = 0;
	return length;
}

static void
scan(cs_hp_t* hp, cs_hp_record_t* private_record) {
	/* Stage 1: Scan HP lists and insert non-null values in plist */
	struct hp_list* plist = list_new();
	cs_hp_record_t* head = hp->head_hp_record;

	while (head) {
		for (int i = 0; i < K; i++) {
			if (head->hp[i]) {
				list_insert(plist, head->hp[i]);
			}
		}
		head = head->next;
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

static void
help_scan(cs_hp_t* hp, cs_hp_record_t* private_record) {
	cs_hp_record_t* head_record = hp->head_hp_record;
	for (; head_record; head_record = head_record->next) {
		if (head_record->active || !cas(&head_record->active, 0, 1)) {
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

static cs_hp_record_t*
hp_record_new() {
	cs_hp_record_t* record = malloc(sizeof(*record));
	record->active = 1;
	record->rcount = 0;
	record->rlist = list_new();
	record->next = NULL;

	for (int i = 0; i < K; i++) {
		record->hp[i] = NULL;
	}

	return record;
}

static void
hp_record_free(cs_hp_record_t* record) {
	list_free(record->rlist);
	free(record);
}

static cs_hp_t*
hp_new() {
	cs_hp_t* hp = malloc(sizeof(*hp));
	hp->H = 0;
	hp->head_hp_record = NULL;
	return hp;
}

static void
hp_free(cs_hp_t* hp) {
	cs_hp_record_t* head = hp->head_hp_record;
	while (head) {
		cs_hp_record_t* tmp = head->next;
		hp_record_free(head);
		head = tmp;
	}
	free(hp);
}

static void
retire_node(cs_hp_t* hp, cs_hp_record_t* private_record, void* node) {
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

static cs_lfqnode_t*
new_qnode(void* data) {
	cs_lfqnode_t* node = malloc(sizeof(*node));
	node->data = data;
	node->next = NULL;
	return node;
}

