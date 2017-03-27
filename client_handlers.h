#pragma once

#include "client.h"

client_retval client_handle_reading_request(client* client);
client_retval client_handle_reading_content(client* client);
client_retval client_handle_writing_response(client* client);
client_retval client_handle_writing_content(client* client);