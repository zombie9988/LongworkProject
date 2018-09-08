#include "stdafx.h"

using namespace std;

bool connectToServer(string ip, string port)
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	
	if (int error = WSAStartup(wVersionRequested, &wsaData))
	{
		printf("WSAStartup failed with error: %d\n", error);
		return false;
	}

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (clientSocket < 0)
	{
		cout << "Socket start error!\n" << WSAGetLastError();

		WSACleanup();
		return false;
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
		return false;
	}

	if (connect(clientSocket, (sockaddr*)&addr, sizeof(addr)))
	{
		cout << "Connect error!\n" << WSAGetLastError();

		WSACleanup();
		return false;
	}

	system("cls");
	cout << "Подключение к серверу произведено!" << endl;

	return true;
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

	if (!connectToServer(ip, port)) // если была ошибка, то прекращаем работу.
	{
		return 42;
	}

	while (true)
	{
		cout << "1. Запуск приложения на целевой системе" << endl;
		cout << "2. Загрузка файла на целевую систему" << endl;
		cout << "3. Получение файла с целевой системы" << endl;
		cout << "4. Удаление файла на целевой системе" << endl;

		int option = 0;
		cin >> option;

		switch (option)
		{
		case 1:
			system("cls");
			cout << "Тех работы :(" << endl;
			break;

		case 2:
			system("cls");
			cout << "Тех работы :)" << endl;
			break;

		case 3:
			system("cls");
			cout << "Тех работы :)" << endl;
			break;

		case 4:
			system("cls");
			cout << "Тех работы :)" << endl;
			break;

		default:
			system("cls");
			cout << "Выберите один из четырех пунктов!\n" << endl;
			break;
		}
	}
}