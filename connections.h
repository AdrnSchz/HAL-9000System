/********************************************************************
 *
 * @Purpose: HAL 9000 System - Config files
 * @Author: Marc Escoté Llopis & Adrián Jorge Sánchez López
 *
 * - This file contains function declarations and structs defined used 
 *   for the connections of sockets.
 *
 ********************************************************************/
#ifndef _CONNECTIONS_H_
#define _CONNECTIONS_H_

#include "functions.h"

#define CHECK_UP_TO 5 + 3

#define ERROR_FRAME "707UNKNOWN\n"
#define T1_POOLE "109NEW_POOLE%s&%s&%d"
#define T1_BOWMAN "110NEW_BOWMAN%s"
#define T1_OK "106CON_OK"
#define T1_OK_BOW "106CON_OK%s&%s&%d"
#define T1_KO "106CON_KO"
#define T2_SONGS "210LIST_SONGS"
#define T2_PLAYLISTS "214LIST_PLAYLISTS"
#define T2_SONGS_RESPONSE "214SONGS_RESPONSE%s" //%s = numsongs#song1&song2&...&songN\0
#define T2_PLAYLISTS_RESPONSE "218PLAYLISTS_RESPONSE%s" //%s = numplaylist\0
#define T3_DOWNLOAD_SONG "313DOWNLOAD_SONG%s" //%s = songname
#define T3_DOWNLOAD_LIST "313DOWNLOAD_LIST%s" //%s = playlistname
#define T4_NEW_FILE "408NEW_FILE%s&%d&%s&%d" //songname&filesize&MD5&id
#define T4_DATA "409FILE_DATA%d&" //id&data
#define T5_OK "508CHECK_OK%d"
#define T5_KO "508CHECK_KO%d"
#define T6 "604EXIT%s"
#define T6_POOLE "608SHUTDOWN%s"
#define T6_OK "606CON_OK"
#define T6_KO "606CON_KO"

/**
 * Structure for storing a header.
*/
typedef struct {
    char type;
    char length[3];
    char* header;
    char* data;
} Frame;

/**
 * Structure for storing a server.
*/
typedef struct {
    char* name;
    int num_users;
    char* ip;
    int port;
} Server;

/**
 * Structure for storing file data.
*/
typedef struct {
    char* file_name;
    int file_size;
    char* md5;
    int id;
    int data_received;
    int fd;
} File;

/**
 * Structure for storing data to be send.
*/
typedef struct {
    char* name;
    int fd_pos;
    int thread_pos;
} Send;

typedef struct {
    long mtype;
    char data[244];
} Msg;

typedef struct {
    int id;
    char* name;
} Ids;
/********************************************************************
 *
 * @Purpose: Configures the server address structure for connections.
 * @Parameters: ip - The IP address.
 *              port - The port number.
 * @Return: The configured server address structure.
 *
 ********************************************************************/
struct sockaddr_in configServer(char* ip, int port);

/********************************************************************
 *
 * @Purpose: Binds a socket to a specific server address and starts listening for connections.
 * @Parameters: sock - The socket file descriptor.
 *              server - The server address structure.
 *              server_type - A string indicating the server type.
 * @Return: 0 if successful, -1 on error.
 *
 ********************************************************************/
int openConnection(struct sockaddr_in server);

/********************************************************************
 *
 * @Purpose: Checks if a given port number is within the valid range.
 * @Parameters: port - The port number to be checked.
 * @Return: 0 if valid, -1 otherwise.
 *
 ********************************************************************/
int checkPort(int port);

/********************************************************************
 *
 * @Purpose: Sends an error frame to the specified socket.
 * @Parameters: sock - The socket file descriptor.
 * @Return: ---
 *
 ********************************************************************/
void sendError(int sock);

/********************************************************************
 *
 * @Purpose: Reads a data frame header from a socket and parses its components.
 * @Parameters: sock - The socket file descriptor to read the header from.
 * @Return: The parsed header structure.
 *
 ********************************************************************/
Frame readFrame(int sock);

/********************************************************************
 *
 * @Purpose: Sends a data frame over a socket, ensuring the frame is of a fixed size.
 * @Parameters: buffer - The data frame to send.
 *              sock - The socket file descriptor to send the frame to.
 * @Return: A null pointer after freeing the buffer.
 *
 ********************************************************************/
char* sendFrame(char* buffer, int sock, int len);

/********************************************************************
 *
 * @Purpose: Frees the memory inside a Frame data structure.
 * @Parameters: frame - The frame structure to be freed.
 * @Return: The the frame freed.
 *
 ********************************************************************/
Frame freeFrame(Frame frame);

/********************************************************************
 *
 * @Purpose: gets the file data from a frame data.
 * @Parameters: data - String with the data to get.
 *              file - The file data structure to be filled.
 * @Return: Returns 0 if the file exists, -1 otherwise.
 *
 ********************************************************************/
int getFileData(char* data, File* file);

/********************************************************************
 *
 * @Purpose: Configure a message queue with the specified key and identifier.
 * @Parameters: key - Pointer to store the generated key using ftok.
 *              id - Pointer to store the message queue identifier.
 * @Return: 0 on success, -1 on error.
 *
 ********************************************************************/
int configQueue(key_t* key, int* id);
#endif