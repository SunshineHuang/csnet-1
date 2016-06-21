#pragma once

#include "csnet_ss.h"

typedef struct csnet_msg {
	csnet_ss_t* ss;
	int size;
	int offset;
	char data[0];
} csnet_msg_t;

csnet_msg_t* csnet_msg_new(int size, csnet_ss_t* ss);
void csnet_msg_free(csnet_msg_t*);
void csnet_msg_append(csnet_msg_t*, char* data, int len);

