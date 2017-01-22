#include <unistd.h>
#include "client.h"

void client_reset(const int efd, const int client_fd, client* client) {
	// minimal reset for a client - reset other fields later
	// when & if we get past the reading stage
	client->socket = client_fd;
	client->stage = CLIENT_STAGE_READING;
	client->bytes_read = 0;
}

void client_close(const int efd, const client* client) {
	close(client->socket);
}

void client_event(const client* client) {

}
