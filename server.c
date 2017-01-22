#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "server.h"
#include "client.h"
#include "ep.h"

int shutting_down = 0;

int make_listener_socket() {

	int listener = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, IPPROTO_TCP);

	if (listener == -1)
		return -1;

	// set to reuse addr
	int optval = 1;
	
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof optval) == -1)
		return -1;

	return listener;
}

int bind_and_listen(const int listener, const char* listen_addr, const int listen_port, const int listen_backlog) {

	// bind
	struct sockaddr_in local_ep;
	local_ep.sin_family = AF_INET;
	local_ep.sin_port = htons(listen_port);
	inet_aton(listen_addr, (struct in_addr*)&local_ep.sin_addr.s_addr);
	bzero(&local_ep.sin_zero, sizeof local_ep.sin_zero);

	if (bind(listener, (struct sockaddr*) &local_ep, sizeof local_ep) != 0) {
		fprintf(stderr, "failure to bind\n");
		return -1;
	}

	// listen
	if (listen(listener, listen_backlog) != 0) {
		fprintf(stderr, "failure to listen\n");
		return -1;
	}

	return 0;
}

int get_listener(const char* listen_addr, const int listen_port, const int listen_backlog) {

	int listener = make_listener_socket();

	if (bind_and_listen(listener, listen_addr, listen_port, listen_backlog) == -1) {
		fprintf(stderr, "bind_and_listen failed\n");
		return -1;
	}

	return listener;
}

void serve(const char* listen_addr, const int listen_port, const int listen_backlog, const int max_clients, const char* content_dir) {

	// TODO - chroot into content dir

	// change into content directory
	if (chdir(content_dir) == -1) {
		fprintf(stderr, "failure to change into content directory\n");
		return;
	}

	// create a non-blocking listener socket
	int listener = get_listener(listen_addr, listen_port, listen_backlog);

	if (listener == -1) {
		fprintf(stderr, "failure to make socket\n");
		return;
	}

 	// max possible events is 1 for the listener plus 2 per client
	int max_epoll_events = 1 + (max_clients * 2);

	// create epoll instance
	int efd = ep_init(max_epoll_events);

	if (efd == -1) {
		fprintf(stderr, "failure to create epoll instance\n");
		return;
	}

	// add the listener
	ep_add(efd, listener, EP_ROLE_LISTENER);

 	// client pool
 	client* client_pool = (client*) malloc(sizeof(client) * max_clients);

 	// file descriptor -> client lookup array
	client** fd_to_client = (client**) malloc(sizeof(client*) * (efd + max_epoll_events));

	// epoll events array
	struct epoll_event* events = (struct epoll_event*) malloc(sizeof(struct epoll_event) * max_epoll_events);

	printf("serving files from %s on %s:%d\n", content_dir, listen_addr, listen_port);

 	int most_clients = 0;
 	int nclients = 0;

	while (!shutting_down) {
		printf("epolling...\n");
		int n = epoll_wait(efd, events, max_epoll_events, -1);

		for (int i = 0; i < n; i++) {

			int fd = events[i].data.fd;

			if (fd == listener) {
				// new client

				// TODO - get client's address
				int client_fd = accept(listener, 0, 0);

				printf("new client: %d\n", client_fd);

				if (nclients == max_clients) {
					// enough clients already
					// TODO - instead shut down the listener so we don't have to deal with
					// requests we can't serve
					close(client_fd);
					continue;
				}

				nclients++;
				client* the_client;

				if (most_clients < nclients) {
					the_client = client_pool + most_clients;
					fd_to_client[client_fd] = the_client;
					most_clients = nclients;
				} else {
					// re-used client
					the_client = fd_to_client[client_fd];
				}

				client_reset(efd, client_fd, the_client);
				ep_add(efd, client_fd, EP_ROLE_CLIENT);

			} else {
				// existing client

				int client_fd = fd;
				client* the_client = fd_to_client[client_fd];

				if (events[i].events & EPOLLRDHUP) {
					// client hung up
					ep_remove(efd, client_fd);
					client_close(efd, the_client);
					nclients--;
					printf("Gone client fd=%d, most_clients=%d nclients=%d\n", client_fd, most_clients, nclients);

				} else if (events[i].events & EPOLLIN) {
					// read from client
					printf("epollin event from %d\n", client_fd);
				} else if (events[i].events & EPOLLOUT) {
					// write to client
					printf("epollout event from %d\n", client_fd);
				}
			}
		}
		printf("------------------------------------\n");
	}

	free(events);
	free(fd_to_client);
	free(client_pool);
}
