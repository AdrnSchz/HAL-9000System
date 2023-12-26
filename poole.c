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

int bow_sock = 0;
Server_conf config;
int* users_fd;
int num_users = 0, num_threads = 0;
char** users;
pthread_t* threads;
int* ids;
pthread_mutex_t terminal = PTHREAD_MUTEX_INITIALIZER, globals = PTHREAD_MUTEX_INITIALIZER;

void* sendFile(void* arg) {
    Send* send = (Send*) arg;
    int fd_file, size = 0, sent = 0;
    char* buffer = NULL, *file = NULL, *md5 = NULL, *data;
    int index = send->thread_pos;

    asprintf(&file, "%s/%s", config.path, send->name);

    // md5sum
    pthread_mutex_lock(&terminal);
    getMd5(file, &md5);
    pthread_mutex_unlock(&terminal);
    if (md5 == NULL) {
        asprintf(&buffer, C_RED "Error getting md5sum.\n" C_RESET);
        printF(buffer);
        free(buffer);
        asprintf(&buffer, T4_NEW_FILE, "-", 0, "-", -1);
        buffer = sendFrame(buffer, users_fd[send->fd_pos], strlen(buffer));
        return NULL;
    }
    file = NULL;
    asprintf(&file, "%s/%s", config.path, send->name);

    // get random(id)
    pthread_mutex_lock(&globals);
    srand(getpid());
    do {
        ids[index] = rand() % (999 + 1);
        for (int i = 0; i < num_threads; i++) {
            if (i == index) {
                continue;
            }
            if (ids[index] == ids[i]) {
                ids[index] = -1;
                break;
            }
        }
    } while (ids[index] == -1);
    pthread_mutex_unlock(&globals);

    // get size and read file
    fd_file = open(file, O_RDONLY);
    if (fd_file == -1) {
        asprintf(&buffer, C_RED "ERROR: %s not found.\n" C_RESET, file);
        printF(buffer);
        free(buffer);
        buffer = NULL;
        asprintf(&buffer, T4_NEW_FILE, "-", 0, "-", -1);
        buffer = sendFrame(buffer, users_fd[send->fd_pos], strlen(buffer));
        return NULL;
    }

    size = (int) lseek(fd_file, 0, SEEK_END);
    lseek(fd_file, 0, SEEK_SET);
    data = (char*) malloc(sizeof(char) * size);
    read(fd_file, data, size);
    close (fd_file);

    // send frame
    asprintf(&buffer, T4_NEW_FILE, send->name, size, md5, ids[index]);
    buffer = sendFrame(buffer, users_fd[send->fd_pos], strlen(buffer));

    asprintf(&buffer, "%d", ids[index]);
    int space = 256 - 3 - 9 - strlen(buffer) - 1;
    int occupied = 3 + 9 + strlen(buffer) + 1;
    free(buffer);
    buffer = NULL;

    //send file
    while (sent < size) {
        if (size - sent < space) {
            space = size - sent;
        }
        asprintf(&buffer, T4_DATA, ids[index]);
        buffer = realloc(buffer, 256);
        memcpy(buffer + strlen(buffer), data + sent, space);
        buffer = sendFrame(buffer, users_fd[send->fd_pos], space + occupied);
        sent += space;
        free(buffer);
        buffer = NULL;
    }

    //mirar de cambiar esto, cuando se descargan muchos a la vez este puede recibir el md5sum de otro
    Frame frame = readFrame(users_fd[send->fd_pos]);

    if (frame.type == '5' && strcmp(frame.header, "CHECK_OK") == 0) asprintf(&buffer, "%sSuccessfully sent %s to %s\n%s", C_GREEN, send->name, users[send->fd_pos], C_RESET);
    else asprintf(&buffer, "%sError sending %s to %s\n%s", C_RED, send->name, users[send->fd_pos], C_RESET);
    printF(buffer);
    free(buffer);
    free(file);
    free(md5);
    free(data);
    free(send->name);
    free(send);
    frame = freeFrame(frame);

    return NULL;
}

