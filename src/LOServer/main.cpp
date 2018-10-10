﻿#include "server.hpp"
#include "mainHeader.hpp"

using namespace std;

int main()
{
    string ip;
	//string ip = "192.168.1.253";
	cout << "Enter IP address: ";
	cin >> ip;

	int socketServer = startServer(ip.c_str());

	if (socketServer > 0)
	{
		while (startListenSocket(socketServer) >= 0);
	}
	else
	{
		return -1;
	}

	return 0;
}
