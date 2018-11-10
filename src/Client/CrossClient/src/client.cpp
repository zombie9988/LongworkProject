#include "client.hpp"
#include "utils.hpp"

int connectToServer(string ip, short port)
{
#ifdef _WIN32
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);

	if (WSAStartup(wVersionRequested, &wsaData))
	{
		cout << "WSAStartup error: " << WSAGetLastError() << endl;
		return -1;
	}
#endif

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

#ifdef _WIN32
	if (clientSocket < 0)
	{
		cout << "Socket start error - " << WSAGetLastError() << endl;

		return -1;
	}

#endif
	struct sockaddr_in addr;

	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;

	if (inet_addr(ip.c_str()) != INADDR_NONE)
	{
		addr.sin_addr.s_addr = inet_addr(ip.c_str());
	}
	else
	{
		cout << "Invalid address!" << endl;
		return -1;
	}

	if (connect(clientSocket, (sockaddr*)&addr, sizeof(addr)))
	{
		cout << "Connect error!" << endl;
		return -1;
	}

	CLEAR
		char* buf = new char[1];
		recv(clientSocket, buf, 1, 0);
		delete[] buf;
	cout << "Connected!" << endl;

	return (int)clientSocket;

}

int sendIdenty(int receivedSocket, const char id)
{
	const char identy[1] = { id };
	return send(receivedSocket, identy, sizeof(char), 0);
}

