#include "csnet-sb.h"
#include "csnet-utils.h"

#include <stdlib.h>
#include <string.h>

#ifdef JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

#define MAX_BUFFER_LEN 1024 * 1024 * 5  /* 5M */

static inline char*
_sb_expand(int size) {
	if (size > MAX_BUFFER_LEN) {
		return NULL;
	}

	return malloc(size);
}

csnet_sb_t*
csnet_sb_new(int size) {
	csnet_sb_t* sb = calloc(1, sizeof(*sb));
	if (!sb) {
		csnet_oom(sizeof(*sb));
	}

	sb->capacity = size;
	sb->data_len = 0;
	sb->buffer = calloc(1, size);

	if (!sb->buffer) {
		csnet_oom(size);
	}

	return sb;
}

void
csnet_sb_free(csnet_sb_t* sb) {
	free(sb->buffer);
	free(sb);
}

int
csnet_sb_append(csnet_sb_t* sb, const char* data, int len) {
	if (sb->capacity - sb->data_len > len) {
		memcpy(sb->buffer + sb->data_len, data, len);
		sb->data_len += len;
		return 0;
	}

	char* new_buffer = _sb_expand(sb->capacity * 2);
	if (!new_buffer) {
		return -1;
	}

	sb->capacity *= 2;
	memcpy(new_buffer, sb->buffer, sb->data_len);
	free(sb->buffer);
	sb->buffer = new_buffer;
	memcpy(sb->buffer + sb->data_len, data, len);
	sb->data_len += len;

	return len;
}

int
csnet_sb_seek(csnet_sb_t* sb, int len) {
	if (len > sb->data_len) {
		/* TODO: data_len and seek should be reset ? */
		return 0;
	}

	int move_len = sb->data_len - len;
	if (move_len == 0) {
		sb->data_len = 0;
		return len;
	}

	memmove(sb->buffer, sb->buffer + len, move_len);
	sb->data_len -= len;

	return len;
}

char*
csnet_sb_data(csnet_sb_t* sb) {
	return sb->buffer;
}

int
csnet_sb_data_len(csnet_sb_t* sb) {
	return sb->data_len;
}

void
csnet_sb_reset(csnet_sb_t* sb) {
	sb->data_len = 0;
}

