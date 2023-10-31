#include "functions.h"

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

void capitalize(char** string) {
    int i = 0;

    while ((*string)[i] != '\0') {
        if ((*string)[i] >= 'a' && (*string)[i] <= 'z') {
            (*string)[i] -= 32;
        }
        i++;
    }
}

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

int checkPort(int port) {


    if (port < 0 || port > 65535) {
        return -1;
    }

    return 0;
}