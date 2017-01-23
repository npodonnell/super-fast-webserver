#include <sys/epoll.h>
#include <stdio.h>

#include "ep.h"

int ep_init(const int max_events) {
	return epoll_create(max_events);
}

int ep_add(const int efd, const int fd, const int role) {

	struct epoll_event event;

	event.data.u64 = (uint64_t)0;
	event.data.fd = fd;

	switch(role) {
		case EP_ROLE_LISTENER:
			event.events = EPOLLET | EPOLLIN;
			break;

		case EP_ROLE_CLIENT:
			event.events = EPOLLET | EPOLLIN | EPOLLOUT | EPOLLRDHUP;
			break;

		case EP_ROLE_FILE:
			event.events = EPOLLET | EPOLLIN;
			break;
	}

 	if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event) == -1) {
 		perror("epoll_ctl failure when adding\n");
 		return -1;
 	}

 	return 0;
}

int ep_remove(const int efd, const int fd) {

	struct epoll_event dummy;

	if (epoll_ctl(efd, EPOLL_CTL_DEL, fd, &dummy) == -1) {
		perror("epoll_ctl failure when removing\n");
		return -1;
	}

	return 0;
}
