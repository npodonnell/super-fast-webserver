#include <string.h>
#include <stdio.h>
#include "http.h"


int http_parse_request(const char* buf, http_request* request) {

	// temporary - hard-coding a standard "GET / HTTP/1.1"
	// TODO - implement
	request->version = HTTP_VERSION_11;
	request->verb = HTTP_VERB_GET;

	strcpy(request->path, "/hello");

	request->content_length = 0;

	return 0;
}

int http_format_response(const http_response* response, char* buf) {

	sprintf(buf, "HTTP/%.1f %d\nContent-Length:%d\n\n", 
		response->version / 10.0, 
		response->status_code, 
		response->content_length);

	return strlen(buf);
}
