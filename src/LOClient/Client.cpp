#include "stdafx.h"

using namespace std;

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
	itoa(cmd.size()*sizeof(char), datasize, 1024);

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

//Функция, которая занимается отправкой файла
int sendFile(int receivedSocket)
{
    //Здесь вводится путь к файлу
    //todo: Написать регулярку, которая будет отсекать имя файла от пути.
	string filePath;

	cout << "Enter path to file:" << endl;
    ifstream file;
    while(!file.is_open())
    {
        filePath = "";

        while (filePath == "")
        {
            getline(cin, filePath);
        }

    file.open (filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) cout << "Bad file path!" << endl;
    }

    //Создаем буффер для имени файла, размер которого равен размеру строки в байтах
	char* fileName = new char[filePath.size()*sizeof(char) + 1];

	//Чтобы избавится от константности не просто приравнием, а копируем строку
	strcpy(fileName, filePath.c_str());

	//Создаем буфер, в котором будет храниться размер передаваемых данных
	char* fileNameSize = new char[1024];

	//Переводим размер данных в строковое представление
	itoa(filePath.size()*sizeof(char), fileNameSize, 1024);

	//Посылаем информацию о размере данных об имени файла
	if (sendall(receivedSocket, fileNameSize, 1024, 0) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        delete fileName;
        delete fileNameSize;
        return -1;
    }

	//Посылаем само имя
	if (sendall(receivedSocket, fileName, (filePath.size() + 1) * sizeof(char), 0) < 0)
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

	char* dataBuf = new char[fileSize*sizeof(char) + 1];

	file.read(dataBuf, fileSize*sizeof(char));

    if (file)
      std::cout << "All characters read successfully.";
    else
      std::cout << "Error: only " << file.gcount() << " could be read";

	//Создаем буфер, в котором будет храниться размер передаваемых данных
	char* datasize = new char[1024];

	//Переводим размер данных в строковое представление
	itoa(fileSize*sizeof(char), datasize, 1024);

    stringstream ss;
    ss <<fileSize*sizeof(char);
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

//В разработке
void getFile(int receivedSocket)
{
	return;
}

//В разработке
void deleteFile(int receivedSocket)
{
	return;
}

int main()
{
	string ip, port;

	cout << "Enter server ip: ";
	cin >> ip;

	cout << "Enter server port: ";
	cin >> port;

	cout << endl;

	int receivedSocket = connectToServer(ip, port);

	if (receivedSocket < 0)  return -1;

	//Здесь начинается обратботка запросов
	while (true)
	{
		cout << "0.  Exit" << endl;
		cout << "1. Run Application" << endl;
		cout << "2. Send File" << endl;
		cout << "3. smth" << endl;
		cout << "4. smth" << endl;

		int option = 0;
		int sent;
		Data data;
		cin >> option;

		//В зависимости от того, какое число выбрал пользователь
		//Мы пошлем разный идетификторы серверу через функцию
		//setidentity()
		//После чего вызовется фукнция, которая непосредственно передает данные
		switch (option)
		{
		case 1:
			system("cls");
			if ((sent = sendIdenty(receivedSocket, '1'))< 0)
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
			if ((sent = sendIdenty(receivedSocket, '2'))< 0)
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
			sendIdenty(receivedSocket, '3');
			getFile(receivedSocket);
			system("cls");
			break;

		case 4:
			system("cls");
			sendIdenty(receivedSocket, '4');
			deleteFile(receivedSocket);
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
