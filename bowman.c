/********************************************************************
 *
 * @Purpose: HAL 9000 System - Bowman User
 * @Authors: Marc Escoté Llopis & Adrián Jorge Sánchez López
 *
 * - The main purpose of the code is to initialize and run the Bowman user that
 *   will connect to the the Discovery server to get a Poole server to connect to.
 * 
 * - The Bowman user reads its configuration from a file passed as a command-line parameter,
 *   establishes a socket connection with the Discovery, and handles user interactions.
 *
 * - The user can input commands and receive responses from the system.
 *
 * - The code includes signal handling for program termination when receiving SIGINT.
 *
 ********************************************************************/
#include "functions.h"
#include "test.h"
#include "configs.h"
#include "connections.h"

User_conf config;

/********************************************************************
 *
 * @Purpose: Handles SIGINT signal for graceful program termination.
 * @Parameters: sigsum - The signal number.
 * @Return: ---
 *
 *******************************************************************/
void sig_handler(int sigsum) {

    switch(sigsum) {
        case SIGINT:
            printF("\nAborting...\n");
            free(config.user);
            free(config.files_path);
            free(config.ip);
            config.user = NULL;
            config.files_path = NULL;
            config.ip = NULL;
            exit(0);
            break;
    }
}

/********************************************************************
 *
 * @Purpose: Sets up a socket connection based on configuration file details.
 * @Parameters: sock - Pointer to the socket variable.
 *              server - Pointer to server address structure.
 * @Return: 0 if successful, -1 otherwise.
 *
 ********************************************************************/
