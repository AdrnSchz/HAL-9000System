/********************************************************************
 *
 * @Purpose: HAL 9000 System - Socket Connections
 * @Authors: Marc Escoté Llopis & Adrián Jorge Sánchez López
 *
 * - This file contains functions fot the server setup, connection handling, 
 *   port checking, and error management.
 *
 ********************************************************************/
#include "connections.h"
#include <netinet/in.h>

struct sockaddr_in configServer(char* ip, int port) {
    struct sockaddr_in server;

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);

    return server;
}

int openConnection(struct sockaddr_in server) {
    int sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        return -1;
    }

    if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        return -1;
    }

    listen(sock, 5);

    return sock;
}

int checkPort(int port) {
    if (port < 0 || port > 65535) {
        return -1;
    }

    return 0;
}

void sendError(int sock) {
    char buffer[256];
    size_t i;

    memset(&buffer, 0, 256);
    for (i = 0; i < strlen(ERROR_FRAME); i++) {
        buffer[i] = ERROR_FRAME[i];
    }
    buffer[i] = '\0';
    write(sock, buffer, 256);
}

Frame readFrame(int sock) {
    Frame frame;
    char* buffer = (char*) malloc(256);
    int i, j;

    read(sock, buffer, 256);
    //printF("Received frame: ");
    //printF(buffer);
    //printF("\n");
    frame.type = buffer[0];
    frame.length[0] = buffer[1];
    frame.length[1] = buffer[2];
    frame.length[2] = '\0';
    frame.header = malloc((atoi(frame.length) + 1) * sizeof(char));
    for (i = 0; i < atoi(frame.length); i++) {
        frame.header[i] = buffer[i + 3];
    }
    frame.header[i] = '\0';
    frame.data = malloc(256 - i - 2);
    for (j = 0; (j + i + 3) < 256; j++) {
        frame.data[j] = buffer[j + i + 3];
    }
    frame.data[j] = '\0';

    free(buffer);
    buffer = NULL;

    return frame;
}

char* sendFrame(char* buffer, int sock, int len) {
    if (len < 256) buffer = (char*) realloc(buffer, 256);
    for (int i = len; i < 256; i++) {
        buffer[i] = '\0';
    }
    write(sock, buffer, 256);
    //printF("Sending frame: ");
    //printF(buffer);
    //printF("\n");
    free(buffer);
    buffer = NULL;

    return buffer;
}

Frame freeFrame(Frame frame) {
    free(frame.header);
    free(frame.data);
    frame.header = NULL;
    frame.data = NULL;

    return frame;
}
int getFileData(char* data, File* file) {
    int j = 0, k = 0, len = strlen(data);
    char** data_split = malloc(sizeof(char*) * 4);

    for (int i = 0; i < 4; i++) {
        data_split[i] = NULL;
    }

    for (int i = 0; i < len; i++) {
        if (data[i] == '&') {
            data_split[j] = (char*)realloc(data_split[j], sizeof(char) * (k + 1));
            data_split[j][k] = '\0';
            k = 0;
            j++;
        } else {
            data_split[j] = (char*)realloc(data_split[j], sizeof(char) * (k + 1));
            data_split[j][k] = data[i];
            k++;
        }
    }

    data_split[j] = (char*)realloc(data_split[j], sizeof(char) * (k + 1));
    data_split[j][k] = '\0';

    for (int i = 0; i < 4; i++) {
        switch (i) {
            case 0:
                file->file_name = data_split[i];
                break;
            case 1: 
                file->file_size = atoi(data_split[i]);
                break;
            case 2:
                file->md5 = data_split[i];  
                break;
            case 3: 
                file->id = atoi(data_split[i]);
                break;
        }
    }

    free(data_split);
    data_split = NULL;

    if (file->id == -1) {
        return -1;
    }

    return 0;
}

int configQueue(key_t *key, int *id) {
    *key = ftok("bowman.c", 12);
    if (*key == (key_t) - 1){
        return -1;
    }

    *id = msgget(*key, 0600 | IPC_CREAT);
    if(*id == - 1){
        return -1;
    }
    return 0;
}
