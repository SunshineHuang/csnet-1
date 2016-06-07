#ifndef cs_priority_queue_h
#define cs_priority_queue_h

typedef enum {
	CS_PQ_LOWEST_PRIORITY,  /* lowest key have highest priorit */
	CS_PQ_HIGHEST_PRIORITY  /* highest key have highest priorit */
} cs_pqueue_mode_t;

typedef struct cs_pqnode cs_pqnode_t;
typedef struct cs_pqueue cs_pqueue_t;

struct cs_pqnode {
	int priority;
	char* value;
	cs_pqnode_t* parent;
	cs_pqnode_t* left;
	cs_pqnode_t* right;
};

struct cs_pqueue {
	int mode;
	cs_pqnode_t* root;
	cs_pqnode_t* lowest;
	cs_pqnode_t* highest;
};

cs_pqueue_t* cs_pqueue_new(cs_pqueue_mode_t mode);
void cs_pqueue_free(cs_pqueue_t*);
int cs_pqueue_push(cs_pqueue_t*, int priority, char* value);

/* Pop the highest priority node */
cs_pqnode_t* cs_pqueue_pop(cs_pqueue_t*);
void cs_pqueue_delete(cs_pqueue_t*, cs_pqnode_t* node);
void cs_pqueue_inorder_walk(cs_pqueue_t*);

#endif  /* cs_priority_queue_h */

