/********************************************************************
 *
 * @Purpose: HAL 9000 System - Config files
 * @Author: Marc Escoté Llopis & Adrián Jorge Sánchez López
 *
 * - This file contains functions used for reading the configuration files.
 *
 ********************************************************************/
#include "configs.h"

Disc_conf readConfigDis(char* file) {
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

User_conf readConfigBow(char* file) {
    User_conf config;
    int fd_config;
    char *buffer;

    fd_config = open(file, O_RDONLY);

    if (fd_config == -1) {
        asprintf(&buffer, C_BOLDRED "ERROR: %s not found.\n" C_RESET, file);
        printF(buffer);
        free(buffer);
        exit(-1);
    }

    readLine(fd_config, &config.user);
    readLine(fd_config, &config.files_path);
    readLine(fd_config, &config.ip);
    readNum(fd_config, &config.port);
    
    close(fd_config);

    return config;
}

Server_conf readConfigPol(char* file) {
    Server_conf config;
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

    return config;
}
