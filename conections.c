#include "connections.h"

sockaddr_in configConnection(char* ip, int port) {
    sockaddr_in server;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);

    return server;
}

int openConnection();