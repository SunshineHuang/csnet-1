#include "bmin-connect-server.h"
#include "business_cmd.h"
#include "head_status.h"
#include "libcsnet.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

extern csnet_log_t* LOG;
extern csnet_ctx_t* CTX;
extern csnet_conntor_t* CONNTOR;
extern cs_lfqueue_t* Q;

struct bmin_connect_server*
bmin_connect_server_new(csnet_ss_t* ss, csnet_head_t* head) {
	struct bmin_connect_server* bm = malloc(sizeof(*bm));
	if (!bm) {
		return NULL;
	}

	bm->ops.rsp = bmin_connect_server_rsp;
	bm->ops.timeout = bmin_connect_server_timeout;
	bm->ops.err = bmin_connect_server_err;
	bm->ss = ss;
	memcpy(&bm->head, head, HEAD_LEN);
	bm->ctxid = csnet_ctx_ctxid(CTX);

	return bm;
}

int64_t
bmin_connect_server_req(void* b, csnet_ss_t* ss, csnet_head_t* head, char* data, int data_len) {
	struct bmin_connect_server* bm = (struct bmin_connect_server *)b;

	csnet_msg_t* csnet_msg;
	csnet_head_t h;

	h.status = HS_OK;
	h.version = head->version;
	h.compress = head->compress;
	h.ctxid = bm->ctxid;
	h.sid = head->sid;
	h.len = HEAD_LEN;
	h.cmd = CSNET_NOTICE_RSP;

	csnet_unpack_t unpack;
	csnet_unpack_init(&unpack, data, data_len);
	int stype = csnet_unpack_geti(&unpack);
	const char* sip = csnet_unpack_getstr(&unpack);
	int sport = csnet_unpack_geti(&unpack);

	csnet_conntor_connect(CONNTOR, stype, sip, sport);

	csnet_msg = csnet_msg_new(h.len, ss);
	csnet_msg_append(csnet_msg, (char*)&h, HEAD_LEN);
	csnet_sendto(Q, csnet_msg);

	return 0;
}

int64_t
bmin_connect_server_rsp(void* b, csnet_head_t* head, char* data, int data_len) {
	return 0;
}

int64_t
bmin_connect_server_timeout(void* b) {
	struct bmin_connect_server* bm = (struct bmin_connect_server *)b;
	csnet_head_t h = {
		.len =  HEAD_LEN,
		.cmd = CSNET_NOTICE_RSP,
		.status = HS_TIMEOUT,
		.version = bm->head.version,
		.compress = bm->head.compress,
		.ctxid = bm->head.ctxid,
		.sid = bm->head.sid
	};
	csnet_msg_t* msg = csnet_msg_new(h.len, bm->ss);
	csnet_msg_append(msg, (char*)&bm->head, HEAD_LEN);
	csnet_sendto(Q, msg);
	return 0;
}

void
bmin_connect_server_err(void* b, csnet_ss_t* ss, csnet_head_t* head) {
	struct bmin_connect_server* bm = (struct bmin_connect_server *)b;

	bm->head.cmd = csnet_echo_msg_rsp;
	bm->head.status = HS_SRV_ERR;
	bm->head.len = HEAD_LEN;

	csnet_msg_t* msg = csnet_msg_new(bm->head.len, bm->ss);
	csnet_msg_append(msg, (char*)&bm->head, HEAD_LEN);
	csnet_sendto(Q, msg);
}

void
bmin_connect_server_free(struct bmin_connect_server* bm) {
	free(bm);
}

