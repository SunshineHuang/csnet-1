#include "server-mgr.h"

#include <stdlib.h>

server_item_t*
server_item_new(int type, int port, char* ip) {
	server_item_t* item = calloc(1, sizeof(*item));
	item->type = type;
	item->port = port;
	item->ip = ip;
	return item;
}

server_item_t*
server_item_copy(server_item_t* item) {
	server_item_t* newitem = calloc(1, sizeof(*newitem));
	newitem->type = item->type;
	newitem->port = item->port;
	newitem->ip = item->ip;
	return newitem;
}

void
server_item_free(server_item_t* item) {
	free(item->ip);
	free(item);
}

server_mgr_t*
server_mgr_new(int slots) {
	server_mgr_t* mgr = calloc(1, sizeof(*mgr));
	mgr->slots = slots;
	csnet_spinlock_init(&mgr->lock);
	mgr->table = calloc(slots, sizeof(cs_slist_t*));
	for (int i = 0; i < slots; i++) {
		mgr->table[i] = cs_slist_new();
	}
	return mgr;
}

void
server_mgr_free(server_mgr_t* mgr) {
	for (int i = 0; i < mgr->slots; i++) {
		cs_slist_free(mgr->table[i]);
	}
	free(mgr->table);
	free(mgr);
}

int
server_mgr_insert(server_mgr_t* mgr, server_item_t* item) {
	csnet_spinlock_lock(&mgr->lock);
	int idx = item->type % mgr->slots;

	cs_sl_node_t* curr = mgr->table[idx]->head;
	while (curr) {
		server_item_t* tmp = curr->data;
		if (tmp->type == item->type
		    && tmp->port == item->port
		    && strcmp(tmp->ip, item->ip) == 0) {
			break;
		}
		curr = curr->next;
	}
	if (!curr) {
		cs_sl_node_t* snode = cs_sl_node_new(item);
		cs_slist_insert(mgr->table[idx], snode);
		csnet_spinlock_unlock(&mgr->lock);
		return 0;
	}
	csnet_spinlock_unlock(&mgr->lock);
	return -1;
}

cs_slist_t*
server_mgr_items(server_mgr_t* mgr, int64_t type) {
	csnet_spinlock_lock(&mgr->lock);
	int idx = type % mgr->slots;
	cs_slist_t* newlist = cs_slist_new();
	cs_sl_node_t* head = mgr->table[idx]->head;
	while (head) {
		server_item_t* item = (server_item_t*)head->data;
		server_item_t* newitem = server_item_copy(item);
		cs_sl_node_t* snode = cs_sl_node_new(newitem);
		cs_slist_insert(newlist, snode);
		head = head->next;
	}
	csnet_spinlock_unlock(&mgr->lock);
	return newlist;
}

