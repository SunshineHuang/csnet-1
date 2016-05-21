#include "csnet_msg.h"

#include <stdlib.h>
#include <string.h>

csnet_msg_t*
csnet_msg_new(int size, csnet_sock_t* sock) {
	csnet_msg_t* m = calloc(1, sizeof(*m) + size);
	m->sock = sock;
	m->size = size;
	m->offset = 0;
	return m;
}

void
csnet_msg_free(csnet_msg_t* m) {
	free(m);
}

void
csnet_msg_append(csnet_msg_t* m, char* data, int len) {
	memcpy(m->data + m->offset, data, len);
	m->offset += len;
}

