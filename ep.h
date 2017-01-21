#pragma once

#define EP_ROLE_LISTENER 0
#define EP_ROLE_CLIENT 1
#define EP_ROLE_FILE 2

int ep_init(const int max_events);
int ep_add(const int efd, const int fd, const int role);
void ep_remove(const int efd, const int fd);
