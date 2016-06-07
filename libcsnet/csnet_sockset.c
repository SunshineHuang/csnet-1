#include "csnet_sockset.h"
#include "csnet_sock.h"
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
csnet_sockset_new(int max_conn, unsigned int start_sid) {
	csnet_sockset_t* set = calloc(1, sizeof(*set) + max_conn * sizeof(csnet_sock_t*));
	if (!set) {
		csnet_oom(sizeof(*set));
	}

	set->start_sid = start_sid;
	set->curr_sid = start_sid;
	set->max_conn = max_conn;
	for (int i = 0; i < max_conn; i++) {
		csnet_sock_t* sock = csnet_sock_new(64 * 1024);
		set->set[i] = sock;
	}

	return set;
}

void
csnet_sockset_free(csnet_sockset_t* set) {
	for (int i = 0; i < set->max_conn; i++) {
		if (set->set[i]->fd > 0) {
			close(set->set[i]->fd);
		}
		csnet_sock_free(set->set[i]);
	}
	free(set);
}

unsigned int
csnet_sockset_put(csnet_sockset_t* set, int fd) {
	int count = set->max_conn;
	unsigned int sid = SOCKID(set);
	csnet_sock_t* sock = set->set[sid % set->max_conn];
	while (sock->sid != 0 && --count > 0) {
		sid = SOCKID(set);
		sock = set->set[sid % set->max_conn];
	}
	sock->sid = sid;
	sock->fd = fd;
	return sid;
}

csnet_sock_t*
csnet_sockset_get(csnet_sockset_t* set, unsigned int sid) {
	return set->set[sid % set->max_conn];
}

void
csnet_sockset_reset_sock(csnet_sockset_t* set, unsigned int sid) {
	csnet_sock_t* sock = set->set[sid % set->max_conn];
	if (sock->sid == sid) {
		sock->sid = 0;
		close(sock->fd);
		csnet_rb_reset(sock->rb);
	}
}

