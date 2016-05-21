#include "csnet_pack.h"

#include <stdio.h>
#include <string.h>

void
csnet_pack_init(csnet_pack_t* p) {
	p->error = PACK_NOERR;
	p->reserve_pos = -1;
	p->len = 0;
}

void
csnet_pack_putc(csnet_pack_t* p, char c) {
	int size = sizeof(char);
	if (p->len + size <= MAX_PACK_LEN) {
		memcpy(p->data + p->len, &c, size);
		p->len += size;
	} else {
		p->error |= PACK_ERROR;
	}
}

void
csnet_pack_putuc(csnet_pack_t* p, unsigned char v) {
	int size = sizeof(unsigned char);
	if (p->len + size <= MAX_PACK_LEN) {
		memcpy(p->data + p->len, &v, size);
		p->len += size;
	} else {
		p->error |= PACK_ERROR;
	}
}

void
csnet_pack_puts(csnet_pack_t* p, short v) {
	int size = sizeof(short);
	if (p->len + size <= MAX_PACK_LEN) {
		memcpy(p->data + p->len, &v, size);
		p->len += size;
	} else {

		p->error |= PACK_ERROR;
	}
}

void
csnet_pack_puti(csnet_pack_t* p, int v) {
	int size = sizeof(int);
	if (p->len + size <= MAX_PACK_LEN) {
		memcpy(p->data + p->len, &v, size);
		p->len += size;
	} else {
		p->error |= PACK_ERROR;
	}
}

void
csnet_pack_putui(csnet_pack_t* p, unsigned int v) {
	int size = sizeof(unsigned int);
	if (p->len + size <= MAX_PACK_LEN) {
		memcpy(p->data + p->len, &v, size);
		p->len += size;
	} else {
		p->error |= PACK_ERROR;
	}
}

void
csnet_pack_putl(csnet_pack_t* p, long v) {
	int size = sizeof(long);
	if (p->len + size <= MAX_PACK_LEN) {
		memcpy(p->data + p->len, &v, size);
		p->len += size;
	} else {
		p->error |= PACK_ERROR;
	}
}

void
csnet_pack_putll(csnet_pack_t* p, long long v) {
	int size = sizeof(long long);
	if (p->len + size <= MAX_PACK_LEN) {
		memcpy(p->data + p->len, &v, size);
		p->len += size;
	} else {
		p->error |= PACK_ERROR;
	}
}

void
csnet_pack_putul(csnet_pack_t* p, unsigned long v) {
	int size = sizeof(unsigned long);
	if (p->len + size <= MAX_PACK_LEN) {
		memcpy(p->data + p->len, &v, size);
		p->len += size;
	} else {
		p->error |= PACK_ERROR;
	}
}

void
csnet_pack_putull(csnet_pack_t* p, unsigned long long v) {
	int size = sizeof(unsigned long long);
	if (p->len + size <= MAX_PACK_LEN) {
		memcpy(p->data + p->len, &v, size);
		p->len += size;
	} else {
		p->error |= PACK_ERROR;
	}
}

void
csnet_pack_putstr(csnet_pack_t* p, const char* str) {
	int size = sizeof(short);
	int str_len = str ? strlen(str) : 0;
	if (p->len + size + str_len + 1 <= MAX_PACK_LEN) {
		memcpy(p->data + p->len, &str_len, size);
		p->len += size;
		if (str) {
			memcpy(p->data + p->len, str, str_len);
			p->data[p->len + str_len] = '\0';
			p->len += str_len + 1;
		}
	} else {
		p->error |= PACK_ERROR;
	}
}

void
csnet_pack_putf(csnet_pack_t* p, float f) {
	int size = sizeof(short);
	char tmp[32] = {0};
	int len = snprintf(tmp, 32, "%0.2f", f);

	if (p->len + len + size <= MAX_PACK_LEN) {
		memcpy(p->data + p->len, &len, size);
		p->len += size;
		memcpy(p->data + p->len, tmp, len);
		p->len += len;
	} else {
		p->error |= PACK_ERROR;
	}
}

void
csnet_pack_reserve_short(csnet_pack_t* p) {
	if (p->reserve_pos > -1) {
		/* The csnet_pack has a reserved block. If you want to reserve a new
		   block, you should fill the reserved block first */
		p->error |= PACK_RESERVED;
		return;
	}

	int size = sizeof(short);
	if (p->len + size > MAX_PACK_LEN) {
		p->error |= PACK_ERROR;
	} else {
		p->reserve_pos = p->len;
		p->len += size;
	}
}

void
csnet_pack_reserve_int(csnet_pack_t* p) {
	if (p->reserve_pos > -1) {
		/* The csnet_pack has a reserved block. If you want to reserve a new
		   block, you should fill the reserved block first */
		p->error |= PACK_RESERVED;
		return;
	}

	int size = sizeof(int);
	if (p->len + size > MAX_PACK_LEN) {
		p->error |= PACK_ERROR;
	} else {
		p->reserve_pos = p->len;
		p->len += size;
	}
}

void
csnet_pack_reserve_long(csnet_pack_t* p) {
	if (p->reserve_pos > -1) {
		/* The csnet_pack has a reserved block. If you want to reserve a new
		   block, you should fill the reserved block first */
		p->error |= PACK_RESERVED;
		return;
	}

	int size = sizeof(long);
	if (p->len + size > MAX_PACK_LEN) {
		p->error |= PACK_ERROR;
	} else {
		p->reserve_pos = p->len;
		p->len += size;
	}
}

void
csnet_pack_fill_short(csnet_pack_t* p, short v) {
	int size = sizeof(short);
	if (p->reserve_pos > -1) {
		memcpy(p->data + p->reserve_pos, &v, size);
		p->reserve_pos = -1;
	}
}

void
csnet_pack_fill_int(csnet_pack_t* p, int v) {
	int size = sizeof(int);
	if (p->reserve_pos > -1) {
		memcpy(p->data + p->reserve_pos, &v, size);
		p->reserve_pos = -1;
	}
}

void
csnet_pack_fill_long(csnet_pack_t* p, long v) {
	int size = sizeof(long);
	if (p->reserve_pos > -1) {
		memcpy(p->data + p->reserve_pos, &v, size);
		p->reserve_pos = -1;
	}
}

