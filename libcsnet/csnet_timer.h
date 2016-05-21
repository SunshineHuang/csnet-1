#ifndef csnet_timer_h
#define csnet_timer_h

#include "hlhashtable.h"

typedef struct csnet_timer_node {
	int fd;
	unsigned int timerid;
	int interval;  /* TODO: We can use this value to reduce moving.
	                  When one csnet_timer recv data, we don't need move this
			  csnet_timer to another slot, we just decrease this value */
} csnet_timer_node_t;

typedef struct csnet_timer {
	int interval;
	int prev_slot;
	int curr_slot;
	unsigned long curr_time;
	hashtable_t* timerid_hashtbl;
	hashtable_t** timer_hashtbls;
} csnet_timer_t;

csnet_timer_t* csnet_timer_new(int interval, int slot_size);
void csnet_timer_free(csnet_timer_t*);
int csnet_timer_insert(csnet_timer_t*, int fd, unsigned int sid);
void csnet_timer_remove(csnet_timer_t*, unsigned int timerid);
void csnet_timer_update(csnet_timer_t*, unsigned int timerid);
int csnet_timer_book_keeping(csnet_timer_t*);

#endif  /* csnet_timer_h */

