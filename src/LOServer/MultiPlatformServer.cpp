#include "pch.h"
#include "libs.h"
#include <fstream>

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

int sendall(int receivedSocket, const char *buf, int len, int flags)
{
	int total = 0;
	int n;

	while (total < len)
	{
		n = send(receivedSocket, buf + total, len - total, flags);
		if (n == -1) { break; }
		total += n;
	}

	return (n == -1 ? -1 : total);
}

int sendIdenty(int receivedSocket, const char id)
{
	const char identy[1] = { id };
	return send(receivedSocket, identy, sizeof(char), 0);
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

int sendFile(int receivedSocket, Data eData)
{
    //Здесь вводится путь к файлу
    string filePath = eData.bufPointer;
	string pathFileName;
    ifstream file;

    file.open (filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        cout << getStrTime() << "Bad file path!" << endl;
        send(receivedSocket, "badPath ", 9, 0);
        return 42;
    }

    send(receivedSocket, "goodPath", 9, 0);

    // отсекаем имя файла
    #ifdef _WIN32
    size_t slashPos = filePath.rfind('\\');
    #else
    size_t slashPos = filePath.rfind('/');
    #endif

    slashPos == string::npos ? pathFileName = filePath : pathFileName = filePath.substr(slashPos + 1);

    //Создаем буффер для имени файла, размер которого равен размеру строки в байтах
	char* fileName = new char[pathFileName.size()*sizeof(char) + 1];

	//Чтобы избавится от константности не просто приравнием, а копируем строку
	strcpy(fileName, pathFileName.c_str());

	//Создаем буфер, в котором будет храниться размер передаваемых данных
	char* fileNameSize = new char[1024];

	//Переводим размер данных в строковое представление
	itoa(pathFileName.size()*sizeof(char), fileNameSize, 10);

	//Посылаем информацию о размере данных об имени файла
	if (sendall(receivedSocket, fileNameSize, 1024, 0) < 0)
    {
        cout << getStrTime() << "Connection with server was lost:" << strerror(errno) << endl;
        delete fileName;
        delete fileNameSize;
        return 0;
    }

	//Посылаем само имя
	if (sendall(receivedSocket, fileName, (pathFileName.size() + 1) * sizeof(char), 0) < 0)
    {
        cout << getStrTime() << "Connection with server was lost:" << strerror(errno) << endl;
        delete fileName;
        delete fileNameSize;
        return 0;
    }

	delete fileName;
	delete fileNameSize;

    //Получаем информацию о размере файла
    file.seekg(0, std::ios_base::end);
    std::ifstream::pos_type endPos = file.tellg();
    file.seekg(0, std::ios_base::beg);
    int fileSize = static_cast<int>(endPos - file.tellg());

	char* dataBuf = new char[fileSize*sizeof(char) + 1];

	file.read(dataBuf, fileSize*sizeof(char));

    if (file)
        cout << getStrTime() << "All characters read successfully." << endl;
    else
        cout << getStrTime() << "Error: only " << file.gcount() << " could be read" << endl;

	//Создаем буфер, в котором будет храниться размер передаваемых данных
	char* datasize = new char[1024];

	//Переводим размер данных в строковое представление
	itoa (fileSize*sizeof(char), datasize, 10);

    stringstream ss;
    ss << fileSize*sizeof(char);
    string myString = ss.str();
    strcpy(datasize, myString.c_str());

	//Посылаем информацию о размере файла
	if (sendall(receivedSocket, datasize, 1024, 0) < 0)
    {
        cout << getStrTime() << "Connection with server was lost:" << strerror(errno) << endl;
        delete dataBuf;
        delete datasize;
        return 0;
    }

	//Посылаем сам файл
	if (sendall(receivedSocket, dataBuf, (fileSize * sizeof(char)) + 1, 0) < 0)
    {
        cout << getStrTime() << "Connection with server was lost:" << strerror(errno) << endl;
        delete dataBuf;
        delete datasize;
        return 0;
    }

    file.close();
	delete dataBuf;
	delete datasize;

	return 1;
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
				cout << getStrTime() << "Wait to file name!" << endl;

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
            {
				cout << getStrTime() << "Wait to file path!" << endl;

				if (!receiveAll(socketClient, eData))
				{
					cout << getStrTime() << "User: " << host->h_name << " was Disconnected" << endl;
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
                    sendall(socketClient, "good", 5, 0);
                }
                else
                {
                    cout << getStrTime() << "User: " << host->h_name << " was Disconnected" << endl;
                    receiveFlag = false;
                    sendall(socketClient, "bad ", 5, 0);
                    break;
                }

				jobIdentifier = '0';
				break;
			}
			case '4':
				cout << "Wait to file path!" << endl;

				if (!receiveAll(socketClient, eData))
				{
					cout << getStrTime() << "User: " << host->h_name << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}

                cout << getStrTime() << "Deleting a file: " << eData.bufPointer << endl;

                string stringPath;

                for (int i = 0; i < eData.len; ++i)
                {
                    stringPath.push_back(eData.bufPointer[i]);

                    #ifdef _WIN32
                    if (eData.bufPointer[i] == '\\')
                        stringPath.push_back('\\');
                    #else
                    if (eData.bufPointer[i] == '/')
                        stringPath.push_back('/');
                    #endif
                }

                remove(stringPath.c_str()) ? printf("%s", "Deleting error!") : printf("%s", "Deleting successful!");

				jobIdentifier = '0';
				break;

                if (jobIdentifier == '5')
                {
                    cout << getStrTime() << "User: " << host->h_name << " disconnected!" << endl;
                    break;
                }
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
