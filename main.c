#include <stdio.h>
#include "server.h"

#define LISTEN_ADDR "0.0.0.0"
#define LISTEN_PORT 8080
#define LISTEN_BACKLOG 50
#define MAX_CLIENTS 5
#define CONTENT_DIR "./content"

int main() {
	printf("non-blocking event-based HTTP server\n");
	printf("Noel P. O'Donnell, 2017\n");

	serve(LISTEN_ADDR, LISTEN_PORT, LISTEN_BACKLOG, MAX_CLIENTS, CONTENT_DIR);
	fprintf(stderr, "\n");
}
