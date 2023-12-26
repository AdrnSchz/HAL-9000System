/********************************************************************
 *
 * @Purpose: HAL 9000 System - Common Functions
 * @Author: Marc Escoté Llopis & Adrián Jorge Sánchez López
 *
 * - This file contains common function declarations shared among the project.
 *
 ********************************************************************/

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
#include <pthread.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/msg.h>

#define printF(x) write(1, x, strlen(x))

#define C_RESET "\033[0m"
#define C_RED "\033[31m"
#define C_GREEN "\033[32m"
#define C_BOLDGREEN "\033[1m\033[32m"
#define BOLD    "\033[1m"

void print(char *str, pthread_mutex_t terminal);
/********************************************************************
 *
 * @Purpose: Reads an integer from a file descriptor and stores it in the specified variable.
 * @Parameters: source - File descriptor to read from.
 *              num - Pointer to the variable where the integer will be stored.
 * @Return: ---
 *
 ********************************************************************/
void readNum(int source, int* num);

/********************************************************************
 *
 * @Purpose: Reads a line from a file descriptor and allocates memory to store the information.
 * @Parameters: source - File descriptor to read from.
 *              string - Pointer to a string pointer where the line will be stored.
 * @Return: ---
 *
 ********************************************************************/
void readLine(int source, char** string);

/********************************************************************
 *
 * @Purpose: Reads a string from a file descriptor until it reaches the character end and return the string address.
 * @Parameters: fd - File descriptor to read from.
 *              end - character to stop reading.
 * @Return: ---
 *
 ********************************************************************/
char* readUntil(int fd, char end);

/********************************************************************
 *
 * @Purpose: Checks and removes '&' characters from a user name.
 * @Parameters: name - Pointer to the string containing the user name.
 * @Return: ---
 *
 ********************************************************************/
void checkName(char** name);

/********************************************************************
 *
 * @Purpose: Capitalizes all characters in a string.
 * @Parameters: string - Pointer to the string to be capitalized.
 * @Return: ---
 *
 ********************************************************************/
void capitalize(char** string);

/********************************************************************
 *
 * @Purpose: Removes extra white spaces from a string, keeping only one space between words.
 * @Parameters: string - Pointer to the string to be processed.
 * @Return: ---
 *
 ********************************************************************/
void removeWhiteSpaces(char** string);

/********************************************************************
 *
 * @Purpose: Checks if input entered in the command line corresponds to a valid command.
 * @Parameters: buffer - The input buffer containing the command.
 * @Return: The command index if valid, -1 otherwise.
 *
 ********************************************************************/
int checkCommand(char* buffer);

/********************************************************************
 *
 * @Purpose: Extracts a substring from a string based on specified indices.
 * @Parameters: from - The starting index.
 *              until - The ending character.
 *              string - The source string.
 * @Return: The extracted substring.
 *
 ********************************************************************/
char* getString(int from, char until, char* string);

/********************************************************************
 *
 * @Purpose: Extract the song name from the download command.
 * @Parameters: string - The source string.
 * @Return: The song name.
 *
 ********************************************************************/
char* getSongName(char* string);

void getMd5(char* file, char** md5);
#endif
