#pragma once

#include "csnet_rb.h"

#include <openssl/ssl.h>

/* ssl/tls protocol */
#define CSNET_SSLV23     0
#define CSNET_SSLV23_S   1
#define CSNET_SSLV23_C   2
#define CSNET_SSLV3      3
#define CSNET_SSLV3_S    4
#define CSNET_SSLV3_C    5
#define CSNET_TLSV1      6
#define CSNET_TLSV1_S    7
#define CSNET_TLSV1_C    8
#define CSNET_TLSV1_1    9
#define CSNET_TLSV1_1_S  10
#define CSNET_TLSV1_1_C  11
#define CSNET_TLSV1_2    12
#define CSNET_TLSV1_2_S  13
#define CSNET_TLSV1_2_C  14

typedef struct csnet_ssock {
	int proto;
	int fd;
	unsigned int sid;
	csnet_rb_t* rb;
	SSL_CTX* ctx;
	SSL* ssl;
} csnet_ssock_t;

/* Call once. */
void csnet_ssock_env_init(void);

csnet_ssock_t* csnet_ssock_new(int proto);
csnet_ssock_t* csnet_ssock_new3(int proto, X509* x, EVP_PKEY* pkey);
void csnet_ssock_free(csnet_ssock_t*);

int csnet_ssock_accept(csnet_ssock_t* ssock);
int csnet_ssock_connect(csnet_ssock_t* ssock, const char* server, int port);

int csnet_ssock_send(csnet_ssock_t* ssock, const char* data, int len);
int csnet_ssock_recv(csnet_ssock_t* ssock);
int csnet_ssock_recv_buff(csnet_ssock_t* ssock, char* buf, int len);

void csnet_ssock_set_fd(csnet_ssock_t* ssock, int fd);
void csnet_ssock_reset(csnet_ssock_t* ss, X509* x, EVP_PKEY* pkey);

