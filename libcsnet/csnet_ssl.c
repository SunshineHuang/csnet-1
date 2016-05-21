#include "csnet_ssl.h"
#include "csnet_log.h"
#include "csnet_socket_api.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <openssl/err.h>

void CS_SSL_ENV_INIT(void) {
	SSL_library_init();  /* openssl's doc say that this api always return '1',
			        so we can discard the return value */
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
}

csnet_ssl_t*
csnet_ssl_new(int protocol) {
	csnet_ssl_t* ssl = calloc(1, sizeof(*ssl));
	if (!ssl) {
		DEBUG("out of memory.");
		return NULL;
	}

	ssl->fd = -1;
	const SSL_METHOD* method;

	switch (protocol) {
	case CS_SSLV23:
		method = SSLv23_method();
		break;

	case CS_SSLV23_S:
		method = SSLv23_server_method();
		break;

	case CS_SSLV23_C:
		method = SSLv23_client_method();
		break;

	case CS_SSLV3:
		method = SSLv3_method();
		break;

	case CS_SSLV3_S:
		method = SSLv3_server_method();
		break;

	case CS_SSLV3_C:
		method = SSLv3_client_method();
		break;

	case CS_TLSV1:
		method = TLSv1_method();
		break;

	case CS_TLSV1_S:
		method = TLSv1_server_method();
		break;

	case CS_TLSV1_C:
		method = TLSv1_client_method();
		break;

	case CS_TLSV1_1:
		method = TLSv1_1_method();
		break;

	case CS_TLSV1_1_S:
		method = TLSv1_1_server_method();
		break;

	case CS_TLSV1_1_C:
		method = TLSv1_1_client_method();
		break;

	case CS_TLSV1_2:
		method = TLSv1_2_method();
		break;

	case CS_TLSV1_2_S:
		method = TLSv1_2_server_method();
		break;

	case CS_TLSV1_2_C:
		method = TLSv1_2_client_method();
		break;

	default:
		DEBUG("unknown protocol");
		method = NULL;
		break;
	}

	if (!method) {
		free(ssl);
		return NULL;
	}

	ssl->ssl_ctx = SSL_CTX_new(method);
	if (!ssl->ssl_ctx) {
		DEBUG("SSL_CTX_new() failed");
		free(ssl);
		return NULL;
	}

	ssl->ssl = SSL_new(ssl->ssl_ctx);
	if (!ssl->ssl) {
		DEBUG("SSL_new() failed");
		SSL_CTX_free(ssl->ssl_ctx);
		free(ssl);
		return NULL;
	}

	return ssl;
}

csnet_ssl_t*
csnet_ssl_new_with_cert_and_pkey(int protocol, X509* x, EVP_PKEY* pkey) {
	csnet_ssl_t* ssl = calloc(1, sizeof(*ssl));
	if (!ssl) {
		DEBUG("out of memory.");
		return NULL;
	}

	const SSL_METHOD* method;

	switch (protocol) {
	case CS_SSLV23:
		method = SSLv23_method();
		break;

	case CS_SSLV23_S:
		method = SSLv23_server_method();
		break;

	case CS_SSLV23_C:
		method = SSLv23_client_method();
		break;

	case CS_SSLV3:
		method = SSLv3_method();
		break;

	case CS_SSLV3_S:
		method = SSLv3_server_method();
		break;

	case CS_SSLV3_C:
		method = SSLv3_client_method();
		break;

	case CS_TLSV1:
		method = TLSv1_method();
		break;

	case CS_TLSV1_S:
		method = TLSv1_server_method();
		break;

	case CS_TLSV1_C:
		method = TLSv1_client_method();
		break;

	case CS_TLSV1_1:
		method = TLSv1_1_method();
		break;

	case CS_TLSV1_1_S:
		method = TLSv1_1_server_method();
		break;

	case CS_TLSV1_1_C:
		method = TLSv1_1_client_method();
		break;

	case CS_TLSV1_2:
		method = TLSv1_2_method();
		break;

	case CS_TLSV1_2_S:
		method = TLSv1_2_server_method();
		break;

	case CS_TLSV1_2_C:
		method = TLSv1_2_client_method();
		break;

	default:
		DEBUG("unknown protocol");
		method = NULL;
		break;
	}

	if (!method) {
		free(ssl);
		return NULL;
	}

	ssl->ssl_ctx = SSL_CTX_new(method);
	if (!ssl->ssl_ctx) {
		DEBUG("SSL_CTX_new() failed");
		free(ssl);
		return NULL;
	}

	ssl->ssl = SSL_new(ssl->ssl_ctx);
	if (!ssl->ssl) {
		DEBUG("SSL_new() failed");
		SSL_CTX_free(ssl->ssl_ctx);
		free(ssl);
		return NULL;
	}

	if (SSL_use_certificate(ssl->ssl, x) != 1) {
		DEBUG("SSL_use_certificate_file() failed");
		unsigned long err_code = ERR_get_error();
		char err_msg[1024];
		DEBUG("%s", ERR_error_string(err_code, err_msg));
		SSL_CTX_free(ssl->ssl_ctx);
		SSL_free(ssl->ssl);
		return NULL;
	}

	if (SSL_use_PrivateKey(ssl->ssl, pkey) != 1) {
		DEBUG("SSL_use_PrivateKey_file() failed");
		unsigned long err_code = ERR_get_error();
		char err_msg[1024];
		DEBUG("%s", ERR_error_string(err_code, err_msg));
		SSL_CTX_free(ssl->ssl_ctx);
		SSL_free(ssl->ssl);
		return NULL;
	}

	return ssl;

}

