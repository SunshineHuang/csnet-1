#include "csnet_timer.h"
#include "csnet_log.h"
#include "csnet_fast.h"
#include "csnet_utils.h"

#include <stdlib.h>

csnet_timer_t*
csnet_timer_new(int interval, int wheel_count) {
	csnet_timer_t* timer = (csnet_timer_t*)calloc(1, sizeof(*timer) + (interval + 1) * sizeof(cs_lfhash_t*));
	if (!timer) {
		csnet_oom(sizeof(*timer));
	}

	timer->interval = interval;
	timer->prev_wheel = interval;
	timer->curr_wheel = 0;
	timer->curr_time = csnet_gettime();
	timer->which_wheel_tbl = cs_lfhash_new(wheel_count * 2);
	for (int i = 0; i < interval + 1; i++) {
		timer->wheels_tbl[i] = cs_lfhash_new(wheel_count);
	}

	return timer;
}

void
csnet_timer_free(csnet_timer_t* timer) {
	cs_lfhash_free(timer->which_wheel_tbl);
	for (int i = 0; i < timer->interval + 1; i++) {
		cs_lfhash_free(timer->wheels_tbl[i]);
	}
	free(timer);
}

int
csnet_timer_insert(csnet_timer_t* timer, int fd, unsigned int sid) {
	int* which_wheel = calloc(1, sizeof(int));
	if (!which_wheel) {
		csnet_oom(sizeof(int));
	}

	*which_wheel = timer->prev_wheel;
	if (cs_lfhash_insert(timer->which_wheel_tbl, sid, which_wheel) == -1) {
		free(which_wheel);
		return -1;
	}

	csnet_timer_node_t* node = (csnet_timer_node_t*)calloc(1, sizeof(*node));
	if (csnet_slow(!node)) {
		csnet_oom(sizeof(*node));
	}

	node->fd = fd;
	node->timerid = sid;
	node->interval = timer->interval;

	if (cs_lfhash_insert(timer->wheels_tbl[*which_wheel], sid, node) == -1) {
		cs_lfhash_search(timer->which_wheel_tbl, sid);
		free(node);
		return -1;
	}

	return 0;
}

void
csnet_timer_update(csnet_timer_t* timer, unsigned int timerid) {
	int* which_wheel = cs_lfhash_search(timer->which_wheel_tbl, timerid);
	if ((!which_wheel) || (*which_wheel == timer->prev_wheel)) {
		return;
	}

	csnet_timer_node_t* timer_node = cs_lfhash_search(timer->wheels_tbl[*which_wheel], timerid);

	if (csnet_fast(timer_node)) {
		cs_lfhash_delete(timer->wheels_tbl[*which_wheel], timerid);
		*which_wheel = timer->curr_wheel;
		cs_lfhash_insert(timer->wheels_tbl[*which_wheel], timerid, timer_node);
	}
}

void
csnet_timer_remove(csnet_timer_t* timer, unsigned int timerid) {
	int* which_wheel = cs_lfhash_search(timer->which_wheel_tbl, timerid);
	if (csnet_slow(!which_wheel)) {
		return;
	}

	int index = *which_wheel;
	free(which_wheel);

	csnet_timer_node_t* timer_node = cs_lfhash_search(timer->wheels_tbl[index], timerid);
	if (timer_node) {
		cs_lfhash_delete(timer->wheels_tbl[index], timerid);
		cs_lfhash_delete(timer->which_wheel_tbl, timerid);
	}
}

/* Return -1 means there is no expired timer in the wheel,
 * other value means there is expired timers in this wheel */
int
csnet_timer_book_keeping(csnet_timer_t* timer) {
	unsigned long now = csnet_gettime();
	if (now - timer->curr_time < 1000000) {
		return -1;
	}

	timer->curr_time = now;
	timer->prev_wheel = timer->curr_wheel;

	if (++timer->curr_wheel > timer->interval) {
		timer->curr_wheel = 0;
	}

	if (cs_lfhash_count(timer->wheels_tbl[timer->curr_wheel]) <= 0) {
		return -1;
	}

	return timer->curr_wheel;
}

