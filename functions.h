#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/ioctl.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits.h>

#define printF(x) write(1, x, strlen(x))

#define C_RESET "\033[0m"
#define C_RED "\033[31m"
#define C_GREEN "\033[32m"
#define C_BOLDGREEN "\033[1m\033[32m"
#define C_BOLDRED   "\033[1m\033[31m"
#define BOLD    "\033[1m"

#define ERROR_FRAME "77UNKNOWN\n" // need to do padding

typedef struct {
    char* server;
    char* path;
    char* discovery_ip;
    int discovery_port;
    char* user_ip;
    int user_port;
} Server_conf;

typedef struct {
    char* user;
    char* files_path;
    char* ip;
    int port;
} User_conf;

typedef struct {
    char* ip_poole;
    int port_poole;
    char* ip_bow;
    int port_bow;
} Disc_conf;

typedef struct {
    char type;
    char length[2];
    char* header;
    char* data;
} Header;

typedef struct {
    char* name;
    int num_users;
    char* ip;
    int port;
} Server;

void readNum(int source, int* num);

void readLine(int source, char** string);

void checkName(char** name);

void capitalize(char** string);

void removeWhiteSpaces(char** string);

int checkCommand(char* buffer);

int checkPort(int port);

void sendError(int sock);

Header readHeader(int sock);

char* getString(int from, char until, char* string);

#endif
