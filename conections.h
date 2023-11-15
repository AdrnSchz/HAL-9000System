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

sockaddr_in configConnection(char* ip, int port);

int openConnection();

#endif