#pragma once

#include <stdbool.h>
#include <sys/epoll.h>

typedef struct epoll_event csnet_epoller_event_t;

typedef struct csnet_epoller {
	int fd;
	int max_events;
	csnet_epoller_event_t* events;
} csnet_epoller_t;

csnet_epoller_t* csnet_epoller_new(int max_events);
void csnet_epoller_free(csnet_epoller_t*);

int csnet_epoller_add(csnet_epoller_t*, int fd, unsigned int sid);
int csnet_epoller_del(csnet_epoller_t*, int fd, unsigned int sid);
int csnet_epoller_mod_read(csnet_epoller_t*, int fd, unsigned int sid);
int csnet_epoller_mod_write(csnet_epoller_t*, int fd, unsigned int sid);
int csnet_epoller_mod_rw(csnet_epoller_t*, int fd, unsigned int sid);

int csnet_epoller_wait(csnet_epoller_t*, int timeout /* milliseconds */);
csnet_epoller_event_t* csnet_epoller_get_event(csnet_epoller_t*, int index);
bool csnet_epoller_event_is_readable(csnet_epoller_event_t* event);
bool csnet_epoller_event_is_writeable(csnet_epoller_event_t* event);
bool csnet_epoller_event_is_error(csnet_epoller_event_t* event);
int csnet_epoller_event_fd(csnet_epoller_event_t* event);
unsigned int csnet_epoller_event_sid(csnet_epoller_event_t* event);

