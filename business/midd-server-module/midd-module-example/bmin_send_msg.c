#include "bmin_send_msg.h"
#include "business_cmd.h"
#include "head_status.h"
#include "csnet_log.h"
#include "csnet_pack.h"
#include "csnet_unpack.h"
#include "csnet_conntor.h"
#include "csnet_ctx.h"
#include "cs-lfqueue.h"
#include "csnet_msg.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

extern csnet_log_t* LOG;
extern csnet_ctx_t* CTX;
extern csnet_conntor_t* CONNTOR;
extern cs_lfqueue_t* Q;

struct bmin_send_msg*
bmin_send_msg_new(csnet_sock_t* sock, csnet_head_t* head) {
	struct bmin_send_msg* bm = malloc(sizeof(*bm));
	if (!bm) {
		return NULL;
	}

	bm->ops.rsp = bmin_send_msg_rsp;
	bm->ops.timeout = bmin_send_msg_timeout;
	bm->ops.err = bmin_send_msg_err;
	bm->sock = sock;
	memcpy(&bm->head, head, HEAD_LEN);
	bm->ctxid = csnet_ctx_ctxid(CTX);

	return bm;
}

int64_t
bmin_send_msg_req(void* b, csnet_sock_t* sock, csnet_head_t* head, char* data, int data_len, cs_hp_record_t* private_record) {
	struct bmin_send_msg* bm = (struct bmin_send_msg *)b;

	csnet_msg_t* csnet_msg;
	csnet_head_t h;

	h.status = HS_OK;
	h.version = head->version;
	h.compress = head->compress;
	h.ctxid = bm->ctxid;
	h.sid = head->sid;
	h.len = HEAD_LEN + data_len;
	h.cmd = csnet_echo_msg_req;

	csnet_sock_t* server_sock = csnet_conntor_get_sock(CONNTOR, ST_EDGE_SERVER);
	csnet_msg = csnet_msg_new(h.len, server_sock);
	csnet_msg_append(csnet_msg, (char*)&h, HEAD_LEN);
	csnet_msg_append(csnet_msg, data, data_len);
	cs_lfqueue_enq(Q, private_record, csnet_msg);

	return h.ctxid;
}

int64_t
bmin_send_msg_rsp(void* b, csnet_head_t* head, char* data, int data_len, cs_hp_record_t* private_record) {
	struct bmin_send_msg* bm = (struct bmin_send_msg *)b;

	bm->head.cmd = csnet_echo_msg_rsp;
	bm->head.status = head->status;
	bm->head.len = HEAD_LEN + data_len;

	/* Check the head's status */
	if (head->status == HS_OK) {
	} else if (head->status == HS_SRV_ERR) {
	} else if (head->status == HS_PKG_ERR) {
	} else {
		LOG_WARNING(LOG, "unknown status: %d", head->status);
	}

	csnet_msg_t* msg = csnet_msg_new(bm->head.len, bm->sock);
	csnet_msg_append(msg, (char*)&bm->head, HEAD_LEN);
	csnet_msg_append(msg, data, data_len);
	cs_lfqueue_enq(Q, private_record, msg);

	return 0;
}

int64_t
bmin_send_msg_timeout(void* b, cs_hp_record_t* private_record) {
	struct bmin_send_msg* bm = (struct bmin_send_msg *)b;
	csnet_head_t h = {
		.len =  HEAD_LEN,
		.cmd = csnet_echo_msg_rsp,
		.status = HS_TIMEOUT,
		.version = bm->head.version,
		.compress = bm->head.compress,
		.ctxid = bm->head.ctxid,
		.sid = bm->head.sid
	};
	csnet_msg_t* msg = csnet_msg_new(h.len, bm->sock);
	csnet_msg_append(msg, (char*)&bm->head, HEAD_LEN);
	cs_lfqueue_enq(Q, private_record, msg);
	return 0;
}

void
bmin_send_msg_err(void* b, csnet_sock_t* sock, csnet_head_t* head, cs_hp_record_t* private_record) {
	struct bmin_send_msg* bm = (struct bmin_send_msg *)b;

	bm->head.cmd = csnet_echo_msg_rsp;
	bm->head.status = HS_SRV_ERR;
	bm->head.len = HEAD_LEN;

	csnet_msg_t* msg = csnet_msg_new(bm->head.len, bm->sock);
	csnet_msg_append(msg, (char*)&bm->head, HEAD_LEN);
	cs_lfqueue_enq(Q, private_record, msg);
}

void
bmin_send_msg_free(struct bmin_send_msg* bm) {
	free(bm);
}

