#pragma once

#include "mainHeader.h"

int runFile(std::string cmd);

std::string getStrTime();

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