#include <unistd.h>
#include <stdio.h>
#include "client.h"

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
	// bytes read
	int br;

	switch(client->stage) {
		case CLIENT_STAGE_READING_REQUEST:
			printf(" CLIENT_STAGE_READING_REQUEST\n");

			if (client->nbytes == CLIENT_INPUT_BUFFER_SIZE) {
				fprintf(stderr, "client's input buffer is full\n");
				return CLIENT_RETVAL_SHOULD_CLOSE;
				break;
			}

			br = read(client->socket, 
				(void*) client->in_buff + client->nbytes,
				 sizeof(client->in_buff) - client->nbytes);

			// TODO - handle error (0 or -1)

			client->nbytes += br;
			printf("read %d bytes, nbytes=%d/%d\n", br, client->nbytes, CLIENT_INPUT_BUFFER_SIZE);

			// scan for a \n\n or \r\n\r\n
			char* scanner = client->in_buff;
			char* last_char = client->in_buff + client->nbytes - 1;

			while(1) {
				while (scanner < last_char && *scanner != '\n')
					scanner++;

				if (scanner == last_char)
					goto have_no_req;

				if (client->in_buff < scanner && 
					scanner + 2 <= last_char &&
					*(scanner - 1) == '\r' && 
					*(scanner + 1) == '\r' &&
					*(scanner + 2) == '\n') {
					client->content_start = scanner + 3;
					goto have_req;
				}

				if (*(scanner + 1) == '\n') {
					client->content_start = scanner + 2;
					goto have_req;
				}

				scanner++;
			}

			have_req:

			printf("read request---\n");
			write(1, client->in_buff, client->content_start - client->in_buff);
			printf("---------------\n");

			// parse the request
			http_parse_request(client->in_buff, &client->request);

			switch(client->request.verb) {
				case HTTP_VERB_GET:
				break;

				case HTTP_VERB_POST:
				break;
			}


			have_no_req:
			break;

		case CLIENT_STAGE_READING_CONTENT:
			printf(" CLIENT_STAGE_READING_CONTENT\n");
			break;
	}

	return CLIENT_RETVAL_OK;
}

client_retval client_write(client* client) {
	return CLIENT_RETVAL_OK;
}
