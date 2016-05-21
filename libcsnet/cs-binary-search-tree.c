#include "cs-binary-search-tree.h"

#include <stdio.h>
#include <stdlib.h>

static cs_bsnode_t* _minimum_node(cs_bsnode_t* node);
static cs_bsnode_t* _successor(cs_bsnode_t* node);
static void _transplant(cs_bstree_t* t, cs_bsnode_t* u, cs_bsnode_t* v);
static void _inorder_walk(cs_bsnode_t* node);
static void _bstree_free(cs_bsnode_t* node);
static cs_bsnode_t* _invert(cs_bsnode_t* node);

cs_bstree_t*
cs_bstree_new() {
	cs_bstree_t* t = calloc(1, sizeof(*t));
	t->root = NULL;
	return t;
}

void
cs_bstree_free(cs_bstree_t* t) {
	_bstree_free(t->root);
	free(t);
}

int
cs_bstree_insert(cs_bstree_t* t, int key, int value) {
	cs_bsnode_t* new_node = calloc(1, sizeof(*new_node));
	new_node->key = key;
	new_node->value = value;
	new_node->parent = NULL;
	new_node->left = NULL;
	new_node->right = NULL;

	cs_bsnode_t* tmp = NULL;
	cs_bsnode_t* root = t->root;

	while (root) {
		tmp = root;
		if (key < root->key) {
			root = root->left;
		} else {
			root = root->right;
		}
	}

	new_node->parent = tmp;
	if (!tmp) {
		/* empty tree */
		t->root = new_node;
	} else if (key < tmp->key) {
		tmp->left = new_node;
	} else {
		tmp->right = new_node;
	}

	return 0;
}

cs_bsnode_t*
cs_bstree_search(cs_bstree_t* t, int key) {
	cs_bsnode_t* node = t->root;
	while (node && (key != node->key)) {
		if (key < node->key) {
			node = node->left;
		} else {
			node = node->right;
		}
	}
	return node;
}

void
cs_bstree_delete(cs_bstree_t* t, cs_bsnode_t* node) {
	if (!node->left) {
		_transplant(t, node, node->right);
	} else if (!node->right) {
		_transplant(t, node, node->left);
	} else {
		cs_bsnode_t* tmp = _minimum_node(node->right);

		if (tmp->parent != node) {
			_transplant(t, tmp, tmp->right);
			tmp->right = node->right;
			tmp->right->parent = tmp;
		}

		_transplant(t, node, tmp);
		tmp->left = node->left;
		tmp->left->parent = tmp;
	}
	free(node);
}

void
cs_bstree_inorder_walk(cs_bstree_t* t) {
	_inorder_walk(t->root);
}

cs_bsnode_t*
cs_bstree_minimum(cs_bstree_t* t) {
	cs_bsnode_t* node = t->root;
	while (node && node->left) {
		node = node->left;
	}
	return node;
}

cs_bsnode_t*
cs_bstree_maximum(cs_bstree_t* t) {
	cs_bsnode_t* node = t->root;
	while (node && node->right) {
		node = node->right;
	}
	return node;

}

void
cs_bstree_invert(cs_bstree_t* t) {
	_invert(t->root);
}

static inline cs_bsnode_t*
_minimum_node(cs_bsnode_t* node) {
	cs_bsnode_t* min_node = node;
	while (node && min_node->left) {
		min_node = min_node->left;
	}
	return min_node;
}

static inline cs_bsnode_t*
_successor(cs_bsnode_t* node) {
	if (node->right) {
		/* If node's right node is nonempty,
		   the minimum node of right tree is its successor */
		return _minimum_node(node->right);
	}

	cs_bsnode_t* parent = node->parent;
	while (parent && (node == parent->right)) {
		node = parent;
		parent = parent->parent;
	}
	return parent;
}

static inline void
_transplant(cs_bstree_t* t, cs_bsnode_t* u, cs_bsnode_t* v) {
	if (!u->parent) {
		t->root = v;
	} else if (u == u->parent->left) {
		u->parent->left = v;
	} else {
		u->parent->right = v;
	}

	if (v) {
		v->parent = u->parent;
	}
}

static inline void
_inorder_walk(cs_bsnode_t* node) {
	if (node) {
		_inorder_walk(node->left);
		printf("node->key: %d\n", node->key);
		_inorder_walk(node->right);
	}
}

static inline void
_bstree_free(cs_bsnode_t* node) {
	if (node) {
		_bstree_free(node->left);
		_bstree_free(node->right);
		free(node);
	}
}

static inline cs_bsnode_t*
_invert(cs_bsnode_t* node) {
	if (!node) {
		return NULL;
	}
	cs_bsnode_t* tmp = node->left;
	node->left = _invert(node->right);
	node->right = _invert(tmp);
	return node;
}

