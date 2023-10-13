#include "functions.h"
#include "test.h"

Server_conf config;

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

    readLine(fd_config, &config.server);
    readLine(fd_config, &config.path);
    readLine(fd_config, &config.discovery_ip);
    readNum(fd_config, &config.discovery_port);
    readLine(fd_config, &config.user_ip);
    readNum(fd_config, &config.user_port);

    close(fd_config);
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printF("Usage: ./poole <config_file>\n");
        return -1;
    }

    else {
        readConfig(argv[1]);
        testPooleConf(config);
    }

    return 0;
}