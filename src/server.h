#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_THREADS 10


int server_socket;

typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
} client_info_t;

void send_http_response(int client_socket, int status_code, const char* content_type, 
                       const char* body, size_t body_length);

void* client_thread(void* arg);

#endif