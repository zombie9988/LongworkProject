#pragma once

#ifdef _WIN32
	#include "winInclude.hpp"
	#define CLOSE closesocket

#elif __linux__
	#include "linuxInclude.hpp"
	#define CLOSE close
	#define START_SOCKET_SERVER() \

#elif __unix__ // all unices not caught above
	#include "linuxInclude.hpp"
	#define CLOSE close
	#define START_SOCKET_SERVER() \

#else
#   error "Unknown compiler"
#endif

using namespace std;