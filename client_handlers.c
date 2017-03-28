#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>

#include "client.h"
#include "client_handlers.h"

client_retval client_handle_reading_request(client* client) {

	ssize_t br, bw;
	int fd;
	struct stat st;

	if (client->nbytes == CLIENT_INPUT_BUFFER_SIZE) {
		fprintf(stderr, "client's input buffer is full\n");
		return CLIENT_RETVAL_SHOULD_CLOSE;
	}

	br = read(client->socket, 
		(void*) client->in_buff + client->nbytes,
		 sizeof(client->in_buff) - client->nbytes);

	if (br < 1) {
		fprintf(stderr, "read returned %ld\n", (long)br);
		return CLIENT_RETVAL_SHOULD_CLOSE;
	}

	client->nbytes += br;
	printf("read %ld bytes, nbytes=%ld/%ld\n", (long)br, (long)client->nbytes, (long)CLIENT_INPUT_BUFFER_SIZE);

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

		printf("writing response---\n");
		write(1, client->out_buff, client->payload_size);
		printf("-------------------\n");

		client->file = fd;
		client->nbytes = 0;
		
		bw = write(client->socket, client->out_buff, client->payload_size);

		if (bw == client->payload_size) {
			// likely: entirity of response headers written out, we skip the writing response
			// stage and go directly to writing content stage
			client->stage = CLIENT_STAGE_WRITING_CONTENT;
			return client_handle_writing_content(client);
		} else if (bw < 0) {
			perror("write failed");
			return CLIENT_RETVAL_SHOULD_CLOSE;
		} else {
			client->nbytes += bw;
			client->stage = CLIENT_STAGE_WRITING_RESPONSE;
		}
	}

	have_no_req:
	return CLIENT_RETVAL_OK;
}

client_retval client_handle_reading_content(client* client) {
	return CLIENT_RETVAL_OK;
}

client_retval client_handle_writing_response(client* client) {
	ssize_t bw;

	bw = write(client->socket, client->out_buff + client->nbytes, client->payload_size - client->nbytes);

	if (bw < 0) {
		perror("write failed");
		return CLIENT_RETVAL_SHOULD_CLOSE;
	}
	
	client->nbytes += bw;
	
	if (client->nbytes == client->payload_size) {
		client->stage = CLIENT_STAGE_WRITING_CONTENT;
		client->nbytes = 0;
		return client_handle_writing_content(client);
	}

	return CLIENT_RETVAL_OK;
}

client_retval client_handle_writing_content(client* client) {
	ssize_t bw;

	http_response* response = &client->response;
	bw = sendfile(client->socket, client->file, 0, response->content_length);

	if (bw < 0) {
		perror("write failed");
		return CLIENT_RETVAL_SHOULD_CLOSE;
	}

	client->nbytes += bw;
	
	if (client->nbytes == response->content_length) {
		// successfully completed request
		printf("successfully completed request\n");
		return CLIENT_RETVAL_SHOULD_CLOSE;
	}	

	return CLIENT_RETVAL_OK;
}

