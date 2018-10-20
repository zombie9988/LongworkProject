#include "server.hpp"
#include "utils.hpp"
//Функция, стартующая сервер
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

	int socketServer = socket(AF_INET, SOCK_STREAM, 0);

	if (socketServer < 0)
	{
		cout << "Socket start error! " << strerror(errno) << endl;
		return -1;
	}

	sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = inet_addr(ip);

	if (bind(socketServer, (sockaddr*)&addr, sizeof(addr)))
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

	while (!stopServer) {

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
			hostent* host = NULL;

            if ((socketClient = accept(socketServer, (sockaddr*)&clientAddr, &clientAddrSize)) < 0)
            {
                cout << getStrTime() << "Failed to accept socket" << strerror(errno) << endl;
                CLOSE(socketServer);
                return -1;
            }

			#ifdef __linux__
			char* buffer = new char[256];
			inet_ntop(AF_INET, &clientAddr.sin_addr.s_addr, buffer, 256);
			string clientIp = buffer;
			delete buffer;
			#elif _WIN32
			string clientIp = (char*)&clientAddr.sin_addr.s_addr;
			#endif

			doListen = false;
			if (socketClient >= 0)
			{
				host = gethostbyaddr(clientIp.c_str(), 4, AF_INET);
				if(host == NULL)
				{
					clientName = clientIp;
					cout << getStrTime() << "Can't get host info - " << clientIp << " " << strerror(h_errno) << endl;
					doListen = false;
                    continue;
                }
				cout << getStrTime() << "User: " << host->h_name << " connected successfully!" << endl;
				clientName = host->h_name;
				doListen = false;
			}
		}

		char jobIdentifier = '0';
		int receiveArg = 0;
		bool receiveFlag = true;

		while (receiveFlag)
		{
		    try
		    {
		        char* buf = new char[BUF_LEN];

                while (receiveArg == 0)
                {
                    receiveArg = recv(socketClient, buf, sizeof(char), 0);
                }

                if (receiveArg == -1)
                {
                    cout << getStrTime() << "User: " << clientName << " disconnected! Because of: " << strerror(errno) << endl;
                    break;
                }

                jobIdentifier = buf[0];
                buf[0] = '0';

                delete buf;	
		    }
		    catch(bad_alloc& ba)
		    {
		        cerr << "Out of memory." << ba.what() << endl;
		        return -1;
		    }

			receiveArg = 0;
			string command;
			Data data;
			Data eData;

			switch (jobIdentifier)
			{
			case '1':
				cout << getStrTime() << "Launch file!" << endl;

				if (!receiveAll(socketClient, data))
				{
					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}

				command = data.getCharString();
				runFile(command);

				jobIdentifier = '0';
				break;
			case '2':
				cout << getStrTime() << "Wait to file name!" << endl;

                if (!receiveAll(socketClient, eData))
				{
					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}

                cout << getStrTime() << "Getting file by name: " << eData.getCharString() << endl;

				if (!receiveAll(socketClient, data))
				{
					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}

                writeFile(data, eData.getCharString());
				jobIdentifier = '0';
				break;
			case '3':
            {
				cout << getStrTime() << "Wait to file path!" << endl;

				if (!receiveAll(socketClient, eData))
				{
					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}

                int result = sendFile(socketClient, eData);

                if(result == 42)
                {
                    break;
                }
                else if(result)
                {
                    cout << getStrTime() << "All is good!" << endl;
                    sendAll(socketClient, "good");
                }
                else
                {
                    cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
                    receiveFlag = false;
                    sendAll(socketClient, "bad ");
                    break;
                }

				jobIdentifier = '0';
				break;
			}
			case '4':
				cout << getStrTime() << "Wait to file path!" << endl;

				if (!receiveAll(socketClient, eData))
				{
					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}

                string stringPath;

                for (int i = 0; i < eData.getDataSize(); ++i)
                {
                    stringPath.push_back(eData.getCharString()[i]);

                    #ifdef _WIN32
                    if (eData.getCharString()[i] == '\\')
                        stringPath.push_back('\\');
                    #else
                    if (eData.getCharString()[i] == '/')
                        stringPath.push_back('/');
                    #endif
                }

				ifstream file;

   				file.open (stringPath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

				if (!file.is_open())
				{
					cout << getStrTime() << "Bad file path!" << endl;
					send(socketClient, "badPath ", 9, 0);
					jobIdentifier = '0';
					break;
				}

				cout << getStrTime() << "Deleting a file: " << eData.getCharString() << endl;
				send(socketClient, "good ", 9, 0);
				file.close();
				remove(stringPath.c_str());

				jobIdentifier = '0';
				break;

                if (jobIdentifier == '5')
                {
                    cout << getStrTime() << "User: " << clientName << " disconnected!" << endl;
                    break;
                }
			}
        }
	}

	CLOSE(socketServer);
	CLOSE(socketClient);

	return 1;
}
