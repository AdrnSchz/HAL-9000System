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
#include "configs.h"
#include "connections.h"

User_conf config;
int discovery_sock, poole_sock = 0;
char* server_name = NULL;
pthread_t thread;
int num_files = 0, downloading = 0;
File* files;
int queue_id = 0;
pthread_mutex_t terminal = PTHREAD_MUTEX_INITIALIZER;

/********************************************************************
 *
 * @Purpose: Establishes a socket connection with the server using the information from the 'config' structure.
 * @Parameters: sock - A pointer to the socket variable.
 *              server - A pointer to the server address structure.
 * @Return: 0 if successful, -1 otherwise.
 *
 ********************************************************************/
int configConnection(struct sockaddr_in* server) {
    char *buffer = NULL;
    if (checkPort(config.port) == -1) {
        asprintf(&buffer, "%sERROR: Invalid port.\n%s", C_RED, C_RESET);
        print(buffer, &terminal);
        free(buffer);

        return -1;
    }

    discovery_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (discovery_sock == -1) {
        asprintf(&buffer, "%sERROR: Could not create socket.\n%s", C_RED, C_RESET);
        print(buffer, &terminal);
        free(buffer);

        return -1;
    }

    *server = configServer(config.ip, config.port);

    return 0;
}

void* downloadSong() {
    char* buffer = NULL;
    Msg msg;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);  

    while (downloading != 0) {
        msgrcv(queue_id, (struct msgbuf *)&msg, sizeof(Msg) - sizeof(long), 1, 0);
        char* aux = getString(0, '&', msg.data);

        int id = atoi(aux);
        int space = 256 - 3 - 9 - (strlen(aux) + 1);
        
        for (int i = 0; i < num_files; i++) {
            if (id == files[i].id) {
                if (files[i].data_received + space > files[i].file_size) {
                    space = files[i].file_size - files[i].data_received;
                }
                files[i].data_received += space;
                
                write(files[i].fd, msg.data + strlen(aux) + 1, space);

                if (files[i].data_received >= files[i].file_size) {
                    close(files[i].fd);
                    files[i].fd = 0;
                    downloading--;
                    char* path;
                    char* md5 = NULL;
                    asprintf(&path, "%s/%s", config.files_path, files[i].file_name);
                    
                    pthread_mutex_lock(&terminal);
                    getMd5(path, &md5);
                    pthread_mutex_unlock(&terminal);

                    if (md5 == NULL || strcmp(md5, files[i].md5) != 0) {
                        asprintf(&buffer, "\n%sError in the integrity of %s\n%s", C_RED, files[i].file_name, C_RESET);
                        print(buffer, &terminal);
                        free(buffer);
                        print(BOLD, &terminal);
                        print("\n$ ", &terminal);

                        asprintf(&buffer, T5_KO, files[i].id);
                        buffer = sendFrame(buffer, poole_sock, strlen(buffer));
                    }
                    else {
                        asprintf(&buffer, "\n%sSuccessfully downloaded %s\n%s", C_GREEN, files[i].file_name, C_RESET);
                        print(buffer, &terminal);
                        free(buffer);
                        print(BOLD, &terminal);
                        print("\n$ ", &terminal);

                        asprintf(&buffer, T5_OK, files[i].id);
                        buffer = sendFrame(buffer, poole_sock, strlen(buffer));
                    }
                    free(md5);
                    path = NULL;
                }
                break;
            }
        }
        free(aux);
        aux = NULL;
    }
    return NULL;
}

