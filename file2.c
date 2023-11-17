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
    
    asprintf(&buffer, "Conecting Server to the system...\n");
    printF(buffer);
    free(buffer);
    buffer = NULL;

    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        printF("Error connecting to the server!\n");

        return -1;
    }

    write(sock, "hola q tal", 256);


    return 0;
}