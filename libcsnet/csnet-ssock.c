#include "csnet-ssock.h"
#include "csnet-log.h"
#include "csnet-socket-api.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <openssl/err.h>

static const SSL_METHOD* protocol_methods[15];

void
csnet_ssock_env_init(void) {
	SSL_library_init();  /* openssl's doc say that this api always return '1',
				so we can discard the return value */
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	protocol_methods[0] = SSLv23_method();
	protocol_methods[1] = SSLv23_server_method();
	protocol_methods[2] = SSLv23_client_method();
	protocol_methods[3] = SSLv3_method();
	protocol_methods[4] = SSLv3_server_method();
	protocol_methods[5] = SSLv3_client_method();
	protocol_methods[6] = TLSv1_method();
	protocol_methods[7] = TLSv1_server_method();
	protocol_methods[8] = TLSv1_client_method();
	protocol_methods[9] = TLSv1_1_method();
	protocol_methods[10] = TLSv1_1_server_method();
	protocol_methods[11] = TLSv1_1_client_method();
	protocol_methods[12] = TLSv1_2_method();
	protocol_methods[13] = TLSv1_2_server_method();
	protocol_methods[14] = TLSv1_2_client_method();
}

csnet_ssock_t*
csnet_ssock_new(int proto) {
	if (proto > CSNET_TLSV1_2_C || proto < CSNET_SSLV23) {
		DEBUG("unknown protocol");
		return NULL;
	}

	csnet_ssock_t* ss = calloc(1, sizeof(*ss));
	ss->proto = proto;
	ss->fd = 0;
	ss->sid = 0;
	ss->rb = csnet_rb_new(8 * 1024);
	ss->ctx = SSL_CTX_new(protocol_methods[proto]);
	ss->ssl = SSL_new(ss->ctx);

	return ss;
}

csnet_ssock_t*
csnet_ssock_new3(int proto, X509* x, EVP_PKEY* pkey) {
	if (proto > CSNET_TLSV1_2_C || proto < CSNET_SSLV23) {
		DEBUG("unknown protocol");
		return NULL;
	}

	csnet_ssock_t* ss = calloc(1, sizeof(*ss));
	ss->proto = proto;
	ss->fd = 0;
	ss->sid = 0;
	ss->rb = csnet_rb_new(8 * 1024);
	ss->ctx = SSL_CTX_new(protocol_methods[proto]);
	ss->ssl = SSL_new(ss->ctx);
	SSL_use_certificate(ss->ssl, x);
	SSL_use_PrivateKey(ss->ssl, pkey);

	return ss;
}

void
csnet_ssock_set_fd(csnet_ssock_t* ss, int fd) {
	ss->fd = fd;
	SSL_set_fd(ss->ssl, fd);
}

void
csnet_ssock_free(csnet_ssock_t* ss) {
	close(ss->fd);
	csnet_rb_free(ss->rb);
	SSL_CTX_free(ss->ctx);
	SSL_free(ss->ssl);
	free(ss);
}

int
csnet_ssock_accept(csnet_ssock_t* ss) {
	SSL_set_accept_state(ss->ssl);
	int ret;
again:
	ret = SSL_accept(ss->ssl);
	if (ret != 1) {
		int ssl_err = SSL_get_error(ss->ssl, ret);
		if (ssl_err == SSL_ERROR_WANT_READ || ssl_err == SSL_ERROR_WANT_WRITE) {
			goto again;
		} else {
			return -1;
		}
	}
	return 0;
}

int
csnet_ssock_connect(csnet_ssock_t* ss, const char* server, int port) {
	int ret;
	ss->fd = blocking_connect(server, port);
	if (ss->fd < 0) {
		return -1;
	}

	set_nonblocking(ss->fd);
	SSL_set_fd(ss->ssl, ss->fd);
	SSL_set_connect_state(ss->ssl);
again:
	ret = SSL_connect(ss->ssl);
	if (ret != -1) {
		int ssl_err = SSL_get_error(ss->ssl, ret);
		if (ssl_err == SSL_ERROR_WANT_READ || ssl_err == SSL_ERROR_WANT_WRITE) {
			goto again;
		} else {
			return -1;
		}
	}
	return 0;
}

void
csnet_ssock_shutdown(csnet_ssock_t* ss) {
	SSL_shutdown(ss->ssl);
}

int
csnet_ssock_recv(csnet_ssock_t* ss) {
	int ret = -1;
	int ssl_err = -1;
	char buffer[8 * 1024] = {0};
again:
	ret = SSL_read(ss->ssl, buffer, 8 * 1024);
	if (ret > 0) {
		csnet_rb_append(ss->rb, buffer, ret);
		return ret;
	}

	if (ret < 0) {
		ssl_err = SSL_get_error(ss->ssl, ret);
		switch (ssl_err) {
		case SSL_ERROR_NONE:
			return 0;

		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:
			goto again;

		default:
			return -1;
		}
	}
	return ret;
}



int
csnet_ssock_recv_buff(csnet_ssock_t* ss, char* buf, int len) {
	int ret = -1;
	int ssl_err = -1;

again:
	ret = SSL_read(ss->ssl, buf, len);
	if (ret > 0) {
		return ret;
	}

	if (ret < 0) {
		ssl_err = SSL_get_error(ss->ssl, ret);
		switch (ssl_err) {
		case SSL_ERROR_NONE:
			return 0;

		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:
			goto again;

		default:
			return -1;
		}
	}
	return ret;
}

int
csnet_ssock_send(csnet_ssock_t* ss, const char* data, int len) {
	int ret = -1;
	int ssl_err = -1;

again:
	ret = SSL_write(ss->ssl, data, len);
	if (ret > 0) {
		return ret;
	}

	if (ret < 0) {
		ssl_err = SSL_get_error(ss->ssl, ret);
		switch (ssl_err) {
		case SSL_ERROR_NONE:
			return 0;

		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:
			goto again;

		default:
			return -1;
		}
	}
	return ret;
}

void
csnet_ssock_reset(csnet_ssock_t* ss, X509* x, EVP_PKEY* pkey) {
	ss->sid = 0;
	close(ss->fd);
	ss->fd = 0;
	csnet_rb_reset(ss->rb);
	SSL_CTX_free(ss->ctx);
	SSL_free(ss->ssl);
	ss->ctx = SSL_CTX_new(protocol_methods[ss->proto]);
	ss->ssl = SSL_new(ss->ctx);
	SSL_use_certificate(ss->ssl, x);
	SSL_use_PrivateKey(ss->ssl, pkey);
}

