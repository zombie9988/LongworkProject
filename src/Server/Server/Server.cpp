#include "stdafx.h"

using namespace std;

void startServer()
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	
	if (int error = WSAStartup(wVersionRequested, &wsaData))
	{
		printf("WSAStartup failed with error: %d\n", error);
		return;
	}

	struct sockaddr_in addr;

	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port   = htons(1337);
	addr.sin_family = AF_INET;

	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (serverSocket < 0)
	{
		cout << "Socket start error!\n" << WSAGetLastError() << endl;

		WSACleanup();
		return;
	}

	if (bind(serverSocket, (SOCKADDR*)&addr, sizeof(addr)))
	{
		cout << "Bind Error!\n" << WSAGetLastError() << endl;
		
		closesocket(serverSocket);
		WSACleanup();
		return;
	}

	if (listen(serverSocket, 0x100))
	{
		cout << "Listen Error!" << endl;

		closesocket(serverSocket);
		WSACleanup();
		return;
	}

	cout << "Ожидание подключений..." << endl;

	SOCKET clientSocket;
	struct sockaddr_in clientAddr;

	int clientAddrSize = sizeof(clientAddr);

	while ((clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize)))
	{
		hostent* host;

		host = gethostbyaddr((char* )&clientAddr.sin_addr.s_addr, 4, AF_INET);

		cout << "Пользователь " << host->h_name << " успешно подключен!" << endl;
	}

	closesocket(serverSocket);
	closesocket(clientSocket);
	WSACleanup();

	return;
}

int main()
{
	setlocale(LC_ALL, "Russian");

	startServer();

	return 0;
}

