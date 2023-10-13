#include "functions.h"

User_conf config;

void readConfig(char* file) {
    int fd_config;
    char *buffer;

    fd_config = open(file, O_RDONLY);

    if (fd_config == -1) {
        asprintf(&buffer, "EROOR: %s not found.\n", file);
        printF(buffer);
        free(buffer);
        exit(-1);
    }

    else {
        readLine(fd_config, &config.user);
        readLine(fd_config, &config.path);
        readLine(fd_config, &config.ip);
        readNum(fd_config, &config.port);

        close(fd_config);
    }
}

void sig_handler(int sigsum) {

    switch(sigsum) {
        case SIGINT:
            printF("\nAborting...\n");
            free(config.user);
            free(config.path);
            free(config.ip);
            exit(0);
            break;
    }
}

int main(int argc, char *argv[]) {
    char reader[1];
    signal(SIGINT, sig_handler);

    while(1) {
        printF("$ ");
        read(0, reader, 1);
    }

    char* buffer;

    if (argc != 2) {
        printF("Usage: ./poole <config_file>\n");
        return -1;
    }
    
    else {
        readConfig(argv[1]);

        asprintf(&buffer, "User: %s\n", config.user);
        printF(buffer);
        free(buffer);
        asprintf(&buffer, "Path: %s\n", config.path);
        printF(buffer);
        free(buffer);
        asprintf(&buffer, "IP: %s\n", config.ip);
        printF(buffer);
        free(buffer);
        asprintf(&buffer, "Port: %d\n", config.port);
        printF(buffer);
        free(buffer);
    }    

    return 0;
}
