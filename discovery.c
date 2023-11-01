#include "functions.h"
#include "test.h"

Disc_conf readConfig(char* file) {
    Disc_conf config;
    int fd_config;
    char *buffer;

    fd_config = open(file, O_RDONLY);

    if (fd_config == -1) {
        asprintf(&buffer,C_BOLDRED "EROOR: %s not found.\n" C_RESET, file);
        printF(buffer);
        free(buffer);
        exit(-1);
    }

    readLine(fd_config, &config.ip_poole);
    readNum(fd_config, &config.port_poole);
    readLine(fd_config, &config.ip_bow);
    readNum(fd_config, &config.port_bow);

    close(fd_config);

    return config;
}

int main(int argc, char *argv[]) {
    Disc_conf config;
    struct sockaddr_in poole, bow;
    int sock_poole, sock_bow;

    if (argc != 2) {
        printF(C_BOLDRED);
        printF("Usage: ./discovery <config_file>\n");
        printF(C_RESET);
        return -1;
    } 

    config = readConfig(argv[1]);
    printF("Reading configuration file\n");
    
    if (checkPort(config.port_poole) == -1) {
        printF(C_BOLDRED);
        printF("ERROR: Invalid poole port.\n");
        printF(C_RESET);
        return -1;
    }

    if (checkPort(config.port_bow) == -1) {
        printF(C_BOLDRED);
        printF("ERROR: Invalid bow port.\n");
        printF(C_RESET);
        return -1;
    }

    sock_poole = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_poole == -1) {
        printF(C_BOLDRED);
        printF("Error creating socket\n");
        printF(C_RESET);
        return -1;
    }

    sock_bow = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_bow == -1) {
        printF(C_BOLDRED);
        printF("Error creating socket\n");
        printF(C_RESET);
        return -1;
    }

    poole.sin_family = AF_INET;
    poole.sin_addr.s_addr = inet_addr(config.ip_poole);
    poole.sin_port = htons(config.port_poole);
    bow.sin_family = AF_INET;
    bow.sin_addr.s_addr = inet_addr(config.ip_bow);
    bow.sin_port = htons(config.port_bow);
    
    

    return 0;
}