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

    

    if (argc != 2) {
        printF(C_BOLDRED);
        printF("Usage: ./discovery <config_file>\n");
        printF(C_RESET);
        return -1;
    } 
    
    else { 
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
    }
    return 0;
}