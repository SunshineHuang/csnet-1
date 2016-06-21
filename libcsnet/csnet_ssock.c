#include "csnet_ssock.h"
#include "csnet_log.h"
#include "csnet_socket_api.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <openssl/err.h>

void
csnet_ssock_env_init(void) {
	SSL_library_init();  /* openssl's doc say that this api always return '1',
				so we can discard the return value */
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
}

csnet_ssock_t*
csnet_ssock_new(int proto) {
	const SSL_METHOD* method;
	csnet_ssock_t* ss = calloc(1, sizeof(*ss));
	ss->fd = 0;
	ss->sid = 0;
	ss->rb = csnet_rb_new(8 * 1024);

	switch (proto) {
	case CSNET_SSLV23:    method = SSLv23_method();         break;
	case CSNET_SSLV23_S:  method = SSLv23_server_method();  break;
	case CSNET_SSLV23_C:  method = SSLv23_client_method();  break;
	case CSNET_SSLV3:     method = SSLv3_method();          break;
	case CSNET_SSLV3_S:   method = SSLv3_server_method();   break;
	case CSNET_SSLV3_C:   method = SSLv3_client_method();   break;
	case CSNET_TLSV1:     method = TLSv1_method();          break;
	case CSNET_TLSV1_S:   method = TLSv1_server_method();   break;
	case CSNET_TLSV1_C:   method = TLSv1_client_method();   break;
	case CSNET_TLSV1_1:   method = TLSv1_1_method();        break;
	case CSNET_TLSV1_1_S: method = TLSv1_1_server_method(); break;
	case CSNET_TLSV1_1_C: method = TLSv1_1_client_method(); break;
	case CSNET_TLSV1_2:   method = TLSv1_2_method();        break;
	case CSNET_TLSV1_2_S: method = TLSv1_2_server_method(); break;
	case CSNET_TLSV1_2_C: method = TLSv1_2_client_method(); break;
	default: DEBUG("unknown protocol"); method = NULL;   break;
	}

	if (!method) {
		csnet_rb_free(ss->rb);
		free(ss);
		return NULL;
	}

	ss->ctx = SSL_CTX_new(method);
	ss->ssl = SSL_new(ss->ctx);

	return ss;
}

csnet_ssock_t*
csnet_ssock_new3(int proto, X509* x, EVP_PKEY* pkey) {
	const SSL_METHOD* method;
	csnet_ssock_t* ss = calloc(1, sizeof(*ss));
	ss->fd = 0;
	ss->sid = 0;
	ss->rb = csnet_rb_new(8 * 1024);


	switch (proto) {
	case CSNET_SSLV23:    method = SSLv23_method();         break;
	case CSNET_SSLV23_S:  method = SSLv23_server_method();  break;
	case CSNET_SSLV23_C:  method = SSLv23_client_method();  break;
	case CSNET_SSLV3:     method = SSLv3_method();          break;
	case CSNET_SSLV3_S:   method = SSLv3_server_method();   break;
	case CSNET_SSLV3_C:   method = SSLv3_client_method();   break;
	case CSNET_TLSV1:     method = TLSv1_method();          break;
	case CSNET_TLSV1_S:   method = TLSv1_server_method();   break;
	case CSNET_TLSV1_C:   method = TLSv1_client_method();   break;
	case CSNET_TLSV1_1:   method = TLSv1_1_method();        break;
	case CSNET_TLSV1_1_S: method = TLSv1_1_server_method(); break;
	case CSNET_TLSV1_1_C: method = TLSv1_1_client_method(); break;
	case CSNET_TLSV1_2:   method = TLSv1_2_method();        break;
	case CSNET_TLSV1_2_S: method = TLSv1_2_server_method(); break;
	case CSNET_TLSV1_2_C: method = TLSv1_2_client_method(); break;
	default: DEBUG("unknown protocol"); method = NULL;   break;
	}

	if (!method) {
		csnet_rb_free(ss->rb);
		free(ss);
		return NULL;
	}

	ss->ctx = SSL_CTX_new(method);
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
csnet_ssock_reset(csnet_ssock_t* ss) {
	ss->sid = 0;
	close(ss->fd);
	ss->fd = 0;
	csnet_rb_reset(ss->rb);
	SSL_CTX_free(ss->ctx);
	SSL_free(ss->ssl);
	ss->ctx = SSL_CTX_new(TLSv1_method());
	ss->ssl = SSL_new(ss->ctx);
}

