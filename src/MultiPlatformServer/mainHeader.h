#pragma once

#ifdef _WIN32
	#include "winInclude.h"
	#define CLOSE closesocket

#elif __linux__
	#include "linuxInclude.h"
	#define CLOSE close
	#define START_SOCKET_SERVER() \

#elif __unix__ // all unices not caught above
	#include "linuxInclude.h"
	#define CLOSE close
	#define START_SOCKET_SERVER() \

#else
#   error "Unknown compiler"
#endif

using namespace std;