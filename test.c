#include "test.h"

void testBowConf(User_conf config) {
    char* buffer;

    asprintf(&buffer, "User: %s\n", config.user);
    printF(buffer);
    free(buffer);
    asprintf(&buffer, "files_path: %s\n", config.files_path);
    printF(buffer);
    free(buffer);
    asprintf(&buffer, "IP: %s\n", config.ip);
    printF(buffer);
    free(buffer);
    asprintf(&buffer, "Port: %d\n", config.port);
    printF(buffer);
    free(buffer);
}

void testPooleConf(Server_conf config) {
    char* buffer;

    asprintf(&buffer, "Server: %s\n", config.server);
    printF(buffer);
    free(buffer);
    asprintf(&buffer, "Path: %s\n", config.path);
    printF(buffer);
    free(buffer);
    asprintf(&buffer, "Discovery ip: %s\n", config.discovery_ip);
    printF(buffer);
    free(buffer);
    asprintf(&buffer, "Discovery port: %d\n", config.discovery_port);
    printF(buffer);
    free(buffer);
    asprintf(&buffer, "User ip: %s\n", config.user_ip);
    printF(buffer);
    free(buffer);
    asprintf(&buffer, "User port: %d\n", config.user_port);
    printF(buffer);
    free(buffer);
}