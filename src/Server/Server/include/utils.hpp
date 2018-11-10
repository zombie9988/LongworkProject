#pragma once

#include "mainHeader.hpp"
#include "data.hpp"

using namespace std;

string getStrTime();
string runFile(string cmd);
int sendIdenty(int receivedSocket, const char id);
int receiveAll(int receivedSocket, Data &data);
int receivePart(int receiveSocket, Data& data, int len);
int sendPart(int receivedSocket, const char* buf, int len);
int sendAll(int receiveSocket, Data data);