#pragma once

#define CLIENT_INPUT_BUFFER_SIZE 4096
#define CLIENT_OUTPUT_BUFFER_SIZE 4096

#define CLIENT_STAGE_READING 0 // reading request from client
#define CLIENT_STAGE_HEADERS 1 // writing HTTP headers to the client
#define CLIENT_STAGE_CONTENT 2 // writing content to the client


typedef struct client {

	// input buffer
	char in_buff[CLIENT_INPUT_BUFFER_SIZE];

	// output buffer (typically used for storing HTTP headers)
	char out_buff[CLIENT_OUTPUT_BUFFER_SIZE];

	// which stage of the request this client is currently in (see above)
	int stage;

	// TCP socket on which we talk to the client
	int socket;

	// file descriptor for file we're sending to the client.
	// only has meaning when stage is STAGE_CONTENT
	int file;

	// number of bytes which have been read into the input buffer
	int bytes_read;

	// size of the payload in the output buffer
	int out_buff_size;

	// number of bytes so-far written
	int bytes_written;

} client;

void client_reset(client* client, const int client_fd);
void client_event(const client* client);
void client_close(const client* client);

