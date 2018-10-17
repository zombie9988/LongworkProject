#pragma once

#include "Data.hpp"
#include "includes.hpp"

#define BUF_LEN 1024

int processRequest(int receivedSocket);
int connectToServer(string ip, string port);
int sendIdenty(int receivedSocket, const char id);
int runApplication(int receivedSocket);
int sendFile(int receivedSocket);
int getFile(int receivedSocket);
int deleteFile(int receivedSocket);
int processRequest(int receivedSocket);
