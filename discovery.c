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
int* num_servers;

void* connectionHandler(void *arg) {
    Header header;
    int sock = *(int*) arg;
    int least_users = INT_MAX, pos;
    char* buffer = NULL;


    while (1) {
        printF("Waiting for frames... \n");
        header = readHeader(sock);
        printF("Received header\n");

        if (header.type == '1') {
            if (strcmp(header.header, "NEW_POOLE") == 0) {
                *num_servers = *num_servers + 1;
                servers = realloc(servers, *num_servers * sizeof(Server));
                servers[*num_servers - 1].name = getString(0, '&', header.data);
                servers[*num_servers - 1].ip = getString(1 + strlen(servers[*num_servers - 1].name), '&', header.data);
                servers[*num_servers - 1].port = atoi(getString(2 + strlen(servers[*num_servers - 1].name) + strlen(servers[*num_servers - 1].ip), '\0', header.data));
                servers[*num_servers - 1].num_users = 0;

                asprintf(&buffer ,"New poole server registered: %s - IP: %s - Port: %d\n", servers[*num_servers - 1].name, servers[*num_servers - 1].ip, servers[*num_servers - 1].port);
                printF(buffer);
                free(buffer);
                buffer = NULL;
                
                asprintf(&buffer, T1_OK);
                buffer = sendFrame(buffer, sock);
            }
            else if (strcmp(header.header, "NEW_BOWMAN") == 0) {
                least_users = INT_MAX;
                asprintf(&buffer, "num servers: %d\n", *num_servers);
                printF(buffer);
                free(buffer);
                buffer = NULL;
                
                if (*num_servers == 0) {
                    asprintf(&buffer, T1_KO);
                    buffer = sendFrame(buffer, sock);
                    continue;
                }
                for (int i = 0; i < *num_servers; i++) {
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
                printF("1\n");
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
    char* buffer;
    struct sockaddr_in server;
    int sock, pid;
    char* server_type;
    num_servers = mmap(NULL, sizeof *num_servers, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *num_servers = 0;

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

    pid = fork();
    switch (pid) {
        case -1:
            printF(" ---- Error on fork ----\n");
            break;
        case 0:
            server = configServer(config.ip_bow, config.port_bow);
            asprintf(&server_type, "bowman");
            break;
        default:
            server = configServer(config.ip_poole, config.port_poole);
            asprintf(&server_type, "poole");
            break;
    }  
    
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1) {
        printF(C_BOLDRED);
        printF("Error creating socket\n");
        printF(C_RESET);

        return -1;
    }

    if (openConnection(sock, server, server_type) == -1) {
        return -1;
    }
    
    if (pid != 0) {
        printF("Waiting for connections...\n");
    }
    int num_threads = 0;
    pthread_t* threads = NULL;
    while (1) {

        struct sockaddr_in client;
        socklen_t c_len = sizeof(client);


        int newsock = accept(sock, (struct sockaddr *) &client, &c_len);

        if (newsock < 0) {
            asprintf(&buffer, "%sError accepting %s socket connection\n%s", C_BOLDRED, server_type, C_RESET);
            printF(buffer);
            free(buffer);
            buffer = NULL;

            return -1;
        }

        asprintf(&buffer, "%sNew %s connection from %s:%hu in %s:%hu\n%s", C_GREEN, server_type, inet_ntoa (client.sin_addr), ntohs (client.sin_port), inet_ntoa (server.sin_addr), ntohs (server.sin_port), C_RESET);
        printF(buffer);
        free(buffer);
        buffer = NULL;
        
        num_threads++;
        threads = (pthread_t*) realloc(threads, num_threads * sizeof(pthread_t));
    
        if (pthread_create(&threads[0], NULL, connectionHandler, &newsock) != 0) {
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
    
    if (pid == 0) {
        exit(0);
    } else {
        wait(NULL);
        munmap(num_servers, sizeof *num_servers);
    }

    return 0;
}