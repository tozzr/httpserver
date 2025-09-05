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

int server_running = 1;

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    char method[16], path[256], version[16];
    char* file_content;
    size_t file_size;
    int bytes_received;
    
    bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        return;
    }
    
    buffer[bytes_received] = '\0';
    
    if (sscanf(buffer, "%15s %255s %15s", method, path, version) != 3) {
        send_http_response(client_socket, 400, "text/plain", "Bad Request", 11);
        return;
    }
    
    if (strcmp(method, "GET") != 0) {
        send_http_response(client_socket, 405, "text/plain", "Method Not Allowed", 18);
        return;
    }
    
    char log_msg[512];
    snprintf(log_msg, sizeof(log_msg), "%s", path);
    logger(log_msg);

    if (strcmp(path, "/") == 0) {
        strcpy(path, "index.html");
    }
    
    if (path[0] == '/') {
        memmove(path, path + 1, strlen(path));
    }

    char dest[256] = "";
    if (strncmp(path, "static", 6) != 0) {
        strcpy(dest, "templates/");
    }

    strcat(dest, path);
    
    file_content = read_file(dest, &file_size);
    if (!file_content) {
        /* 404 Error senden */
        const char* error_msg = "<!DOCTYPE html>\n"
                               "<html><head><title>404 Not Found</title></head>\n"
                               "<body><h1>404 - File Not Found</h1>\n"
                               "<p>The requested file was not found on this server.</p>\n"
                               "</body></html>";
        send_http_response(client_socket, 404, "text/html", error_msg, strlen(error_msg));
        return;
    }
    
    const char* content_type = "text/plain";
    if (strstr(path, ".html") || strstr(path, ".htm")) {
        content_type = "text/html";
        file_content = str_replace(file_content, "{{ hello }}", "world again!");
    } else if (strstr(path, ".css")) {
        content_type = "text/css";
    } else if (strstr(path, ".js")) {
        content_type = "application/javascript";
    } else if (strstr(path, ".png")) {
        content_type = "image/png";
    } else if (strstr(path, ".jpg") || strstr(path, ".jpeg")) {
        content_type = "image/jpeg";
    }

    send_http_response(client_socket, 200, content_type, file_content, file_size);
    
    free(file_content);
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
    
    /* Socket-Optionen setzen (Adresse wiederverwenden) */
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_socket);
        exit(1);
    }
    
    /* Server-Adresse konfigurieren */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    /* Socket binden */
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind fehlgeschlagen");
        close(server_socket);
        exit(1);
    }
    
    if (listen(server_socket, MAX_THREADS) < 0) {
        perror("Listen fehlgeschlagen");
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