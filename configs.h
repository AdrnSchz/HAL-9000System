/********************************************************************
 *
 * @Purpose: HAL 9000 System - Config files
 * @Author: Marc Escoté Llopis & Adrián Jorge Sánchez López
 *
 * - This file contains function declarations and structs definitions used 
 *   for reading the configuration files.
 ********************************************************************/
#ifndef _CONFIGS_H_
#define _CONFIGS_H_

#include "functions.h"

/**
 * Structure for storing server configuration data.
*/
typedef struct {
    char* server;
    char* path;
    char* discovery_ip;
    int discovery_port;
    char* user_ip;
    int user_port;
} Server_conf;

/**
 * Structure for storing user configuration data.
*/
typedef struct {
    char* user;
    char* files_path;
    char* ip;
    int port;
} User_conf;

/**
 * Structure for storing Discovery server configuration data.
*/
typedef struct {
    char* ip_poole;
    int port_poole;
    char* ip_bow;
    int port_bow;
} Disc_conf;

/**
 * Structure for storing plalist data.
*/
typedef struct {
    int num_songs;
    char* name;
    char** songs;
} Playlist;

/********************************************************************
 *
 * @Purpose: Reads the configuration from a file and initializes the Disc_conf structure.
 * @Parameters: file - The path to the configuration file.
 * @Return: The Disc_conf structure with the configuration data.
 *
 ********************************************************************/
Disc_conf readConfigDis(char* file);

/********************************************************************
*
* @Purpose: Reads the configuration from a file and stores it in the User_conf structure.
* @Parameters: file - The path to the configuration file.
* @Return: ---
*
*********************************************************************/
User_conf readConfigBow(char* file);

/********************************************************************
 *
 * @Purpose: Reads the configuration from a file and stores it in the Server_conf structure.
 * @Parameters: file - The path to the configuration file.
 * @Return: The Server_conf structure with the configuration file data.
 *
 ********************************************************************/
Server_conf readConfigPol(char* file);

/********************************************************************
 *
 * @Purpose: Reads the configuration of the songs.
 * @Parameters: file - The path to the configuration file.
  *          
 * @Return: The Server_conf structure with the configuration file data.
 *
 ********************************************************************/
char **readSongs(char* file, int *num_songs);

Playlist* readPlaylists(char* file, int *num_playlists);

#endif