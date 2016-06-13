#pragma once

#include "csnet_sock.h"

typedef struct csnet_sockset {
	int max_conn;
	unsigned int start_sid;
	unsigned int curr_sid;
	csnet_sock_t* set[0];
} csnet_sockset_t;

csnet_sockset_t* csnet_sockset_new(int max_conn, unsigned int start_sid);
void csnet_sockset_free(csnet_sockset_t*);
unsigned int csnet_sockset_put(csnet_sockset_t*, int fd);
csnet_sock_t* csnet_sockset_get(csnet_sockset_t*, unsigned int sid);
void csnet_sockset_reset_sock(csnet_sockset_t*, unsigned int fd);

