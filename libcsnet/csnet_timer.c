#include "csnet_timer.h"
#include "csnet_log.h"
#include "csnet_utils.h"

#include <stdlib.h>
#include <sys/time.h>

static unsigned long gettime();

csnet_timer_t*
csnet_timer_new(int interval, int slot_size) {
	csnet_timer_t* timer = (csnet_timer_t*)calloc(1, sizeof(*timer));
	if (!timer) {
		csnet_oom(sizeof(*timer));
	}

	timer->interval = interval;
	timer->prev_slot = interval;
	timer->curr_slot = 0;
	timer->curr_time = gettime();
	timer->timerid_hashtbl = ht_create(slot_size, slot_size * 2, NULL);
	timer->timer_hashtbls = (hashtable_t**)calloc(interval + 1, sizeof(hashtable_t*));

	for (int i = 0; i < interval + 1; i++) {
		timer->timer_hashtbls[i] = ht_create(slot_size, slot_size * 2, NULL);
	}

	return timer;
}

void
csnet_timer_free(csnet_timer_t* timer) {
	ht_destroy(timer->timerid_hashtbl);
	for (int i = 0; i < timer->interval + 1; i++) {
		ht_destroy(timer->timer_hashtbls[i]);
	}

	free(timer->timer_hashtbls);
	free(timer);
}

int
csnet_timer_insert(csnet_timer_t* timer, int fd, unsigned int sid) {
	int* which_slot = calloc(1, sizeof(int));
	if (!which_slot) {
		csnet_oom(sizeof(int));
	}

	*which_slot = timer->prev_slot;
	if (ht_set(timer->timerid_hashtbl, &sid, sizeof(unsigned int), which_slot, sizeof(int*)) == -1) {
		free(which_slot);
		return -1;
	}

	csnet_timer_node_t* node = (csnet_timer_node_t*)calloc(1, sizeof(*node));
	if (!node) {
		csnet_oom(sizeof(*node));
	}

	node->fd = fd;
	node->timerid = sid;
	node->interval = timer->interval;

	if (ht_set(timer->timer_hashtbls[*which_slot], &sid, sizeof(unsigned int), node, sizeof(csnet_timer_node_t)) == -1) {
		ht_get(timer->timerid_hashtbl, &sid, sizeof(unsigned int), NULL);
		free(node);
		return -1;
	}

	return 0;
}

void
csnet_timer_update(csnet_timer_t* timer, unsigned int timerid) {
	int* which_slot = ht_get(timer->timerid_hashtbl, &timerid, sizeof(unsigned int), NULL);
	if ((!which_slot) || (*which_slot == timer->prev_slot)) {
		return;
	}

	size_t node_size = 0;
	csnet_timer_node_t* timer_node = ht_get(timer->timer_hashtbls[*which_slot], &timerid, sizeof(unsigned int), &node_size);

	if (timer_node) {
		ht_delete(timer->timer_hashtbls[*which_slot], &timerid, sizeof(unsigned int), NULL, NULL);
		*which_slot = timer->curr_slot;
		ht_set(timer->timer_hashtbls[*which_slot], &timerid, sizeof(unsigned int), timer_node, node_size);
	}
}

void
csnet_timer_remove(csnet_timer_t* timer, unsigned int timerid) {
	int* which_slot = ht_get(timer->timerid_hashtbl, &timerid, sizeof(unsigned int), NULL);
	if (!which_slot) {
		return;
	}

	int index = *which_slot;
	free(which_slot);

	csnet_timer_node_t* timer_node = ht_get(timer->timer_hashtbls[index], &timerid, sizeof(unsigned int), NULL);
	if (timer_node) {
		ht_delete(timer->timer_hashtbls[index], &timerid, sizeof(unsigned int), NULL, NULL);
		ht_delete(timer->timerid_hashtbl, &timerid, sizeof(unsigned int), NULL, NULL);
	}
}

int
csnet_timer_book_keeping(csnet_timer_t* timer) {
	unsigned long now = gettime();
	if (now - timer->curr_time < 1000000) {
		return -1;
	}

	timer->curr_time = now;
	timer->prev_slot = timer->curr_slot;

	if (++timer->curr_slot > timer->interval) {
		timer->curr_slot = 0;
	}

	if (ht_count(timer->timer_hashtbls[timer->curr_slot]) <= 0) {
		return -1;
	}

	return timer->curr_slot;
}

static inline unsigned long
gettime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

