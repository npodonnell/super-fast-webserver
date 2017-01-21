#include "client.h"

// TODO - make these macros

void client_reset(const client* client) {
	client->stage = STAGE_READING;
	client->bytes_read = 0;
}

void client_event(const client* client) {

}

void client_close(const client* client) {

}
