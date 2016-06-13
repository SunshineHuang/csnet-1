#pragma once

typedef struct cs_queue_node cs_queue_node_t;
typedef struct cs_queue cs_queue_t;

struct cs_queue_node {
	int data;
	cs_queue_node_t* next;
};

struct cs_queue {
	cs_queue_node_t* head;
	cs_queue_node_t* tail;
};

cs_queue_node_t* cs_queue_node_new(int data);
void cs_queue_node_free(cs_queue_node_t* node);

cs_queue_t* cs_queue_new();
void cs_queue_free(cs_queue_t*);
void cs_queue_enq(cs_queue_t*, cs_queue_node_t* node);
cs_queue_node_t* cs_queue_deq(cs_queue_t*);

