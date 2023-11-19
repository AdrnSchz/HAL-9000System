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

#define ERROR_FRAME "707UNKNOWN\n"
#define T1_POOLE "109NEW_POOLE%s&%s&%d"
#define T1_BOWMAN "110NEW_BOWMAN%s"
#define T1_OK "106CON_OK"
#define T1_OK_BOW "106CON_OK%s&%s&%d"
#define T1_KO "106CON_KO"
#define T2_SONGS "210LIST_SONGS"
#define T2_PLAYLISTS "214LIST_PLAYLISTS"
#define T2_SONGS_RESPONSE "214SONGS_RESPONSE%d" //%s = numsongs\0
#define T2_PLAYLISTS_RESPONSE "218PLAYLISTS_RESPONSE%d" //%s = numplaylist\0
#define T6 "604EXIT%s"
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
} Header;

/**
 * Structure for storing a server.
*/
typedef struct {
    char* name;
    int num_users;
    char* ip;
    int port;
} Server;

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
int openConnection(int sock, struct sockaddr_in server, char* server_type);

/********************************************************************
 *
 * @Purpose: Accepts new incoming connections on a server socket and dynamically 
 *           reallocates client file descriptors.
 * @Parameters: num_clients - Pointer to the number of clients.
 *              clients_fd - Array of client file descriptors.
 *              server_type - A string indicating the server type.
 *              sock - The server socket file descriptor.
 * @Return: 0 if successful, -1 on error.
 *
 ********************************************************************/

int acceptConnection(int* num_clients, int* clients_fd, char* server_type, int sock);
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
Header readHeader(int sock);

/********************************************************************
 *
 * @Purpose: Sends a data frame over a socket, ensuring the frame is of a fixed size.
 * @Parameters: buffer - The data frame to send.
 *              sock - The socket file descriptor to send the frame to.
 * @Return: A null pointer after freeing the buffer.
 *
 ********************************************************************/
char* sendFrame(char* buffer, int sock);

#endif