#include "connections.h"
#include <netinet/in.h>

struct sockaddr_in configServer(char* ip, int port) {
    struct sockaddr_in server;

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);

    return server;
}

int openConnection(int sock, struct sockaddr_in server, char* server_type) {
    char* buffer;

    if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        asprintf(&buffer, "%sError binding the socket for %s\n%s", C_BOLDRED, server_type, C_RESET);
        printF(buffer);
        free(buffer);
        buffer = NULL;

        return -1;
    }

    listen(sock, 5);

    return 0;
}

int acceptConnection(int* num_clients, int* clients_fd, char* server_type, int sock) {
    char* buffer;
    struct sockaddr_in client_addr; 
    socklen_t client_len = sizeof(client_addr);

    clients_fd[*num_clients] = accept(sock, (struct sockaddr *) &client_addr, &client_len);

    if (clients_fd[*num_clients] < 0) {
        asprintf(&buffer, "%sError accepting %s socket connection\n%s", C_BOLDRED, server_type, C_RESET);
        printF(buffer);
        free(buffer);
        buffer = NULL;
        return -1;
    }

    asprintf(&buffer, "%sNew %s connection from %s:%hu\n%s", C_GREEN, server_type, inet_ntoa (client_addr.sin_addr), ntohs (client_addr.sin_port), C_RESET);
    printF(buffer);
    free(buffer);
    buffer = NULL;

    *num_clients = *num_clients + 1;
    clients_fd = (int*) realloc(clients_fd, sizeof(int) * (*num_clients + 1));

    return 0;
}

int checkPort(int port) {

    if (port < 0 || port > 65535) {
        return -1;
    }

    return 0;
}

void sendError(int sock) {
    char buffer[256];
    size_t i;

    memset(&buffer, 0, 256);
    for (i = 0; i < strlen(ERROR_FRAME); i++) {
        buffer[i] = ERROR_FRAME[i];
    }
    buffer[i] = '\0';
    write(sock, buffer, 256);
}

Header readHeader(int sock) {
    Header header;
    char* buffer = (char*) malloc(256);
    int i, j;

    read(sock, buffer, 256);
    printF("Received frame: ");
    printF(buffer);
    printF("\n");
    header.type = buffer[0];
    header.length[0] = buffer[1];
    header.length[1] = buffer[2];
    header.length[2] = '\0';
    header.header = (char*) malloc((atoi(header.length) + 1)* sizeof(char));
    for (i = 0; i < atoi(header.length); i++) {
        header.header[i] = buffer[i + 3];
    }
    header.header[i + 3] = '\0';
    header.data = (char*) malloc(strlen(buffer) - i - 2);
    for (j = 0; buffer[j + i + 3] != '\0'; j++) {
        header.data[j] = buffer[j + i + 3];
    }
    header.data[j + i + 3] = '\0';
    return header;
}

char* sendFrame(char* buffer, int sock) {
    int len = strlen(buffer);
    buffer = (char*) realloc(buffer, 256);
    for (int i = len; i < 256; i++) {
            buffer[i] = '\0';
    }
    write(sock, buffer, 256);
    printF("Sending frame: ");
    printF(buffer);
    printF("\n");
    free(buffer);
    buffer = NULL;

    return buffer;
}