#include "csnet_socket_api.h"
#include "csnet_log.h"

#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define BACKLOG 65535

int
set_nonblocking(int sfd) {
	int flags = fcntl(sfd, F_GETFL, 0);
	if (flags == -1) {
		return -1;
	}
	flags |= O_NONBLOCK;
	return fcntl(sfd, F_SETFL, flags);
}

int
listen_port(int port) {
	int lfd;
	struct sockaddr_in serv_addr;

	lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1) {
		DEBUG("socket(): %s", strerror(errno));
		return -1;
	}

	int reuse = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &reuse, (socklen_t)sizeof(reuse));
	setsockopt(lfd, SOL_SOCKET, SO_REUSEPORT, &reuse, (socklen_t)sizeof(reuse));

	bzero(&serv_addr, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	if (bind(lfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
		DEBUG("bind(): %s", strerror(errno));
		close(lfd);
		return -1;
	}

	if (listen(lfd, BACKLOG) == -1) {
		DEBUG("listen(): %s", strerror(errno));
		close(lfd);
		return -1;
	}

	return lfd;
}

int
blocking_connect(const char* host, int port) {
	struct addrinfo hints;
	struct addrinfo* result;
	struct addrinfo* rp;
	int sock;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	char portptr[6];
	snprintf(portptr, 6, "%d", port);

	if (getaddrinfo(host, portptr, &hints, &result) != 0) {
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sock == -1) {
			continue;
		}

		if (connect(sock, rp->ai_addr, rp->ai_addrlen) != -1) {
			break;
		}

		close(sock);
	}

	freeaddrinfo(result);
	return (rp == NULL) ? -1 : sock;
}

int
nonblocking_connect(const char* host, int port, int timeout) {
	struct addrinfo hints;
	struct addrinfo* result;
	struct addrinfo* rp;
	int sock;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	char portptr[6];
	snprintf(portptr, 6, "%d", port);

	if (getaddrinfo(host, portptr, &hints, &result) != 0) {
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sock == -1) {
			continue;
		}

		set_nonblocking(sock);

		if (connect(sock, rp->ai_addr, rp->ai_addrlen) == -1) {
			if (errno == EINPROGRESS) {
				struct pollfd pollfd;
				pollfd.fd = sock;
				pollfd.events = POLLIN | POLLOUT;

				/* If the remote host is down or occurs network issue,
				   poll() will block `timeout` milliseconds here */

				int nready = poll(&pollfd, 1, timeout);
				if (nready < 0) {
					DEBUG("poll(): %s", strerror(errno));
					close(sock);
					continue;
				}

				if (nready == 0) {
					DEBUG("poll() timeout: 100 milliseconds");
					close(sock);
					continue;
				}

				int result;
				socklen_t result_len = sizeof(result);

				if (getsockopt(pollfd.fd, SOL_SOCKET, SO_ERROR, &result, &result_len) < 0) {
					close(sock);
					continue;
				}

				if (result != 0) {
					DEBUG("SO_ERROR: %d", result);
					close(sock);
					continue;
				}
				break;
			} else {
				DEBUG("connect to host: %s, port: %d error: %s", host, port, strerror(errno));
				close(sock);
				continue;
			}
		}
	}

	freeaddrinfo(result);
	return (rp == NULL) ? -1 : sock;
}

void
wait_milliseconds(int milliseconds) {
	poll(NULL, 0, milliseconds);
}

void
modin(int epfd, int socket, unsigned int sid) {
	struct epoll_event ev = {
		.events = EPOLLIN,
		.data.u32 = sid
	};
	epoll_ctl(epfd, EPOLL_CTL_MOD, socket, &ev);
}

void
modout(int epfd, int socket, unsigned int sid) {
	struct epoll_event ev = {
		.events = EPOLLOUT,
		.data.u32 = sid
	};
	epoll_ctl(epfd, EPOLL_CTL_MOD, socket, &ev);
}

void
modinout(int epfd, int socket, unsigned int sid) {
	struct epoll_event ev = {
		.events = EPOLLIN | EPOLLOUT,
		.data.u32 = sid
	};
	epoll_ctl(epfd, EPOLL_CTL_MOD, socket, &ev);
}

void
modadd(int epfd, int socket, unsigned int sid) {
	struct epoll_event ev = {
		.events = EPOLLIN,
		.data.u32 = sid
	};
	epoll_ctl(epfd, EPOLL_CTL_ADD, socket, &ev);
}

void
moddel(int epfd, int socket, unsigned int sid) {
	struct epoll_event ev = {
		.events = EPOLLIN,
		.data.u32 = sid
	};
	epoll_ctl(epfd, EPOLL_CTL_DEL, socket, &ev);
}

