#include "server.h"

#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "handler.h"
#include "logger.h"

#define MAX_THREADS 10

void send_http_response(int client_socket, int status_code, const char* content_type, 
                       const char* body, size_t body_length) {
    char header[512];
    const char* status_text;
    
    switch(status_code) {
        case 200: status_text = "OK"; break;
        case 404: status_text = "Not Found"; break;
        case 500: status_text = "Internal Server Error"; break;
        default: status_text = "Unknown"; break;
    }
    
    sprintf(header, "HTTP/1.1 %d %s\r\n"
                   "Content-Type: %s\r\n"
                   "Content-Length: %zu\r\n"
                   "Connection: close\r\n"
                   "Server: C89-HTTP-Server/1.0\r\n"
                   "\r\n",
                   status_code, status_text, content_type, body_length);
    
    send(client_socket, header, strlen(header), 0);
    if (body && body_length > 0) {
        send(client_socket, body, body_length, 0);
    }
}

void* client_thread(void* arg) {
    client_info_t* client_info = (client_info_t*)arg;
    
    /*printf("Neue Verbindung von Client [Thread: %lu]\n", pthread_self());*/
    
    handle_request(client_info->client_socket);
    
    close(client_info->client_socket);
    free(client_info);
    
    /*printf("Client-Verbindung geschlossen [Thread: %lu]\n", pthread_self());*/
    
    return NULL;
}

server_info server;

void signal_handler(int sig) {
    printf("server is about to exit...\n");
    server.running = 0;
    close(server.socket);
}

int start_server(int port) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    int client_socket;
    pthread_t thread_id;
    client_info_t* client_info;
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    server.running = 1;
    server.socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server.socket == -1) {
        perror("socket creation failed");
        exit(1);
    }
    
    int opt = 1;
    if (setsockopt(server.socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server.socket);
        exit(1);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server.socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server.socket);
        exit(1);
    }
    
    if (listen(server.socket, MAX_THREADS) < 0) {
        perror("listen fehlgeschlagen");
        close(server.socket);
        exit(1);
    }
    
    printf("HTTP server started on http://localhost:%d\n", port);
    printf("Max. Threads: %d\n", MAX_THREADS);
    printf("press Ctrl+C to exit\n\n");
    
    while (server.running) {
        client_len = sizeof(client_addr);
        client_socket = accept(server.socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            if (server.running) {
                perror("Accept failed");
            }
            continue;
        }
        
        client_info = malloc(sizeof(client_info_t));
        if (!client_info) {
            logger("memory allocation failed.");
            close(client_socket);
            continue;
        }
        
        client_info->client_socket = client_socket;
        client_info->client_addr = client_addr;
        
        if (pthread_create(&thread_id, NULL, client_thread, client_info) != 0) {
            perror("thread creation failed");
            close(client_socket);
            free(client_info);
            continue;
        }
        
        pthread_detach(thread_id);
    }
    
    close(server.socket);
    printf("server exited.");
    
    return 0;
}