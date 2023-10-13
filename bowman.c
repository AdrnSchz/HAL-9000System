#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#define printF(x) write(1, x, strlen(x))

typedef struct {
    char* user;
    char* download_path;
    char* ip;
    int port;
} Config;

Config config;

void sig_handler(int sigsum) {

    switch(sigsum) {
        case SIGINT:
            printF("\nAborting...\n");
            free(config.user);
            free(config.download_path);
            free(config.ip);
            exit(0);
            break;
    }
}

int main() {
    char reader[1];
    signal(SIGINT, sig_handler);

    while(1) {
        printF("$ ");
        read(0, reader, 1);
    }
    

    return 0;
}
