#include "cs-lfstack.h"

#include <stdio.h>
#include <stdlib.h>

static unsigned char
dwcas(cs_tagged_pointer_t* ptr, cs_lfstack_node_t* old, uint64_t oldtag, cs_lfstack_node_t* new, uint64_t newtag) {
	unsigned char cas_result;
	__asm__ __volatile__(
		"lock;"            /* make cmpxchg16b atomic */
		"cmpxchg16b %0;"   /* cmpxchg16b set ZF on success */
		"setz %3;"         /* if ZF set, set cas_result to 1 */
		/* output */
		: "+m" (*ptr), "+a" (old), "+d" (oldtag), "=q" (cas_result)
		/* input */
		: "b" (new), "c" (newtag)
		/* clobbered */
		: "cc", "memory"
	);
	return cas_result;
}

inline cs_lfstack_node_t*
cs_lfstack_node_new(int data) {
	cs_lfstack_node_t* node = calloc(1, sizeof(*node));
	node->next = NULL;
	node->data = data;
	return node;
}

inline void
cs_lfstack_node_free(cs_lfstack_node_t* node) {
	free(node);
}

cs_lfstack_t*
cs_lfstack_new() {
	cs_lfstack_t* s = calloc(1, sizeof(*s));
	s->top = calloc(1, sizeof(*s->top));
	s->top->node = NULL;
	s->top->tag = 0;
	return s;
}

void
cs_lfstack_free(cs_lfstack_t* s) {
	free(s->top);
	free(s);
}

void
cs_lfstack_push(cs_lfstack_t* s, cs_lfstack_node_t* node) {
	while (1) {
		cs_lfstack_node_t* oldtop = s->top->node;
		uint64_t oldtag = s->top->tag;
		node->next = oldtop;

		if (dwcas(s->top, oldtop, oldtag, node, oldtag + 1)) {
			return;
		}
	}
}

cs_lfstack_node_t*
cs_lfstack_pop(cs_lfstack_t* s) {
	cs_lfstack_node_t* oldtop;
	while (1) {
		oldtop = s->top->node;
		uint64_t oldtag = s->top->tag;

		if (!oldtop) {
			return NULL;
		}

		if (s->top->node != oldtop) {
			continue;
		}

		if (dwcas(s->top, oldtop, oldtag, oldtop->next, oldtag + 1)) {
			return oldtop;
		}
	}
}

