#include <stdio.h>

#define printF(x) write(1, x, strlen(x))

typedef struct {
    char* name;
    char* path;
    char* ip;
    int port;
} Config;

Config config;

void sig_handler(int sigsum) {

    switch(sigsum) {
        case SIGINT:
            printF("Aborting\n");
            free(config.name);
            free(config.path);
            free(config.ip);
            exit(0);
            break;
    }
}

int main() {

    signal(SIGINT, sig_handler);
    

    return 0;
}
