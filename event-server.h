#pragma once

#define CLIENT_INPUT_BUFFER_SIZE 1000

typedef struct client {
	char in_buff[CLIENT_INPUT_BUFFER_SIZE];
	int in_buff_pos;
} client;

