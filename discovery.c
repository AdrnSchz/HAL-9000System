/********************************************************************
 *
 * @Purpose: HAL 9000 System - Discovery Server
 * @Authors: Marc Escoté Llopis & Adrián Jorge Sánchez López
 *
 * - The main purpose of the code is to initialize and run the Discovery server,
 *   managing interactions with Poole and Bowman servers.
 *
 * - The Discovery server reads its configuration from a file passed as a command-line 
 *   parameter, establishing socket connections with both Poole and Bowman servers.
 *
 * - The Discovery server differentiates between Poole and Bowman servers,
 *   allocating resources accordingly and maintaining server information.
 *
 * - The server continuously listens for incoming frames, handling the registration
 *   of new Poole servers and distributing the connected Bowman users.
 *
 ********************************************************************/
#include "functions.h"
#include "test.h"
#include "configs.h"
#include "connections.h"

Server* servers;
int* clients_fd;
int num_servers = 0, num_clients = 0;

void connectionHandler(int sock) {
    Header header;
    int least_users = INT_MAX, pos;
    char* buffer = NULL;

    printF("Waiting for frames... \n");
    header = readHeader(sock);
    printF("Received header\n");

    if (header.type == '1') {
        if (strcmp(header.header, "NEW_POOLE") == 0) {
            num_servers++;
            servers = realloc(servers, num_servers * sizeof(Server));
            servers[num_servers - 1].name = getString(0, '&', header.data);
            servers[num_servers - 1].ip = getString(1 + strlen(servers[num_servers - 1].name), '&', header.data);
            servers[num_servers - 1].port = atoi(getString(2 + strlen(servers[num_servers - 1].name) + strlen(servers[num_servers - 1].ip), '\0', header.data));
            servers[num_servers - 1].num_users = 0;

            asprintf(&buffer ,"New poole server registered: %s - IP: %s - Port: %d\n", servers[num_servers - 1].name, servers[num_servers - 1].ip, servers[num_servers - 1].port);
            printF(buffer);
            free(buffer);
            buffer = NULL;
            
            asprintf(&buffer, T1_OK);
            buffer = sendFrame(buffer, sock);
        }
        else if (strcmp(header.header, "NEW_BOWMAN") == 0) {
            least_users = INT_MAX;
            asprintf(&buffer, "num servers: %d\n", num_servers);
            printF(buffer);
            free(buffer);
            buffer = NULL;

            if (num_servers == 0) {
                asprintf(&buffer, T1_KO);
                buffer = sendFrame(buffer, sock);
                return;
            }

            for (int i = 0; i < num_servers; i++) {
                if (servers[i].num_users == 0) {
                    printF("1.1\n");
                    pos = i;
                    break;
                }
                else if (least_users > servers[i].num_users) {
                    printF("1.2\n");
                    least_users = servers[i].num_users;
                    pos = i;
                }
            }
            servers[pos].num_users++;
            servers[pos].users = realloc(servers[pos].users, servers[pos].num_users * sizeof(char*));
            printF("2\n");
            servers[pos].users[servers[pos].num_users - 1] = getString(0, '\0', header.data);
            printF("3\n");
            
            asprintf(&buffer, T1_OK_BOW, servers[pos].name, servers[pos].ip, servers[pos].port);
            buffer = sendFrame(buffer, sock);
        }
        else {
            asprintf(&buffer, T1_KO);
            buffer = sendFrame(buffer, sock);
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
}

/********************************************************************
 *
 * @Purpose: Main function of the Discovery server.
 * @Parameters: argc - The number of command-line arguments.
 *              argv - An array of the command-line arguments.
 * @Return: 0 on success, -1 on failure.
 *
 ********************************************************************/
int main(int argc, char *argv[]) {
    Disc_conf config;
    fd_set readfds;
    struct sockaddr_in server_p, server_b;
    int bowman_sock, poole_sock;
    clients_fd = (int*) malloc(sizeof(int));

    if (argc != 2) {
        printF(C_BOLDRED);
        printF("Usage: ./discovery <config_file>\n");
        printF(C_RESET);
        return -1;
    }

    config = readConfigDis(argv[1]);
    printF("Reading configuration file\n");

    if (checkPort(config.port_poole) == -1 || checkPort(config.port_bow) == -1) {
        printF(C_BOLDRED);
        printF("ERROR: Invalid poole or bow port.\n");
        printF(C_RESET);
        return -1;
    }

    server_b = configServer(config.ip_bow, config.port_bow);
    server_p = configServer(config.ip_poole, config.port_poole);  
    
    poole_sock = socket(AF_INET, SOCK_STREAM, 0);
    bowman_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (poole_sock == -1 || bowman_sock == -1) {
        printF(C_BOLDRED);
        printF("Error creating socket\n");
        printF(C_RESET);

        return -1;
    }

    if (openConnection(poole_sock, server_p, "poole") == -1 || openConnection(bowman_sock, server_b, "bowman") == -1) {
        return -1;
    }

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(poole_sock, &readfds);
        FD_SET(bowman_sock, &readfds);
        for (int i = 0; i < num_clients; i++) {
            FD_SET(clients_fd[i], &readfds);
        }

        printF("Waiting for connections...\n");

        int ready = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
        
        if (ready == -1) {
            printF("Error in select\n");
            return -1;
        }
        else if (ready == 0) {
            printF("Timeout\n");
        }
        else {
            if (FD_ISSET(poole_sock, &readfds)) {
                if (acceptConnection(&num_clients, clients_fd, "poole", poole_sock) == -1) {
                    return -1;
                }
            }
            if (FD_ISSET(bowman_sock, &readfds)) {
                if (acceptConnection(&num_clients, clients_fd, "bowman", bowman_sock) == -1) {
                    return -1;
                }
            for (int i = 0; i < num_clients; i++) {
                if (FD_ISSET(clients_fd[i], &readfds)) {
                    connectionHandler(clients_fd[i]);
                }
            }
        }
    }

    close (poole_sock);
    close (bowman_sock);
    for (int i = 0; i < num_clients; i++) {
        close(clients_fd[i]);
    }
    
    return 0;
}