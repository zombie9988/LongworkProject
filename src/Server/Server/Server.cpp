#include "stdafx.h"

using namespace std;

void startServer()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		printf("WSAStartup failed with error: %d\n", err);
		return;
	}

	struct sockaddr_in addr;

	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1337);
	addr.sin_family = AF_INET;

	int socketNumber = socket(AF_INET, SOCK_STREAM, 0);

	if (socketNumber < 0)
	{
		cout << "Socket start error!\n";
		cout << WSAGetLastError();
		WSACleanup();
		return;
	}

	if (bind(socketNumber, (SOCKADDR*)&addr, sizeof(addr)))
	{
		cout << "Bind Error!\n";
		cout << WSAGetLastError();
		closesocket(socketNumber);
		WSACleanup();
		return;
	}

	if (listen(socketNumber, 0x100))
	{
		cout << "Listen Error!";
		closesocket(socketNumber);
		WSACleanup();
		return;
	}

	cout << "Waiting for connection...";

	SOCKET clientSocket;
	struct sockaddr_in clientAddr;

	int clientAddrSize = sizeof(clientAddr);

	while ((clientSocket = accept(socketNumber, (sockaddr*)&clientAddr, &clientAddrSize)))
	{
		hostent* host;

		host = gethostbyaddr((char* )&clientAddr.sin_addr.s_addr, 4, AF_INET);
		cout << "Connected!";
	}

	closesocket(socketNumber);
	closesocket(clientSocket);
	WSACleanup();

	return;
}

int main()
{
	startServer();

	return 0;
}

