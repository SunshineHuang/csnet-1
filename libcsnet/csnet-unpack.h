#pragma once

#define UNPACK_NOERR 0
#define UNPACK_ERROR 1

typedef struct csnet_unpack {
	char error;
	int seek;
	int remain;
	const char* data;
} csnet_unpack_t;

void csnet_unpack_init(csnet_unpack_t*, const char* data, int len);
char csnet_unpack_getc(csnet_unpack_t*);
unsigned char csnet_unpack_getuc(csnet_unpack_t*);
short csnet_unpack_gets(csnet_unpack_t*);
int csnet_unpack_geti(csnet_unpack_t*);
unsigned int csnet_unpack_getui(csnet_unpack_t*);
long csnet_unpack_getl(csnet_unpack_t*);
long long csnet_unpack_getll(csnet_unpack_t*);
unsigned long csnet_unpack_getul(csnet_unpack_t*);
unsigned long long csnet_unpack_getull(csnet_unpack_t*);
float csnet_unpack_getf(csnet_unpack_t*);
const char* csnet_unpack_getstr(csnet_unpack_t*);

