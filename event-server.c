#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

#include "event-server.h"

// max number of clients we can serve simultaneously
#define MAX_CLIENTS 5

// max possible number of events ready for I/O after
// an epoll_wait
#define MAX_EPOLL_EVENTS 1 + MAX_CLIENTS * 2

const char* LISTEN_ADDR = "0.0.0.0";
const int LISTEN_PORT = 8080;
const char* CONTENT_DIR = "./content";

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

void serve(const char* listen_addr, const int listen_port, const char* content_dir) {

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
	event.events = EPOLLIN|EPOLLET;

 	if (epoll_ctl(efd, EPOLL_CTL_ADD, listener, &event) == -1) {
 		fprintf(stderr, "epoll_ctl failure");
 		return;
 	}

 	int max_client_index = -1;
 	int nclients = 0;
 	client clients[MAX_CLIENTS];
	client* fd_client_map[efd + (MAX_CLIENTS * 2)];

	bzero(&fd_client_map, sizeof fd_client_map);

	printf("serving files from %s on %s:%d\n", content_dir, listen_addr, listen_port);

	while (1) {
		printf("polling...\n");
		int n = epoll_wait(efd, events, MAX_EPOLL_EVENTS, -1);
		printf("n=%d\n", n);

		for (int i = 0; i < n; i++) {

			printf(" i=%d fd=%d\n", i ,events[i].data.fd);

			if (events[i].data.fd == listener) {
				// new client

				int client_fd = accept(listener, 0, 0);
				client* the_client = fd_client_map[client_fd];

				if (!the_client) {
					printf("using a fresh client\n");
					the_client = &clients[++max_client_index];
					fd_client_map[client_fd] = the_client;
				}

				// reset the client object
				the_client->in_buff_pos = 0;

				// add client_fd to the epoll instance
				event.data.fd = client_fd;
				event.events = EPOLLIN|EPOLLOUT|EPOLLRDHUP|EPOLLET;
				epoll_ctl(efd, EPOLL_CTL_ADD, client_fd, &event);

				nclients++;
				printf("Accepted connection (fd=%d, max_client_index=%d nclients=%d)\n", client_fd, max_client_index, nclients);

			} else {
				// existing client

				int client_fd = events[i].data.fd;
				client* the_client = fd_client_map[client_fd];

				if (events[i].events & EPOLLIN) {
					printf("epollin event from %d\n", client_fd);

					ssize_t br = read(client_fd, the_client->in_buff + the_client->in_buff_pos, sizeof the_client->in_buff - the_client->in_buff_pos);
					printf("read %u bytes from client_fd %d\n", (unsigned int)br, client_fd);
					the_client->in_buff_pos += br;

					the_client->in_buff[the_client->in_buff_pos] = '\0';
					printf("buffer now contains: %s", the_client->in_buff);
				}

				if (events[i].events & EPOLLOUT) {
					printf("epollout event from %d\n", client_fd);
				}

				if (events[i].events & EPOLLRDHUP) {
					printf("epollrdhup event from %d\n", client_fd);

					// remove client_fd from the epoll instance and close the socket
					epoll_ctl(efd, EPOLL_CTL_DEL, client_fd, &event);
					close(client_fd);

					nclients--;
					printf("Client Disconnected (fd=%d, max_client_index=%d nclients=%d)\n", client_fd, max_client_index, nclients);
				}
			}
		}
	}
}

int main() {
	printf("non-blocking event-based HTTP server\n");
	printf("Noel P. O'Donnell, 2017\n");

	serve(LISTEN_ADDR, LISTEN_PORT, CONTENT_DIR);
	fprintf(stderr, "\n");
}