void newFile(Frame frame) {
    File file;
    char* buffer = NULL;

    if (getFileData(frame.data, &file) == 0) {
        //asprintf(&buffer, "%sDownload started!%s\n", C_GREEN, C_RESET);
        //print(buffer, &terminal);
        //free(buffer);
        num_files++;
        files = realloc(files, sizeof(File) * (num_files));
        file.data_received = 0;

        char* path;
        asprintf(&path, "%s/%s", config.files_path, file.file_name);
        file.fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, 0666);

        if (file.fd == -1) {
            asprintf(&buffer, "%s%s\nError creating/opening the mp3 file\n%s", C_RESET, C_RED, C_RESET);
            print(buffer, &terminal);
            print(BOLD, &terminal);
            print("\n$ ", &terminal);
            return;
        }
        free(path);

        files[num_files - 1] = file;
        downloading++;
        
        if (downloading - 1 == 0) {
            pthread_create(&thread, NULL, downloadSong, NULL);
        }
    }
    else {
        asprintf(&buffer, "%s%s\nSong or list does not exist\n%s", C_RESET, C_RED, C_RESET);
        print(buffer, &terminal);
        free(buffer);
        print(BOLD, &terminal);
        print("\n$ ", &terminal);
    }
}

void newData(Frame frame) {
    Msg msg = {0};
    msg.mtype = 1;
    memset(msg.data, 0, sizeof(msg.data));
    memcpy(msg.data, frame.data, 256 - 12);
    msgsnd(queue_id, (struct msgbuf *)&msg, sizeof(Msg) - sizeof(long), 0);
}

Frame getFrameLoop(int sock) {
    Frame frame = readFrame(sock);

    while (frame.type == '4') {
        if (strcmp(frame.header, "NEW_FILE") == 0) {
            newFile(frame);
        }
        else if (strcmp(frame.header, "FILE_DATA") == 0) {
            newData(frame);
        }
        frame = freeFrame(frame);
        frame = readFrame(sock);
    }

    return frame;
}
/********************************************************************
 *
 * @Purpose: Connect and ask discovery for a server and connect to the given server
 * @Parameters: poole - sockaddr_in with the configuration to connect to the poole server.
 *              discovery - sockaddr_in with the configuration to connect to the discovery server.
 * @Return: ---.
 *
 ********************************************************************/
void connection(struct sockaddr_in* poole, struct sockaddr_in discovery, int select) {
    char* buffer = NULL, *aux = NULL;
    Frame frame;
    
    if (configConnection(&discovery) == -1) {
        asprintf(&buffer, "%sError configuring connection with discovery\n%s", C_RED, C_RESET);
        print(buffer, &terminal);
        free(buffer);
    
        return;
    }

    if (connect(discovery_sock, (struct sockaddr *) &discovery, sizeof(discovery)) < 0) {
        asprintf(&buffer, "%sERROR: Could not connect to discovery server.\n%s", C_RED, C_RESET);
        print(buffer, &terminal);
        free(buffer);
        
        if (select == 1) {
            print(BOLD, &terminal);
            print("\n$ ", &terminal);
        }
        return;
    }

    asprintf(&buffer, T1_BOWMAN, config.user);
    buffer = sendFrame(buffer, discovery_sock, strlen(buffer));

    //frame = readFrame(discovery_sock);
    frame = getFrameLoop(discovery_sock);
    
    if (frame.type == '1' && strcmp(frame.header, "CON_OK") == 0) {
        server_name = getString(0, '&', frame.data);
        buffer = getString(1 + strlen(server_name), '&', frame.data);
        aux = getString(2 + strlen(server_name) + strlen(buffer), '\0', frame.data);
        *poole = configServer(buffer, atoi(aux));
        free(buffer);
        free(aux);
        buffer = NULL;
        aux = NULL;

        poole_sock = socket(AF_INET, SOCK_STREAM, 0);

        if (connect(poole_sock, (struct sockaddr *) poole, sizeof(*poole)) < 0) {
            asprintf(&buffer, "%sError trying to connect to HAL 9000 system\n%s", C_RED, C_RESET);
            print(buffer, &terminal);
            free(buffer);
            
            if (select == 1) {
                print(BOLD, &terminal);
                print("\n$ ", &terminal);
            }
            return;
        }

        asprintf(&buffer, T1_BOWMAN, config.user);
        buffer = sendFrame(buffer, poole_sock, strlen(buffer));
        
        frame = freeFrame(frame);
        // frame = readFrame(poole_sock);
        frame = getFrameLoop(poole_sock);

        if (frame.type == '1' && strcmp(frame.header, "CON_OK") == 0) {
            asprintf(&buffer, "%s%s connected to HAL 9000 system, welcome music lover!\n%s", C_GREEN, config.user, C_RESET);
            print(buffer, &terminal);
            free(buffer);
        }
        else if (frame.type == '1' && strcmp(frame.header, "CON_KO") == 0) {
            asprintf(&buffer, "%sCould not establish connection.\n%s", C_RED, C_RESET);
            print(buffer, &terminal);
            free(buffer);
        }
        else {
            asprintf(&buffer, "%sReceived wrong frame\n%s", C_RED, C_RESET);
            print(buffer, &terminal);
            free(buffer);
            sendError(poole_sock);
        }
    }
    else if (frame.type == '1' && strcmp(frame.header, "CON_KO") == 0) {
        asprintf(&buffer, "%s%sThere are no poole server to which connect.\n%s", C_RESET, C_RED, C_RESET);
        print(buffer, &terminal);
        free(buffer);
    }
    else if (frame.type == '7') {
        asprintf(&buffer, "%s%sSent wrong frame\n%s", C_RESET, C_RED, C_RESET);
        print(buffer, &terminal);
        free(buffer);
    }
    else {
        asprintf(&buffer, "%s%sReceived wrong frame\n%s", C_RESET, C_RED, C_RESET);
        print(buffer, &terminal);
        free(buffer);
        
        sendError(discovery_sock);
    }
    frame = freeFrame(frame);
    close(discovery_sock);
    discovery_sock = 0;

    if (select == 1) {
        print(BOLD, &terminal);
        print("\n$ ", &terminal);
    }
}

