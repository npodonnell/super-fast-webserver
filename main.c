#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "server.h"

#define LISTEN_ADDR "0.0.0.0"
#define LISTEN_PORT 8080
#define LISTEN_BACKLOG 50
#define MAX_CLIENTS 4
#define CONTENT_DIR "./content"

void handle_signal(const int signal) {
	printf("caught signal %d\n", signal);
	shutting_down = 1;
}

int main() {
	printf("non-blocking event-based HTTP server\n");
	printf("Noel P. O'Donnell, 2017\n");
	printf("PID: %d\n", getpid());

	// handle signals
	struct sigaction sa;

	sa.sa_handler = &handle_signal;
	sa.sa_flags = SA_RESTART;
	
	sigfillset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
    	perror("Error: cannot handle SIGINT");
    	return -1;
    }

    if (sigaction(SIGHUP, &sa, NULL) == -1) {
    	perror("Error: cannot handle SIGHUP");
    	return -1;
    }

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
    	perror("Error: cannot handle SIGTERM");
    	return -1;
    }

	serve(LISTEN_ADDR, LISTEN_PORT, LISTEN_BACKLOG, MAX_CLIENTS, CONTENT_DIR);
	printf("Bye\n");
}