void
csnet_ssl_free(csnet_ssl_t* ssl) {
	close(ssl->fd);
	SSL_CTX_free(ssl->ssl_ctx);
	SSL_free(ssl->ssl);
	free(ssl);
}

int
csnet_ssl_connect(csnet_ssl_t* ssl, const char* host, int port, bool nonblocking, int timeout) {
	int ret = -1;
	unsigned long err_code = -1;
	char err_msg[1024] = {0};

	if (ssl->fd == -1) {
		ssl->fd = blocking_connect(host, port);
		if (ssl->fd < 0) {
			DEBUG("can not connect to: %s:%d", host, port);
			return -1;
		}

		if (nonblocking) {
			set_nonblocking(ssl->fd);
		}

		if (SSL_set_fd(ssl->ssl, ssl->fd) == 0) {
			close(ssl->fd);
			err_code = ERR_get_error();
			DEBUG("%s", ERR_error_string(err_code, err_msg));
			return -1;
		}
	}

	/*
	 * If using blocking socket, SSL_connect() will returns only once; if using non-blocking,
	 * SSL_connect() whill returns more than once. We only monitor the POLLIN event. If server
	 * side close the peer after call SSL_accept() failed, both POLLIN and POLLOUT will happen,
	 * SSL_connect() will be called infinite.
	 */

again:
	ret = SSL_connect(ssl->ssl);
	if (ret != 1) {
		err_code = SSL_get_error(ssl->ssl, ret);
		switch (err_code) {
		case SSL_ERROR_NONE:
			return 0;

		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE: {
			struct pollfd pollfd;
			pollfd.fd = ssl->fd;
			pollfd.events = POLLIN;
			while (1) {
				int n = poll(&pollfd, 1, timeout);
				if (n > 0)
					goto again;
				return -1;
			}

		default:
			return -1;
		}
		}
	}

	return 0;
}

int
csnet_ssl_accept(csnet_ssl_t* ssl, int fd, bool nonblocking, int timeout) {
	int ret = -1;
	unsigned long err_code = -1;
	char err_msg[1024] = {0};

	if (ssl->fd == -1) {
		if (nonblocking) {
			set_nonblocking(ssl->fd);
		}
		ssl->fd = fd;
		if (SSL_set_fd(ssl->ssl, ssl->fd) == 0) {
			close(ssl->fd);
			err_code = ERR_get_error();
			DEBUG("%s", ERR_error_string(err_code, err_msg));
			return -1;
		}
	}

again:
	ret = SSL_accept(ssl->ssl);
	if (ret != 1) {
		err_code = SSL_get_error(ssl->ssl, ret);
		switch (err_code) {
		case SSL_ERROR_NONE:
			return 0;

		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE: {
			struct pollfd pollfd;
			pollfd.fd = ssl->fd;
			pollfd.events = POLLIN;
			while (1) {
				int n = poll(&pollfd, 1, timeout);
				if (n > 0)
					goto again;
				return -1;
			}
		}

		default:
			return -1;
		}
	}

	return 0;
}

void
csnet_ssl_shutdown(csnet_ssl_t* ssl) {
	SSL_shutdown(ssl->ssl);
}

int
csnet_ssl_recv(csnet_ssl_t* ssl, char* buf, int len) {
	int ret = -1;
	int ssl_err = -1;

again:
	ret = SSL_read(ssl->ssl, buf, len);
	if (ret > 0) {
		return ret;
	}

	if (ret < 0) {
		ssl_err = SSL_get_error(ssl->ssl, ret);
		switch (ssl_err) {
		case SSL_ERROR_NONE:
			return 0;

		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE: {
			struct pollfd pollfd;
			pollfd.fd = ssl->fd;
			pollfd.events = POLLIN | POLLOUT;
			while (1) {
				int n = poll(&pollfd, 1, 1000);
				if (n > 0)
					goto again;
				return -1;
			}
		}

		default:
			return -1;
		}
	}
	return ret;
}

int
csnet_ssl_send(csnet_ssl_t* ssl, const char* data, int len) {
	int ret = -1;
	int ssl_err = -1;

again:
	ret = SSL_write(ssl->ssl, data, len);
	if (ret > 0) {
		return ret;
	}

	if (ret < 0) {
		ssl_err = SSL_get_error(ssl->ssl, ret);
		switch (ssl_err) {
		case SSL_ERROR_NONE:
			return 0;

		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE: {
			struct pollfd pollfd;
			pollfd.fd = ssl->fd;
			pollfd.events = POLLIN | POLLOUT;
			while (1) {
				int n = poll(&pollfd, 1, 1000);
				if (n > 0)
					goto again;
				return -1;
			}
		}

		default:
			return -1;
		}
	}
	return ret;
}

