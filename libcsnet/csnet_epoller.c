#include "csnet_epoller.h"
#include "csnet_log.h"
#include "csnet_utils.h"

#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

csnet_epoller_t*
csnet_epoller_new(int max_events) {
	csnet_epoller_t* epoller = calloc(1, sizeof(*epoller));
	if (!epoller) {
		csnet_oom(sizeof(*epoller));
	}

	epoller->fd = epoll_create(1024);
	if (epoller->fd == -1) {
		DEBUG("epoll_create(): %s", strerror(errno));
		free(epoller);
		return NULL;
	}

	epoller->max_events = max_events;
	epoller->events = calloc(max_events, sizeof(csnet_epoller_event_t));

	if (!epoller->events) {
		csnet_oom(max_events * sizeof(csnet_epoller_event_t));
	}

	return epoller;
}

void
csnet_epoller_free(csnet_epoller_t* epoller) {
	assert(epoller);
	close(epoller->fd);
	free(epoller->events);
	free(epoller);
}

int
csnet_epoller_add(csnet_epoller_t* epoller, int fd, unsigned int sid) {
	assert(epoller);
	struct epoll_event ev = {
		.events = EPOLLIN | EPOLLET,
		.data.u64 = (unsigned long)fd << 32 | sid
	};
	return epoll_ctl(epoller->fd, EPOLL_CTL_ADD, fd, &ev);
}

int
csnet_epoller_del(csnet_epoller_t* epoller, int fd, unsigned int sid) {
	assert(epoller);
	struct epoll_event ev = {
		.events = EPOLLIN | EPOLLET,
		.data.u64 = (unsigned long)fd << 32 | sid
	};
	return epoll_ctl(epoller->fd, EPOLL_CTL_DEL, fd, &ev);
}

int
csnet_epoller_mod_read(csnet_epoller_t* epoller, int fd, unsigned int sid) {
	assert(epoller);
	struct epoll_event ev = {
		.events = EPOLLIN | EPOLLET,
		.data.u64 = (unsigned long)fd << 32 | sid
	};
	return epoll_ctl(epoller->fd, EPOLL_CTL_MOD, fd, &ev);
}

int
csnet_epoller_mod_write(csnet_epoller_t* epoller, int fd, unsigned int sid) {
	assert(epoller);
	struct epoll_event ev = {
		.events = EPOLLOUT | EPOLLET,
		.data.u64 = (unsigned long)fd << 32 | sid
	};
	return epoll_ctl(epoller->fd, EPOLL_CTL_MOD, fd, &ev);
}

int
csnet_epoller_mod_rw(csnet_epoller_t* epoller, int fd, unsigned int sid) {
	assert(epoller);
	struct epoll_event ev = {
		.events = EPOLLIN | EPOLLOUT | EPOLLET,
		.data.u64 = (unsigned long)fd << 32 | sid
	};
	return epoll_ctl(epoller->fd, EPOLL_CTL_MOD, fd, &ev);
}

int
csnet_epoller_wait(csnet_epoller_t* epoller, int timeout /* milliseconds */) {
	assert(epoller);
	return epoll_wait(epoller->fd, epoller->events, epoller->max_events, timeout);
}

csnet_epoller_event_t*
csnet_epoller_get_event(csnet_epoller_t* epoller, int index) {
	assert(epoller);
	if (index < epoller->max_events) {
		return &epoller->events[index];
	}
	return NULL;
}

bool
csnet_epoller_event_is_readable(csnet_epoller_event_t* event) {
	assert(event);
	return event->events & EPOLLIN;
}

bool
csnet_epoller_event_is_writeable(csnet_epoller_event_t* event) {
	assert(event);
	return event->events & EPOLLOUT;
}

bool
csnet_epoller_event_is_error(csnet_epoller_event_t* event) {
	assert(event);
	return event->events & (EPOLLERR | EPOLLHUP);
}

int
csnet_epoller_event_fd(csnet_epoller_event_t* event) {
	assert(event);
	return event->data.u64 >> 32;
}

unsigned int
csnet_epoller_event_sid(csnet_epoller_event_t* event) {
	assert(event);
	return event->data.u64 & 0xffffUL;
}

