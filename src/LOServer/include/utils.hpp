#pragma once

#include "mainHeader.hpp"

using namespace std;
struct Data
{
	size_t len;
	char* bufPointer = new char[1];

	char* setDataBuff()
	{
		delete bufPointer;
		return bufPointer = new char[len + 1];
	}

	~Data()
	{
		delete bufPointer;
	}
};
string getStrTime();
int runFile(string cmd);
int writeFile(Data &data, char* fileName);
int receiveAll(int receivedSocket, Data &data);
int sendall(int receivedSocket, const char *buf, int len);
int sendIdenty(int receivedSocket, const char id);
int sendFile(int receivedSocket, Data eData);
