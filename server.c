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

	if (listener == -1) {
		perror("failed to create non-blocking listener socket");
		return -1;
	}

	// set to reuse addr
	int optval = 1;
	
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof optval) == -1) {
		perror("failed to set listener to SO_REUSEADDR");
		return -1;
	}

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
		perror("listener failed to bind");
		return -1;
	}

	// listen
	if (listen(listener, listen_backlog) != 0) {
		perror("listener failed to listen");
		return -1;
	}

	return 0;
}

int get_listener(const char* listen_addr, const int listen_port, const int listen_backlog) {

	int listener = make_listener_socket();

	if (bind_and_listen(listener, listen_addr, listen_port, listen_backlog) == -1) {
		perror("bind_and_listen failed");
		return -1;
	}

	return listener;
}

void serve(const char* listen_addr, const int listen_port, const int listen_backlog, const int max_clients, const char* content_dir) {

	// TODO - chroot into content dir

	// change into content directory
	if (chdir(content_dir) == -1) {
		perror("failure to change into content directory");
		return;
	}

	// create a non-blocking listener socket
	int listener = get_listener(listen_addr, listen_port, listen_backlog);

	if (listener == -1) {
		perror("failure to get listener socket");
		return;
	}

 	// max possible events is 1 for the listener plus 2 per client
	int max_epoll_events = 1 + (max_clients * 2);

	// create epoll instance
	int efd = ep_init(max_epoll_events);

	if (efd == -1) {
		perror("failure to create epoll instance");
		return;
	}

	// add the listener
	ep_add(efd, listener, EP_ROLE_LISTENER);

 	// client pool
 	client* client_pool = (client*) malloc(sizeof(client) * max_clients);

 	for (int i = 0; i < max_clients; i++)
 		client_pool[i].stage = CLIENT_STAGE_EMPTY;

 	// file descriptor -> client lookup array
	client** fd_to_client = (client**) malloc(sizeof(client*) * (efd + max_epoll_events));

	// epoll events array
	struct epoll_event* events = (struct epoll_event*) malloc(sizeof(struct epoll_event) * max_epoll_events);

	printf("serving files from %s on %s:%d\n", content_dir, listen_addr, listen_port);

 	int most_clients = 0;
 	int nclients = 0;

 	// event loop
	while (!shutting_down) {
		int nevents = epoll_wait(efd, events, max_epoll_events, -1);

		for (int i = 0; i < nevents; i++) {
			int fd = events[i].data.fd;
			if (fd == listener) {
				// new client

				// TODO - get client's address
				int client_fd = accept(listener, 0, 0);

				if (nclients == max_clients) {
					// enough clients already
					// TODO - instead shut down the listener so we don't have to deal with
					// requests we can't serve
					printf("closed accepted connection\n");
					close(client_fd);
					continue;
				}

				nclients++;
				client* client;

				if (most_clients < nclients) {
					// brand-new client
					client = client_pool + most_clients;
					fd_to_client[client_fd] = client;
					most_clients = nclients;
				} else {
					// re-used client
					client = fd_to_client[client_fd];
				}

				// initalize client
				if (client_init(client, client_fd) != CLIENT_RETVAL_OK) {
					client_close(client);
					nclients--;
					continue;
				}

				// add to epoll
				if (ep_add(efd, client_fd, EP_ROLE_CLIENT) == -1) {
					client_close(client);
					nclients--;
					continue;
				}

			} else {
				// existing client
				client* client = fd_to_client[fd];

				if (events[i].events & EPOLLRDHUP) {

					// client hung up
					goto close_client;

				} else if (events[i].events & EPOLLIN) {

					// read from client
					switch(client_read(client)) {
						case CLIENT_RETVAL_SHOULD_CLOSE:
							goto close_client;
					}

				} else if (events[i].events & EPOLLOUT) {

					// write to client
					switch(client_read(client)) {
						case CLIENT_RETVAL_SHOULD_CLOSE:
							goto close_client;
					}
				}

				continue;

				close_client:
				printf("Closing client %d\n", client->socket);
				ep_remove(efd, client->socket);
				client_close(client);
				nclients--;
			}
		}

		printf("nclients=%d\n", nclients);
	}

	// close any active clients
	for (int i = max_clients - 1; -1 < i; i--) {
		if (client_pool[i].stage != CLIENT_STAGE_EMPTY) {
			client* client = client_pool + i;
			ep_remove(efd, client->socket);
			client_close(client);
			nclients--;
		}
	}

	printf("on closing nclients=%d\n", nclients);

	free(events);
	free(fd_to_client);
	free(client_pool);

	close(efd);
	close(listener);
}
