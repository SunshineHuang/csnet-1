#ifndef csnet_sb_h
#define csnet_sb_h

typedef struct csnet_sb {
	int capacity;
	int data_len;
	char* buffer;
} csnet_sb_t;

csnet_sb_t* csnet_sb_new(int size);
void csnet_sb_free(csnet_sb_t*);
int csnet_sb_append(csnet_sb_t*, const char* data, int len);
int csnet_sb_seek(csnet_sb_t*, int len);
void csnet_sb_reset(csnet_sb_t*);
char* csnet_sb_data(csnet_sb_t*);
int csnet_sb_data_len(csnet_sb_t*);

#endif  /* csnet_sb_h */

