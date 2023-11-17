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
    char* buffer = NULL;
    int sock, connected = 0;
    struct sockaddr_in server;
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

    if (configConnection(&sock, &server) == -1) {
        return -1;
    }

    asprintf(&buffer, "\n%s user initialized\n", config.user);
    printF(BOLD);
    printF(buffer);
    printF(C_RESET);
    free(buffer);
    buffer = NULL;

    if (checkPort(config.port) == -1) {
        printF(C_BOLDRED);
        printF("ERROR: Invalid port.\n");
        printF(C_RESET);
        return -1;
    }
    
    while(1) {
        printF("$ ");
        readLine(0, &buffer);
        switch (checkCommand(buffer)) {
            case 0:
                if (connected == 1) {
                    printF(C_RED);
                    printF("ERROR: Already connected to HAL 9000 system\n");
                    printF(C_RESET);
                    break;
                }


                if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
                    printF(C_BOLDRED);
                    printF("Error trying to connect to HAL 9000 system\n");
                    printF(C_RESET);
                    
                    return -1;
                }

                free(buffer);
                buffer = NULL;
                asprintf(&buffer, T1_BOWMAN, config.user);
                buffer = sendFrame(buffer, sock);

                header = readHeader(sock);

                if (header.type == '1' && strcmp(header.header, "CON_OK") == 0) {
                    //close(sock); //como cerrarlo
                    buffer = getString(0, '&', header.data);
                    char* aux = getString(1 + strlen(buffer), '&', header.data);
                    server = configServer(aux, atoi(getString(2 + strlen(buffer) + strlen(aux), '\0', header.data)));

                    free(buffer);
                    free(aux);
                    buffer = NULL;
                    aux = NULL;

                    sock = socket(AF_INET, SOCK_STREAM, 0);

                    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
                        printF(C_BOLDRED);
                        printF("Error trying to connect to HAL 9000 system\n");
                        printF(C_RESET);
                        break;
                    }

                    asprintf(&buffer, C_GREEN "%s connected to HAL 9000 system, welcome music lover!\n" C_RESET, config.user);
                    printF(buffer);
                    connected = 1;
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
                    sendError(sock);
                    //close(sock);
                }
                break;
            case 1:
                if (connected == 1) {
                    close(sock);
                }
                printF(C_GREEN);
                printF("Thanks for using HAL 9000, see you soon, music lover!\n");
                printF(C_RESET);
                goto end;
                break;
            case 2:
                printF(C_GREEN);
                printF("OK\n");
                printF(C_RESET);
                break;
            case 3:
                printF(C_GREEN);
                printF("OK\n");
                printF(C_RESET);
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
