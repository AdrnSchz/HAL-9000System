/********************************************************************
*
* @Purpose: HAL 9000 System - Poole Server
* @Author: Marc Escoté Llopis & Adrián Jorge Sánchez López
*
* - The main purpose of the code is to initialize and run the Poole server, 
*   handling interactions with the Discovery server.
*
* - The Poole server reads its configuration from a file passed as command-line 
*   parameter and establishes a connection with the Discovery server.
*
* - Upon successful registration with the Discovery server, the Poole server awaits 
*   further instructions.
*
********************************************************************/

#include "functions.h"
#include "test.h"
#include "configs.h"

/********************************************************************
 *
 * @Purpose: Configures the connection to the Discovery server using the provided Server_conf data.
 * @Parameters: sock: Pointer to the socket variable to store the created socket.
 *              server: Pointer to the sockaddr_in structure to store server information.
 *              config: The Server_conf structure with configuration data.
 * @Return: 0 on success, -1 on failure.
 *
 ********************************************************************/
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

/********************************************************************
 *
 * @Purpose: Main function that initializes the Poole server.
 * @Parameters: argc - The number of command-line arguments.
 *              argv - An array of the command-line arguments.
 * @Return: 0 if successful, -1 otherwise.
 *
 ********************************************************************/
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

    config = readConfigPol(argv[1]);
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