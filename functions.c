/********************************************************************
 *
 * @Purpose: HAL 9000 System - Common Functions
 * @Author: Marc Escoté Llopis & Adrián Jorge Sánchez López
 *
 * - This file contains common functions shared among the project.
 *
 ********************************************************************/

#include "functions.h"

/********************************************************************
 *
 * @Purpose: Reads an integer from a file descriptor and stores it in the specified variable.
 * @Parameters: source - File descriptor to read from.
 *              num - Pointer to the variable where the integer will be stored.
 * @Return: ---
 *
 ********************************************************************/
void readNum(int source, int* num) {
    char *buffer;
    int i = 0;

    buffer = (char*) malloc(sizeof(char));

    read(source, &buffer[i], sizeof(char));

    while (buffer[i] != '\n') {
        i++;
        buffer = (char*) realloc(buffer, sizeof(char) * (i + 1));
        read(source, &buffer[i], sizeof(char));
    }

    buffer[i] = '\0';
    *num = atoi(buffer);


    free(buffer);
    buffer = NULL;
}

/********************************************************************
 *
 * @Purpose: Reads a line from a file descriptor and allocates memory to store the information.
 * @Parameters: source - File descriptor to read from.
 *              string - Pointer to a string pointer where the line will be stored.
 * @Return: ---
 *
 ********************************************************************/
void readLine(int source, char** string) {
    char* buffer;
    int i = 0;

    free(*string);
    buffer = (char*) malloc(sizeof(char));

    read(source, &buffer[i], sizeof(char));

    while (buffer[i] != '\n') {
        i++;
        buffer = (char*) realloc(buffer, sizeof(char) * (i + 1));
        read(source, &buffer[i], sizeof(char));
    }

    buffer[i] = '\0';

    *string = buffer;
}

/********************************************************************
 *
 * @Purpose: Checks and removes '&' characters from a user name.
 * @Parameters: name - Pointer to the string containing the user name.
 * @Return: ---
 *
 ********************************************************************/
void checkName(char** name) {
    int i = 0, j = 0, num = 0;

    while ((*name)[i] != '\0') {
        if ((*name)[i] == '&' || (*name)[i] == '\r') {
            j = i;
            while ((*name)[j] != '\0') {
                (*name)[j] = (*name)[j + 1];
                j++;
            }
            num++;
            i--;
        }
        i++;
    }

    *name = (char*) realloc(*name, sizeof(char) * i);
    
}

/********************************************************************
 *
 * @Purpose: Capitalizes all characters in a string.
 * @Parameters: string - Pointer to the string to be capitalized.
 * @Return: ---
 *
 ********************************************************************/
void capitalize(char** string) {
    int i = 0;

    while ((*string)[i] != '\0') {
        if ((*string)[i] >= 'a' && (*string)[i] <= 'z') {
            (*string)[i] -= 32;
        }
        i++;
    }
}

/********************************************************************
 *
 * @Purpose: Removes extra white spaces from a string, keeping only one space between words.
 * @Parameters: string - Pointer to the string to be processed.
 * @Return: ---
 *
 ********************************************************************/
void removeWhiteSpaces(char** string) {
    int i = 0, len = 0;
    char* processed = (char*)malloc(sizeof(char) * (strlen(*string) + 1));

    for (i = 0; (*string)[i] != '\0'; i++) {
        if ((*string)[i] != ' ' || (i > 0 && (*string)[i - 1] != ' ')) {
            processed[len] = (*string)[i];
            len++;
        }
    }

    processed[len] = '\0';

    // Free the old string and assign the processed string
    free(*string);
    *string = processed;
}

/********************************************************************
 *
 * @Purpose: Checks if input entered in the command line corresponds to a valid command.
 * @Parameters: buffer - The input buffer containing the command.
 * @Return: The command index if valid, -1 otherwise.
 *
 ********************************************************************/
int checkCommand(char* buffer) {
    int i = 0, j, error, correct;
    char commands[7][20] = {"CONNECT", "LOGOUT", "LIST SONGS", "LIST PLAYLISTS", "DOWNLOAD", "CHECK DOWNLOADS", "CLEAR DOWNLOADS"};
    char* command = (char*) malloc(sizeof(char) * strlen(buffer));
    
    strcpy(command, buffer);
    capitalize(&command);
    removeWhiteSpaces(&command);
    
    for (i = 0; i < 7; i++) {
        error = 0;
        correct = 0;
        j = 0;
        while (j < strlen(commands[i]) && j < strlen(command)) {
            if (command[j] == commands[i][j]) {
                correct++;
            }
            else {
                error++;
            }
            j++;
        }

        if (i == 4) {
            error = -1;
            j++;
            while (j < strlen(command)) {
                if (command[j] == ' ') {
                    error = -1;
                    break;
                }

                else {
                    error = 0;
                }
                j++;
            }
        }

        if (correct == strlen(commands[i]) && strlen(commands[i]) != strlen(command) && i != 4) {
            free(command);
            command = NULL;
            return 7;
        }
        else if (correct == strlen(commands[i]) && i != 4) {
            free(command);
            command = NULL;
            return i;
        }
        else if (correct == strlen(commands[i]) && (i == 4 && error != -1)) {
            free(command);
            command = NULL;
            return i;
        }
    }
    
    free(command);
    command = NULL;
    return -1;
}

/********************************************************************
 *
 * @Purpose: Checks if a given port number is within the valid range.
 * @Parameters: port - The port number to be checked.
 * @Return: 0 if valid, -1 otherwise.
 *
 ********************************************************************/
int checkPort(int port) {

    if (port < 0 || port > 65535) {
        return -1;
    }

    return 0;
}

/********************************************************************
 *
 * @Purpose: Sends an error frame to the specified socket.
 * @Parameters: sock - The socket file descriptor.
 * @Return: ---
 *
 ********************************************************************/
void sendError(int sock) {
    //memset(buffer, 0, 256); preguntar q hace esto y q hace el bind y el accept

    write(sock, ERROR_FRAME, strlen(ERROR_FRAME));
}

/********************************************************************
 *
 * @Purpose: Reads a header from a socket.
 * @Parameters: sock - The socket file descriptor.
 * @Return: The read header structure.
 *
 ********************************************************************/
Header readHeader(int sock) {
    Header header;
    
    read(sock, &header.type, sizeof(char));
    read(sock, &header.length, sizeof(char) * 2);
    for (int i = 0; i < atoi(header.length); i++) {
        read(sock, &header.header[i], sizeof(char));
    }
    read(sock, &header.data, 256 - 3 - atoi(header.length));

    return header;
}

/********************************************************************
 *
 * @Purpose: Extracts a substring from a string based on specified indices.
 * @Parameters: from - The starting index.
 *              until - The ending character.
 *              string - The source string.
 * @Return: The extracted substring.
 *
 ********************************************************************/
char* getString(int from, char until, char* string) {
    char* buffer;
    int j = 0;

    buffer = (char*) malloc(sizeof(char));

    for (int i = from; string[i] != until; i++) {
        buffer[j] = string[i];
        buffer = (char*) realloc(buffer, sizeof(char) * (j + 1));
        j++;
    }

    buffer[j] = '\0';
    
    return buffer;
}