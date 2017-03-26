#pragma once

#include "http.h"

#define CLIENT_INPUT_BUFFER_SIZE             4096
#define CLIENT_OUTPUT_BUFFER_SIZE            4096

#define CLIENT_STAGE_EMPTY                   0       // client object is free
#define CLIENT_STAGE_READING_REQUEST         1       // reading request from client
#define CLIENT_STAGE_READING_CONTENT         2       // reading request from client
#define CLIENT_STAGE_WRITING_RESPONSE        3       // writing HTTP headers to the client
#define CLIENT_STAGE_WRITING_CONTENT         4       // writing content to the client

#define CLIENT_RETVAL_OK                     0       // ok
#define CLIENT_RETVAL_FAILURE                1       // general failure
#define CLIENT_RETVAL_SHOULD_CLOSE           2       // something happened that means the client should be closed

// See above
typedef int client_retval;

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
	// only has meaning when stage is CLIENT_STAGE_WRITING_CONTENT
	int file;

	// number of bytes so-far written or read, depending on stage
	int nbytes;

	// pointer in in_buff where content starts
	char* content_start;

	// http request and response
	http_request request;
	http_response response;

} client;

client_retval client_init(client* client, const int client_fd);
client_retval client_close(client* client);
client_retval client_read(client* client);
client_retval client_write(client* client);
