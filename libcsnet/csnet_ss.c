#include "csnet_ss.h"

#include <stdlib.h>
#include <unistd.h>

csnet_ss_t*
csnet_ss_new(int type, X509* x, EVP_PKEY* pkey) {
	csnet_ss_t* ss = calloc(1, sizeof(*ss));
	ss->type = type;
	if (type == sock_type) {
		ss->ss.sock = csnet_sock_new(16 * 1024);
	} else if (type == ssock_type) {
		ss->ss.ssock = csnet_ssock_new3(CSNET_TLSV1, x, pkey);
	} else {
		abort();
	}

	return ss;
}

void
csnet_ss_free(csnet_ss_t* ss) {
	if (ss->type == sock_type) {
		close(ss->ss.sock->fd);
		csnet_sock_free(ss->ss.sock);
	} else if (ss->type == ssock_type) {
		close(ss->ss.ssock->fd);
		csnet_ssock_free(ss->ss.ssock);
	}
	free(ss);
}

int
csnet_ss_recv(csnet_ss_t* ss) {
	if (ss->type == sock_type) {
		return csnet_sock_recv(ss->ss.sock);
	} else if (ss->type == ssock_type) {
		return csnet_ssock_recv(ss->ss.ssock);
	}
	return -1;
}

int
csnet_ss_send(csnet_ss_t* ss, char* data, int len) {
	if (ss->type == sock_type) {
		return csnet_sock_send(ss->ss.sock, data, len);
	} else if (ss->type == ssock_type) {
		return csnet_ssock_send(ss->ss.ssock, data, len);
	}
	return -1;
}

char*
csnet_ss_data(csnet_ss_t* ss) {
	if (ss->type == sock_type) {
		return csnet_rb_data(ss->ss.sock->rb);
	} else if (ss->type == ssock_type) {
		return csnet_rb_data(ss->ss.ssock->rb);
	}
	return NULL;
}

unsigned int
csnet_ss_data_len(csnet_ss_t* ss) {
	if (ss->type == sock_type) {
		return csnet_rb_data_len(ss->ss.sock->rb);
	} else if (ss->type == ssock_type) {
		return csnet_rb_data_len(ss->ss.ssock->rb);
	}
	return 0;
}

int
csnet_ss_fd(csnet_ss_t* ss) {
	if (ss->type == sock_type) {
		return ss->ss.sock->fd;
	} else if (ss->type == ssock_type) {
		return ss->ss.ssock->fd;
	}
	return -1;
}

int
csnet_ss_seek(csnet_ss_t* ss, unsigned int len) {
	if (ss->type == sock_type) {
		return csnet_rb_seek(ss->ss.sock->rb, len);
	} else if (ss->type == ssock_type) {
		return csnet_rb_seek(ss->ss.ssock->rb, len);
	}
	return -1;

}
