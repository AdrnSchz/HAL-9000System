#include "functions.h"

User_conf config;

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
