#include "bmin-register.h"
#include "server-mgr.h"
#include "business-cmd.h"
#include "head-status.h"

#include "libcsnet.h"

#include <stdlib.h>
#include <string.h>

extern csnet_log_t* LOG;
extern cs_lfqueue_t* Q;
extern server_mgr_t* mgr;

struct bmin_register*
bmin_register_new(csnet_ss_t* ss, csnet_head_t* head) {
	struct bmin_register* bm = malloc(sizeof(*bm));
	if (!bm) {
		return NULL;
	}
	bm->ops.rsp = bmin_register_rsp;
	bm->ops.err = bmin_register_err;
	bm->ops.timeout = bmin_register_timeout;
	bm->ss = ss;
	memcpy(&bm->head, head, HEAD_LEN);
	return bm;
}

int64_t
bmin_register_req(void* b, csnet_ss_t* ss, csnet_head_t* head, char* data, int data_len) {
	csnet_msg_t* csnet_msg;
	csnet_unpack_t unpack;
	csnet_unpack_init(&unpack, data, data_len);

	int stype = csnet_unpack_geti(&unpack);
	const char* sip = csnet_unpack_getstr(&unpack);
	int sport = csnet_unpack_geti(&unpack);

	csnet_head_t h = {
		.version = head->version,
		.compress = head->compress,
		.cmd = CSNET_REGISTER_RSP,
		.status = HS_OK,
		.ctxid = head->ctxid,
		.sid = head->sid
	};

	if (unpack.error == UNPACK_NOERR) {
		server_item_t* item = server_item_new(stype, sport, strdup(sip));
		if (server_mgr_insert(mgr, item) == 0) {
			LOG_DEBUG(LOG, "register done. stype: %d, sip: %s, sport: %d", stype, sip, sport);
		} else {
			server_item_free(item);
			LOG_WARNING(LOG, "server item: stype: %d, sip: %s, sport: %d has already existed",
				stype, sip, sport);
		}
		h.len = HEAD_LEN;
		csnet_msg = csnet_msg_new(h.len, ss);
		csnet_msg_append(csnet_msg, (char*)&h, HEAD_LEN);
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
bmin_register_rsp(void* b, csnet_head_t* head, char* data, int data_len) {
	return 0;
}

int64_t
bmin_register_timeout(void* b) {
	return 0;
}

void
bmin_register_err(void* b, csnet_ss_t* ss, csnet_head_t* head) {

}

void
bmin_register_free(struct bmin_register* bm) {
	free(bm);
}

