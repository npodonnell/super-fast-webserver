#pragma once

#define HTTP_VERB_GET         0
#define HTTP_VERB_POST        1

#define HTTP_VERSION_10       10
#define HTTP_VERSION_11       11

#define PATH_BUFFER_SIZE      1024

typedef struct http_request {
	int version;
	int verb;
	char path[PATH_BUFFER_SIZE];
	int content_length;
} http_request;


typedef struct http_response {
	int version;
	int status_code;
	int content_length;
} http_response;

int http_parse_request(const char* buf, http_request* request);
int http_format_response(const http_response* response, char* buf);
