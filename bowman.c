#include "functions.h"
#include "test.h"

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
        readLine(fd_config, &config.files_path);
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
            free(config.files_path);
            free(config.ip);
            exit(0);
            break;
    }
}

int main(int argc, char *argv[]) {
    char reader[1];
    signal(SIGINT, sig_handler);

    if (argc != 2) {
        printF("Usage: ./poole <config_file>\n");
        return -1;
    }
    
    readConfig(argv[1]);
    testBowConf(config);

    while(1) {
        printF("$ ");
        read(0, reader, 1);
    }
    return 0;
}
