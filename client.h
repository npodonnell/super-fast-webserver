#pragma once

#define CLIENT_INPUT_BUFFER_SIZE 4096
#define CLIENT_OUTPUT_BUFFER_SIZE 4096

#define STAGE_READING 0
#define STAGE_HEADERS 1
#define STAGE_CONTENT 2


typedef struct client {

	// input buffer
	char in_buff[CLIENT_INPUT_BUFFER_SIZE];
	char* in_buff_term;

	// pointer to output
	char out_buff[CLIENT_OUTPUT_BUFFER_SIZE];
	char* out_buff_term;
	char* out_buff_cursor;

	// which stage of the request this client is currently in (see above)
	int stage;

} client;

void RESET_CLIENT(client* client);


