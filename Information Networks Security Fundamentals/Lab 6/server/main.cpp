#include "server.h"


int main() {
    Server* server = new Server(true);
    server->run();
    delete server;
    server = nullptr;
    return 0;
}