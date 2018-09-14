#pragma once

#include "mainHeader.h"

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

int runFile(std::string cmd);
int writeFile(Data &data, char* fileName);
std::string getStrTime();
