#include <unistd.h>
#include <stdio.h>
#include "client.h"
#include "ep.h"

int client_init(const int efd, const int client_fd, client* client) {
	// minimal reset for a client - reset other fields later
	// when & if we get past the reading stage
	client->socket = client_fd;
	client->stage = CLIENT_STAGE_READING_REQUEST;
	client->nbytes = 0;

	if (ep_add(efd, client_fd, EP_ROLE_CLIENT) == -1) {
		perror("ep_add failed\n");
		return -1;
	}

	return 0;
}

void client_close(const int efd, client* client) {

	printf("closing client %d\n", client->socket);

	if (client->stage == CLIENT_STAGE_WRITING_CONTENT)
		close(client->file);

	ep_remove(efd, client->socket);
	close(client->socket);

	client->stage = CLIENT_STAGE_EMPTY;
}

void client_event(client* client, const int event_type) {

	switch(event_type) {
		case CLIENT_EVENT_READ_SOCKET:
			printf("CLIENT_EVENT_READ_SOCKET\n");

			switch(client->stage) {

				// bytes read
				int br;

				case CLIENT_STAGE_READING_REQUEST:
					printf(" CLIENT_STAGE_READING_REQUEST\n");

					br = read(client->socket, 
						(void*) client->in_buff + client->nbytes,
						 sizeof(client->in_buff) - client->nbytes);

					// TODO - handle error (0 or -1)

					client->nbytes += br;
					printf("read %d bytes, nbytes=%d/%d\n", br, client->nbytes, CLIENT_INPUT_BUFFER_SIZE);

					if (client->nbytes == CLIENT_INPUT_BUFFER_SIZE) {
						fprintf(stderr, "client's input buffer is full\n");

						// TODO - close client
						break;
					}

					// scan for a \n\n
					char* scanner = client->in_buff;
					char* last_char = client->in_buff + client->nbytes - 1;

					while(1) {
						while (scanner < last_char && *scanner != '\n')
							scanner++;

						if (scanner == last_char)
							goto noheaders;

						if (*(scanner + 1) == '\n')
							goto headers;

						scanner++;
					}

					headers:

					//client->stage = CLIENT_STAGE_READING_CONTENT;
					client->content_start = scanner + 2;

					printf("read request---\n");
					write(1, client->in_buff, client->content_start - client->in_buff);
					printf("---------------\n");

					// TODO - parse the request



					noheaders:
					break;

				case CLIENT_STAGE_READING_CONTENT:
					printf(" CLIENT_STAGE_READING_CONTENT\n");
					break;
			}

			break;

		case CLIENT_EVENT_WRITE_SOCKET:
			printf("CLIENT_EVENT_WRITE_SOCKET\n");
			break;
	}
}
