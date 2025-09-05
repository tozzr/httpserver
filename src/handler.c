#include "handler.h"

#include <stdlib.h>

#include "server.h"
#include "string_utils.h"
#include "file_io.h"
#include "logger.h"

#define BUFFER_SIZE 1024

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
