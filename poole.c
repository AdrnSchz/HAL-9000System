#include "functions.h"
#include "test.h"

Server_conf readConfig(char* file) {
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

int configConection(int* sock, struct sockaddr_in* server, Server_conf config) {

    if (checkPort(config.discovery_port) == -1 || checkPort(config.user_port) == -1) {
        printF(C_BOLDRED);
        printF("ERROR: Invalid port.\n");
        printF(C_RESET);

        return -1;
    }

    *sock = socket(AF_INET, SOCK_STREAM, 0);

    if (*sock == -1) {
        printF(C_BOLDRED);
        printF("Error creating socket\n");
        printF(C_RESET);

        return -1;
    }

    server->sin_family = AF_INET;
    server->sin_addr.s_addr = inet_addr(config.discovery_ip);
    server->sin_port = htons(config.discovery_port);

    return 0;
}

int main(int argc, char *argv[]) {
    char* buffer;
    struct sockaddr_in server;
    int sock;
    Server_conf config;
    Header header;

    if (argc != 2) {
        printF(C_BOLDRED);
        printF("Usage: ./poole <config_file>\n");
        printF(C_RESET);
        return -1;
    }

    config = readConfig(argv[1]);
    printF("Reading configuration file\n");

    if (configConection(&sock, &server, config) == -1) {
        return -1;
    }

    asprintf(&buffer, "Conecting %s Server to the system...\n", config.server);
    printF(buffer);
    free(buffer);
    buffer = NULL;

    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        printF(C_BOLDRED);
        printF("Error connecting to the server!\n");
        printF(C_RESET);

        return -1;
    }

    asprintf(&buffer, "109NEW_POOLE%s\n", config.server); // padding????
    write(sock, buffer, strlen(buffer));
    free(buffer);
    buffer = NULL;

    header = readHeader(sock);

    if (header.type == '1' && strcmp(header.header, "CON_OK\n") == 0) {
        close(sock);
        
        //bind????
        //accept????

        asprintf(&buffer, C_GREEN "Connected to HAL 9000 System, ready to listen to Bowmans petitions\n" C_RESET);
        printF(buffer);
        free(buffer);
        buffer = NULL;
    }
    else {
        printF(C_BOLDRED);
        printF("Error trying to connect to HAL 9000 system\n");
        printF(C_RESET);
        sendError(sock);
        close(sock);
        return -1;
    }

    printF("Waiting for conections...");
    
    while (1) {
        header = readHeader(sock);

        switch (header.type) {
            case '1':
            //mandar frames desde el server preguntar
            break;
            case '2':
            break;
            case '3':
            case '5':
            break;
            case '6':
            break;
            case '7':
                printF(C_BOLDRED);
                printF("Sent wrong frame\n");
                printF(C_RESET);
            break;
        }
    }
    
    return 0;
}