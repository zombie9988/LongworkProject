#pragma once

#include "stdafx.h"
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
