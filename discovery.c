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

/********************************************************************
 *
 * @Purpose: Manages connections and interactions on the Discovery server, 
 *           including request handling, server registration, and error management.
 * @Parameters: sock - Socket descriptor for the connection.
 * @Return: 0 on successful handling, -1 on disconnect or error.
 *
 ********************************************************************/
int connectionHandler(int sock) {
    Frame frame;
    int least_users = INT_MAX, pos;
    char* buffer = NULL;

    frame = readFrame(sock);

    if (frame.type == '1') {
        if (strcmp(frame.header, "NEW_POOLE") == 0) {
            num_servers++;
            servers = realloc(servers, num_servers * sizeof(Server));
            servers[num_servers - 1].name = getString(0, '&', frame.data);
            servers[num_servers - 1].ip = getString(1 + strlen(servers[num_servers - 1].name), '&', frame.data);
            servers[num_servers - 1].port = atoi(getString(2 + strlen(servers[num_servers - 1].name) + strlen(servers[num_servers - 1].ip), '\0', frame.data));
            servers[num_servers - 1].num_users = 0;

            asprintf(&buffer ,"New poole server registered: %s - IP: %s - Port: %d\n", servers[num_servers - 1].name, servers[num_servers - 1].ip, servers[num_servers - 1].port);
            printF(buffer);
            free(buffer);
            buffer = NULL;
            
            asprintf(&buffer, T1_OK);
            buffer = sendFrame(buffer, sock, strlen(buffer));
        }
        else if (strcmp(frame.header, "NEW_BOWMAN") == 0) {
            least_users = INT_MAX;

            if (num_servers == 0) {
                asprintf(&buffer, T1_KO);
                buffer = sendFrame(buffer, sock, strlen(buffer));
                return -1;
            }

            for (int i = 0; i < num_servers; i++) {
                if (servers[i].num_users == 0) {
                    pos = i;
                    break;
                }
                else if (least_users > servers[i].num_users) {
                    least_users = servers[i].num_users;
                    pos = i;
                }
            }
            servers[pos].num_users++;
            
            asprintf(&buffer, T1_OK_BOW, servers[pos].name, servers[pos].ip, servers[pos].port);
            buffer = sendFrame(buffer, sock, strlen(buffer));
        }
        else {
            asprintf(&buffer, T1_KO);
            buffer = sendFrame(buffer, sock, strlen(buffer));
        }
        return -1;
    }
    else if (frame.type == '6' && strcmp(frame.header, "EXIT") == 0) {
            asprintf(&buffer, T6_OK);
            buffer = sendFrame(buffer, sock, strlen(buffer));
            asprintf(&buffer, "\n%sUser disconnected from server %s%s\n", C_RED, frame.data, C_RESET);
            printF(buffer);
            free(buffer);
            buffer = NULL;

            for (int i = 0; i < num_servers; i++) {
                if (strcmp(frame.data, servers[i].name) == 0) {
                    servers[i].num_users--;
                    break;
                }
            }
            return -1;
    }
    else if (frame.type == '6' && strcmp(frame.header, "SHUTDOWN") == 0) {
        asprintf(&buffer, T6_OK);
        buffer = sendFrame(buffer, sock, strlen(buffer));
        asprintf(&buffer, "\n%sServer %s got unexpectedly disconnected\n%s", C_RED, frame.data, C_RESET);
        printF(buffer);
        free(buffer);
        buffer = NULL;

        for (int i = 0; i < num_servers; i++) {
            if (strcmp(frame.data, servers[i].name) == 0) {
                for (int j = i; j < num_servers - 1; j++) {
                    servers[j] = servers[j + 1];
                }
                num_servers--;
                servers = realloc(servers, num_servers * sizeof(Server));
                break;
            }
        }
        return -1;
    }
    else if (frame.type == '7') {
        printF(C_RED);
        printF("Sent wrong frame\n");
        printF(C_RESET);
    }
    else {
        printF("Wrong frame\n");
        sendError(sock);
    }

    return 0;
}

