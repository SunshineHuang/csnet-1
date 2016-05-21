#include "csnet_unpack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_UNPACK_LEN 32 * 1024

void
csnet_unpack_init(csnet_unpack_t* upk, const char* data, int len) {
	upk->error = UNPACK_NOERR;
	upk->seek = 0;
	upk->remain = len;
	upk->data = data;
}

char
csnet_unpack_getc(csnet_unpack_t* upk) {
	int size = sizeof(char);
	if (upk->remain >= size) {
		int seek = upk->seek;
		upk->seek += size;
		upk->remain -= size;
		return *(char*)(upk->data + seek);
	} else {
		upk->error |= UNPACK_ERROR;
		return -1;
	}
}

unsigned char
csnet_unpack_getuc(csnet_unpack_t* upk) {
	int size = sizeof(unsigned char);
	if (upk->remain >= size) {
		int seek = upk->seek;
		upk->seek += size;
		upk->remain -= size;
		return *(unsigned char*)(upk->data + seek);
	} else {
		upk->error |= UNPACK_ERROR;
		return -1;
	}
}

short
csnet_unpack_gets(csnet_unpack_t* upk) {
	int size = sizeof(short);
	if (upk->remain >= size) {
		int seek = upk->seek;
		upk->seek += size;
		upk->remain -= size;
		return *(short*)(upk->data + seek);
	} else {
		upk->error |= UNPACK_ERROR;
		return -1;
	}
}

int
csnet_unpack_geti(csnet_unpack_t* upk) {
	int size = sizeof(int);
	if (upk->remain >= size) {
		int seek = upk->seek;
		upk->seek += size;
		upk->remain -= size;
		return *(int*)(upk->data + seek);
	} else {
		upk->error |= UNPACK_ERROR;
		return -1;
	}
}

unsigned int
csnet_unpack_getui(csnet_unpack_t* upk) {
	int size = sizeof(unsigned int);
	if (upk->remain >= size) {
		int seek = upk->seek;
		upk->seek += size;
		upk->remain -= size;
		return *(unsigned int*)(upk->data + seek);
	} else {
		upk->error |= UNPACK_ERROR;
		return -1;
	}
}

long
csnet_unpack_getl(csnet_unpack_t* upk) {
	int size = sizeof(long);
	if (upk->remain >= size) {
		int seek = upk->seek;
		upk->seek += size;
		upk->remain -= size;
		return *(long*)(upk->data + seek);
	} else {
		upk->error |= UNPACK_ERROR;
		return -1;
	}
}

long long
csnet_unpack_getll(csnet_unpack_t* upk) {
	int size = sizeof(long long);
	if (upk->remain >= size) {
		int seek = upk->seek;
		upk->seek += size;
		upk->remain -= size;
		return *(long long*)(upk->data + seek);
	} else {
		upk->error |= UNPACK_ERROR;
		return -1;
	}
}

unsigned long
csnet_unpack_getul(csnet_unpack_t* upk) {
	int size = sizeof(unsigned long);
	if (upk->remain >= size) {
		int seek = upk->seek;
		upk->seek += size;
		upk->remain -= size;
		return *(unsigned long*)(upk->data + seek);
	} else {
		upk->error |= UNPACK_ERROR;
		return -1;
	}
}

unsigned long long
csnet_unpack_getull(csnet_unpack_t* upk) {
	int size = sizeof(unsigned long long);
	if (upk->remain >= size) {
		int seek = upk->seek;
		upk->seek += size;
		upk->remain -= size;
		return *(unsigned long long*)(upk->data + seek);
	} else {
		upk->error |= UNPACK_ERROR;
		return -1;
	}
}

const char*
csnet_unpack_getstr(csnet_unpack_t* upk) {
	int size = sizeof(short);
	if (upk->remain >= size) {
		short len = 0;
		memcpy(&len, upk->data + upk->seek, size);
		upk->seek += size;
		upk->remain -= size;
		if (len > upk->remain) {
			upk->error |= UNPACK_ERROR;
			return NULL;
		} else if (len < 0) {
			upk->error |= UNPACK_ERROR;
			return NULL;
		} else if (len == 0) {
			return NULL;
		} else {
			const char* s = NULL;
			int seek = upk->seek;
			upk->seek += len + 1;
			upk->remain -= len + 1;
			s = upk->data + seek;
			return s;
		}
	} else {
		upk->error |= UNPACK_ERROR;
		return NULL;
	}
}

/*
void
csnet_unpack_getf(csnet_unpack_t* upk, float* v) {
	char buf[32];
	if (unpack_getstr(upk, buf)) {
		*v = atof(buf);
	} else {
		upk->error |= UNPACK_ERROR;
	}
}
*/

