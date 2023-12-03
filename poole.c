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

/********************************************************************
 *
 * @Purpose: Handles interactions with connected Bowman users, processing 
 *           requests and managing user sessions.
 * @Parameters: sock - Socket descriptor
 *              user_pos - Position in the user array
 *              users - Array of user names.
 * @Return: 0 on success, -1 on user disconnection or error.
 *
 ********************************************************************/
int bowmanHandler(int sock, int user_pos, char** users, Server_conf config) {
    Frame frame;
    char* buffer = NULL;

    frame = readFrame(sock);

    if (frame.type == '1' && strcmp(frame.header, "NEW_BOWMAN") == 0) {
        
        asprintf(&buffer, T1_OK);
        buffer = sendFrame(buffer, sock);
        
        users[user_pos] = getString(0, '\0', frame.data);
        asprintf(&buffer, "%s\nNew user connected: %s.\n%s", C_GREEN, users[user_pos], C_RESET);
        printF(buffer);
        free(buffer);
        buffer = NULL;
    }
    else if (frame.type == '2') {
        if (strcmp(frame.header, "LIST_SONGS") == 0) {
            asprintf(&buffer, "\n%sNew request - %s requires the list of songs.\n%sSending song list to %s\n", C_GREEN, users[user_pos], C_RESET, users[user_pos]);
            printF(buffer);
            free(buffer);
            buffer = NULL;

            //get number of songs and songs
            int num_songs = 0;
            char* file = NULL;
            asprintf(&file, "%s/songs.txt", config.path); 
            printF(config.path);
            printF("\n");
            char** songs = readSongs(file, &num_songs);
            free(file);
            file = NULL;

            //char* num_songs_str = itoa(num_songs, 10);
 
            asprintf(&buffer, T2_SONGS_RESPONSE, "7#");

            int buffer_length = strlen(buffer);
            int remaining_space = 256 - buffer_length - 1;

            for (int i = 0; i < num_songs; i++) {
                int song_length = strlen(songs[i]);
                
                if (song_length <= remaining_space) {
                    if (i != 0) {
                        buffer = realloc(buffer, buffer_length + 2);
                        buffer[buffer_length + 1] = '\0';
                        buffer[buffer_length] = '&';
                        buffer_length++;
                        remaining_space -= 1;
                    }
                    buffer = realloc(buffer, buffer_length + song_length + 1);
                    buffer[buffer_length + song_length] = '\0';
                    strcpy(buffer + buffer_length, songs[i]);

                    buffer_length += song_length;               
                    remaining_space -= song_length; 
                    printf("bflen = %d\n", buffer_length);
                } else {
                    // Not enough space -> send the current buffer
                    printF("\n\n1\n");
                    printF(buffer);
                    printF("\n");
                    sendFrame(buffer, sock);

                    // Reset the buffer for the next iteration
                    asprintf(&buffer, T2_SONGS_RESPONSE, "7#");
                    buffer_length = strlen(buffer);
                    remaining_space = 256 - buffer_length - 1;

                    // Process the song again
                    i--;
                }
            }
            if ((int)buffer_length > (int)strlen(T2_SONGS_RESPONSE)) {
                printF("\n\n2\n");
                printF(buffer);
                printF("\n");
                sendFrame(buffer, sock);
            }
        }
        else if (strcmp(frame.header, "LIST_PLAYLISTS") == 0) {
            asprintf(&buffer, "\n%sNew request - %s requires the list of playlists.\n%sSending playlist list to %s\n", C_GREEN, users[user_pos], C_RESET, users[user_pos]);
            printF(buffer);
            free(buffer);
            buffer = NULL;

            //get number of playlists and songs
            int num_playlists = 0;
            char* file;
            asprintf(&file, "%s/playlists.txt", config.path); 
            Playlist* playlists = readPlaylists(file, &num_playlists);
            free(file);
            file = NULL;

            asprintf(&buffer, T2_PLAYLISTS_RESPONSE"#%d", num_playlists, playlists[0].num_songs); 
            buffer = sendFrame(buffer, sock);

            for (int i = 0; i < num_playlists; i++) {
                //poner name playlist
                for (int j = 0; j < playlists[i].num_songs; j++) {
                    //poner songs hasta 256 bytes
                    //mandar frame
                    //vaciar frame
                    //rellenar frame type, length, header
                    // volver a inicio y poner songs otra vez 
                }
            }
        }
        else {
            printF("Wrong frame\n");
            sendError(sock);
        }

    }
    else if (frame.type == '6' && strcmp(frame.header, "EXIT") == 0) {
        asprintf(&buffer, T6_OK);
        buffer = sendFrame(buffer, sock);
        asprintf(&buffer, "\n%sUser %s disconnected%s\n", C_RED, frame.data, C_RESET);
        printF(buffer);
        free(buffer);
        buffer = NULL;

        return -1;
    }
    else if (frame.type == '7') {
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

/********************************************************************
 *
 * @Purpose: Accepts incoming connections from Bowman users, managing users 
 *           and allocating resources.
 * @Parameters: sock - Server socket descriptor.
 * @Return: 0 on success, -1 on error.
 *
 ********************************************************************/
static int listenConnections(int sock, Server_conf config) {
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
                if (acceptConnection(&num_users, &users_fd, "bowman", sock, 0) == -1) {
                    return -1;
                }
                users = (char**) realloc(users, sizeof(char*) * num_users);
            }
            for (int i = 0; i < num_users; i++) {
                if (FD_ISSET(users_fd[i], &readfds)) {
                    if (bowmanHandler(users_fd[i], i, users, config) == -1) {
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

void sig_handler(int sigsum) {
    switch(sigsum) {
        case SIGINT:
            printF("\nAborting...\n");

            
            exit(0);
            break;
    }
}
/********************************************************************
 *
 * @Purpose: Initializes the Poole server, connecting to the Discovery server 
 *           and managing incoming Bowman user connections.
 * @Parameters: argc - The number of command-line arguments.
 *              argv - An array of the command-line arguments.
 * @Return: 0 if successful, -1 otherwise.
 *
 ********************************************************************/
int main(int argc, char *argv[]) {
    char* buffer;
    struct sockaddr_in server;
    int sock;
    Server_conf config;
    Frame frame;
    signal(SIGINT, sig_handler);
    
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

    frame = readFrame(sock);

    if (frame.type == '1' && strcmp(frame.header, "CON_OK") == 0) {
        close(sock); 

        server = configServer(config.user_ip, config.user_port);
        sock = socket(AF_INET, SOCK_STREAM, 0);
        
        if (openConnection(sock, server, "bowman") == -1) {
            return -1;
        }

        asprintf(&buffer, C_GREEN "Connected to HAL 9000 System, ready to listen to Bowmans petitions\n" C_RESET);
        printF(buffer);
        free(buffer);
        buffer = NULL;

        if (listenConnections(sock, config) == -1) {
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