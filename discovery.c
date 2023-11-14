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
    char* buffer;
    struct sockaddr_in server;
    int sock, pid, num_servers = 0, least_users = INT_MAX, pos;
    char* server_type;
    Server* servers;
    Header header;

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

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1) {
        printF(C_BOLDRED);
        printF("Error creating socket\n");
        printF(C_RESET);

        return -1;
    }

    pid = fork();
    switch (pid) {
        case -1:
            printF(" ---- Error on fork ----\n");
            break;
        case 0:
            server.sin_family = AF_INET;
            server.sin_addr.s_addr = inet_addr(config.ip_bow);
            server.sin_port = htons(config.port_bow);
            asprintf(&server_type, "bowman");
            break;
        default:
            server.sin_family = AF_INET;
            server.sin_addr.s_addr = inet_addr(config.ip_poole);
            server.sin_port = htons(config.port_poole);
            asprintf(&server_type, "poole");
            break;
    }
    
    // ***********************************************************bind and accept instead of connect???**********************************
    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        printF(C_BOLDRED);
        asprintf(&buffer, "Error connecting to %s socket\n", server_type);
        printF(buffer);
        printF(C_RESET);
        free(buffer);
        buffer = NULL;

        return -1;
    }

    while (1) {
        header = readHeader(sock);

        if (header.type == '1') {
            if (strcmp(header.header, "NEW_POOLE") == 0) {
                num_servers++;
                servers = realloc(servers, num_servers * sizeof(Server));
                strcpy(servers[num_servers - 1].name, getString(0, '&', header.data));
                strcpy(servers[num_servers - 1].ip, getString(0, '&', header.data));
                servers[num_servers - 1].port = atoi(getString(0, '\n', header.data));
                servers[num_servers - 1].num_users = 0;
            }
            else if (strcmp(header.header, "NEW_BOWMAN") == 0) {
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
                //mandar frame con la ip, el port y el no,bre del server a bowman

                least_users = INT_MAX;
            }
            else {
                sendError(sock);
            }
        }
        else if (header.type == '7') {
            printF(C_BOLDRED);
            printF("Sent wrong frame\n");
            printF(C_RESET);
        }
        else {
            sendError(sock);
        }
    }

    close (sock);

    if (pid == 0) {
        exit(0);
    } else {
        wait(NULL);
    }

    return 0;
}