#ifndef csnet_pack_h
#define csnet_pack_h

#define MAX_PACK_LEN 32 * 1024

#define PACK_NOERR    0
#define PACK_ERROR    1
#define PACK_RESERVED 2

typedef struct csnet_pack {
	char error;
	int reserve_pos;
	int len;
	char data[MAX_PACK_LEN];
} csnet_pack_t;

void csnet_pack_init(csnet_pack_t*);
void csnet_pack_putc(csnet_pack_t*, char c);
void csnet_pack_putuc(csnet_pack_t*, unsigned char uc);
void csnet_pack_puts(csnet_pack_t*, short s);
void csnet_pack_puti(csnet_pack_t*, int i);
void csnet_pack_putui(csnet_pack_t*, unsigned int ui);
void csnet_pack_putl(csnet_pack_t*, long l);
void csnet_pack_putll(csnet_pack_t*, long long ll);
void csnet_pack_putul(csnet_pack_t*, unsigned long ul);
void csnet_pack_putull(csnet_pack_t*, unsigned long long ull);
void csnet_pack_putf(csnet_pack_t*, float f);
void csnet_pack_putstr(csnet_pack_t*, const char* str);

void csnet_pack_reserve_short(csnet_pack_t*);
void csnet_pack_reserve_int(csnet_pack_t*);
void csnet_pack_reserve_long(csnet_pack_t*);
void csnet_pack_fill_short(csnet_pack_t*, short s);
void csnet_pack_fill_int(csnet_pack_t*, int i);
void csnet_pack_fill_long(csnet_pack_t*, long l);

#endif  /* csnet_pack_h */

