#pragma once

int make_listener_socket();
char* extract_pathname(const char* buf, const char* buff_term);
void serve(const char* listen_addr, const int listen_port, const int listen_backlog, const int max_clients, const char* content_dir);
