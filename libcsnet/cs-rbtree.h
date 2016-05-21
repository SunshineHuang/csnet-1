#ifndef cs_rbtree_h
#define cs_rbtree_h

typedef struct cs_rbnode cs_rbnode_t;
typedef struct cs_rbtree cs_rbtree_t;

struct cs_rbnode {
	int color;
	int key;
	int value;
	cs_rbnode_t* parent;
	cs_rbnode_t* left;
	cs_rbnode_t* right;
};

struct cs_rbtree {
	cs_rbnode_t* root;
	cs_rbnode_t* sentinel;
};

cs_rbnode_t* cs_rbnode_new(int key, int value);

cs_rbtree_t* cs_rbtree_new();
void cs_rbtree_free(cs_rbtree_t*);
void cs_rbtree_insert(cs_rbtree_t*, cs_rbnode_t* z);
void cs_rbtree_delete(cs_rbtree_t*, cs_rbnode_t* z);
cs_rbnode_t* cs_rbtree_search(cs_rbtree_t*, int key);
void cs_rbtree_inorder_walk(cs_rbtree_t*);

#endif  /* cs_rbtree_h */