int configConnection(int* sock, struct sockaddr_in* server) {

    if (checkPort(config.port) == -1) {
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

    *server = configServer(config.ip, config.port);

    return 0;
}

/********************************************************************
 *
 * @Purpose: Initializes the Bowman user, manages socket connections to 
 *           Discovery and Poole servers, and handles user commands and responses.
 * @Parameters: argc - The number of command-line arguments.
 *              argv - An array of the command-line arguments.
 * @Return: 0 on success, -1 on error.
 *
 ********************************************************************/
int main(int argc, char *argv[]) {
    char* buffer = NULL, *server = NULL;
    int discovery_sock, poole_sock, connected = 0;
    struct sockaddr_in discovery, poole;
    Header header;
    signal(SIGINT, sig_handler);

    if (argc != 2) {
        printF(C_BOLDRED);
        printF("Usage: ./bowman <config_file>\n");
        printF(C_RESET);
        return -1;
    }
    
    config = readConfigBow(argv[1]);
    checkName(&config.user);

    if (configConnection(&discovery_sock, &discovery) == -1) {
        return -1;
    }

    asprintf(&buffer, "\n%s user initialized\n\n", config.user);
    printF(buffer);
    free(buffer);
    buffer = NULL;

    if (checkPort(config.port) == -1) {
        printF(C_BOLDRED);
        printF("ERROR: Invalid port.\n");
        printF(C_RESET);
        return -1;
    }
    
    while(1) {
        printF(BOLD);
        printF("$ ");
        readLine(0, &buffer);
        printF(C_RESET);
        switch (checkCommand(buffer)) {
            case 0:
                if (connected == 1) {
                    printF(C_RED);
                    printF("ERROR: Already connected to HAL 9000 system\n");
                    printF(C_RESET);
                    break;
                }


                if (connect(discovery_sock, (struct sockaddr *) &discovery, sizeof(discovery)) < 0) {
                    printF(C_BOLDRED);
                    printF("Error trying to connect to HAL 9000 system\n");
                    printF(C_RESET);
                    
                    return -1;
                }

                free(buffer);
                buffer = NULL;
                asprintf(&buffer, T1_BOWMAN, config.user);
                buffer = sendFrame(buffer, discovery_sock);

                header = readHeader(discovery_sock);

                if (header.type == '1' && strcmp(header.header, "CON_OK") == 0) {
                    //close(discovery_sock); //como cerrarlo
                    server = getString(0, '&', header.data);
                    buffer = getString(1 + strlen(server), '&', header.data);
                    poole = configServer(buffer, atoi(getString(2 + strlen(server) + strlen(buffer), '\0', header.data)));

                    free(buffer);
                    buffer = NULL;

                    poole_sock = socket(AF_INET, SOCK_STREAM, 0);

                    if (connect(poole_sock, (struct sockaddr *) &poole, sizeof(poole)) < 0) {
                        printF(C_BOLDRED);
                        printF("Error trying to connect to HAL 9000 system\n");
                        printF(C_RESET);
                        break;
                    }

                    asprintf(&buffer, T1_BOWMAN, config.user);
                    buffer = sendFrame(buffer, poole_sock);

                    header = readHeader(poole_sock);

                    if (header.type == '1' && strcmp(header.header, "CON_OK") == 0) {
                        asprintf(&buffer, C_GREEN "%s connected to HAL 9000 system, welcome music lover!\n" C_RESET, config.user);
                        printF(buffer);
                        connected = 1;
                    }
                    else if (header.type == '1' && strcmp(header.header, "CON_KO") == 0) {
                        printF(C_BOLDRED);
                        printF("Could not establish connection.\n");
                        printF(C_RESET);
                    }
                    else {
                        printF(C_BOLDRED);
                        printF("Received wrong frame\n");
                        printF(C_RESET);
                        sendError(poole_sock);
                        //close(sock);
                    }
                }
                else if (header.type == '1' && strcmp(header.header, "CON_KO") == 0) {
                    printF(C_BOLDRED);
                    printF("Could not establish connection.\n");
                    printF(C_RESET);
                }
                else if (header.type == '7') {
                    printF(C_BOLDRED);
                    printF("Sent wrong frame\n");
                    printF(C_RESET);
                    //close(sock);
                }
                else {
                    printF(C_BOLDRED);
                    printF("Received wrong frame\n");
                    printF(C_RESET);
                    sendError(discovery_sock);
                    //close(sock);
                }
                printF("\n");
                break;
            case 1:
                if (connected == 1) {
                    //cerrar poole sock y mandar frames
                    //close(poole_sock);

                    asprintf(&buffer, T6, config.user);
                    buffer = sendFrame(buffer, poole_sock);
                    header = readHeader(poole_sock);
                    
                    asprintf(&buffer, T6, server);
                    buffer = sendFrame(buffer, discovery_sock);
                    Header header2 = readHeader(discovery_sock);
                    if (header.type == '6' && strcmp(header.header, "CON_OK") == 0 && header2.type == '6' && strcmp(header2.header, "CON_OK") == 0) {                    
                        printF(C_GREEN);
                        printF("Thanks for using HAL 9000, see you soon, music lover!\n");
                        printF(C_RESET);
                        close(poole_sock);
                        close(discovery_sock);
                    }
                    else if ((header.type == '6' && strcmp(header.header, "CON_KO") == 0) || (header2.type == '6' && strcmp(header2.header, "CON_KO") == 0)) {
                        printF(C_RED);
                        printF("Could not disconnect from HAL 9000 system\n");
                        printF(C_RESET);
                    }
                    else {
                        printF(C_BOLDRED);
                        printF("Received wrong frame\n");
                        printF(C_RESET);
                        sendError(poole_sock);
                    }
                }
                
                goto end;
                break;
            case 2:
                //list songs
                if (connected == 0) {
                    printF(C_RED);
                    printF("ERROR: Not connected to HAL 9000 system\n");
                    printF(C_RESET);
                    break;
                }

                asprintf(&buffer, T2_SONGS);
                buffer = sendFrame(buffer, poole_sock);

                header = readHeader(poole_sock);
                asprintf(&buffer, "%sThere are %s songs available for download:\n%s", C_GREEN, header.data, C_RESET);
                printF(buffer);
                free(buffer);
                buffer = NULL;

                int num_songs = atoi(header.data);
                for (int i = 0; i < num_songs; i++) {
                    header = readHeader(poole_sock); //change read header for read songs/playlist function in phase3
                    asprintf(&buffer, "%d. %s\n", i + 1, header.data);
                    printF(buffer);
                    free(buffer);
                    buffer = NULL;
                }
                printF("\n");
                break;
            case 3:
                //list playlists
                if (connected == 0) {
                    printF(C_RED);
                    printF("ERROR: Not connected to HAL 9000 system\n");
                    printF(C_RESET);
                    break;
                }

                asprintf(&buffer, T2_PLAYLISTS);
                buffer = sendFrame(buffer, poole_sock);

                header = readHeader(poole_sock);
                asprintf(&buffer, "%sThere are %s playlists available for download:\n%s", C_GREEN, header.data, C_RESET);
                printF(buffer);
                free(buffer);
                buffer = NULL;

                int num_playlists = atoi(header.data);
                for (int i = 0; i < num_playlists; i++) {
                    header = readHeader(poole_sock); //change read header for read songs/playlist function in phase3
                    asprintf(&buffer, "%d. %s\n", i + 1, header.data);
                    printF(buffer);
                    free(buffer);
                    buffer = NULL;
                }
                printF("\n");
                break;
            case 4:
                printF(C_GREEN);
                printF("OK\n");
                printF(C_RESET);
                break;
            case 5:
                printF(C_GREEN);
                printF("OK\n");
                printF(C_RESET);
                break;
            case 6:
                printF(C_GREEN);
                printF("OK\n");
                printF(C_RESET);
                break;
            case 7:
                printF(C_BOLDRED);
                printF("KO\n");
                //printF("Unknown command.\n");
                printF(C_RESET);
                break;
            default:
                printF(C_BOLDRED);
                printF("KO\n");
                //printF("ERROR: Please input a valid command.\n");
                printF(C_RESET);
                break;
        }

        free(buffer);
        buffer = NULL;
    }

    end:
    free(buffer);
    free(config.user);
    free(config.files_path);
    free(config.ip);
    buffer = NULL;
    config.user = NULL;
    config.files_path = NULL;
    config.ip = NULL;

    return 0;
}