fd_set buildSelect(int poole_sock, int bowman_sock) {
    fd_set readfds;
    char* buffer = NULL;
    
    FD_ZERO(&readfds);
    FD_SET(poole_sock, &readfds);
    FD_SET(bowman_sock, &readfds);
    for (int i = 0; i < num_clients; i++) {
        FD_SET(clients_fd[i], &readfds);
        asprintf(&buffer, "Added client %d to select\n", clients_fd[i]);
        printF(buffer);
        free(buffer);
    }

    return readfds;

}
/********************************************************************
 *
 * @Purpose: Initializes and runs the Discovery server, setting up connections 
 *           and handling interactions with Poole and Bowman servers.
 * @Parameters: argc - The number of command-line arguments.
 *              argv - An array of the command-line arguments.
 * @Return: 0 on success, -1 on failure.
 *
 ********************************************************************/
int main(int argc, char *argv[]) {
    char* buffer = NULL;
    Disc_conf config;
    fd_set readfds;
    struct sockaddr_in server_p, server_b;
    int bowman_sock, poole_sock;
    clients_fd = (int*) malloc(sizeof(int));

    if (argc != 2) {
        printF(C_RED);
        printF("Usage: ./discovery <config_file>\n");
        printF(C_RESET);
        return -1;
    }

    config = readConfigDis(argv[1]);
    printF("Reading configuration file\n");

    if (checkPort(config.port_poole) == -1 || checkPort(config.port_bow) == -1) {
        printF(C_RED);
        printF("ERROR: Invalid poole or bow port.\n");
        printF(C_RESET);
        return -1;
    }

    server_b = configServer(config.ip_bow, config.port_bow);
    server_p = configServer(config.ip_poole, config.port_poole);  

    poole_sock = openConnection(server_p);
    bowman_sock = openConnection(server_b);

    if (bowman_sock == -1 || poole_sock == -1) {
        asprintf(&buffer, "%sError opening the socket\n%s", C_RED, C_RESET);
        printF(buffer);
        free(buffer);

        return -1;
    }

    printF("Waiting for connections...\n");
    while (1) {
        readfds = buildSelect(poole_sock, bowman_sock);

        int ready = select(CHECK_UP_TO, &readfds, NULL, NULL, NULL);
        
        if (ready == -1) {
            printF("Error in select\n");
            return -1;
        }
        else {
            if (FD_ISSET(poole_sock, &readfds)) {
                clients_fd[num_clients] = accept(poole_sock, NULL, NULL);
                if (clients_fd[num_clients] == -1) {
                    asprintf(&buffer, "%sError accepting %s socket connection\n%s", C_RED, "poole", C_RESET);
                    printF(buffer);
                    free(buffer);

                    return -1;
                }
                num_clients++;

                clients_fd = realloc(clients_fd, sizeof(int) * (num_clients + 1));

                asprintf(&buffer, "\n%sNew %s connection from x\n%s", C_GREEN, "poole", C_RESET);
                printF(buffer);
                free(buffer);
            }
            if (FD_ISSET(bowman_sock, &readfds)) {
                clients_fd[num_clients] = accept(bowman_sock, NULL, NULL);
                if (clients_fd[num_clients] == -1) {
                    asprintf(&buffer, "%sError accepting %s socket connection\n%s", C_RED, "bowman", C_RESET);
                    printF(buffer);
                    free(buffer);

                    return -1;
                }

                num_clients++;
                clients_fd = realloc(clients_fd, sizeof(int) * (num_clients + 1));

                asprintf(&buffer, "\n%sNew %s connection from x\n%s", C_GREEN, "bowman", C_RESET);
                printF(buffer);
                free(buffer);
            }
            for (int i = 0; i < num_clients; i++) {
                if (FD_ISSET(clients_fd[i], &readfds)) {
                    if (connectionHandler(clients_fd[i]) == -1) {
                        close(clients_fd[i]);
                        FD_CLR(clients_fd[i], &readfds);
                        for (int j = i; j < num_clients - 1; j++) {
                            clients_fd[j] = clients_fd[j + 1];
                        }
                        num_clients--;
                    }
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