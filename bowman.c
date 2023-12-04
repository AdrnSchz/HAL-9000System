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
int discovery_sock, poole_sock = 0;
char* server_name = NULL;

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
 * @Purpose: Connect and ask discovery for a server and connect to the given server
 * @Parameters: poole - sockaddr_in with the configuration to connect to the poole server.
 *              discovery - sockaddr_in with the configuration to connect to the discovery server.
 * @Return: ---
 *
 ********************************************************************/
void connection(struct sockaddr_in poole, struct sockaddr_in discovery, int opt) {
    char* buffer = NULL, *aux = NULL;
    Frame frame;

    if (opt == 0) {
        if (connect(discovery_sock, (struct sockaddr *) &discovery, sizeof(discovery)) < 0) {
            printF(C_RED);
            printF("Error trying to connect to load balancer\n");
            printF(C_RESET);
            
            return;
        }
    }

    asprintf(&buffer, T1_BOWMAN, config.user);
    buffer = sendFrame(buffer, discovery_sock);

    frame = readFrame(discovery_sock);

    if (frame.type == '1' && strcmp(frame.header, "CON_OK") == 0) {
        server_name = getString(0, '&', frame.data);
        buffer = getString(1 + strlen(server_name), '&', frame.data);
        aux = getString(2 + strlen(server_name) + strlen(buffer), '\0', frame.data);
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
            return;
        }

        asprintf(&buffer, T1_BOWMAN, config.user);
        buffer = sendFrame(buffer, poole_sock);
        
        frame = freeFrame(frame);
        frame = readFrame(poole_sock);

        if (frame.type == '1' && strcmp(frame.header, "CON_OK") == 0) {
            asprintf(&buffer, "%s%s connected to HAL 9000 system, welcome music lover!\n%s", C_GREEN, config.user, C_RESET);
            printF(buffer);
            free(buffer);
            buffer = NULL;
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
}

/********************************************************************
 *
 * @Purpose: Logout from the poole
 * @Return: ---
 *
 ********************************************************************/
void logout() {
    char* buffer = NULL;
    Frame frame, frame2;

    asprintf(&buffer, T6, config.user);
    buffer = sendFrame(buffer, poole_sock);
    frame = readFrame(poole_sock);
    
    asprintf(&buffer, T6, server_name);
    buffer = sendFrame(buffer, discovery_sock);
    frame2 = readFrame(discovery_sock);
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

int frameInput() {
    Frame frame;
    char* buffer = NULL;

    frame = readFrame(poole_sock);

    if (frame.type == '6' && strcmp(frame.header, "SHUTDOWN") == 0) {
        asprintf(&buffer, T6_OK);
        buffer = sendFrame(buffer, poole_sock);
        asprintf(&buffer, "\n%sServer %s got unexpectedly disconnected\n%s", C_RED, frame.data, C_RESET);
        printF(buffer);
        free(buffer);
        buffer = NULL;

        close(poole_sock);
        frame = freeFrame(frame);
        
        return 6;
    }

    return 0;
}
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

            if (poole_sock != 0) {
                logout();
            }
            free(server_name);
            server_name = NULL;
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
 * @Purpose: Main function that initializes the Bowman user and handles user interactions.
 * @Parameters: argc - The number of command-line arguments.
 *              argv - An array of the command-line arguments.
 * @Return: 0 if successful, -1 otherwise.
 *
 ********************************************************************/
int main(int argc, char *argv[]) {
    char* buffer = NULL;
    int alreadyPrinted = 0;
    int totalSongs = 0;
    int totalPlaylists = 0;
    char* num_songs_str = NULL;
    char* num_playlists_str = NULL;
    char* song = NULL;
    int num_songs = 0;
    char* playlist_name = NULL;

    struct sockaddr_in discovery, poole;
    Frame frame;
    fd_set readfds;
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
        FD_ZERO(&readfds);
        FD_SET(poole_sock, &readfds);
        FD_SET(0, &readfds);

        printF(BOLD);
        printF("\n$ ");

        int ready = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);

        if (ready <= 0) {
            printF(C_RED);
            printF("ERROR: Select failed.\n");
            printF(C_RESET);

            return -1;
        }

        else {
            if (FD_ISSET(0, &readfds)) {
                readLine(0, &buffer);
                printF(C_RESET);
                switch (checkCommand(buffer)) {
                    case 0:
                        free(buffer);
                        buffer = NULL;
                        if (poole_sock != 0) {
                            printF(C_RED);
                            printF("ERROR: Already connected to HAL 9000 system\n");
                            printF(C_RESET);
                            break;
                        }

                        connection(poole, discovery, 0);

                break;
            case 1:
                free(buffer);
                buffer = NULL;
                if (poole_sock != 0) {
                    logout();
                }
                
                goto end;
                break;
            case 2:
                // ==================================================
                // LIST SONGS
                // ==================================================
                free(buffer);
                buffer = NULL;
                if (poole_sock == 0) {
                    printF(C_RED);
                    printF("ERROR: Not connected to HAL 9000 system\n");
                    printF(C_RESET);
                    break;
                }
                
                asprintf(&buffer, T2_SONGS);
                buffer = sendFrame(buffer, poole_sock);
                alreadyPrinted = 0;

                while (1) {
                    frame = readFrame(poole_sock);
                    num_songs_str = strtok(frame.data, "#");

                    if (alreadyPrinted == 0) {
                        asprintf(&buffer, "%sThere are %s songs available for download:\n%s", C_GREEN, num_songs_str, C_RESET);
                        printF(buffer);
                        free(buffer);
                        buffer = NULL;
                        alreadyPrinted = 1;
                    }

                    song = strtok(NULL, "&"); // NULL to continue from last strtok
                    num_songs = atoi(num_songs_str);

                    for (int i = 0; i < num_songs; i++) {
                        if (song != NULL) {
                            totalSongs += 1;
                            asprintf(&buffer, "%d. %s\n", totalSongs, song);
                            printF(buffer);
                            free(buffer);
                            buffer = NULL;

                            song = strtok(NULL, "&"); // NULL to continue from last strtok
                        }
                    }
                    if (totalSongs >= num_songs) {
                        break;
                    }
                }
                totalSongs = 0;
                frame = freeFrame(frame);
                break;
            case 3:
                // ==================================================
                // LIST PLAYLISTS
                // ==================================================
                free(buffer);
                buffer = NULL;
                if (poole_sock == 0) {
                    printF(C_RED);
                    printF("ERROR: Not connected to HAL 9000 system\n");
                    printF(C_RESET);
                    break;
                }
                
                asprintf(&buffer, T2_PLAYLISTS);
                buffer = sendFrame(buffer, poole_sock);
                alreadyPrinted = 0;

                while (1) {
                    frame = readFrame(poole_sock);
                    num_playlists_str = strtok(frame.data, "#");

                    if (alreadyPrinted == 0) {
                        asprintf(&buffer, "%sThere are %s playlists available for download:\n%s", C_GREEN, num_playlists_str, C_RESET);
                        printF(buffer);
                        free(buffer);
                        buffer = NULL;
                        alreadyPrinted = 1;
                    }
                    
                    int num_playlists = atoi(num_playlists_str);
                    for (int i = 0; i < num_playlists; i++) {
                        totalPlaylists += 1;
                        num_songs_str = strtok(NULL, "#");
                        int num_songs = atoi(num_songs_str);
                        playlist_name = strtok(NULL, "&");

                        asprintf(&buffer, "%d. %s\n", i + 1, playlist_name);
                        printF(buffer);
                        free(buffer);
                        buffer = NULL;

                        for (int j = 0; j < num_songs; j++) {
                            song = strtok(NULL, "&#");
                            asprintf(&buffer, "\t%d. %s\n", j + 1, song);
                            printF(buffer);
                            free(buffer);
                            buffer = NULL;
                        }
                    }
                    if (totalPlaylists >= num_playlists) {
                        break;
                    }
                }
                totalPlaylists = 0;
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
            else if (FD_ISSET(poole_sock, &readfds)) {
                if (frameInput() == 6) {
                    connection(poole, discovery, 1);
                }
            }

        }
    }

    end:
    free(server_name);
    server_name = NULL;
    free(config.user);
    free(config.files_path);
    free(config.ip);
    config.user = NULL;
    config.files_path = NULL;
    config.ip = NULL;

    return 0;
}
