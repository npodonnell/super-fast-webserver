#include <unistd.h>
#include <stdio.h>
#include "client.h"
#include "ep.h"

int client_init(const int efd, const int client_fd, client* client) {
	// minimal reset for a client - reset other fields later
	// when & if we get past the reading stage
	client->socket = client_fd;
	client->stage = CLIENT_STAGE_READING_HEADERS;
	client->nbytes = 0;

	if (ep_add(efd, client_fd, EP_ROLE_CLIENT) == -1) {
		perror("ep_add failed\n");
		return -1;
	}

	return 0;
}

void client_close(const int efd, client* client) {

	printf("closing client %d\n", client->socket);

	if (client->stage == CLIENT_STAGE_WRITING_CONTENT)
		close(client->file);

	ep_remove(efd, client->socket);
	close(client->socket);

	client->stage = CLIENT_STAGE_EMPTY;
}

void client_event(const client* client, const int event_type) {

	switch(event_type) {
		case CLIENT_EVENT_READ_SOCKET:
			printf("CLIENT_EVENT_READ_SOCKET\n");
			break;

		case CLIENT_EVENT_WRITE_SOCKET:
			printf("CLIENT_EVENT_WRITE_SOCKET\n");
			break;

		case CLIENT_EVENT_READ_FILE:
			printf("CLIENT_EVENT_READ_FILE\n");
			break;
	}
}
