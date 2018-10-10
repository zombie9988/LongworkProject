#include "client.hpp"
#include "utils.hpp"

int connectToServer(string ip, string port)
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);

	if (WSAStartup(wVersionRequested, &wsaData))
	{
		cout << "WSAStartup error: " << WSAGetLastError() << endl;
		return -1;
	}

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (clientSocket < 0)
	{
		cout << "Socket start error - " << WSAGetLastError() << endl;

		WSACleanup();
		return -1;
	}

	struct sockaddr_in addr;

	addr.sin_port   = htons(1337);
	addr.sin_family = AF_INET;

	if (inet_addr(ip.c_str()) != INADDR_NONE)
	{
		addr.sin_addr.s_addr = inet_addr(ip.c_str());
	}
	else
	{
		cout << "Invalid address!" << endl;
		WSACleanup();
		return -1;
	}

	if (connect(clientSocket, (sockaddr*)&addr, sizeof(addr)))
	{
		cout << "Connect error - " <<  WSAGetLastError();
		WSACleanup();
		return -1;
	}

	system("cls");
	cout << "Connected!" << endl;

	return clientSocket;

}

int sendIdenty(int receivedSocket, const char id)
{
	const char identy[1] = { id };
	return send(receivedSocket, identy, sizeof(char), 0);
}

int runApplication(int receivedSocket)
{
	//Здесь вводится команда для выполения
	string cmd;
	cout << "Enter command:" << endl;

	while (cmd == "")
	{
		getline(cin, cmd);
	}

	//Создаем буффер для команды, размер которого равен размеру строки в байтах
	char* command = new char[cmd.size()*sizeof(char) + 1];
	//Чтобы избавится от константности не просто приравнием, а копируем строку
	strcpy(command, cmd.c_str());
	//Создаем буфер, в котором будет храниться размер передаваемых данных
	char* datasize = new char[1024];
	//Переводим размер данных в строковое представление
	itoa(cmd.size()*sizeof(char), datasize, 10);

	//Посылаем информацию о размере файла
	if (sendall(receivedSocket, datasize, 1024, 0) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        delete command;
        delete datasize;
        return -1;
    }

	//Посылаем сам файл
	if (sendall(receivedSocket, command, (cmd.size() + 1) * sizeof(char), 0) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        delete command;
        delete datasize;
        return -1;
    }

	delete command;
	delete datasize;

	return 1;
}

int sendFile(int receivedSocket)
{
    //Здесь вводится путь к файлу
	string filePath;
	string pathFileName;
    ifstream file;

	cout << "Enter path to file: " << endl;

    do
    {
        getline(cin, filePath);

        file.open (filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

        if (!file.is_open())
            cout << "Bad file path!" << endl;
    }
    while (!file.is_open());

    // отсекаем имя файла
    size_t slashPos = filePath.rfind('\\');

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
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        delete fileName;
        delete fileNameSize;
        return -1;
    }

	//Посылаем само имя
	if (sendall(receivedSocket, fileName, (pathFileName.size() + 1) * sizeof(char), 0) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        delete fileName;
        delete fileNameSize;
        return -1;
    }

	delete fileName;
	delete fileNameSize;

    //Получаем информацию о размере файла
    file.seekg(0, std::ios_base::end);
    std::ifstream::pos_type endPos = file.tellg();
    file.seekg(0, std::ios_base::beg);
    int fileSize = static_cast<int>(endPos - file.tellg());

	char* dataBuf = new char(fileSize*sizeof(char) + 1);

	file.read(dataBuf, fileSize*sizeof(char));

    if (file)
      std::cout << "All characters read successfully.";
    else
      std::cout << "Error: only " << file.gcount() << " could be read";

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
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        delete dataBuf;
        delete datasize;
        return -1;
    }

	//Посылаем сам файл
	if (sendall(receivedSocket, dataBuf, (fileSize * sizeof(char)) + 1, 0) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        delete dataBuf;
        delete datasize;
        return -1;
    }

    file.close();
	delete dataBuf;
	delete datasize;

	return 1;
}

