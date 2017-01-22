#pragma once

extern int shutting_down;

int make_listener_socket();
int bind_and_listen(const int listener, const char* listen_addr, const int listen_port, const int listen_backlog);
int get_listener(const char* listen_addr, const int listen_port, const int listen_backlog);
char* extract_pathname(const char* buf, const char* buff_term);
void serve(const char* listen_addr, const int listen_port, const int listen_backlog, const int max_clients, const char* content_dir);void serve(const char* listen_addr, const int listen_port, const int listen_backlog, const int max_clients, const char* content_dir);
