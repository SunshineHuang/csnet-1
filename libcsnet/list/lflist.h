#ifndef lflist_h
#define lflist_h

/*
 * Lock-free list based hazard pointers
 * Hazard Pointers: Safe Memory Reclamation for Lock-Free Objects
 * IEEE TRANSACTIONS ON PARALLEL AND DISTRIBUTED SYSTEMS, VOL. 15, NO. 6, JUNE 2004
 */

#define K 2  /* Each thread has K hazard pointers */

struct hp_list; 
struct hp;

struct lflnode {
	int data;
	struct lflnode* next;
};

struct hp_record {
	unsigned int active;
	unsigned int rcount;   /* Retired count */
	struct hp_list* rlist; /* Retired list 
	                          TODO: this can be implemented as a balanced search tree */
	struct hp_record* hp_next;
	void* hp[K];
	struct lflnode** prev;
	struct lflnode* curr;
	struct lflnode* next;
};

struct lflist {
	struct lflnode* head;
	struct hp* hp;
};

/* Each thread call this to allocate a private hp record */
struct hp_record* allocate_thread_private_hp_record(struct hp* hp);

struct lflist* lflist_new();
void lflist_free(struct lflist* l);
int lflist_add(struct lflist* l, struct hp_record* private_record, int data);
int lflist_del(struct lflist* l, struct hp_record* private_record, int data);
int lflist_search(struct lflist* l, struct hp_record* private_record, int data);

#endif  /* lflist_h */

