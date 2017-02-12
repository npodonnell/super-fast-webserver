#pragma once

#define CLIENT_INPUT_BUFFER_SIZE 4096
#define CLIENT_OUTPUT_BUFFER_SIZE 4096

#define CLIENT_STAGE_EMPTY 0 // client object is free
#define CLIENT_STAGE_READING_HEADERS 1 // reading request from client
#define CLIENT_STAGE_WRITING_HEADERS 2 // writing HTTP headers to the client
#define CLIENT_STAGE_WRITING_CONTENT 3 // writing content to the client

#define CLIENT_EVENT_READ_SOCKET 0
#define CLIENT_EVENT_WRITE_SOCKET 1
#define CLIENT_EVENT_READ_FILE 2
#define CLIENT_EVENT_HUP 3

typedef struct client {

	// input buffer
	char in_buff[CLIENT_INPUT_BUFFER_SIZE];

	// output buffer (typically used for storing HTTP headers)
	char out_buff[CLIENT_OUTPUT_BUFFER_SIZE];

	// size of the payload in the output buffer
	int payload_size;

	// which stage of the request this client is currently in (see above)
	int stage;

	// TCP socket on which we talk to the client
	int socket;

	// file descriptor for file we're sending to the client.
	// only has meaning when stage is STAGE_CONTENT
	int file;

	// number of bytes so-far written or read, depending on stage
	int nbytes;

} client;

int client_init(const int efd, const int client_fd, client* client);
void client_close(const int efd, client* client);
void client_event(const client* client, const int event_type);

