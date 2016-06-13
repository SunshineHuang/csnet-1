#pragma once

#include <stdint.h>

typedef struct cs_lfstack cs_lfstack_t;
typedef struct cs_lfstack_node cs_lfstack_node_t;
typedef struct cs_tagged_pointer cs_tagged_pointer_t;

struct cs_lfstack_node {
	int data;
	cs_lfstack_node_t* next;
};

struct cs_tagged_pointer {
	cs_lfstack_node_t* node;
	uint64_t tag;
} __attribute__((aligned(16)));

struct cs_lfstack {
	cs_tagged_pointer_t* top;
};

cs_lfstack_node_t* cs_lfstack_node_new(int data);
void cs_lfstack_node_free(cs_lfstack_node_t*);

cs_lfstack_t* cs_lfstack_new();
void cs_lfstack_free(cs_lfstack_t*);

void cs_lfstack_push(cs_lfstack_t*, cs_lfstack_node_t* node);
cs_lfstack_node_t* cs_lfstack_pop(cs_lfstack_t*);