/********************************************************************
 *
 * @Purpose: Logout from the poole
 * @Parameters: ---.
 * @Return: ---.
 *
 ********************************************************************/
void logout() {
    char* buffer = NULL;
    Frame frame, frame2;

    //joinear threads y acabar download o algo
    if (thread != 0) pthread_join(thread, NULL);

    asprintf(&buffer, T6, config.user);
    buffer = sendFrame(buffer, poole_sock, strlen(buffer));
    frame = readFrame(poole_sock);
    
    asprintf(&buffer, T6, server_name);
    buffer = sendFrame(buffer, discovery_sock, strlen(buffer));
    frame2 = readFrame(discovery_sock);
    if (frame.type == '6' && strcmp(frame.header, "CON_OK") == 0 && frame2.type == '6' && strcmp(frame2.header, "CON_OK") == 0) {    
        asprintf(&buffer, "%sThanks for using HAL 9000, see you soon, music lover!\n%s", C_GREEN, C_RESET);                
        print(buffer, &terminal);
        free(buffer);

        close(poole_sock);
        close(discovery_sock);
    }
    else if ((frame.type == '6' && strcmp(frame.header, "CON_KO") == 0) || (frame2.type == '6' && strcmp(frame2.header, "CON_KO") == 0)) {
        asprintf(&buffer, "%sCould not disconnect from HAL 9000 system\n%s", C_RED, C_RESET);
        print(buffer, &terminal);
        free(buffer);
    }
    else {
        asprintf(&buffer, "%sReceived wrong frame\n%s", C_RED, C_RESET);
        print(buffer, &terminal);
        free(buffer);
        
        sendError(poole_sock);
    }
    frame = freeFrame(frame);
    frame2 = freeFrame(frame2);

    if (msgctl(queue_id, IPC_RMID, NULL) == -1) {
        print("Error deleting the message queue\n", &terminal);
    }
}

