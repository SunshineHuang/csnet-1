#include "csnet_sockset.h"
#include "csnet_utils.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

static inline unsigned int
SOCKID(csnet_sockset_t* set) { 
	if (++set->curr_sid == 0xbabeface) {
		set->curr_sid = set->start_sid;
	}
	return set->curr_sid;
}

csnet_sockset_t*
csnet_sockset_new(int max_conn, unsigned int start_sid, int type, X509* x, EVP_PKEY* pkey) {
	csnet_sockset_t* set = calloc(1, sizeof(*set) + max_conn * sizeof(csnet_ss_t*));
	if (!set) {
		csnet_oom(sizeof(*set));
	}

	set->start_sid = start_sid;
	set->curr_sid = start_sid;
	set->max_conn = max_conn;
	set->x = x;
	set->pkey = pkey;
	for (int i = 0; i < max_conn; i++) {
		csnet_ss_t* ss = csnet_ss_new(type, x, pkey);
		set->set[i] = ss;
	}

	return set;
}

void
csnet_sockset_free(csnet_sockset_t* set) {
	for (int i = 0; i < set->max_conn; i++) {
		csnet_ss_free(set->set[i]);
	}
	free(set);
}

unsigned int
csnet_sockset_put(csnet_sockset_t* set, int fd) {
	int count = set->max_conn;
	unsigned int sid = SOCKID(set);
	csnet_ss_t* ss = set->set[sid % set->max_conn];
	if (ss->type == sock_type) {
		while (ss->ss.sock->sid != 0 && --count > 0) {
			sid = SOCKID(set);
			ss = set->set[sid % set->max_conn];
		}
		ss->ss.sock->sid = sid;
		ss->ss.sock->fd = fd;
		return sid;
	} else if (ss->type == ssock_type) {
		while (ss->ss.ssock->sid != 0 && --count > 0) {
			sid = SOCKID(set);
			ss = set->set[sid % set->max_conn];
		}
		ss->ss.ssock->sid = sid;
		csnet_ssock_set_fd(ss->ss.ssock, fd);
		return sid;
	} else {
		return -1;
	}
}

csnet_ss_t*
csnet_sockset_get(csnet_sockset_t* set, unsigned int sid) {
	return set->set[sid % set->max_conn];
}

void
csnet_sockset_reset_ss(csnet_sockset_t* set, unsigned int sid) {
	csnet_ss_t* ss = set->set[sid % set->max_conn];
	if (ss->type == sock_type) {
		if (ss->ss.sock->sid == sid) {
			ss->ss.sock->sid = 0;
			close(ss->ss.sock->fd);
			csnet_rb_reset(ss->ss.sock->rb);
		}
	} else if (ss->type == ssock_type) {
		csnet_ss_t* ss = set->set[sid % set->max_conn];
		if (ss->ss.ssock->sid == sid) {
			close(ss->ss.ssock->fd);
			csnet_ssock_reset(ss->ss.ssock, set->x, set->pkey);
		}
	}
}

