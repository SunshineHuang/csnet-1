#pragma once

#include <stdbool.h>
#include <openssl/ssl.h>

/*
 * ssl/tls protocol
 */
#define CS_SSLV23     0
#define CS_SSLV23_S   1
#define CS_SSLV23_C   2
#define CS_SSLV3      3
#define CS_SSLV3_S    4
#define CS_SSLV3_C    5
#define CS_TLSV1      6
#define CS_TLSV1_S    7
#define CS_TLSV1_C    8
#define CS_TLSV1_1    9
#define CS_TLSV1_1_S  10
#define CS_TLSV1_1_C  11
#define CS_TLSV1_2    12
#define CS_TLSV1_2_S  13
#define CS_TLSV1_2_C  14

#define HEADER_MAX_LEN 512
#define CONTENT_MAX_LEN 4096

typedef struct csnet_ssl {
	int fd;
	SSL_CTX* ssl_ctx;
	SSL* ssl;
} csnet_ssl_t;

/*
 * Call once.
 */
void CS_SSL_ENV_INIT(void);

csnet_ssl_t* csnet_ssl_new(int protocol);
csnet_ssl_t* cs_ssl_new_with_cert_and_pkey(int protocol, X509* x, EVP_PKEY* pkey);
void csnet_ssl_shutdown(csnet_ssl_t*);
void csnet_ssl_free(csnet_ssl_t*);

/*
 * Return value:
 *  0: succeed
 * -1: failed
 */
int csnet_ssl_connect(csnet_ssl_t*, const char* ip, int port, bool nonblocking, int timeout /* milliseconds */);

/*
 * Return value:
 *  0: succeed
 * -1: failed
 */
int csnet_ssl_accept(csnet_ssl_t*, int fd, bool nonblocking, int timeout /* milliseconds */);

int csnet_ssl_send(csnet_ssl_t*, const char* data, int len);
int csnet_ssl_recv(csnet_ssl_t*, char* buf, int len);

