#include "client.h"

// TODO - make these macros

void RESET_CLIENT(client* client) {
	client->in_buff_term = client->in_buff;
	client->stage = STAGE_HEADERS;
}


