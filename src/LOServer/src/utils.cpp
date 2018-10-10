#include "utils.hpp"

int runFile(string cmd)
{
    std::cout << getStrTime() << "Doing command: " << cmd << std::endl;

    return system(cmd.c_str());
}

int writeFile(Data &data, char* fileName)
{
    ofstream outFile(fileName, ios::binary);

    outFile.write(data.bufPointer, data.len);

    cout << getStrTime() << fileName << " Was written!" << endl;

    outFile.close();

    return 1;
}

string getStrTime()
{
	time_t seconds = time(NULL);
	tm* timeinfo = localtime(&seconds);
	char buffer[80];
	const char* format = "[%H:%M:%S] ";

	strftime(buffer, 80, format, timeinfo);

	string time = buffer;

	return time;
}

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