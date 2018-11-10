#pragma once

#include "Data.hpp"
#include "mainHeader.hpp"

int connectToServer(string ip, short port);
int sendIdenty(int receivedSocket, const char id);
int runApplication(int receivedSocket, string cmd = "");
int sendFile(int receivedSocket, string filePath = "");
int getFile(int receivedSocket, string filePath = "");
int deleteFile(int receivedSocket, string filePath = "");
int processRequest(int receivedSocket, int option = -1, string path = "");

using namespace std;