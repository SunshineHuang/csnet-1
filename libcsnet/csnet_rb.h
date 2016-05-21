#ifndef csnet_rb_h
#define csnet_rb_h

typedef struct csnet_rb {
	int capacity;
	int data_len;
	int seek;
	char* buffer;
} csnet_rb_t;

csnet_rb_t* csnet_rb_new(int size);
void csnet_rb_free(csnet_rb_t*);
int csnet_rb_append(csnet_rb_t*, const char* data, int len);
int csnet_rb_seek(csnet_rb_t*, int len);
void csnet_rb_reset(csnet_rb_t*);
char* csnet_rb_data(csnet_rb_t*);
int csnet_rb_data_len(csnet_rb_t*);

#endif  /* csnet_rb_h */

