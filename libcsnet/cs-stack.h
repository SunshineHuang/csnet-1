#pragma once

typedef struct cs_stack_node cs_stack_node_t;
typedef struct cs_stack cs_stack_t;

struct cs_stack_node {
	int data;
	cs_stack_node_t* next;
};

struct cs_stack {
	cs_stack_node_t* top;
};

cs_stack_node_t* cs_stack_node_new(int data);
void cs_stack_node_free(cs_stack_node_t* node);

cs_stack_t* cs_stack_new();
void cs_stack_free(cs_stack_t*);
cs_stack_node_t* cs_stack_pop(cs_stack_t*);
void cs_stack_push(cs_stack_t*, cs_stack_node_t* node);

