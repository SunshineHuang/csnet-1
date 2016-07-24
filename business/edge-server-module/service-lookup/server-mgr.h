#pragma once

#include "libcsnet.h"

typedef struct server_item {
	int type;
	int port;
	char* ip;
} server_item_t;

typedef struct server_mgr {
	int slots;
	csnet_spinlock_t lock;
	cs_slist_t** table;
} server_mgr_t;

server_item_t* server_item_new(int type, int port, char* ip);
void server_item_free(server_item_t* item);

server_mgr_t* server_mgr_new(int slots);
void server_mgr_free(server_mgr_t* mgr);
int server_mgr_insert(server_mgr_t* mgr, server_item_t* item);
cs_slist_t* server_mgr_items(server_mgr_t* mgr, int64_t type);

