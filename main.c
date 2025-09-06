#include "src/server.h"

int main() {
    server_t* app = new_server();

    app->handle_static("./static");

    int port = 8080;
    return app->start(port);
}
