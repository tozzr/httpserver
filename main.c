#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#include "src/logger.h"
#include "src/file_io.h"
#include "src/string_utils.h"
#include "src/server.h"
#include "src/handler.h"

int server_running = 1;

void signal_handler(int sig) {
    printf("server is about to exit...\n");
    server_running = 0;
    close(server_socket);
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    int client_socket;
    pthread_t thread_id;
    client_info_t* client_info;
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket creation failed");
        exit(1);
    }
    
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_socket);
        exit(1);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_socket);
        exit(1);
    }
    
    if (listen(server_socket, MAX_THREADS) < 0) {
        perror("listen fehlgeschlagen");
        close(server_socket);
        exit(1);
    }
    
    printf("HTTP server started on http://localhost:%d\n", PORT);
    printf("Max. Threads: %d\n", MAX_THREADS);
    printf("press Ctrl+C to exit\n\n");
    
    while (server_running) {
        client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            if (server_running) {
                perror("Accept fehlgeschlagen");
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
    
    close(server_socket);
    printf("server exited.");
    
    return 0;
}