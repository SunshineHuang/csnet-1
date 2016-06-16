#pragma once

typedef struct csnet_rb {
	unsigned int capacity;
	unsigned int data_len;
	unsigned int seek;
	char* buffer;
} csnet_rb_t;

csnet_rb_t* csnet_rb_new(unsigned int size);
void csnet_rb_free(csnet_rb_t*);
int csnet_rb_append(csnet_rb_t*, const char* data, unsigned int len);
unsigned int csnet_rb_seek(csnet_rb_t*, unsigned int len);
void csnet_rb_reset(csnet_rb_t*);
char* csnet_rb_data(csnet_rb_t*);
unsigned int csnet_rb_data_len(csnet_rb_t*);

