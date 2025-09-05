#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int socket;
    int running;
} server_info;

typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
} client_info_t;

int start_server(int port);

void send_http_response(int client_socket, int status_code, const char* content_type, 
                       const char* body, size_t body_length);

#endif