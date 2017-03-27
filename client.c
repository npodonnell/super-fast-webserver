#include <unistd.h>
#include <stdio.h>

#include "client.h"
#include "client_handlers.h"

client_retval client_init(client* client, const int client_fd) {

	// minimal reset for a client - reset other fields later
	// when & if we get past the reading stage
	client->socket = client_fd;
	client->stage = CLIENT_STAGE_READING_REQUEST;
	client->nbytes = 0;

	return CLIENT_RETVAL_OK;
}

client_retval client_close(client* client) {

	if (client->stage == CLIENT_STAGE_WRITING_CONTENT)
		close(client->file);

	close(client->socket);
	client->stage = CLIENT_STAGE_EMPTY;

	return CLIENT_RETVAL_OK;
}

client_retval client_read(client* client) {
	switch(client->stage) {
	case CLIENT_STAGE_READING_REQUEST:
		printf("CLIENT_STAGE_READING_REQUEST handler for %d\n", client->socket);
		return client_handle_reading_request(client);

	case CLIENT_STAGE_READING_CONTENT:
		printf("CLIENT_STAGE_READING_CONTENT handler for %d\n", client->socket);
		return client_handle_reading_content(client);
	}
}

client_retval client_write(client* client) {
	switch(client->stage) {
	case CLIENT_STAGE_WRITING_RESPONSE:
		printf("CLIENT_STAGE_WRITING_RESPONSE handler for %d\n", client->socket);
		return client_handle_writing_response(client);

	case CLIENT_STAGE_WRITING_CONTENT:
		printf("CLIENT_STAGE_WRITING_CONTENT handler for %d\n", client->socket);
		return client_handle_writing_content(client);
	}
}
