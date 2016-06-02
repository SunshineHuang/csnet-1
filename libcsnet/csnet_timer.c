#include "csnet_timer.h"
#include "csnet_log.h"
#include "csnet_utils.h"

#include <stdlib.h>
#include <sys/time.h>

static unsigned long gettime();

csnet_timer_t*
csnet_timer_new(int interval, int wheel_count) {
	csnet_timer_t* timer = (csnet_timer_t*)calloc(1, sizeof(*timer));
	if (!timer) {
		csnet_oom(sizeof(*timer));
	}

	timer->interval = interval;
	timer->prev_wheel = interval;
	timer->curr_wheel = 0;
	timer->curr_time = gettime();
	timer->which_wheel_tbl = ht_create(wheel_count, wheel_count * 2, NULL);
	timer->wheels_tbl = (hashtable_t**)calloc(interval + 1, sizeof(hashtable_t*));

	for (int i = 0; i < interval + 1; i++) {
		timer->wheels_tbl[i] = ht_create(wheel_count, wheel_count * 2, NULL);
	}

	return timer;
}

void
csnet_timer_free(csnet_timer_t* timer) {
	ht_destroy(timer->which_wheel_tbl);
	for (int i = 0; i < timer->interval + 1; i++) {
		ht_destroy(timer->wheels_tbl[i]);
	}

	free(timer->wheels_tbl);
	free(timer);
}

int
csnet_timer_insert(csnet_timer_t* timer, int fd, unsigned int sid) {
	int key_size = sizeof(unsigned int);
	int* which_wheel = calloc(1, sizeof(int));
	if (!which_wheel) {
		csnet_oom(sizeof(int));
	}

	*which_wheel = timer->prev_wheel;
	if (ht_set(timer->which_wheel_tbl, &sid, key_size, which_wheel, sizeof(int*)) == -1) {
		free(which_wheel);
		return -1;
	}

	csnet_timer_node_t* node = (csnet_timer_node_t*)calloc(1, sizeof(*node));
	if (!node) {
		csnet_oom(sizeof(*node));
	}

	node->fd = fd;
	node->timerid = sid;
	node->interval = timer->interval;

	if (ht_set(timer->wheels_tbl[*which_wheel], &sid, key_size, node, sizeof(csnet_timer_node_t)) == -1) {
		ht_get(timer->which_wheel_tbl, &sid, key_size, NULL);
		free(node);
		return -1;
	}

	return 0;
}

void
csnet_timer_update(csnet_timer_t* timer, unsigned int timerid) {
	int key_size = sizeof(unsigned int);
	int* which_wheel = ht_get(timer->which_wheel_tbl, &timerid, key_size, NULL);
	if ((!which_wheel) || (*which_wheel == timer->prev_wheel)) {
		return;
	}

	size_t node_size = 0;
	csnet_timer_node_t* timer_node = ht_get(timer->wheels_tbl[*which_wheel], &timerid, key_size, &node_size);

	if (timer_node) {
		ht_delete(timer->wheels_tbl[*which_wheel], &timerid, key_size, NULL, NULL);
		*which_wheel = timer->curr_wheel;
		ht_set(timer->wheels_tbl[*which_wheel], &timerid, key_size, timer_node, node_size);
	}
}

void
csnet_timer_remove(csnet_timer_t* timer, unsigned int timerid) {
	int key_size = sizeof(unsigned int);
	int* which_wheel = ht_get(timer->which_wheel_tbl, &timerid, sizeof(unsigned int), NULL);
	if (!which_wheel) {
		return;
	}

	int index = *which_wheel;
	free(which_wheel);

	csnet_timer_node_t* timer_node = ht_get(timer->wheels_tbl[index], &timerid, key_size, NULL);
	if (timer_node) {
		ht_delete(timer->wheels_tbl[index], &timerid, key_size, NULL, NULL);
		ht_delete(timer->which_wheel_tbl, &timerid, key_size, NULL, NULL);
	}
}

/* Return -1 means there is no expired timer in the wheel,
 * other value means there is expired timers in this wheel */
int
csnet_timer_book_keeping(csnet_timer_t* timer) {
	unsigned long now = gettime();
	if (now - timer->curr_time < 1000000) {
		return -1;
	}

	timer->curr_time = now;
	timer->prev_wheel = timer->curr_wheel;

	if (++timer->curr_wheel > timer->interval) {
		timer->curr_wheel = 0;
	}

	if (ht_count(timer->wheels_tbl[timer->curr_wheel]) <= 0) {
		return -1;
	}

	return timer->curr_wheel;
}

static inline unsigned long
gettime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

