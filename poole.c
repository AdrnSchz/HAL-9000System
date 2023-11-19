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

int bowmanHandler(int sock, int user_pos, char** users) {
    Header header;
    char* buffer = NULL;

    header = readHeader(sock);

    if (header.type == '1' && strcmp(header.header, "NEW_BOWMAN") == 0) {
        
        asprintf(&buffer, T1_OK);
        buffer = sendFrame(buffer, sock);
        
        users[user_pos] = getString(0, '\0', header.data);
        asprintf(&buffer, "\nNew user connected: %s.\n", users[user_pos]);
        printF(buffer);
        free(buffer);
        buffer = NULL;

    }
    else if (header.type == '2') {
        if (strcmp(header.header, "LIST_SONGS") == 0) {
            //get number of songs and songs

            asprintf(&buffer, T2_SONGS_RESPONSE, 0); //0 will be the num of songs to be sent
            buffer = sendFrame(buffer, sock);

            for (int i = 0; i < 0; i++) { //0 will be the num of songs to be sent
                char end = '&';
                if (i == 0) {
                    end = '\0';
                }
                asprintf(&buffer, "name of song%c", end); 
                buffer = sendFrame(buffer, sock);
            }
        }
        else if (strcmp(header.header, " LIST_PLAYLISTS") == 0) {
            //get number of playlists and songs

            asprintf(&buffer, T2_PLAYLISTS_RESPONSE, 0); //0 will be the num of songs to be sent
            buffer = sendFrame(buffer, sock);

            for (int i = 0; i < 0; i++) { //0 will be the num of songs to be sent
                char end = '&';
                if (i == 0) {
                    end = '\0';
                }
                asprintf(&buffer, "name of playlist%c", end); 
                buffer = sendFrame(buffer, sock);
            }
        }
        else {
            printF("Wrong frame\n");
            sendError(sock);
        }

    }
    else if (header.type == '6') {
        header = readHeader(sock);

        if (header.type == '6' && strcmp(header.header, "EXIT") == 0) {
            asprintf(&buffer, T6_OK);
            buffer = sendFrame(buffer, sock);
            asprintf(&buffer, "User %s disconnected\n", header.data);
            printF(buffer);
            free(buffer);
            buffer = NULL;

            return -1;
        }
        else {
            printF("Wrong frame\n");
            sendError(sock);
        }
    }
    else if (header.type == '7') {
        printF(C_BOLDRED);
        printF("Sent wrong frame\n");
        printF(C_RESET);
    }
    else {
        printF("Wrong frame\n");
        sendError(sock);
    }

    return 0;
}

static int acceptConnections(int sock) {
    fd_set readfds;
    int* users_fd = (int*) malloc(sizeof(int));
    int num_users = 0;
    char** users = (char**) malloc(sizeof(char*));

    printF("\nWaiting for connections...\n");
    
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        for (int i = 0; i < num_users; i++) {
            FD_SET(users_fd[i], &readfds);
        }

        int ready = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
        
        if (ready == -1) {
            printF("Error in select\n");
            return -1;
        }
        else if (ready == 0) {
            printF("Timeout\n");
        }
        else {
            if (FD_ISSET(sock, &readfds)) {
                if (acceptConnection(&num_users, users_fd, "bowman", sock) == -1) {
                    return -1;
                }
                users = (char**) realloc(users, sizeof(char*) * num_users);
            }
            for (int i = 0; i < num_users; i++) {
                if (FD_ISSET(users_fd[i], &readfds)) {
                    if (bowmanHandler(users_fd[i], i, users) == -1) {
                        //cerrar sock
                        close(users_fd[i]);
                        FD_CLR(users_fd[i], &readfds);
                        for (int j = i; j < num_users - 1; j++) {
                            users_fd[j] = users_fd[j + 1];
                            users[j] = users[j + 1];
                        }
                        num_users--;
                    }
                }
            }
        }
    }

    close (sock);
    for (int i = 0; i < num_users; i++) {
        close(users_fd[i]);
    }

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
    Header header;

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

    header = readHeader(sock);

    if (header.type == '1' && strcmp(header.header, "CON_OK") == 0) {
        //close(sock); 

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
    return 0;
}