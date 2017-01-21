#include <sys/epoll.h>
#include "ep.h"

int ep_init() {
	return epoll_create(1);
}


void ep_add(const int efd, const int fd, const int role) {

	struct epoll_event event;

	switch(role) {
		case EP_ROLL_LISTENER:

		break;

		case EP_ROLL_CLIENT:

		break;

		case EP_ROLL_FILE:

		break;
	}
}

void ep_remove(const int efd, const int fd) {

}
