#ifndef csnet_head_h
#define csnet_head_h

#include <stdint.h>

#define HEAD_LEN 22
#define MAX_LEN  64 * 1024  /* 64K */

#define VERSION 1
#define COMPRESS_NON 0

#pragma pack(push, 1)

typedef struct csnet_head {
	unsigned char  version;
	unsigned char  compress;
	unsigned short cmd;
	unsigned short status;
	int64_t        ctxid;
	unsigned int   sid;
	unsigned int   len;
} csnet_head_t;

#pragma pack(pop)

#endif  /* csnet_head_h */

