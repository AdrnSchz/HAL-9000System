/********************************************************************
 *
 * @Purpose: HAL 9000 System - Config files
 * @Author: Marc Escoté Llopis & Adrián Jorge Sánchez López
 *
 * - This file contains function declarations and structs defined regarding the configs for each code.
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
    char** users;
    char* ip;
    int port;
} Server;

struct sockaddr_in configServer(char* ip, int port);

int openConnection(int sock, struct sockaddr_in server, char* server_type);

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
 * @Purpose: Reads a header from a socket.
 * @Parameters: sock - The socket file descriptor.
 * @Return: The read header structure.
 *
 ********************************************************************/
Header readHeader(int sock);

char* sendFrame(char* buffer, int sock);

#endif