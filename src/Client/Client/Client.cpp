#include "stdafx.h"

using namespace std;

int connectToServer(string ip, string port)
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	
	if (int error = WSAStartup(wVersionRequested, &wsaData))
	{
		printf("WSAStartup failed with error: %d\n", error);
		return -1;
	}

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (clientSocket < 0)
	{
		cout << "Socket start error!\n" << WSAGetLastError();

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
		cout << "Invalid address!" << WSAGetLastError();

		WSACleanup();
		return -1;
	}

	if (connect(clientSocket, (sockaddr*)&addr, sizeof(addr)))
	{
		cout << "Connect error!\n" << WSAGetLastError();

		WSACleanup();
		return -1;
	}

	system("cls");
	cout << "Подключение к серверу произведено!" << endl;

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

void runApplication(int receivedSocket)
{
	//Здесь вводится команда для выполения
	string cmd;
	cout << "Введите команду для выполнения:" << endl;

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
	_itoa(cmd.size()*sizeof(char), datasize, 1024);

	//Посылаем информацию о размере файла
	sendall(receivedSocket, datasize, 1024, 0);

	//Посылаем сам файл
	sendall(receivedSocket, command, (cmd.size() + 1) * sizeof(char), 0);

	delete command;
	delete datasize;

	return;
}

//В разработке
void sendFile(int receivedSocket)
{
	return;
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
	setlocale(LC_ALL, "Russian");

	string ip, port;

	cout << "Укажите адрес сервера: ";
	cin >> ip;

	cout << "Укажите порт  сервера: ";
	cin >> port;

	cout << endl;
	
	int receivedSocket = connectToServer(ip, port);

	if (receivedSocket < 0) // если была ошибка, то прекращаем работу.
	{
		return 42;
	}
	
	//Здесь начинается обратботка запросов
	while (true)
	{
		cout << "0. Выход" << endl;
		cout << "1. Запуск приложения на целевой системе" << endl;
		cout << "2. Загрузка файла на целевую систему" << endl;
		cout << "3. Получение файла с целевой системы" << endl;
		cout << "4. Удаление файла на целевой системе" << endl;

		int option = 0;
		cin >> option;

		//В зависимости от того, какое число выбрал пользователь
		//Мы пошлем разный идетификторы серверу через функцию
		//setidentity()
		//После чего вызовется фукнция, которая непосредственно передает данные
		switch (option)
		{
		case 1:
			system("cls");
			sendIdenty(receivedSocket, '1');
			runApplication(receivedSocket);
			system("cls");
			break;

		case 2:
			system("cls");
			sendIdenty(receivedSocket, '2');
			sendFile(receivedSocket);
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
			cout << "Выберите один из четырех пунктов!\n" << endl;
			break;
		}
	}
}