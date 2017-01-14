#pragma once

#define CLIENT_INPUT_BUFFER_SIZE 1000

#define STAGE_HEADERS 0
#define STAGE_BODY 1


typedef struct client {

	// input buffer
	char in_buff[CLIENT_INPUT_BUFFER_SIZE];

	// pointer to character after the last
	// byte read into in_buff
	char* in_buff_term;

	// which stage of the request this client is currently in (see above)
	int stage;

} client;

void RESET_CLIENT(client* client);


