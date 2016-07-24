#include "bmin-get-servers.h"
#include "server-mgr.h"
#include "business-cmd.h"
#include "head-status.h"

#include "libcsnet.h"

#include <stdlib.h>
#include <string.h>

extern csnet_log_t* LOG;
extern cs_lfqueue_t* Q;
extern server_mgr_t* mgr;

struct bmin_get_servers*
bmin_get_servers_new(csnet_ss_t* ss, csnet_head_t* head) {
	struct bmin_get_servers* bm = malloc(sizeof(*bm));
	if (!bm) {
		return NULL;
	}
	bm->ops.rsp = bmin_get_servers_rsp;
	bm->ops.err = bmin_get_servers_err;
	bm->ops.timeout = bmin_get_servers_timeout;
	bm->ss = ss;
	memcpy(&bm->head, head, HEAD_LEN);
	return bm;
}

int64_t
bmin_get_servers_req(void* b, csnet_ss_t* ss, csnet_head_t* head, char* data, int data_len) {
	csnet_msg_t* csnet_msg;
	csnet_unpack_t unpack;
	csnet_unpack_init(&unpack, data, data_len);

	int stype = csnet_unpack_geti(&unpack);

	csnet_head_t h = {
		.version = head->version,
		.compress = head->compress,
		.cmd = CSNET_GET_SERVERS_RSP,
		.status = HS_OK,
		.ctxid = head->ctxid,
		.sid = head->sid
	};

	if (unpack.error == UNPACK_NOERR) {
		cs_slist_t* list = server_mgr_items(mgr, stype);
		if (list) {
			csnet_pack_t pack;
			csnet_pack_init(&pack);
			csnet_pack_reserve_int(&pack);

			int num = 0;
			cs_sl_node_t* head = list->head;
			while (head) {
				cs_sl_node_t* next = head->next;
				++num;
				server_item_t* item = head->data;
				csnet_pack_puti(&pack, item->type);
				csnet_pack_putstr(&pack, item->ip);
				csnet_pack_puti(&pack, item->port);
				free(item);
				free(head);
				head = next;
			}
			csnet_pack_fill_int(&pack, num);
			h.len = HEAD_LEN + pack.len;
			csnet_msg = csnet_msg_new(h.len, ss);
			csnet_msg_append(csnet_msg, (char*)&h, HEAD_LEN);
			csnet_msg_append(csnet_msg, pack.data, pack.len);
			csnet_sendto(Q, csnet_msg);
		} else {
			h.status = HS_GET_SERVERS_ERR;
		}
	} else {
		LOG_ERROR(LOG, "unpack error");
		h.len = HEAD_LEN;
		h.status = HS_PKG_ERR;
		csnet_msg = csnet_msg_new(h.len, ss);
		csnet_msg_append(csnet_msg, (char*)&h, HEAD_LEN);
		csnet_sendto(Q, csnet_msg);
	}

	return 0;
}

int64_t
bmin_get_servers_rsp(void* b, csnet_head_t* head, char* data, int data_len) {
	return 0;
}

int64_t
bmin_get_servers_timeout(void* b) {
	return 0;
}

void
bmin_get_servers_err(void* b, csnet_ss_t* ss, csnet_head_t* head) {

}

void
bmin_get_servers_free(struct bmin_get_servers* bm) {
	free(bm);
}