int runApplication(int receivedSocket, string cmd)
{
	if (cmd == "")
	{
		cout << "Enter command:" << endl;

		while (cmd == "")
		{
			getline(cin, cmd);
		}
	}

	if (sendAll(receivedSocket, cmd) < 0)
	{
		cout << "Connection with server was lost:" << strerror(errno) << endl;
		return -1;
	}

	Data request;

	if (receiveAll(receivedSocket, request) < 0)
	{
		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	if (request.getString() == "0")
	{
		cout << "Command \"" << cmd << "\" does not found on server!" << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return 0;
	}

	cout << request.getString() << endl;
	cout << "Press ENTER to continue..." << endl;
	cin.get();

	return 1;
}

int sendFile(int receivedSocket, string filePath)
{
	//string filePath;
	string pathFileName;

	FILE* fp = nullptr;

	if (filePath == "")
	{
		cout << "Enter path to file:" << endl;
		getline(cin, filePath);
	}
	// TODO: Открывает ли fopen линуксовые пути? если нет, то попробовать пропарсить filePath и добавить / к каждому слешу
	while (!(fp = fopen(filePath.c_str(), "rb"))) 
	{
		cout << "File is not found!" << endl;
		cout << "Enter path to file:" << endl;

		getline(cin, filePath); 
	}

#ifdef _WIN32  
	size_t slashPos = filePath.rfind('\\');
#elif
	size_t slashPos = filePath.rfind('/'); 
#endif

	// отрезаем имя файла
	slashPos == string::npos ? pathFileName = filePath : pathFileName = filePath.substr(slashPos + 1);

	if (sendAll(receivedSocket, pathFileName) < 0) // отправляем имя файла
	{
		fclose(fp);

		cout << "Connection with server was lost:" << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	Data request;

	// принимаем ответ сервера

	if (receiveAll(receivedSocket, request) < 0)
	{
		fclose(fp);

		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	if (request.getString() == "1")
	{
		cout << "Preparing to send file: " << pathFileName << endl;
	}
	else
	{
		fclose(fp);

		cout << "Connection with server was lost" << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	long long countOfBlocks = 0;
	string info;

	while (!fseek(fp, BLOCK_SIZE, SEEK_CUR))
	{
		if (fgetc(fp) == EOF)
		{
			if (feof(fp))
			{
				info = "1";
				break;
			}
			else
			{
				info = "0";
				break;
			}
		}

		fseek(fp, -1, SEEK_CUR);

		++countOfBlocks;
	}

	if (sendAll(receivedSocket, info) < 0) // посылаем информацию о том нужно ли дальше продолжать работу с сервером
	{
		fclose(fp);

		cout << "Connection with server was lost:" << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}
	
	if (info == "0")
	{
		fclose(fp);

		cout << "Problem with sending file. Error on server." << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	if (sendAll(receivedSocket, to_string(countOfBlocks)) < 0) // если все успешно, то сервер готов принимать количество блоков
	{
		fclose(fp);

		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	if (receiveAll(receivedSocket, request) < 0) // приняли ответ от сервера, что он готов/не готов принимать блоки данных
	{
		fclose(fp);

		cout << "Connection with server was lost" << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	if (request.getString() == "0")
	{
		fclose(fp);

		cout << "Problem with sending file. Error on server." << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	fseek(fp, 0, SEEK_SET);

	cout << "Start sending..." << endl;

	long long progress = 0;
	long long result   = 0;
	long long procent  = countOfBlocks / 100;

	try
	{
		// отправляем блоки данных и ждем ответа от сервера

		char buff[BLOCK_SIZE];

		for (long long i = 0; i < countOfBlocks; ++i)
		{	
			fread(buff, 1, BLOCK_SIZE, fp);

			Data data(buff);

			if (sendAll(receivedSocket, data) < 0)
			{
				fclose(fp);

				cout << "Connection with server was lost: " << strerror(errno) << endl;
				cout << "Press ENTER to continue..." << endl;
				cin.get();
				return -1;
			}

			if (receiveAll(receivedSocket, request) < 0) // приняли ответ от сервера, что он готов/не готов принимать следующий блок
			{
				fclose(fp);

				cout << "Connection with server was lost" << strerror(errno) << endl;
				cout << "Press ENTER to continue..." << endl;
				cin.get();
				return -1;
			}

			if (request.getString() == "0")
			{
				fclose(fp);

				cout << "Connection with server was lost " << strerror(errno) << endl;
				cout << "Press ENTER to continue..." << endl;
				cin.get();
				return -1;
			}

			result += 1;
			
			if (result >= procent)
			{
				++progress;
				result = 0;
				cout << "Progress: " << progress << "%" << '\r';
			}
		}

		long lastBytesCount = 0; // оставшийся хвост ( <= BLOCK_SIZE )

		while (!fseek(fp, 1, SEEK_CUR))
		{
			if (fgetc(fp) == EOF)
			{
				if (feof(fp))
				{
					info = "1";

					if (sendAll(receivedSocket, info) < 0)
					{
						fclose(fp);

						cout << "Connection with server was lost: " << strerror(errno) << endl;
						cout << "Press ENTER to continue..." << endl;
						cin.get();
						return -1;
					}

					break;
				}
				else
				{
					info = "0";

					if (sendAll(receivedSocket, info) < 0)
					{
						fclose(fp);

						cout << "Connection with server was lost: " << strerror(errno) << endl;
						cout << "Press ENTER to continue..." << endl;
						cin.get();
						return -1;
					}

					fclose(fp);

					cout << "Error write" << endl;
					cout << "Press ENTER to continue..." << endl;
					cin.get();
					return -1;
				}
			}

			fseek(fp, -1, SEEK_CUR);

			++lastBytesCount;
		}

		fseek(fp, -lastBytesCount - 1, SEEK_CUR);

		if (sendAll(receivedSocket, to_string(lastBytesCount)) < 0)
		{
			fclose(fp);

			cout << "Connection with server was lost: " << strerror(errno) << endl;
			cout << "Press ENTER to continue..." << endl;
			cin.get();
			return -1;
		}

		if (receiveAll(receivedSocket, request) < 0) // приняли ответ от сервера, что он готов/не готов принимать следующий блок
		{
			fclose(fp);

			cout << "Connection with server was lost" << strerror(errno) << endl;
			cout << "Press ENTER to continue..." << endl;
			cin.get();
			return -1;
		}

		if (request.getString() == "0")
		{
			fclose(fp);

			cout << "Connection with server was lost" << strerror(errno) << endl;
			cout << "Press ENTER to continue..." << endl;
			cin.get();
			return -1;
		}

		fread(buff, 1, lastBytesCount + 1, fp);

		if (sendAll(receivedSocket, buff) < 0)
		{
			fclose(fp);

			cout << "Connection with server was lost: " << strerror(errno) << endl;
			cout << "Press ENTER to continue..." << endl;
			cin.get();
			return -1;
		}

		cout << "Progress: 100%" << endl;
	}
	catch (bad_alloc &ba)
	{
		fclose(fp);

		throw runtime_error(ba.what());
	}

	if (receiveAll(receivedSocket, request) < 0) // приняли ответ от сервера, о том, что файл успешно/неуспешно передан
	{
		fclose(fp);

		cout << "Connection with server was lost" << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	if (request.getString() == "0")
	{
		fclose(fp);

		cout << "Problem with sending file. Error on server." << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	cout << "File: \"" << pathFileName << "\" is successfully sending!" << endl;

	fclose(fp);

	cout << "Press ENTER to continue..." << endl;
	cin.get();

	return 1;
}

int getFile(int receivedSocket, string filePath)
{
	//string filePath;
	string pathFileName;
	string info;
	Data request;

	do
	{
		
		if (filePath == "") 
		{
			cout << "Enter path to file: " << endl;
			getline(cin, filePath);
		}

		if (sendAll(receivedSocket, filePath) < 0)
		{
			cout << "Connection with server was lost: " << strerror(errno) << endl;
			cout << "Press ENTER to continue..." << endl;
			cin.get();
			return -1;
		}

		if (receiveAll(receivedSocket, request) < 0)
		{
			cout << "Connection with server was lost: " << strerror(errno) << endl;
			cout << "Press ENTER to continue..." << endl;
			cin.get();
			return -1;
		}

		if (request.getString() == "0")
		{
			cout << "File is not found!" << endl;
		}

		filePath = "";
	} 
	while (request.getString() == "0");

	if (receiveAll(receivedSocket, request) < 0) // принимаем имя файла
	{
		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	pathFileName = request.getString();

	cout << "Prepare to start getting file: " << pathFileName << endl;
	
	FILE* fp = nullptr;

	if (!(fp = fopen(pathFileName.c_str(), "wb"))) // создаем файл
	{
		cout << "File \"" << pathFileName << "\" is not create!" << endl;

		string error = "0";

		if (sendAll(receivedSocket, error) < 0) // в случае неудачи отвечаем серверу нулем
		{
			fclose(fp);

			cout << "Connection with server was lost: " << strerror(errno) << endl;
			cout << "Press ENTER to continue..." << endl;
			cin.get();
			return -1;
		}
	}

	info = "1";

	if (sendAll(receivedSocket, info) < 0) // если все хорошо, то отсылаем единицу
	{
		fclose(fp);

		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	if (receiveAll(receivedSocket, request) < 0) // принимаем ответ клиента об успехе/неуспехе подсчета количества блоков
	{
		fclose(fp);

		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	if (request.getString() == "0") // если все плохо, прекращаем работу функции
	{
		fclose(fp);

		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	if (receiveAll(receivedSocket, request) < 0) // тут количество блоков
	{
		fclose(fp);

		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	long long countOfBlocks = 0;

	try
	{
		countOfBlocks = atoll(request.getString().c_str()); // string to long long
	}
	catch (bad_alloc& ba)
	{
		fclose(fp);

		cout << "Problem with getting file: " << pathFileName << endl;

		throw runtime_error(ba.what());
	}

	if (sendAll(receivedSocket, info) < 0) // если все хорошо, то отсылаем единицу. Готовы начать принимать файл
	{
		fclose(fp);

		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	long long progress = 0;
	long long result = 0;
	long long procent = countOfBlocks / 100;

	for (int i = 0; i < countOfBlocks; ++i)
	{
		if (receiveAll(receivedSocket, request) < 0)
		{
			fclose(fp);

			cout << "Connection with server was lost: " << strerror(errno) << endl;
			cout << "Press ENTER to continue..." << endl;
			cin.get();
			return -1;
		}

		if (!fwrite(request.getBuffer(), 1, BLOCK_SIZE, fp)) // пытаемся записать блок
		{
			info = "0";

			if (sendAll(receivedSocket, info) < 0)
			{
				fclose(fp);

				cout << "Connection with server was lost: " << strerror(errno) << endl;
				cout << "Press ENTER to continue..." << endl;
				cin.get();
				return -1;
			}
		}

		info = "1";

		if (sendAll(receivedSocket, info) < 0)
		{
			fclose(fp);

			cout << "Connection with server was lost: " << strerror(errno) << endl;
			cout << "Press ENTER to continue..." << endl;
			cin.get();
			return -1;
		}

		result += 1;

		if (result >= procent)
		{
			++progress;
			result = 0;
			cout << "Progress: " << progress << "%" << '\r';
		}
	}

	if (receiveAll(receivedSocket, request) < 0)
	{
		fclose(fp);

		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	if (request.getString() == "0")
	{
		fclose(fp);

		cout << "Problem with getting file: " << pathFileName << endl;
		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	if (receiveAll(receivedSocket, request) < 0)
	{
		fclose(fp);

		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	long lastBytesCount = 0;

	try
	{
		lastBytesCount = atol(request.getString().c_str()); // string to long long
	}
	catch (bad_alloc& ba)
	{
		info = "0";

		if (sendAll(receivedSocket, info) < 0)
		{
			fclose(fp);

			cout << "Connection with server was lost: " << strerror(errno) << endl;
			cout << "Press ENTER to continue..." << endl;
			cin.get();
			return -1;
		}

		fclose(fp);

		cout << "Problem with getting file: " << pathFileName << endl;
		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		throw runtime_error(ba.what());
	}

	info = "1";

	if (sendAll(receivedSocket, info) < 0)
	{
		fclose(fp);

		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	if (receiveAll(receivedSocket, request) < 0) // приняли остатки файли
	{
		fclose(fp);

		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	if (!fwrite(request.getBuffer(), 1, lastBytesCount + 1, fp))
	{
		info = "0";

		if (sendAll(receivedSocket, info) < 0)
		{
			fclose(fp);

			cout << "Connection with server was lost: " << strerror(errno) << endl;
			cout << "Press ENTER to continue..." << endl;
			cin.get();
			return -1;
		}
	}

	cout << "Progress: 100%" << endl;

	info = "1";

	if (sendAll(receivedSocket, info) < 0)
	{
		fclose(fp);

		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	cout << pathFileName << " was successfully getting!" << endl;
	cout << "Press ENTER to continue..." << endl;
	cin.get();

	fclose(fp);
	return 1;
}

int deleteFile(int receivedSocket, string filePath)
{
	//string filePath;
	string pathFileName;
	Data request;

	if (filePath == "")
	{
		cout << "Enter path to file: " << endl;
		getline(cin, filePath);
	}

#ifdef _WIN32  
	size_t slashPos = filePath.rfind('\\');
#elif
	size_t slashPos = filePath.rfind('/');
#endif

	// отрезаем имя файла
	slashPos == string::npos ? pathFileName = filePath : pathFileName = filePath.substr(slashPos + 1);

	if (sendAll(receivedSocket, filePath) < 0) // отправляем путь
	{
		cout << "Connection with server was lost:" << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	if (sendAll(receivedSocket, pathFileName) < 0) // отправляем имя файла
	{
		cout << "Connection with server was lost:" << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	if (receiveAll(receivedSocket, request) < 0) // принимаем ответ от сервера. Смог\не смог удалить файл 
	{
		cout << "Connection with server was lost: " << strerror(errno) << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return -1;
	}

	if (request.getString() == "-2")
	{
		cout << "File \"" << pathFileName << "\" was not found!" << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return 0;
	}

	if (request.getString() == "-1")
	{
		cout << "File \"" << pathFileName << "\" is busy!" << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return 1;
	}

	if (request.getString() == "0")
	{
		cout << "File \"" << pathFileName << "\" was not found!" << endl;
		cout << "Press ENTER to continue..." << endl;
		cin.get();
		return 0;
	}

	cout << "File \"" << pathFileName << "\" was successfully deleted!" << endl;
	cout << "Press ENTER to continue..." << endl;
	cin.get();

	return 1;
}

int processRequest(int receivedSocket, int option, string path)
{
	int sent;

	if (option != 0)
	{
		switch (option)
		{
		case 1:
			CLEAR
				if ((sent = sendIdenty(receivedSocket, '1')) < 0)
				{
					cout << "Connection with server was lost" << endl;
					return -1;
				}

			if ((runApplication(receivedSocket, path)) < 0)
			{
				cout << "Connection with server was lost" << endl;
				return -1;
			}

			CLEAR
				return 1;

		case 2:
			CLEAR
				if ((sent = sendIdenty(receivedSocket, '2')) < 0)
				{
					cout << "Connection with server was lost" << endl;

					return -1;
				}

			if ((sendFile(receivedSocket, path)) < 0)
			{
				cout << "Connection with server was lost" << endl;

				return -1;
			}

			CLEAR
				return 1;

		case 3:
			CLEAR
				if ((sent = sendIdenty(receivedSocket, '3')) < 0)
				{
					cout << "Connection with server was lost: " << endl;

					return -1;
				}

			if (getFile(receivedSocket, path) < 0)
			{
				cout << "Connection with server was lost: " << endl;

				return -1;
			}

			CLEAR
				return 1;

		case 4:
			CLEAR
				if ((sent = sendIdenty(receivedSocket, '4')) < 0)
				{
					cout << "Connection with server was lost: " << endl;

					return -1;
				}

			if ((deleteFile(receivedSocket, path)) < 0)
			{
				cout << "Connection with server was lost: " << endl;

				return -1;
			}

			return 1;
		}

		while (true)
		{
			cout << "1. Run Application" << endl;
			cout << "2. Send File" << endl;
			cout << "3. Get file" << endl;
			cout << "4. Delete file" << endl << endl;
			cout << "0. Exit" << endl << endl;

			option = 0;

			(cin >> option).get();

			switch (option)
			{
			case 1:
				CLEAR
					if ((sent = sendIdenty(receivedSocket, '1')) < 0)
					{
						cout << "Connection with server was lost" << endl;
						return -1;
					}

				if ((runApplication(receivedSocket)) < 0)
				{
					cout << "Connection with server was lost" << endl;
					return -1;
				}

				CLEAR
					break;

			case 2:
				CLEAR
					if ((sent = sendIdenty(receivedSocket, '2')) < 0)
					{
						cout << "Connection with server was lost" << endl;

						return -1;
					}

				if ((sendFile(receivedSocket)) < 0)
				{
					cout << "Connection with server was lost" << endl;

					return -1;
				}

				CLEAR
					break;

			case 3:
				CLEAR
					if ((sent = sendIdenty(receivedSocket, '3')) < 0)
					{
						cout << "Connection with server was lost: " << endl;

						return -1;
					}

				if (getFile(receivedSocket) < 0)
				{
					cout << "Connection with server was lost: " << endl;

					return -1;
				}

				CLEAR
					break;

			case 4:
				CLEAR
					if ((sent = sendIdenty(receivedSocket, '4')) < 0)
					{
						cout << "Connection with server was lost: " << endl;

						return -1;
					}

				if ((deleteFile(receivedSocket)) < 0)
				{
					cout << "Connection with server was lost: " << endl;

					return -1;
				}

				CLEAR
					break;
			case 0:
				CLEAR
					sendIdenty(receivedSocket, '5');
				CLEAR
					return 0;

			default:
				CLEAR
					cout << "Choose one of the point\n" << endl;
				break;
			}
		}
	}
}
