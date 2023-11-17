#define _GNU_SOURCE
#define _XOPEN_SOURCE 500 //threads
#define _POSIX_C_SOURCE 1 //threads
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/ioctl.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits.h>
#include <pthread.h>

#define printF(x) write(1, x, strlen(x))

struct sockaddr_in configConnection(char* ip, int port) {
    struct sockaddr_in server;

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);

    return server;
}

int openConnection(int sock, struct sockaddr_in server) {
    char* buffer;

    if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        asprintf(&buffer, "Error binding to socket\n");
        printF(buffer);
        free(buffer);
        buffer = NULL;

        return -1;
    }

    listen(sock, 5);

    return 0;
}

int main(int argc, char *argv[]) {
    char* buffer;
    struct sockaddr_in server;
    int sock;

    server = configConnection("127.0.0.1", 8580); 
    
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1) {
        printF("Error creating socket\n");

        return -1;
    }

    if (openConnection(sock, server) == -1) {
        return -1;
    }
    
    printF("Waiting for connections...\n");
    
    while (1) {

        struct sockaddr_in client;
        socklen_t c_len = sizeof(client);

        int newsock = accept(sock, (struct sockaddr *) &client, &c_len);

        if (newsock < 0) {
            asprintf(&buffer, "Error accepting socket connection\n");
            printF(buffer);
            free(buffer);
            buffer = NULL;

            return -1;
        }

        asprintf(&buffer, "New connection from %s:%hu in %s:%hu\n", inet_ntoa (client.sin_addr), ntohs (client.sin_port), inet_ntoa (server.sin_addr), ntohs (server.sin_port));
        printF(buffer);
        free(buffer);
        buffer = NULL;

        char* received = (char*) malloc(1);
        printF("Reading...\n");
        read(newsock, received, 1);
        while (received[0] != '\0') {
            asprintf(&buffer, "%c", received[0]);
            printF(buffer);
            read(newsock, received, 1);
        }
        printF(buffer);
        break;

    }

    return 0;
}