void downloadSong(char* song, int user_pos, int isList) {
    char* buffer, *file = NULL;
    int num_songs = 0, found = 0;
    Send* send = malloc(sizeof(Send));

    if (isList == 0) {
        asprintf(&buffer, "\n%sNew request - %s wants to download %s.\n%s", C_GREEN, users[user_pos], song, C_RESET);
        printF(buffer);
        free(buffer);
        buffer = NULL;
    }
    
    asprintf(&file, "%s/songs.txt", config.path);
    int fd_file = open(file, O_RDONLY);

    if (fd_file == -1) {
        asprintf(&buffer,C_RED "ERROR: %s not found.\n" C_RESET, file);
        printF(buffer);
        free(buffer);
        buffer = NULL;
        asprintf(&buffer, T4_NEW_FILE, "-", 0, "-", -1);
        buffer = sendFrame(buffer, users_fd[user_pos], strlen(buffer));
        return;
    }
    
    readNum(fd_file, &num_songs);
    for (int i = 0; i < num_songs; i++) {
        readLine(fd_file, &buffer);

        if (strcmp(buffer, song) == 0) {
            found = 1;
            send->name = malloc(strlen(buffer) + 1);
            strcpy(send->name, buffer);
            send->fd_pos = user_pos;
            buffer = NULL;
            break;
        }
        free(buffer);
        buffer = NULL;
    }
    close(fd_file);

    if (found == 0) {
        asprintf(&buffer, "Song not found\n");
        printF(buffer);
        free(buffer);
        buffer = NULL;
        asprintf(&buffer, T4_NEW_FILE, "-", 0, "-", -1);
        buffer = sendFrame(buffer, users_fd[user_pos], strlen(buffer));
        return;
    }
    asprintf(&buffer, "Sending %s to %s\n", song, users[user_pos]);
    printF(buffer);
    free(buffer);
    buffer = NULL;

    free(file);
    file = NULL;

    pthread_mutex_lock(&globals);
    ids = realloc(ids, sizeof(int) * (num_threads + 1));
    ids[num_threads] = -1;
    send->thread_pos = num_threads;
    num_threads++;
    threads = realloc(threads, sizeof(pthread_t) * (num_threads));
    pthread_mutex_unlock(&globals);
    pthread_create(&threads[send->thread_pos], NULL, sendFile, send);
}

