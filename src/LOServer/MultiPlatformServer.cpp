#include "pch.h"
#include "libs.h"

//функция, которая принимает все отправленные байты
int receiveAll(int receivedSocket, Data &data)
{
	//Сначала инициализируем бфер на 1024 байта
	//Это буфер, в который мы будем передавать размер передаваемого пакета
	//то есть по омему представлению мы сначала говорим сколько байт мы будем передавать
	//Для этого и нужен первый receive
	char buf[1024];

	//В received записывается, сколько байт он принял, сами же байты
	//идут в буфер
	int received = recv(receivedSocket, buf, sizeof(char) * 1024, 0);

	if (received < 0) return 0;

	//Так как соединение не особо стабильное, нужно удостовериться
	//Что все байты переданы
	while (received != sizeof(char) * 1024)
	{
		int lastReceived = recv(receivedSocket, buf + received, sizeof(char) * 1024 - received, 0);

		//Обрабатываем ошибку, когда соединение было прервано
		if (lastReceived < 0) return 0;

		received += lastReceived;
	}

	//Из полученных данных, узнаем какой у нас размер данных
	data.len = atoi(buf) + 1;

	//Получаем первую пачку данных
	received = recv(receivedSocket, data.setDataBuff(), sizeof(char)*data.len, 0);

	if (received < 0) return 0;

	//Получаем все остальное, если осталось
	while (received != data.len * sizeof(char))
	{
		int lastReceived = recv(receivedSocket, buf + received, sizeof(char) * 1024 - received, 0);

		//Обрабатываем ошибку, когда соединение было прервано
		if (lastReceived < 0) return 0;

		received += recv(receivedSocket, data.bufPointer + received, sizeof(char)*data.len - received, 0);
	}

	//Возвращаем данные в виде набора байтов
	return 1;
}

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
	addr.sin_port = htons(1337);
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

		if (listen(socketServer, 0x100))
		{
			cout << "Listen failed!" << strerror(errno) << endl;
			CLOSE(socketServer);
			return -1;
		}

		bool doListen = true;
		hostent* host = NULL;

		while (doListen)
		{
            if ((socketClient = accept(socketServer, (sockaddr*)&clientAddr, &clientAddrSize)) < 0)
            {
                cout << "Failed to accept socket" << strerror(errno) << endl;
                CLOSE(socketServer);
                return -1;
            }

			if (socketClient >= 0)
			{
				host = gethostbyaddr((char*)&clientAddr.sin_addr.s_addr, 4, AF_INET);
				if(host == NULL)
				{
                    cout << "Can't get host - " << h_errno << endl;
                    CLOSE(socketServer);
                    return -1;
                }
				cout << getStrTime() << "User: " << host->h_name << " connected successfully!" << endl;
				doListen = false;
			}
		}

		char jobIdentifier = '0';
		char* buf = new char[1024];
		int receiveArg = 0;
		bool receiveFlag = true;
		//Вот с этого момента мы начинаем прослушивать сокет, с которого идет запрос
		while (receiveFlag)
		{
			//Пока мы приняли меньше чем 0 байт мы продолжаем слушать
			while (receiveArg == 0)
			{
				receiveArg = recv(socketClient, buf, sizeof(char), 0);
			}

			//Обработка ошибки, на случай, если отключится пользователь
			if (receiveArg == -1)
			{
				cout << getStrTime() << "User: " << host->h_name << " disconnected!" << strerror(errno) << endl;
				break;
			}

			//Получаем идентификатор необходимого действия
			jobIdentifier = buf[0];
			buf[0] = '0';
			receiveArg = 0;
			string command;
			Data data;
			Data eData;

			//В зависимости от него, смотрим, какую задачу выполнять
			switch (jobIdentifier)
			{
			case '1':
				cout << getStrTime() << "Launch file!" << endl;

				if (!receiveAll(socketClient, data))
				{
					cout << getStrTime() << "User: " << host->h_name << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}

				command = data.bufPointer;
				runFile(command);

				jobIdentifier = '0';
				break;
			case '2':
				cout << "Wait to file name!" << endl;

                //Сначала принимаем имя файла
                if (!receiveAll(socketClient, eData))
				{
					cout << getStrTime() << "User: " << host->h_name << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}

                cout << getStrTime() << "Getting file by name: " << eData.bufPointer << endl;

                //Здесь принимаем сам файл
				if (!receiveAll(socketClient, data))
				{
					cout << getStrTime() << "User: " << host->h_name << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}

                //Функция по записи файла
                writeFile(data, eData.bufPointer);
				jobIdentifier = '0';
				break;
			case '3':
				cout << "Succeed third identy!" << endl;
				jobIdentifier = '0';
				break;
			case '4':
				cout << "Succeed fourth identy!" << endl;
				jobIdentifier = '0';
				break;
			}

			if (jobIdentifier == '5')
			{
				cout << getStrTime() << "User: " << host->h_name << " disconnected!" << endl;
				break;
			}
		}
	}

	CLOSE(socketServer);
	CLOSE(socketClient);

	return 1;
}

int main()
{
	string ip;
	cout << "Enter IP address: ";
	cin >> ip;

	int socketServer = startServer(ip.c_str());

	if (socketServer > 0)
	{
		while (startListenSocket(socketServer) >= 0) {}
	}
	else
	{
		return -1;
	}

	return 0;
}
