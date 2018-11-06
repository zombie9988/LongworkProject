#include "clientProcessor.hpp"

int processClient(int socketClient)
{
	string clientIp, clientName;
	hostent* host = NULL;
	#ifdef __linux__
	char* buffer = new char[256];
	inet_ntop(AF_INET, &clientAddr.sin_addr.s_addr, buffer, 256);
	clientIp = buffer;
	delete buffer;
	elif _WIN32
	clientIp = (char*)&clientAddr.sin_addr.s_addr;
	#endif

	bool doListen = false;
	if (socketClient >= 0)
	{
		host = gethostbyaddr(clientIp.c_str(), 4, AF_INET);

		if (host == NULL)
		{
			clientName = clientIp;
			cout << getStrTime() << "Can't get host info - " << clientIp << " " << strerror(h_errno) << endl;
			doListen = false;
		}
		else 
		{
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
		catch (bad_alloc& ba)
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
		{
			if (receiveAll(socketClient, data) < 0)
			{
				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			command = data.getCharString();
			runFile(command); // позже допилить результат команды! Ќа клиент нужно что-то возвращать!

			jobIdentifier = '0';
			break;
		}
		case '2':
		{
			cout << getStrTime() << "Wait to file name!" << endl;

			if (receiveAll(socketClient, eData) < 0)
			{
				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			string fileName = eData.getCharString();

			cout << getStrTime() << "Getting file by name: " << fileName << endl;

			FILE* fp = nullptr;

			if (!(fp = fopen(eData.getCharString(), "wb")))
			{
				cout << getStrTime() << "File \"" << fileName << "\" is not create!" << endl;

				string error = "0";

				if (sendAll(socketClient, error) < 0) // в случае неудачи отвечаем клиенту нулем
				{
					fclose(fp);

					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}
			}

			string request = "1";

			if (sendAll(socketClient, request) < 0) // если все хорошо, то отсылаем единицу
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			if (receiveAll(socketClient, eData) < 0) // принимаем ответ клиента об успехе/неуспехе подсчета количества блоков
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			if (eData.getString() == "0") // если все плохо, прекращаем работу функции
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " can't send file" << endl;
				receiveFlag = false;
				break;
			}

			if (receiveAll(socketClient, eData) < 0) // тут количество блоков
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			long long countOfBlocks = 0;

			try
			{
				countOfBlocks = atoll(eData.getString().c_str()); // string to long long
			}
			catch (bad_alloc& ba)
			{
				fclose(fp);

				cout << "Problem with getting file: " << fileName << endl;

				throw runtime_error(ba.what());
			}

			if (sendAll(socketClient, request) < 0) // если все хорошо, то отсылаем единицу. √отовы начать принимать файл
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			/*if (request == "0")
			{
				fclose(fp);

				cout << "Problem with getting file: " << fileName << endl;
				break;
			}*/

			for (int i = 0; i < countOfBlocks; ++i)
			{
				if (receiveAll(socketClient, eData) < 0)
				{
					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}

				if (!fwrite(eData.getBuffer(), 1, BLOCK_SIZE, fp)) // пытаемс€ записать блок
				{
					request = "0";

					if (sendAll(socketClient, request) < 0)
					{
						cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
						receiveFlag = false;
						break;
					}
				}

				request = "1";

				if (sendAll(socketClient, request) < 0)
				{
					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}
			}

			if (receiveAll(socketClient, eData) < 0)
			{
				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			if (eData.getString() == "0")
			{
				fclose(fp);

				cout << "Problem with getting file: " << fileName << endl;
				break;
			}

			if (receiveAll(socketClient, eData) < 0)
			{
				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			long lastBytesCount = 0;

			try
			{
				lastBytesCount = atol(eData.getString().c_str()); // string to long long
			}
			catch (bad_alloc& ba)
			{
				request = "0";

				if (sendAll(socketClient, request) < 0)
				{
					fclose(fp);

					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}

				fclose(fp);

				cout << "Problem with getting file: " << fileName << endl;

				throw runtime_error(ba.what());
			}

			request = "1";

			if (sendAll(socketClient, request) < 0)
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			if (receiveAll(socketClient, eData) < 0) // прин€ли остатки файла
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			if (!fwrite(eData.getBuffer(), 1, lastBytesCount + 1, fp))
			{
				request = "0";

				if (sendAll(socketClient, request) < 0)
				{
					fclose(fp);

					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}
			}

			request = "1";

			if (sendAll(socketClient, request) < 0)
			{
				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			cout << getStrTime() << fileName << " was successfully writen!" << endl;

			fclose(fp);

			jobIdentifier = '0';
			break;
		}
		case '3':
		{
			FILE* fp = nullptr;
			string info;
			string filePath;
			string oldPath;
			string pathFileName;

			cout << getStrTime() << "Wait to file path!" << endl;

			do
			{
				if (receiveAll(socketClient, eData) < 0)
				{
					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}

				oldPath = eData.getString();

				for (int i = 0; i < oldPath.length(); ++i) // “ут парсим путь под нашу операционку
				{

					if (oldPath[i] == '\\' || oldPath[i] == '/')
					{
#ifdef _WIN32 
						filePath.push_back('\\');
#elif
						filePath.push_back('/');
#endif
					}
					else
					{
						filePath.push_back(oldPath[i]);
					}
				}

				if (fp = fopen(filePath.c_str(), "rb"))
				{
					info = "1";
				}
				else
				{
					info = "0";
				}

				if (sendAll(socketClient, info) < 0) // отправл€ем данные о том, смогли/не смогли открыть файл
				{
					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}
			} while (info == "0");

#ifdef _WIN32 // TODO: 2 слеша или один на линуксе? ѕоидее же один? 
			size_t slashPos = filePath.rfind('\\');
#elif
			size_t slashPos = filePath.rfind('/');
#endif

			// отрезаем им€ файла
			slashPos == string::npos ? pathFileName = filePath : pathFileName = filePath.substr(slashPos + 1);

			if (sendAll(socketClient, pathFileName) < 0)  // отправл€ем им€ файла
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			if (receiveAll(socketClient, eData) < 0)
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			if (eData.getString() == "1")
			{
				cout << getStrTime() << "Preparing to send file: " << pathFileName << endl;
			}
			else
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			long long countOfBlocks = 0;

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

			if (sendAll(socketClient, info) < 0) // посылаем информацию о том нужно ли дальше продолжать работу с сервером
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			if (info == "0")
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			if (sendAll(socketClient, to_string(countOfBlocks)) < 0) // если все успешно, то сервер готов принимать количество блоков
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			if (receiveAll(socketClient, eData) < 0) // прин€ли ответ от сервера, что он готов/не готов принимать блоки данных
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			if (eData.getString() == "0")
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			fseek(fp, 0, SEEK_SET);

			cout << getStrTime() << "Start sending..." << endl;

			try
			{
				// отправл€ем блоки данных и ждем ответа от сервера

				char buff[BLOCK_SIZE];

				for (long long i = 0; i < countOfBlocks; ++i)
				{
					fread(buff, 1, BLOCK_SIZE, fp);

					Data data(buff);

					if (sendAll(socketClient, data) < 0)
					{
						fclose(fp);

						cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
						receiveFlag = false;
						break;
					}

					if (receiveAll(socketClient, eData) < 0) // прин€ли ответ от сервера, что он готов/не готов принимать следующий блок
					{
						fclose(fp);

						cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
						receiveFlag = false;
						break;
					}

					if (eData.getString() == "0")
					{
						fclose(fp);

						cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
						receiveFlag = false;
						break;
					}
				}

				long lastBytesCount = 0; // оставшийс€ хвост ( <= BLOCK_SIZE )

				while (!fseek(fp, 1, SEEK_CUR))
				{
					if (fgetc(fp) == EOF)
					{
						if (feof(fp))
						{
							info = "1";

							if (sendAll(socketClient, info) < 0)
							{
								fclose(fp);

								cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
								receiveFlag = false;
								break;
							}

							break;
						}
						else
						{
							info = "0";

							if (sendAll(socketClient, info) < 0)
							{
								fclose(fp);

								cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
								receiveFlag = false;
								break;
							}

							fclose(fp);

							cout << getStrTime() << "Error write" << endl;
							return -1;
						}
					}

					fseek(fp, -1, SEEK_CUR);

					++lastBytesCount;
				}

				fseek(fp, -lastBytesCount - 1, SEEK_CUR);

				if (sendAll(socketClient, to_string(lastBytesCount)) < 0)
				{
					fclose(fp);

					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}

				if (receiveAll(socketClient, eData) < 0) // прин€ли ответ от сервера, что он готов/не готов принимать следующий блок
				{
					fclose(fp);

					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}

				if (eData.getString() == "0")
				{
					fclose(fp);

					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}

				fread(buff, 1, lastBytesCount + 1, fp);

				if (sendAll(socketClient, buff) < 0)
				{
					fclose(fp);

					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}
			}
			catch (bad_alloc &ba)
			{
				fclose(fp);

				throw runtime_error(ba.what());
			}

			if (receiveAll(socketClient, eData) < 0) // прин€ли ответ от сервера, о том, что файл успешно/неуспешно передан
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			if (eData.getString() == "0")
			{
				fclose(fp);

				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			cout << getStrTime() << "File: \"" << pathFileName << "\" is successfully sending!" << endl;

			fclose(fp);

			jobIdentifier = '0';
			break;
		}
		case '4':
		{
			cout << getStrTime() << "Wait to file path!" << endl;

			if (receiveAll(socketClient, eData) < 0) // принимаем путь
			{
				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			string oldPath = eData.getString();

			if (receiveAll(socketClient, eData) < 0) // принимаем им€ файла
			{
				cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
				receiveFlag = false;
				break;
			}

			string pathFileName = eData.getString();
			string filePath;
			string info;

			for (int i = 0; i < oldPath.length(); ++i) // “ут парсим путь под нашу операционку
			{

				if (oldPath[i] == '\\' || oldPath[i] == '/')
				{
#ifdef _WIN32 
					filePath.push_back('\\');
#elif
					filePath.push_back('/');
#endif
				}
				else
				{
					filePath.push_back(oldPath[i]);
				}
			}

			if (remove(filePath.c_str())) // удал€ем файл
			{
				info = "0";

				cout << getStrTime() << "File: " << pathFileName << " was not found!" << endl;

				if (sendAll(socketClient, info) < 0) // в случае неудачи отвечаем клиенту нулем
				{
					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}
			}
			else
			{
				info = "1";

				cout << getStrTime() << "File: " << pathFileName << " was successfully found!" << endl;

				if (sendAll(socketClient, info) < 0) // в случае удачного удалени€ отвечаем клиенту нулем
				{
					cout << getStrTime() << "User: " << clientName << " was Disconnected" << endl;
					receiveFlag = false;
					break;
				}
			}

			jobIdentifier = '0';
			break;
		}
		}
	}
}
