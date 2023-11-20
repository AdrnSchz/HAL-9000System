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
* @Purpose: Handles the SIGINT signal for aborting the program.
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
 * @Purpose: Establishes a socket connection with the server using the information from the 'config' structure.
 * @Parameters: sock - A pointer to the socket variable.
 *              server - A pointer to the server address structure.
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
 * @Purpose: Main function that initializes the Bowman user and handles user interactions.
 * @Parameters: argc - The number of command-line arguments.
 *              argv - An array of the command-line arguments.
 * @Return: 0 if successful, -1 otherwise.
 *
 ********************************************************************/
int main(int argc, char *argv[]) {
    char* buffer = NULL, *server = NULL;
    int discovery_sock, poole_sock, connected = 0;
    struct sockaddr_in discovery, poole;
    Frame frame;
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

    asprintf(&buffer, "%s user initialized\n", config.user);
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
        printF("\n$ ");
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

                frame = readFrame(discovery_sock);

                if (frame.type == '1' && strcmp(frame.header, "CON_OK") == 0) {
                    server = getString(0, '&', frame.data);
                    buffer = getString(1 + strlen(server), '&', frame.data);
                    char* aux = getString(2 + strlen(server) + strlen(buffer), '\0', frame.data);
                    poole = configServer(buffer, atoi(aux));
                    

                    free(buffer);
                    buffer = NULL;
                    free(aux);
                    aux = NULL;

                    poole_sock = socket(AF_INET, SOCK_STREAM, 0);

                    if (connect(poole_sock, (struct sockaddr *) &poole, sizeof(poole)) < 0) {
                        printF(C_BOLDRED);
                        printF("Error trying to connect to HAL 9000 system\n");
                        printF(C_RESET);
                        break;
                    }

                    asprintf(&buffer, T1_BOWMAN, config.user);
                    buffer = sendFrame(buffer, poole_sock);
                    
                    frame = freeFrame(frame);
                    frame = readFrame(poole_sock);

                    if (frame.type == '1' && strcmp(frame.header, "CON_OK") == 0) {
                        asprintf(&buffer, C_GREEN "%s connected to HAL 9000 system, welcome music lover!\n" C_RESET, config.user);
                        printF(buffer);
                        connected = 1;
                    }
                    else if (frame.type == '1' && strcmp(frame.header, "CON_KO") == 0) {
                        printF(C_RED);
                        printF("Could not establish connection.\n");
                        printF(C_RESET);
                    }
                    else {
                        printF(C_RED);
                        printF("Received wrong frame\n");
                        printF(C_RESET);
                        sendError(poole_sock);
                    }
                }
                else if (frame.type == '1' && strcmp(frame.header, "CON_KO") == 0) {
                    printF(C_RED);
                    printF("Could not establish connection.\n");
                    printF(C_RESET);
                }
                else if (frame.type == '7') {
                    printF(C_RED);
                    printF("Sent wrong frame\n");
                    printF(C_RESET);
                }
                else {
                    printF(C_RED);
                    printF("Received wrong frame\n");
                    printF(C_RESET);
                    sendError(discovery_sock);
                }
                frame = freeFrame(frame);
                break;
            case 1:
                free(buffer);
                buffer = NULL;
                if (connected == 1) {
                    asprintf(&buffer, T6, config.user);
                    buffer = sendFrame(buffer, poole_sock);
                    frame = readFrame(poole_sock);
                    
                    asprintf(&buffer, T6, server);
                    buffer = sendFrame(buffer, discovery_sock);
                    Frame frame2 = readFrame(discovery_sock);
                    if (frame.type == '6' && strcmp(frame.header, "CON_OK") == 0 && frame2.type == '6' && strcmp(frame2.header, "CON_OK") == 0) {                    
                        printF(C_GREEN);
                        printF("Thanks for using HAL 9000, see you soon, music lover!\n");
                        printF(C_RESET);
                        close(poole_sock);
                        close(discovery_sock);
                    }
                    else if ((frame.type == '6' && strcmp(frame.header, "CON_KO") == 0) || (frame2.type == '6' && strcmp(frame2.header, "CON_KO") == 0)) {
                        printF(C_RED);
                        printF("Could not disconnect from HAL 9000 system\n");
                        printF(C_RESET);
                    }
                    else {
                        printF(C_RED);
                        printF("Received wrong frame\n");
                        printF(C_RESET);
                        sendError(poole_sock);
                    }
                    frame = freeFrame(frame);
                    frame2 = freeFrame(frame2);
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
                
                free(buffer);
                buffer = NULL;
                asprintf(&buffer, T2_SONGS);
                buffer = sendFrame(buffer, poole_sock);

                frame = readFrame(poole_sock);
                asprintf(&buffer, "%sThere are %s songs available for download:\n%s", C_GREEN, frame.data, C_RESET);
                printF(buffer);
                free(buffer);
                buffer = NULL;

                int num_songs = atoi(frame.data);
                for (int i = 0; i < num_songs; i++) {
                    frame = freeFrame(frame);
                    frame = readFrame(poole_sock); //change read header for read songs/playlist function in phase3
                    asprintf(&buffer, "%d. %s\n", i + 1, frame.data);
                    printF(buffer);
                    free(buffer);
                    buffer = NULL;
                }

                frame = freeFrame(frame);
                break;
            case 3:
                //list playlists
                if (connected == 0) {
                    printF(C_RED);
                    printF("ERROR: Not connected to HAL 9000 system\n");
                    printF(C_RESET);
                    break;
                }

                free(buffer);
                buffer = NULL;
                asprintf(&buffer, T2_PLAYLISTS);
                buffer = sendFrame(buffer, poole_sock);

                frame = readFrame(poole_sock);
                asprintf(&buffer, "%sThere are %s playlists available for download:\n%s", C_GREEN, frame.data, C_RESET);
                printF(buffer);
                free(buffer);
                buffer = NULL;

                int num_playlists = atoi(frame.data);
                for (int i = 0; i < num_playlists; i++) {
                    frame = freeFrame(frame);
                    frame = readFrame(poole_sock); //change read header for read songs/playlist function in phase3
                    asprintf(&buffer, "%d. %s\n", i + 1, frame.data);
                    printF(buffer);
                    free(buffer);
                    buffer = NULL;
                }
                frame = freeFrame(frame);
                break;
            case 4:
                free(buffer);
                buffer = NULL;
                printF(C_GREEN);
                printF("OK\n");
                printF(C_RESET);
                break;
            case 5:
                free(buffer);
                buffer = NULL;
                printF(C_GREEN);
                printF("OK\n");
                printF(C_RESET);
                break;
            case 6:
                free(buffer);
                buffer = NULL;
                printF(C_GREEN);
                printF("OK\n");
                printF(C_RESET);
                break;
            case 7:
                free(buffer);
                buffer = NULL;
                printF(C_RED);
                //printF("KO\n");
                printF("Unknown command.\n");
                printF(C_RESET);
                break;
            default:
                free(buffer);
                buffer = NULL;
                printF(C_RED);
                //printF("KO\n");
                printF("ERROR: Please input a valid command.\n");
                printF(C_RESET);
                break;
        }
    }

    end:
    free(server);
    server = NULL;
    free(config.user);
    free(config.files_path);
    free(config.ip);
    config.user = NULL;
    config.files_path = NULL;
    config.ip = NULL;

    return 0;
}