void downloadList(char* list, int user_pos) {
    char* buffer, *file = NULL;
    int num_playlists = 0, num_songs = 0, found = 0;

    asprintf(&buffer, "\n%sNew request - %s wants to download the playlist %s.\n%s", C_GREEN, users[user_pos], list, C_RESET);
    printF(buffer);
    free(buffer);
    buffer = NULL;
    
    asprintf(&file, "%s/playlists.txt", config.path);
    int fd_file;
    fd_file = open(file, O_RDONLY);

    if (fd_file == -1) {
        asprintf(&buffer,C_RED "ERROR: %s not found.\n" C_RESET, file);
        printF(buffer);
        free(buffer);
        buffer = NULL;
        asprintf(&buffer, T4_NEW_FILE, "-", 0, "-", -1);
        buffer = sendFrame(buffer, users_fd[user_pos], strlen(buffer));
        return;
    }

    readNum(fd_file, &num_playlists);
    for (int i = 0; i < num_playlists; i++) {
        readNum(fd_file, &num_songs);
        readLine(fd_file, &buffer);
        
        if (strcmp(buffer, list) == 0) {
            found = 1;
            free(buffer);
            for (int j = 0; j < num_songs; j++) {
                readLine(fd_file, &buffer);
                downloadSong(buffer, user_pos, 1);
                free(buffer);
                buffer = NULL;
            }
            break;
        }

        for (int j = 0; j < num_songs; j++) {
            readLine(fd_file, &buffer);
            free(buffer);
            buffer = NULL;
        }
    }
    close(fd_file);

    if (found == 0) {
        asprintf(&buffer, "Playlist not found\n");
        printF(buffer);
        free(buffer);
        buffer = NULL;
        asprintf(&buffer, T4_NEW_FILE, "-", 0, "-", -1);
        buffer = sendFrame(buffer, users_fd[user_pos], strlen(buffer));
        return;
    }
    asprintf(&buffer, "Sending %s to %s. A total of %d songs will be sent", list, users[user_pos], num_songs);
    printF(buffer);
    free(buffer);
    buffer = NULL;

    free(file);
    file = NULL;
}
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
int bowmanHandler(int sock, int user_pos) {
    Frame frame;
    char* buffer = NULL;
    int buffer_length = 0;
    int remaining_space = 0;

    frame = readFrame(sock);

    if (frame.type == '1' && strcmp(frame.header, "NEW_BOWMAN") == 0) {
        int found = 0;
        asprintf(&buffer, T1_OK);
        buffer = sendFrame(buffer, sock, strlen(buffer));
        
        users[user_pos] = getString(0, '\0', frame.data);

        for (int i = 0; i < num_users; i++) {
            if (strcmp(users[user_pos], users[i]) == 0) {
                found++;
            }
        }

        if (found == 1) {
            asprintf(&buffer, "%s\nNew user connected: %s.\n%s", C_GREEN, users[user_pos], C_RESET);
            printF(buffer);
            free(buffer);
            buffer = NULL;
        }
    }
    else if (frame.type == '2') {
        if (strcmp(frame.header, "LIST_SONGS") == 0) {
            asprintf(&buffer, "\n%sNew request - %s requires the list of songs.\n%sSending song list to %s\n", C_GREEN, users[user_pos], C_RESET, users[user_pos]);
            printF(buffer);
            free(buffer);
            buffer = NULL;

            // Get number of songs and songs
            int num_songs = 0;
            char* file = NULL;
            asprintf(&file, "%s/songs.txt", config.path);
            printF(config.path);
            printF("\n");
            char** songs = readSongs(file, &num_songs);
            free(file);
            file = NULL;

            char* num_songs_str = NULL;

            asprintf(&num_songs_str, "%d#", num_songs);
            asprintf(&buffer, T2_SONGS_RESPONSE, num_songs_str);
            free(num_songs_str);
            num_songs_str = NULL;

            buffer_length = strlen(buffer);
            remaining_space = 256 - buffer_length - 1;

            for (int i = 0; i < num_songs; i++) {
                int song_length = strlen(songs[i]);
                
                if (song_length <= remaining_space) {
                    if (i != 0) {
                        buffer = realloc(buffer, buffer_length + 2);
                        //buffer[buffer_length + 1] = '\0';
                        buffer[buffer_length] = '&';
                        buffer_length++;
                        remaining_space -= 1;
                    }
                    buffer = realloc(buffer, buffer_length + song_length + 1);
                    buffer[buffer_length + song_length] = '\0';
                    strcpy(buffer + buffer_length, songs[i]);

                    buffer_length += song_length;               
                    remaining_space -= song_length; 
                } else {
                    // Not enough space -> send the current buffer
                    buffer = sendFrame(buffer, sock, strlen(buffer));

                    // Reset the buffer for the next iteration
                    asprintf(&num_songs_str, "%d#", num_songs);
                    asprintf(&buffer, T2_SONGS_RESPONSE, num_songs_str);
                    free(num_songs_str);
                    num_songs_str = NULL;
                    buffer_length = strlen(buffer);
                    remaining_space = 256 - buffer_length - 1;

                    // Process the song again
                    i--;
                }
            }
            // Enough space -> send the current buffer
            buffer = sendFrame(buffer, sock, strlen(buffer));
            buffer_length = 0;
            remaining_space = 0;
        }
        else if (strcmp(frame.header, "LIST_PLAYLISTS") == 0) {
            asprintf(&buffer, "\n%sNew request - %s requires the list of playlists.\n%sSending playlist list to %s\n", C_GREEN, users[user_pos], C_RESET, users[user_pos]);
            printF(buffer);
            free(buffer);
            buffer = NULL;

            // Get number of playlists and songs
            int num_playlists = 0;
            char* file;
            asprintf(&file, "%s/playlists.txt", config.path); 
            Playlist* playlists = readPlaylists(file, &num_playlists);
            free(file);
            file = NULL;

            char* num_playlists_str = NULL;
            char* num_songs_str = NULL;
        
            asprintf(&num_playlists_str, "%d", num_playlists);
            asprintf(&buffer, T2_PLAYLISTS_RESPONSE, num_playlists_str);
            free(num_playlists_str);
            num_playlists_str = NULL;

            buffer_length = strlen(buffer);
            remaining_space = 256 - buffer_length - 1;
            
            for (int i = 0; i < num_playlists; i++) {
                int playlist_length = strlen(playlists[i].name);
                if (playlist_length <= remaining_space) {
                    
                    asprintf(&num_songs_str, "#%d#", playlists[i].num_songs);
                    buffer = realloc(buffer, buffer_length + playlist_length + strlen(num_songs_str) + 1);
                    buffer[buffer_length + playlist_length] = '\0';
                    strcat(buffer + buffer_length, num_songs_str);

                    buffer_length += strlen(num_songs_str);
                    remaining_space -= strlen(num_songs_str);

                    free(num_songs_str);
                    
                    buffer = realloc(buffer, buffer_length + playlist_length + 1);
                    buffer[buffer_length + playlist_length] = '\0';
                    strcpy(buffer + buffer_length, playlists[i].name);

                    buffer_length += playlist_length;
                    remaining_space -= playlist_length;

                    // Add songs to playlist
                    for (int j = 0; j < playlists[i].num_songs; j++) {
                        int song_length = strlen(playlists[i].songs[j]);
                        if (song_length <= remaining_space) {
                            
                            buffer = realloc(buffer, buffer_length + 2);
                            buffer[buffer_length + 1] = '\0';
                            buffer[buffer_length] = '&';
                            buffer_length++;
                            remaining_space -= 1;

                            buffer = realloc(buffer, buffer_length + song_length + 1);
                            buffer[buffer_length + song_length] = '\0';
                            strcpy(buffer + buffer_length, playlists[i].songs[j]);

                            buffer_length += song_length;               
                            remaining_space -= song_length; 
                        } else {
                            // Not enough space -> send the current buffer
                            printF("\n\n3\n");
                            buffer = sendFrame(buffer, sock, strlen(buffer));

                            // Reset the buffer for the next iteration
                            asprintf(&num_playlists_str, "%d", num_playlists);
                            asprintf(&buffer, T2_PLAYLISTS_RESPONSE, num_playlists_str);
                            free(num_playlists_str);
                            num_playlists_str = NULL;

                            buffer_length = strlen(buffer);
                            remaining_space = 256 - buffer_length - 1;

                            // Process the playlist again
                            i -= 1;
                            // Process the song again
                            j = 0;
                            break;
                        }
                    }
                } else {
                    // Not enough space -> send the current buffer
                    printF("\n\n1\n");
                    buffer = sendFrame(buffer, sock, strlen(buffer));

                    // Reset the buffer for the next iteration
                    asprintf(&num_playlists_str, "%d", num_playlists);
                    asprintf(&buffer, T2_PLAYLISTS_RESPONSE, num_playlists_str);
                    free(num_playlists_str);
                    num_playlists_str = NULL;

                    buffer_length = strlen(buffer);
                    remaining_space = 256 - buffer_length - 1;

                    // Process the playlist again
                    i -= 1;
                }   
            }
            printF("\n\n2\n");
            buffer = sendFrame(buffer, sock, strlen(buffer));
        }
    }
    else if (frame.type == '3' && strcmp(frame.header, "DOWNLOAD_SONG") == 0) {
        downloadSong(frame.data, user_pos, 0);
        frame = freeFrame(frame);
    }
    else if (frame.type == '3' && strcmp(frame.header, "DOWNLOAD_LIST") == 0) {
        downloadList(frame.data, user_pos);
        frame = freeFrame(frame);
    }
    else if (frame.type == '6' && strcmp(frame.header, "EXIT") == 0) {
        asprintf(&buffer, T6_OK);
        buffer = sendFrame(buffer, sock, strlen(buffer));
        asprintf(&buffer, "\n%sUser %s disconnected%s\n", C_RED, frame.data, C_RESET);
        printF(buffer);
        free(buffer);
        buffer = NULL;

        return -1;
    }
    else if (frame.type == '7') {
        printF(C_RED);
        printF("Sent wrong frame\n");
        printF(C_RESET);
        frame = freeFrame(frame);
    }
    else {
        printF("Wrong frame\n");
        sendError(sock);
        frame = freeFrame(frame);
    }

    return 0;
}