/********************************************************************
*
* @Purpose: Lists the available songs on the Poole server.
* @Parameters: ---.
* @Return: ---.
*
*******************************************************************/
void listSongs() {
    int totalSongs = 0, num_songs = 0, already_printed = 0;
    char *buffer = NULL, *num_songs_str = NULL, *song = NULL;
    Frame frame;
    
    asprintf(&buffer, T2_SONGS);
    buffer = sendFrame(buffer, poole_sock, strlen(buffer));

    while (1) {
        // frame = readFrame(poole_sock);
        frame = getFrameLoop(poole_sock);
        num_songs_str = strtok(frame.data, "#");

        if (already_printed == 0) {
            asprintf(&buffer, "%sThere are %s songs available for download:\n%s", C_GREEN, num_songs_str, C_RESET);
            print(buffer, &terminal);
            free(buffer);
            buffer = NULL;        
            already_printed = 1;
        }

        song = strtok(NULL, "&"); // NULL to continue from last strtok
        num_songs = atoi(num_songs_str);

        for (int i = 0; i < num_songs; i++) {
            if (song != NULL) {
                totalSongs += 1;
                asprintf(&buffer, "%d. %s\n", totalSongs, song);
                print(buffer, &terminal);
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
    free(buffer);
}

/********************************************************************
*
* @Purpose: Lists the available playlists on the Poole server.
* @Parameters: ---.
* @Return: ---.
*
*******************************************************************/
void listPlaylists() {
    int totalSongs = 0, totalPlaylists = 0, num_songs = 0, already_printed = 0;
    char *buffer = NULL, *num_playlists_str, *num_songs_str = NULL, *song = NULL, *playlist_name = NULL, abc = 'a';
    Frame frame;
    size_t total_bytes = 0;

    asprintf(&buffer, T2_PLAYLISTS);
    buffer = sendFrame(buffer, poole_sock, strlen(buffer));

    //frame = readFrame(poole_sock);
    frame = getFrameLoop(poole_sock);

    total_bytes = 0;
    size_t data_len = strlen(frame.data);
    num_playlists_str = strtok(frame.data, "#");
    total_bytes += strlen(num_playlists_str) + 1;

    if (already_printed == 0) {
        asprintf(&buffer, "%sThere are %s playlists available for download:\n%s", C_GREEN, num_playlists_str, C_RESET);
        print(buffer, &terminal);
        free(buffer);
        buffer = NULL;
        already_printed = 1;
    }
    
    
    int num_playlists = atoi(num_playlists_str);
    
    // Iterate through playlists
    for (int i = 0; i < num_playlists; i++) {
        totalSongs = 0;
        totalPlaylists++;
        nextFrame:
        num_songs_str = strtok(NULL, "#");
        total_bytes += strlen(num_songs_str) + 1;
        num_songs = atoi(num_songs_str);
        playlist_name = strtok(NULL, "&");
        total_bytes += strlen(playlist_name) + 1;

        asprintf(&buffer, "%d. %s\n", i + 1, playlist_name);
        print(buffer, &terminal);
        free(buffer);
        buffer = NULL;

        for (int j = totalSongs; j < num_songs; j++) {
            totalSongs++;
            song = strtok(NULL, "#&\0");
            total_bytes += strlen(song) + 1;
            asprintf(&buffer, "\t%c. %s\n", abc, song);
            print(buffer, &terminal);
            free(buffer);
            buffer = NULL;
            abc++;

            if (totalSongs < num_songs && (total_bytes == data_len + 1)) {
                //frame = readFrame(poole_sock);
                frame = getFrameLoop(poole_sock);
                num_playlists_str = strtok(frame.data, "#");
                total_bytes += strlen(num_playlists_str) + 1;
                goto nextFrame;
            } 
            if (total_bytes == strlen(frame.data) + 1) {
                break;
            }
        }
        abc = 'a';
        totalSongs = 0;
    }
    totalPlaylists = 0;
    frame = freeFrame(frame);
}

/********************************************************************
 *
 * @Purpose: Sends a download command to the Poole server for a given song.
 * @Parameters: song - The name of the song to download.
 * @Return: ---.
 *
 ********************************************************************/
void downloadCommand(char* song) {
    char* buffer = NULL;

    asprintf(&buffer, "%s%sDownload started!\n%s", C_RESET, C_GREEN, C_RESET);
    print(buffer, &terminal);
    free(buffer);

    if (song[strlen(song) - 4] == '.') {
        asprintf(&buffer, T3_DOWNLOAD_SONG, song);
    } 
    else {
        asprintf(&buffer, T3_DOWNLOAD_LIST, song);
    }
    buffer = sendFrame(buffer, poole_sock, strlen(buffer));
}

/********************************************************************
 *
 * @Purpose: Checks the status of ongoing downloads and displays progress.
 * @Parameters: ---.
 * @Return: ---.
 *
 ********************************************************************/
void checkDownloads() {
    char* buffer = NULL;

    if (num_files == 0) {
        asprintf(&buffer, "%s%sYou have no ongoing or finished downloads\n%s", C_RESET, C_GREEN, C_RESET);
        print(buffer, &terminal);
        free(buffer);
    }
    
    for (int i = 0; i < num_files; i++) {
        asprintf(&buffer, "%s", files[i].file_name);
        print(buffer, &terminal);
        free(buffer);
        
        int percent = (files[i].data_received * 100) / files[i].file_size;
        char* space;
        if (percent < 10) {
            space = "  ";
        }
        else if (percent < 100) {
            space = " ";
        }
        else {
            space = "";
        }
        asprintf(&buffer, "\t%d%% %s|", percent, space);
        print(buffer, &terminal);
        
        int num_hashes = (files[i].data_received * 20) / files[i].file_size;  // Each hash represents 5%
        for (int j = 0; j < num_hashes; j++) {
            print("=", &terminal);
        }

        // Add spaces for the remaining percentage
        for (int j = num_hashes; j < 20; j++) {
            print(" ", &terminal);
        }

        print("%|\n", &terminal);
        free(buffer);
    }
}

/********************************************************************
 *
 * @Purpose: Clears completed downloads from the file list.
 * @Parameters: ---.
 * @Return: ---.
 *
 ********************************************************************/
void clearDownloads() {
    int j = 0;

    for (int i = 0; i < num_files; i++) {
        if (files[i].fd != 0) {
            // Only delete fully downloaded files
            if (files[i].data_received >= files[i].file_size) {
                close(files[i].fd);
                files[i].fd = 0;
                memset(&files[i], 0, sizeof(File));
            } else {
                files[j] = files[i];
                j++;
            }
        }
    }

    files = realloc(files, sizeof(File) * j);
    num_files = j;
    downloading = num_files;
    
    checkDownloads();
}

/********************************************************************
 *
 * @Purpose: Checks for incoming frames from the Poole server and handles them.
 * @Parameters: ---.
 * @Return: 0 if successful, 6 if the server initiated shutdown.
 *
 ********************************************************************/
int checkFrame() {
    Frame frame;
    char* buffer = NULL;

    frame = readFrame(poole_sock);

    if (frame.type == '4') {
        if (strcmp(frame.header, "NEW_FILE") == 0) {
            newFile(frame);
        }
        else if (strcmp(frame.header, "FILE_DATA") == 0) {
            newData(frame);
        }
    }          
    else if (frame.type == '6' && strcmp(frame.header, "SHUTDOWN") == 0) {
        asprintf(&buffer, T6_OK);
        buffer = sendFrame(buffer, poole_sock, strlen(buffer));
        asprintf(&buffer, "\n%s%sServer %s got unexpectedly disconnected\n%s", C_RESET, C_RED, frame.data, C_RESET);
        print(buffer, &terminal);
        free(buffer);
        buffer = NULL;

        close(poole_sock);
        poole_sock = 0;
        frame = freeFrame(frame);
        
        return 6;
    }
    else {
        asprintf(&buffer, "%s%s\nReceived wrong frame\n%s", C_RESET, C_RED, C_RESET);
        print(buffer, &terminal);
        free(buffer);

        sendError(poole_sock);
    }

    frame = freeFrame(frame);
    
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
            print("\nAborting...\n", &terminal);

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
    char *buffer = NULL;
    key_t key;
    thread = 0;

    struct sockaddr_in discovery, poole;
    fd_set readfds;
    signal(SIGINT, sig_handler);

    if (argc != 2) {
        asprintf(&buffer, "%sUsage: ./bowman <config_file>\n%s", C_RED, C_RESET);
        print(buffer, &terminal);
        free(buffer);

        return -1;
    }

    config = readConfigBow(argv[1]);
    checkName(&config.user);

    asprintf(&buffer, "%s user initialized\n", config.user);
    print(buffer, &terminal);
    free(buffer);
    buffer = NULL;

    print(BOLD, &terminal);
    print("\n$ ", &terminal);

    while(1) {
        FD_ZERO(&readfds);
        FD_SET(poole_sock, &readfds);
        FD_SET(0, &readfds);

        int ready = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);

        if (ready <= 0) {
            asprintf(&buffer, "%sERROR: Select failed.\n%s", C_RED, C_RESET);
            print(buffer, &terminal);
            free(buffer);

            return -1;
        }

        else {
            if (FD_ISSET(0, &readfds)) {
                readLine(0, &buffer);
                print(C_RESET, &terminal);
                switch (checkCommand(buffer)) {
                    case 0:
                        // ==================================================
                        // CONNECT
                        // ==================================================
                        free(buffer);
                        buffer = NULL;
                        if (poole_sock != 0) {
                            asprintf(&buffer, "%sERROR: Already connected to HAL 9000 system\n%s", C_RED, C_RESET);
                            print(buffer, &terminal);
                            free(buffer);

                            break;
                        }

                        connection(&poole, discovery, 0);
                    break;
                    case 1:
                        // ==================================================
                        // LOGOUT
                        // ==================================================
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
                            asprintf(&buffer, "%sERROR: Not connected to HAL 9000 system\n%s", C_RED, C_RESET);
                            print(buffer, &terminal);
                            free(buffer);

                            break;
                        }
                        listSongs();
                        break;
                    case 3:
                        // ==================================================
                        // LIST PLAYLISTS
                        // ==================================================
                        free(buffer);
                        buffer = NULL;
                        if (poole_sock == 0) {
                            asprintf(&buffer, "%sERROR: Not connected to HAL 9000 system\n%s", C_RED, C_RESET);
                            print(buffer, &terminal);
                            free(buffer);

                            break;
                        }
                        listPlaylists();
                        break;
                    case 4:
                        // ==================================================
                        // DOWNLOAD
                        // ==================================================
                        if (poole_sock == 0) {
                            asprintf(&buffer, "%sERROR: Not connected to HAL 9000 system\n%s", C_RED, C_RESET);
                            print(buffer, &terminal);
                            free(buffer);
                            buffer = NULL;
                            break;
                        }
                        removeWhiteSpaces(&buffer);
                        char* song = getSongName(buffer);

                        if (queue_id == 0) {
                            if (configQueue(&key, &queue_id) == -1) {
                                asprintf(&buffer, "%sError creating queue\n%s", C_RED, C_RESET);
                                print(buffer, &terminal);
                                free(buffer);
                            }
                        }

                        downloadCommand(song);
                        free(buffer);
                        buffer = NULL;
                        free(song);
                        song = NULL;
                        break;
                    case 5:
                        // ==================================================
                        // CHECK DOWNLOADS
                        // ==================================================
                        checkDownloads();
                        free(buffer);
                        buffer = NULL;
                        break;
                    case 6:
                        // ==================================================
                        // CLEAR DOWNLOADS
                        // ==================================================
                        clearDownloads();
                        free(buffer);
                        buffer = NULL;
                        break;
                    case 7:
                        // ==================================================
                        // UNKNOWN COMMAND
                        // ==================================================
                        free(buffer);
                        buffer = NULL;
                        asprintf(&buffer, "%sUnknown command.\n%s", C_RED, C_RESET);
                        print(buffer, &terminal);
                        free(buffer);
                        break;
                    default:
                        // ==================================================
                        // INVALID COMMAND
                        // ==================================================
                        free(buffer);
                        buffer = NULL;
                        asprintf(&buffer, "%sERROR: Please input a valid command.\n%s", C_RED, C_RESET);
                        print(buffer, &terminal);
                        free(buffer);
                        
                        break;
                }
                print(BOLD, &terminal);
                print("\n$ ", &terminal);
            } 
            else if (FD_ISSET(poole_sock, &readfds)) {
                int res = checkFrame();
                if (res == 6) {
                    connection(&poole, discovery, 1);
                    
                }
            }
        }
        if (downloading == 0 && thread != 0) {
            pthread_join(thread, NULL);
            thread = 0;
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
