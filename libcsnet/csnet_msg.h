#ifndef csnet_msg_h
#define csnet_msg_h

#include "csnet_sock.h"

typedef struct csnet_msg {
	csnet_sock_t* sock;
	int size;
	int offset;
	char data[0];
} csnet_msg_t;

csnet_msg_t* csnet_msg_new(int size, csnet_sock_t* sock);
void csnet_msg_free(csnet_msg_t*);
void csnet_msg_append(csnet_msg_t*, char* data, int len);

#endif  /* csnet_msg_h */

