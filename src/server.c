#include "server.h"

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