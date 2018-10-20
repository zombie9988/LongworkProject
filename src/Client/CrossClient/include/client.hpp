#pragma once

#include "Data.hpp"
#include "mainHeader.hpp"

int processRequest(int receivedSocket);
int connectToServer(string ip, short port);
int sendIdenty(int receivedSocket, const char id);
int runApplication(int receivedSocket);
int sendFile(int receivedSocket);
int getFile(int receivedSocket);
int deleteFile(int receivedSocket);
int processRequest(int receivedSocket);
