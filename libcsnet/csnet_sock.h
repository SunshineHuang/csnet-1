#pragma once

#include "csnet_rb.h"

typedef struct csnet_sock {
	int fd;
	unsigned int sid;
	csnet_rb_t* rb;
} csnet_sock_t;

csnet_sock_t* csnet_sock_new(int rb_size);
void csnet_sock_free(csnet_sock_t*);
int csnet_sock_recv(csnet_sock_t*);
int csnet_sock_send(csnet_sock_t*, char* buff, int len);

