#pragma once

#include "Data.hpp"
#include "includes.hpp"

int writeFile(Data &data, char* fileName);
int receiveAll(int receivedSocket, Data &data);
int sendall(int receivedSocket, const char *buf, int len);
