#include <unistd.h>
#include "client.h"

// TODO - make these macros

void client_reset(client* client, const int client_fd) {
	// minimal reset for a client - reset other fields later
	// when & if we get past the reading stage
	client->socket = client_fd;
	client->stage = CLIENT_STAGE_READING;
	client->bytes_read = 0;
}

void client_event(const client* client) {

}

void client_close(const client* client) {
	close(client->socket);
}
