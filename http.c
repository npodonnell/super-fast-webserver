#include "http.h"

void http_parse_request(const char* buf, http_request* request) {

	request->version = HTTP_VERSION_11;
	request->verb = HTTP_VERB_GET;
	request->path[0] = '/';
	request->path[1] = '\0';
}
