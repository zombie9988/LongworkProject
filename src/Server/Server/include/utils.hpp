#pragma once

#include "mainHeader.hpp"
#include "data.hpp"

using namespace std;

string getStrTime();
int runFile(string cmd);
int writeFile(Data &data, const char* fileName);
int sendIdenty(int receivedSocket, const char id);
int sendFile(int receivedSocket, Data eData);
int receiveAll(int receivedSocket, Data &data);
int receivePart(int receiveSocket, Data& data, int len);
int sendPart(int receivedSocket, string buf, int len);
int sendAll(int receiveSocket, Data data);