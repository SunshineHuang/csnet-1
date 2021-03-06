#include "bmin-send-msg.h"
#include "business-cmd.h"
#include "head-status.h"

#include "libcsnet.h"

#include <stdlib.h>
#include <string.h>

extern csnet_log_t* LOG;
extern cs_lfqueue_t* Q;

struct bmin_send_msg*
bmin_send_msg_new(csnet_ss_t* ss, csnet_head_t* head) {
	struct bmin_send_msg* bm = malloc(sizeof(*bm));
	if (!bm) {
		return NULL;
	}
	bm->ops.rsp = bmin_send_msg_rsp;
	bm->ops.err = bmin_send_msg_err;
	bm->ops.timeout = bmin_send_msg_timeout;
	bm->ss = ss;
	memcpy(&bm->head, head, HEAD_LEN);
	return bm;
}

int64_t
bmin_send_msg_req(void* b, csnet_ss_t* ss, csnet_head_t* head, char* data, int data_len) {
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
		csnet_msg = csnet_msg_new(h.len, ss);
		csnet_msg_append(csnet_msg, (char*)&h, HEAD_LEN);
		csnet_msg_append(csnet_msg, pack.data, pack.len);
		csnet_sendto(Q, csnet_msg);
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
bmin_send_msg_rsp(void* b, csnet_head_t* head, char* data, int data_len) {
	return 0;
}

int64_t
bmin_send_msg_timeout(void* b) {
	return 0;
}

void
bmin_send_msg_err(void* b, csnet_ss_t* ss, csnet_head_t* head) {

}

void
bmin_send_msg_free(struct bmin_send_msg* bm) {
	free(bm);
}

