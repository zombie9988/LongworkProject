#include "server.hpp"
#include "utils.hpp"
#include "clientProcessor.hpp"
#include "threadController.hpp"

int startServer(const char* ip)
{
#ifdef _WIN32
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);

	if (int error = WSAStartup(wVersionRequested, &wsaData))
	{
		printf("WSAStartup failed with error: %d\n", error);
		return -1;
	}
#endif

	int socketServer = (int)socket(AF_INET, SOCK_STREAM, 0);

	if (socketServer < 0)
	{
		cout << "Socket start error! " << strerror(errno) << endl;
		return -1;
	}

	sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = inet_addr(ip);

	if (bind((SOCKET)socketServer, (const sockaddr*)&addr, (int)sizeof(addr)))
	{
		cout << "Bind fail! " << strerror(errno) << endl;
		CLOSE(socketServer);
		return -1;
	}

	if (listen(socketServer, 0x100))
	{
		cout << "listen fail! " << strerror(errno) << endl;
		CLOSE(socketServer);
		return -1;
	}

	cout << getStrTime() << "Waiting for connect..." << endl;

	return socketServer;
}

int startListenSocket(int socketServer)
{
	bool stopServer = false;
	int socketClient;
	threadController controller(THREAD_SIZE);
	while (!stopServer) 
	{
		sockaddr_in clientAddr;
		socklen_t clientAddrSize = sizeof(clientAddr);

		if (listen(socketServer, 0x1))
		{
			cout << "Listen failed!" << strerror(errno) << endl;
			CLOSE(socketServer);
			return -1;
		}

		bool doListen = true;
		string clientName;

		while (doListen)
		{
			if ((socketClient = (int)accept(socketServer, (sockaddr*)&clientAddr, &clientAddrSize)) < 0)
			{
				cout << getStrTime() << "Failed to accept socket" << strerror(errno) << endl;
				CLOSE(socketServer);
				return -1;
			}

			controller.setThread(processClient, socketClient);
		}
	}

	CLOSE(socketServer);
	CLOSE(socketClient);

	return 1;
}