fd_set buildSelect() {
    fd_set readfds;
    
    FD_ZERO(&readfds);
    FD_SET(bow_sock, &readfds);
    for (int i = 0; i < num_users; i++) {
        FD_SET(users_fd[i], &readfds);
    }

    return readfds;
}
/********************************************************************
 *
 * @Purpose: Accepts incoming connections from Bowman users, managing users 
 *           and allocating resources.
 * @Parameters: sock - Server socket descriptor.
 * @Return: 0 on success, -1 on error.
 *
 ********************************************************************/
static int listenConnections() {
    fd_set readfds;
    users_fd = malloc(sizeof(int));
    users = malloc(sizeof(char*));
    char* buffer = NULL;

    printF("\nWaiting for connections...\n");
    
    while (1) {
        readfds = buildSelect();

        int ready = select(CHECK_UP_TO, &readfds, NULL, NULL, NULL);
        
        if (ready == -1) {
            printF("Error in select\n");
            return -1;
        }
        else {
            if (FD_ISSET(bow_sock, &readfds)) {
                users_fd[num_users] =  accept(bow_sock, NULL, NULL);
                if (users_fd[num_users] == -1) {
                    asprintf(&buffer, "%sError accepting %s socket connection\n%s", C_RED, "bowman", C_RESET);
                    printF(buffer);
                    free(buffer);
                    buffer = NULL;
                    return -1;
                }
                num_users++; 
                users = realloc(users, sizeof(char*) * num_users + 1);
                users_fd = realloc(users_fd, sizeof(int) * (num_users + 1));
            }
            for (int i = 0; i < num_users; i++) {
                if (FD_ISSET(users_fd[i], &readfds)) {
                    if (bowmanHandler(users_fd[i], i) == -1) {
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

    close (bow_sock);
    for (int i = 0; i < num_users; i++) {
        close(users_fd[i]);
    }

    return 0;
}

void logout() { // closear download sock
    char* buffer = NULL;
    Frame frame;
    int disc_sock;
    struct sockaddr_in discovery;

    // Close Discovery connection
    discovery = configServer(config.discovery_ip, config.discovery_port);

    disc_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (disc_sock == -1) {
        printF(C_RED);
        printF("Error creating socket\n");
        printF(C_RESET);

        return;
    }

    if (connect(disc_sock, (struct sockaddr *) &discovery, sizeof(discovery)) < 0) {
        printF(C_RED);
        printF("Error connecting to the server!\n");
        printF(C_RESET);

        return;
    }

    asprintf(&buffer, T6_POOLE, config.server);
    buffer = sendFrame(buffer, disc_sock, strlen(buffer));

    frame = readFrame(disc_sock);
    if (frame.type == '6' && strcmp(frame.header, "CON_OK") == 0) {
        asprintf(&buffer, "%sSuccessfully aborted\n%s", C_GREEN, C_RESET);
        printF(buffer);
        free(buffer);
        buffer = NULL;
        close(disc_sock);
    }
    else {
        asprintf(&buffer, "%sCouldn't abort successfully\n%s", C_RED, C_RESET);
        printF(buffer);
        free(buffer);
        buffer = NULL;
    }
    frame = freeFrame(frame);

    // Close Bowman connections

    for (int i = 0; i < num_users; i++) {
        asprintf(&buffer, T6_POOLE, config.server);
        buffer = sendFrame(buffer, users_fd[i], strlen(buffer));

        frame = readFrame(users_fd[i]);

        if (frame.type == '6' && strcmp(frame.header, "CON_OK") == 0) {
            asprintf(&buffer, "%sDisconnected user %s\n%s", C_GREEN, users[i], C_RESET);
            printF(buffer);
            free(buffer);
            buffer = NULL;
            close(users_fd[i]);
            free(users[i]);
            users[i] = NULL;
        }
        else {
            asprintf(&buffer, "%sCouldn't close %s user connection\n%s", C_RED, users[i], C_RESET);
            printF(buffer);
            free(buffer);
            buffer = NULL;
        }
        frame = freeFrame(frame);
    }
}

void sig_handler(int sigsum) {
    switch(sigsum) {
        case SIGINT:
            printF("\nAborting...\n");
            logout();
            free(config.server);
            free(config.path);
            free(config.discovery_ip);
            free(config.user_ip);
            config.server = NULL;
            config.path = NULL;
            config.discovery_ip = NULL;
            config.user_ip = NULL;
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
    int disc_sock;
    Frame frame;
    signal(SIGINT, sig_handler);
    
    if (argc != 2) {
        printF(C_RED);
        printF("Usage: ./poole <config_file>\n");
        printF(C_RESET);
        return -1;
    }

    config = readConfigPol(argv[1]);
    printF("Reading configuration file\n");


    if (checkPort(config.discovery_port) == -1 || checkPort(config.user_port) == -1) {
        printF(C_RED);
        printF("ERROR: Invalid port.\n");
        printF(C_RESET);

        return -1;
    }

    server = configServer(config.discovery_ip, config.discovery_port);

    disc_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (disc_sock == -1) {
        printF(C_RED);
        printF("Error creating socket\n");
        printF(C_RESET);

        return -1;
    }
    
    asprintf(&buffer, "Conecting %s Server to the system...\n", config.server);
    printF(buffer);
    free(buffer);
    buffer = NULL;

    if (connect(disc_sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        printF(C_RED);
        printF("Error connecting to the server!\n");
        printF(C_RESET);

        return -1;
    }

    asprintf(&buffer, T1_POOLE, config.server, config.user_ip, config.user_port);
    buffer = sendFrame(buffer, disc_sock, strlen(buffer));

    frame = readFrame(disc_sock);

    if (frame.type == '1' && strcmp(frame.header, "CON_OK") == 0) {
        close(disc_sock);
        server = configServer(config.user_ip, config.user_port);
        
        bow_sock = openConnection(server);

        if (bow_sock == -1) {
            asprintf(&buffer, "%sError opening the socket for %s\n%s", C_RED, "bowman", C_RESET);
            printF(buffer);
            free(buffer);

            return -1;
        }

        asprintf(&buffer, C_GREEN "Connected to HAL 9000 System, ready to listen to Bowmans petitions\n" C_RESET);
        printF(buffer);
        free(buffer);
        buffer = NULL;

        if (listenConnections() == -1) {
            return -1;
        }
    }
    else {
        printF(C_RED);
        printF("Error trying to connect to HAL 9000 system\n");
        printF(C_RESET);
        close(disc_sock);
        return -1;
    } 
    return 0;
}