int getFile(int receivedSocket)
{
    string filePath;
    Data eData, data, resData;

    cout << "Enter path to file: " << endl;

    getline(cin, filePath);

    char* path     = new char[filePath.size()*sizeof(char) + 1];
    char* pathSize = new char[1024];

    strcpy(path, filePath.c_str());
    itoa(filePath.size()*sizeof(char), pathSize, 10);

    if (sendall(receivedSocket, pathSize, 1024, 0) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        delete path;
        delete pathSize;
        return -1;
    }

    if (sendall(receivedSocket, path, (filePath.size() + 1) * sizeof(char), 0) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        delete path;
        delete pathSize;
        return -1;
    }

    char* filePathResult = new char[9];
    recv(receivedSocket, filePathResult, 9, 0);
    string pathRes = filePathResult;

    if(pathRes == "badPath ")
    {
        cout << "Bad file path!" << endl;
        system("pause");
        delete path;
        delete pathSize;
        delete filePathResult;
        return 1;
    }

    if (!receiveAll(receivedSocket, eData))
    {
        cout << "Connection with server was lost!" << endl;
    }

    cout << "Getting file by name: " << eData.bufPointer << endl;

    //Здесь принимаем сам файл
    if (!receiveAll(receivedSocket, data))
    {
        cout << "Connection with server was lost!" << endl;
    }

    char buf[5];

	int received = recv(receivedSocket, buf, sizeof(char) * 5, 0);

    string result = buf;

    if (result == "good")
    {
        writeFile(data, eData.bufPointer);
        cout << "File: " << eData.bufPointer << " was written!" << endl;
        system("pause");
    }
    else
    {
        cout << "File: " << eData.bufPointer << " wasn't written" << endl;
        delete path;
        delete pathSize;
        return 0;
    }

	delete path;
	delete pathSize;

	return 1;
}

int deleteFile(int receivedSocket)
{
    string filePath;

    cout << "Enter path to file: " << endl;

    getline(cin, filePath);

    char* path     = new char[filePath.size()*sizeof(char) + 1];
    char* pathSize = new char[1024];

    strcpy(path, filePath.c_str());
    itoa(filePath.size()*sizeof(char), pathSize, 10);

    if (sendall(receivedSocket, pathSize, 1024, 0) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        delete path;
        delete pathSize;
        return -1;
    }

    if (sendall(receivedSocket, path, (filePath.size() + 1) * sizeof(char), 0) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        delete path;
        delete pathSize;
        return -1;
    }

    char* filePathResult = new char[9];
    recv(receivedSocket, filePathResult, 9, 0);
    string pathRes = filePathResult;

    if(pathRes == "badPath ")
    {
        cout << "Bad file path!" << endl;
        system("pause");
        delete path;
        delete pathSize;
        delete filePathResult;
        return 0;
    }

    cout << "File was deleted!" << endl;
    system("pause");

	delete path;
	delete pathSize;

	return 1;
}

int processRequest(int receivedSocket)
{
    while (true)
	{
		cout << "1. Run Application" << endl;
		cout << "2. Send File" << endl;
		cout << "3. Get file" << endl;
		cout << "4. Delete file" << endl << endl;
		cout << "0. Exit" << endl << endl;

		int option = 0;
		int sent;
		Data data;

		(cin >> option).get(); // считаываем \n после getline

		//В зависимости от того, какое число выбрал пользователь
		//Мы пошлем разный идетификторы серверу через функцию
		//setidentity()
		//После чего вызовется фукнция, которая непосредственно передает данные
		switch (option)
		{
		case 1:
			system("cls");
			if ((sent = sendIdenty(receivedSocket, '1')) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                WSACleanup();
                return -1;
            }

			if ((runApplication(receivedSocket)) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                WSACleanup();
                return -1;
            }

			system("cls");
			break;

		case 2:
			system("cls");
			if ((sent = sendIdenty(receivedSocket, '2')) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                WSACleanup();
                return -1;
            }

			if ((sendFile(receivedSocket)) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                WSACleanup();
                return -1;
            }

			system("cls");
			break;

		case 3:
			system("cls");
			if ((sent = sendIdenty(receivedSocket, '3')) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                WSACleanup();
                return -1;
            }

			if (getFile(receivedSocket) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                WSACleanup();
                return -1;
            }

			system("cls");
			break;

		case 4:
			system("cls");
			if ((sent = sendIdenty(receivedSocket, '4')) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                WSACleanup();
                return -1;
            }

			if((deleteFile(receivedSocket)) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                WSACleanup();
                return -1;
            }

			system("cls");
			break;
		case 0:
			system("cls");
			sendIdenty(receivedSocket, '5');
			system("cls");
			return 0;

		default:
			system("cls");
			cout << "Choose one of the point\n" << endl;
			break;
		}
	}
}
