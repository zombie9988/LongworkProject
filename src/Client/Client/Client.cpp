#include "stdafx.h"

using namespace std;

void connectToServer(string ip, string port)
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

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (clientSocket < 0)
	{
		cout << "Socket start error!\n";
		cout << WSAGetLastError();
		WSACleanup();
		return;
	}

	struct sockaddr_in addr;
	hostent* host;
	addr.sin_port = htons(1337);
	addr.sin_family = AF_INET;

	if (inet_addr(ip.c_str()) != INADDR_NONE)
	{
		addr.sin_addr.s_addr = inet_addr(ip.c_str());
	}
	else
	{
		cout << "Invalid address!";
		cout << WSAGetLastError();
		WSACleanup();
		return;
	}

	if (connect(clientSocket, (sockaddr*)&addr, sizeof(addr)))
	{
		cout << "Connect error!\n";
		cout << WSAGetLastError();
		WSACleanup();
		return;
	}

	cout << "Connect success!";

	return;
}


int main()
{
	string ip, port;
	cin >> ip >> port;

	connectToServer(ip, port);

    return 0;
}