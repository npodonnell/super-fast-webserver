#include <unistd.h>
#include <stdio.h>
#include "client.h"
#include "ep.h"

void client_init(const int efd, const int client_fd, client* client) {
	// minimal reset for a client - reset other fields later
	// when & if we get past the reading stage
	client->socket = client_fd;
	client->stage = CLIENT_STAGE_READING;
	client->nbytes = 0;
	ep_add(efd, client_fd, EP_ROLE_CLIENT);
}

void client_close(const int efd, const client* client) {

	if (client->stage == CLIENT_STAGE_CONTENT)
		close(client->file);

	ep_remove(efd, client->socket);
	close(client->socket);
}

void client_event(const client* client, const int event_type) {

	switch(event_type) {
		case CLIENT_EVENT_READ_SOCKET:
			break;

		case CLIENT_EVENT_WRITE_SOCKET:
			break;

		case CLIENT_EVENT_READ_FILE:
			break;
	}
}
