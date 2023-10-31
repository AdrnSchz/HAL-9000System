#include "functions.h"
#include "test.h"

Server_conf config;

void readConfig(char* file) {
    int fd_config;
    char *buffer;

    fd_config = open(file, O_RDONLY);

    if (fd_config == -1) {
        asprintf(&buffer,C_BOLDRED "EROOR: %s not found.\n" C_RESET, file);
        printF(buffer);
        free(buffer);
        exit(-1);
    }

    readLine(fd_config, &config.server);
    readLine(fd_config, &config.path);
    readLine(fd_config, &config.discovery_ip);
    readNum(fd_config, &config.discovery_port);
    readLine(fd_config, &config.user_ip);
    readNum(fd_config, &config.user_port);

    close(fd_config);
}

int main(int argc, char *argv[]) {
    char* buffer;

    if (argc != 2) {
        printF(C_BOLDRED);
        printF("Usage: ./poole <config_file>\n");
        printF(C_RESET);
        return -1;
    }

    else {
        readConfig(argv[1]);
        printF("Reading configuration file\n");

        if (checkPort(config.discovery_port) == -1) {
            printF(C_BOLDRED);
            printF("ERROR: Invalid discovery port.\n");
            printF(C_RESET);
            return -1;
        }

        if (checkPort(config.user_port) == -1) {
            printF(C_BOLDRED);
            printF("ERROR: Invalid user port.\n");
            printF(C_RESET);
            return -1;
        }

        asprintf(&buffer, "Conecting %s Server to the system...\n", config.server);
        printF(buffer);
        free(buffer);
        buffer = NULL;

        
        
    }

    return 0;
}