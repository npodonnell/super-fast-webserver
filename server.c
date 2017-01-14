#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#include "server.h"
#include "client.h"

// temporary!!!
#define RESPONSE "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 5\n\nHello"


inline int make_listener_socket() {

	int listener = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, IPPROTO_TCP);

	if (listener == -1)
		return -1;

	// set to reuse addr
	int optval = 1;
	
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof optval) == -1)
		return -1;

	return listener;
}

inline char* extract_pathname(const char* buf, const char* buff_term) {

	// stupidly-simple Http-(ish) request parser, looks for a 'GET ' then if so,
	// reads characters until the next space and returns that

	if (buff_term - buf < 6)
		return 0;

	if (strncmp("GET ", buf, 4) != 0)
		return 0;

	char* start = (char*)buf + 4;
	char* end = start;

	while (end < buff_term) {
		end++;

		if (*end == ' ') {
			*end = '\0';
			return start;
		}
	}

	return 0;
}

void serve(const char* listen_addr, const int listen_port, const int listen_backlog, const int max_clients, const char* content_dir) {

	// change into content dir
	if (chdir(content_dir) == -1) {
		fprintf(stderr, "failure to change into content directory");
		return;
	}

	// create a non-blocking listener socket
	int listener = make_listener_socket();

	if (listener == -1) {
		fprintf(stderr, "failure to make socket");
		return;
	}

	// bind
	struct sockaddr_in local_ep;
	local_ep.sin_family = AF_INET;
	local_ep.sin_port = htons(listen_port);
	inet_aton(listen_addr, (struct in_addr*)&local_ep.sin_addr.s_addr);
	bzero(&local_ep.sin_zero, sizeof local_ep.sin_zero);

	if (bind(listener, (struct sockaddr*) &local_ep, sizeof local_ep) != 0) {
		fprintf(stderr, "failure to bind");
		return;
	}

	// listen
	if (listen(listener, listen_backlog) != 0) {
		fprintf(stderr, "failure to listen");
		return;
	}

	// create epoll instance
	int efd = epoll_create1(0);

	if (efd == -1) {
		fprintf(stderr, "epoll_create1 failure");
		return;
	}

	// re-usable epoll_event object
	struct epoll_event event;

	// add listener to the epoll instance
	event.data.fd = listener;
	event.events = EPOLLIN|EPOLLET;
 	if (epoll_ctl(efd, EPOLL_CTL_ADD, listener, &event) == -1) {
 		fprintf(stderr, "epoll_ctl failure");
 		return;
 	}

 	// client pool
 	client* clients = (client*) malloc(sizeof(client) * max_clients);

 	// max possible events is 1 for the listener plus 2 per client
	int max_epoll_events = 1 + (max_clients * 2);

 	// file descriptor -> client lookup array
	client** fd_to_client = (client**) malloc(sizeof(client*) * (efd + max_epoll_events));

	// epoll events array
	struct epoll_event* events = (struct epoll_event*) malloc(sizeof(struct epoll_event) * max_epoll_events);

	printf("serving files from %s on %s:%d\n", content_dir, listen_addr, listen_port);

 	int most_clients = 0;
 	int nclients = 0;

	while (1) {
		printf("epolling...\n");
		int n = epoll_wait(efd, events, max_epoll_events, -1);
		printf("n=%d\n", n);

		for (int i = 0; i < n; i++) {

			int fd = events[i].data.fd;

			printf(" i=%d fd=%d\n", i , fd);

			if (fd == listener) {
				// new client

				// TODO - get client's address
				int client_fd = accept(listener, 0, 0);

				if (nclients == max_clients) {
					// enough clients already
					close(client_fd);
					continue;
				}

				client* the_client = fd_to_client[client_fd];

				if (most_clients < max_clients) {
					printf("using a fresh client\n");
					the_client = clients + (most_clients++);
					fd_to_client[client_fd] = the_client;
				}

				// reset the client object
				RESET_CLIENT(the_client);

				// add client_fd to the epoll instance
				event.data.fd = client_fd;
				event.events = EPOLLIN|EPOLLOUT|EPOLLRDHUP|EPOLLET;
				epoll_ctl(efd, EPOLL_CTL_ADD, client_fd, &event);

				nclients++;
				printf("New client fd=%d, most_clients=%d nclients=%d\n", client_fd, most_clients, nclients);

			} else {
				// existing client

				int client_fd = fd;
				client* the_client = fd_to_client[client_fd];

				if (events[i].events & EPOLLRDHUP) {
					printf("epollrdhup event from %d\n", client_fd);

					// remove client_fd from the epoll instance and close the socket
					// event parameter is ignored here
					//epoll_ctl(efd, EPOLL_CTL_DEL, client_fd, &event);
					close(client_fd);

					nclients--;
					printf("New client fd=%d, most_clients=%d nclients=%d\n", client_fd, most_clients, nclients);

				} else if (events[i].events & EPOLLIN) {
					printf("epollin event from %d\n", client_fd);

					int buff_left = sizeof the_client->in_buff - (the_client->in_buff_term - the_client->in_buff);

					ssize_t br = read(client_fd, the_client->in_buff_term, buff_left);
					printf("read %u bytes from client_fd %d\n", (unsigned int)br, client_fd);
					the_client->in_buff_term += br;

					// attempt to extract a path from what was read
					char* pathname = extract_pathname(the_client->in_buff, the_client->in_buff_term);

					if (pathname) {
						printf("  got pathname '%s'\n", pathname);

						int file_fd = open(pathname, O_RDONLY);

						if (file_fd == -1) {
							printf("   failed to open\n");
						} else {
							printf("   opened\n");

							ssize_t bw = write(client_fd, RESPONSE, sizeof RESPONSE);
							printf("wrote %u/%lu bytes from client_fd %d\n", (unsigned int)bw, sizeof RESPONSE, client_fd);
							
							// TODO: fix the response

							close(file_fd);
						}
					}
				} else if (events[i].events & EPOLLOUT) {
					printf("epollout event from %d\n", client_fd);
				}
			}
		}
	}
}
