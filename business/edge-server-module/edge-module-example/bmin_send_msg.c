#include "bmin_send_msg.h"
#include "business_cmd.h"
#include "head_status.h"

#include "cs-lfqueue.h"
#include "csnet_log.h"
#include "csnet_msg.h"
#include "csnet_pack.h"
#include "csnet_unpack.h"
#include "csnet_conntor.h"

#include <stdlib.h>
#include <string.h>

extern csnet_log_t* LOG;
extern cs_lfqueue_t* Q;

struct bmin_send_msg*
bmin_send_msg_new(csnet_sock_t* sock, csnet_head_t* head) {
	struct bmin_send_msg* bm = malloc(sizeof(*bm));
	if (!bm) {
		return NULL;
	}
	bm->ops.rsp = bmin_send_msg_rsp;
	bm->ops.err = bmin_send_msg_err;
	bm->ops.timeout = bmin_send_msg_timeout;
	bm->sock = sock;
	memcpy(&bm->head, head, HEAD_LEN);
	return bm;
}

int64_t
bmin_send_msg_req(void* b, csnet_sock_t* sock, csnet_head_t* head, char* data, int data_len) {
	csnet_msg_t* csnet_msg;
	csnet_unpack_t unpack;
	csnet_unpack_init(&unpack, data, data_len);

	const char* msg = csnet_unpack_getstr(&unpack);

	csnet_head_t h = {
		.version = head->version,
		.compress = head->compress,
		.cmd = csnet_echo_msg_rsp,
		.status = HS_OK,
		.ctxid = head->ctxid,
		.sid = head->sid
	};

	if (unpack.error == UNPACK_NOERR) {
		csnet_pack_t pack;
		csnet_pack_init(&pack);
		csnet_pack_putstr(&pack, msg);
		h.len = HEAD_LEN + pack.len;
		csnet_msg = csnet_msg_new(h.len, sock);
		csnet_msg_append(csnet_msg, (char*)&h, HEAD_LEN);
		csnet_msg_append(csnet_msg, pack.data, pack.len);
		cs_lfqueue_enq(Q, csnet_msg);
	} else {
		LOG_ERROR(LOG, "unpack error");
		h.len = HEAD_LEN;
		h.status = HS_PKG_ERR;
		csnet_msg = csnet_msg_new(h.len, sock);
		csnet_msg_append(csnet_msg, (char*)&h, HEAD_LEN);
		cs_lfqueue_enq(Q, csnet_msg);
	}

	return 0;
}

int64_t
bmin_send_msg_rsp(void* b, csnet_head_t* head, char* data, int data_len) {
	return 0;
}

int64_t
bmin_send_msg_timeout(void* b) {
	return 0;
}

void
bmin_send_msg_err(void* b, csnet_sock_t* sock, csnet_head_t* head) {

}

void
bmin_send_msg_free(struct bmin_send_msg* bm) {
	free(bm);
}

