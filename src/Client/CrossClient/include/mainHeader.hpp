#pragma once

#ifdef _WIN32
	#include "winInclude.hpp"
	#define CLOSE closesocket
	#define CLEAR system("cls");

#elif __linux__
	#include "linuxInclude.hpp"
	#define CLOSE close
	#define CLEAR system("clear");
	#define START_SOCKET_SERVER() \

#elif __unix__ // all unices not caught above
	#include "linuxInclude.hpp"
	#define CLOSE close
	#define START_SOCKET_SERVER() \

#else
#   error "Unknown compiler"
#endif

#define BUF_LEN 256
#define PORT 1337
#define RESULT_LEN 9

using namespace std;
