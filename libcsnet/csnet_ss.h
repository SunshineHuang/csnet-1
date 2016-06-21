#pragma once

#include "csnet_sock.h"
#include "csnet_ssock.h"

typedef struct csnet_ss {
	enum {
		sock_type, ssock_type
	} type;
	union {
		csnet_sock_t* sock;
		csnet_ssock_t* ssock;
	} ss;
} csnet_ss_t;

csnet_ss_t* csnet_ss_new(int type, X509* x, EVP_PKEY* pkey);
void csnet_ss_free(csnet_ss_t* ss);
int csnet_ss_recv(csnet_ss_t* ss);
int csnet_ss_send(csnet_ss_t* ss, char* data, int len);
int csnet_ss_fd(csnet_ss_t* ss);
char* csnet_ss_data(csnet_ss_t* ss);
unsigned int csnet_ss_data_len(csnet_ss_t* ss);
int csnet_ss_seek(csnet_ss_t* ss, unsigned int len);

