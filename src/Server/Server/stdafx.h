#pragma once
#pragma comment(lib,"Ws2_32.lib")

#include "targetver.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS

#include <errno.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <tchar.h>
#include <stdio.h> 
#include <winsock2.h> 
#include <stdlib.h> 
#include <string.h> 
#include <ws2tcpip.h>