#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <strings.h>

// TODO: dynamically allocate memory for epoll events based on
// the number of active file descriptors
#define MAX_EPOLL_EVENTS 10

const char* LISTEN_ADDR = "0.0.0.0";
const int LISTEN_PORT = 8085;
const char* CONTENT_DIR = "./content";


void serve(const char* listen_addr, const int listen_port, const char* content_dir) {

	// create a non-blocking listener socket
	int listener = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, IPPROTO_TCP);

	if (listener == -1) {
		fprintf(stderr, "failure to make socket");
		return;
	}


	// bind
	struct sockaddr_in local_ep;

	local_ep.sin_family = AF_INET;
	local_ep.sin_port = htons(listen_port);
	inet_aton(listen_addr, (struct in_addr*)&local_ep.sin_addr.s_addr);
	bzero(&local_ep.sin_zero, sizeof(local_ep.sin_zero));

	if (bind(listener, (struct sockaddr*) &local_ep, sizeof(local_ep)) != 0) {
		fprintf(stderr, "failure to bind");
		return;
	}

	// listen
	if (listen(listener, 1) != 0) {
		fprintf(stderr, "failure to listen");
		return;
	}


	// create epoll instance
	int efd = epoll_create1(0);

	if (efd == -1) {
		fprintf(stderr, "epoll_create1 failure");
		return;
	}

	// register listener with epoll instance
	struct epoll_event events[MAX_EPOLL_EVENTS];
	struct epoll_event event;
	event.data.fd = listener;
	event.events = EPOLLIN|EPOLLOUT|EPOLLET;

 	if (epoll_ctl(efd, EPOLL_CTL_ADD, listener, &event) == -1) {
 		fprintf(stderr, "epoll_ctl failure");
 		return;
 	}

	printf("serving files from %s on %s:%d\n", content_dir, listen_addr, listen_port);

	while (1) {
		int n = epoll_wait(efd, events, MAX_EPOLL_EVENTS, -1);

		for (int i = 0; i < n; i++) {

		}
	}
}

int main() {
	printf("non-blocking event-based HTTP server\n");
	printf("Noel P. O'Donnell, 2017\n");

	serve(LISTEN_ADDR, LISTEN_PORT, CONTENT_DIR);
	fprintf(stderr, "\n");
}
