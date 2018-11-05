#pragma once

#include "Data.hpp"
#include "mainHeader.hpp"

int receiveAll(int receivedSocket, Data &data);
int receivePart(int receiveSocket, Data& data, int len);
int sendPart(int receivedSocket, const char* buf, int len);
int sendAll(int receiveSocket, Data data);