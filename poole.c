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
#include "connections.h"

void* bowmanHandler(void *arg) {
    int sock = *(int*) arg;
    char* buffer = NULL;

    asprintf(&buffer, "connectionHandler from poole %d", sock);
    printF(buffer);
    free(buffer);
    buffer = NULL;

    return NULL;
}

static int acceptConnections(int sock) {
    char* buffer = NULL;
    int num_threads = 0;
    pthread_t* threads = NULL;

    printF("\nWaiting for connections...\n");
    
    while (1) {
        struct sockaddr_in client;
        socklen_t c_len = sizeof(client);

        int newsock = accept(sock, (struct sockaddr *) &client, &c_len);

        if (newsock < 0) {
            asprintf(&buffer, "%sError accepting socket connection\n%s", C_BOLDRED, C_RESET);
            printF(buffer);
            free(buffer);
            buffer = NULL;

            return -1;
        }
        
        num_threads++;
        threads = (pthread_t*) realloc(threads, num_threads * sizeof(pthread_t));
    
        if (pthread_create(&threads[0], NULL, bowmanHandler, &newsock) != 0) {
            asprintf(&buffer,  "%sError creating the poole or bowman thread\n%s", C_BOLDRED, C_RESET);
            printF(buffer);
            free(buffer);
            buffer = NULL;
            
            return -1;
        }

    }

    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            asprintf(&buffer, "%sError synchronizing threads\n%s", C_BOLDRED, C_RESET);
            printF(buffer);
            free(buffer);
            buffer = NULL;

            return -1;
        }
    }

    close (sock);
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
    //size_t i = 0;
    Server_conf config;
    //Header header;

    if (argc != 2) {
        printF(C_BOLDRED);
        printF("Usage: ./poole <config_file>\n");
        printF(C_RESET);
        return -1;
    }

    config = readConfigPol(argv[1]);
    printF("Reading configuration file\n");


    if (checkPort(config.discovery_port) == -1 || checkPort(config.user_port) == -1) {
        printF(C_BOLDRED);
        printF("ERROR: Invalid port.\n");
        printF(C_RESET);

        return -1;
    }

    server = configServer(config.discovery_ip, config.discovery_port);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1) {
        printF(C_BOLDRED);
        printF("Error creating socket\n");
        printF(C_RESET);

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

    asprintf(&buffer, T1_POOLE, config.server, config.user_ip, config.user_port);
    buffer = sendFrame(buffer, sock);

    Header header = readHeader(sock);

    if (header.type == '1' && strcmp(header.header, "CON_OK") == 0) {
        //close(sock); //como cerrarlo

        server = configServer(config.user_ip, config.user_port);
        sock = socket(AF_INET, SOCK_STREAM, 0);
        
        if (openConnection(sock, server, "bowman") == -1) {
            return -1;
        }

        asprintf(&buffer, C_GREEN "Connected to HAL 9000 System, ready to listen to Bowmans petitions\n" C_RESET);
        printF(buffer);
        free(buffer);
        buffer = NULL;

        if (acceptConnections(sock) == -1) {
            return -1;
        }
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
/*    
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
  */  
    return 0;
}