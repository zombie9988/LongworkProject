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
// 16384
#define BLOCK_SIZE 16384
#define BUF_LEN 256
#define PORT 1337
#define THREAD_SIZE 1000
using namespace std;
