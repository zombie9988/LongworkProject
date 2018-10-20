#pragma once

#include "Data.hpp"
#include "mainHeader.hpp"

int writeFile(Data &data, string fileName);
int receiveAll(int receivedSocket, Data &data);
int receivePart(int receiveSocket, Data& data, int len);
int sendPart(int receivedSocket, string buf, int len);
int sendAll(int receiveSocket, Data data);