#pragma once

typedef struct cs_bsnode cs_bsnode_t;
typedef struct cs_bstree cs_bstree_t;

struct cs_bsnode {
	int key;
	int value;
	cs_bsnode_t* parent;
	cs_bsnode_t* left;
	cs_bsnode_t* right;
};

struct cs_bstree {
	cs_bsnode_t* root;
};

cs_bstree_t* cs_bstree_new();
void cs_bstree_free(cs_bstree_t*);
int cs_bstree_insert(cs_bstree_t*, int key, int value);
cs_bsnode_t* cs_bstree_search(cs_bstree_t*, int key);
void cs_bstree_delete(cs_bstree_t*, cs_bsnode_t* node);
void cs_bstree_inorder_walk(cs_bstree_t*);
cs_bsnode_t* cs_bstree_minimum(cs_bstree_t*);
cs_bsnode_t* cs_bstree_maximum(cs_bstree_t*);
void cs_bstree_invert(cs_bstree_t*);

