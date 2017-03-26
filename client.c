#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

	// bytes read, bytes written
	ssize_t br, bw;

	int fd;
	struct stat st;

	switch(client->stage) {
	case CLIENT_STAGE_READING_REQUEST:
		printf("CLIENT_STAGE_READING_REQUEST handler for %d\n", client->socket);

		if (client->nbytes == CLIENT_INPUT_BUFFER_SIZE) {
			fprintf(stderr, "client's input buffer is full\n");
			return CLIENT_RETVAL_SHOULD_CLOSE;
		}

		br = read(client->socket, 
			(void*) client->in_buff + client->nbytes,
			 sizeof(client->in_buff) - client->nbytes);

		if (br < 1) {
			fprintf(stderr, "read returned %d\n", br);
			return CLIENT_RETVAL_SHOULD_CLOSE;
		}

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

		if (http_parse_request(client->in_buff, &client->request) != 0) {
			// client sent an unparsable request- don't send a response
			// just close instead.
			return CLIENT_RETVAL_SHOULD_CLOSE;
		}

		http_request* request = &client->request;
		http_response* response = &client->response;

		response->version = request->version;

		switch(request->verb) {
		case HTTP_VERB_GET:

			//client->stage = CLIENT_STAGE_WRITING_RESPONSE;

			fd = open(request->path, O_RDONLY);

			if (fd == -1) {
				// cannot open - user will see a 404 no matter what
				perror("failed to open file");
				response->status_code = 404;
				response->content_length = 0;
				break;
			}

			if (fstat(fd, &st) != 0) {
				// shouldn't get here. TODO: investigate if we can remove this check
				perror("fstat failed");
				close(fd);
				response->status_code = 404;
				response->content_length = 0;
				break;
			}

			// TODO - put another check here to make sure file really is a file
			// and can be read

			printf("size of %s is %d\n",request->path, (int)st.st_size);

			response->status_code = 200;
			response->content_length = st.st_size;

			client->payload_size = http_format_response(response, client->out_buff);

			printf("------\n");
			printf("%s", client->out_buff);
			printf("------\n");
/*
			client->file = fd;
			client->nbytes = 0;
*/			
			bw = write(client->socket, client->out_buff, client->payload_size);

			// TODO - check bw

			bw = sendfile(client->socket, fd, 0, response->content_length);

			break;

		case HTTP_VERB_POST:
			break;
		}


		have_no_req:
		break;

	case CLIENT_STAGE_READING_CONTENT:
		printf("CLIENT_STAGE_READING_CONTENT handler for %d\n", client->socket);
		break;
	}

	return CLIENT_RETVAL_OK;
}

client_retval client_write(client* client) {

	// bytes read, bytes written
	ssize_t br, bw;

	switch(client->stage) {
	case CLIENT_STAGE_WRITING_RESPONSE:
		printf("CLIENT_STAGE_WRITING_RESPONSE handler for %d\n", client->socket);
		break;

	case CLIENT_STAGE_WRITING_CONTENT:
		printf("CLIENT_STAGE_WRITING_CONTENT handler for %d\n", client->socket);
		break;
	}

	return CLIENT_RETVAL_OK;
}
