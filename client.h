#pragma once

#define CLIENT_INPUT_BUFFER_SIZE 4096
#define CLIENT_OUTPUT_BUFFER_SIZE 4096

#define STAGE_HEADERS 0
#define STAGE_CONTENT 1


typedef struct client {

	// input buffer
	char in_buff[CLIENT_INPUT_BUFFER_SIZE];
	char* in_buff_term;

	// pointer to output
	char* out_buff[CLIENT_OUTPUT_BUFFER_SIZE];
	char* out_buff_term;

	// which stage of the request this client is currently in (see above)
	int stage;

} client;

void RESET_CLIENT(client* client);


