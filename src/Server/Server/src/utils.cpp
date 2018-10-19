#include "utils.hpp"

int runFile(string cmd)
{
    std::cout << getStrTime() << "Doing command: " << cmd << std::endl;

    return system(cmd.c_str());
}

int writeFile(Data &data, const char* fileName)
{
    ofstream outFile(fileName, ios::binary);

    outFile.write(data.getCharString(), data.getDataSize());

    cout << getStrTime() << fileName << " Was written!" << endl;

    outFile.close();

    return 1;
}

string getStrTime()
{
	time_t seconds = time(NULL);
	tm* timeinfo = localtime(&seconds);
	char buffer[80];
	string format = "[%H:%M:%S] ";

	strftime(buffer, 80, format.c_str(), timeinfo);

	string time = buffer;

	return time;
}

int receivePart(int receivedSocket, Data& data, int len)
{
	int received = 0;

	data.createBuffer(len);

	while (received != len)
	{
		int lastReceived = recv(receivedSocket, data.getBuffer() + received, len - received, 0);

		if (lastReceived < 0)
		{
			return -1;
		}

		received += lastReceived;
	}

	data = data.getBuffer();

	return received;
}

int receiveAll(int receivedSocket, Data& data)
{
	//Принимаем данные в data
	Data fileSize;

	//Принимаем размер данных
	if (receivePart(receivedSocket, fileSize, BUF_LEN) < 0)
	{
		throw runtime_error("Could't receive data, check connection");
		return -1;
	}

	//Принимаем сами данные
	if (receivePart(receivedSocket, data, atoi(fileSize.getCharString())) < 0)
	{
		throw runtime_error("Could't receive data, check connection");
		return -1;
	}

	return 1;
}

int sendPart(int receivedSocket, string buf, int len)
{
	int total = 0;

	while (total < len)
	{
		int n = send(receivedSocket, buf.c_str() + total, len - total, 0);

		if (n == -1)
		{
			return -1;
		}

		total += n;
	}

	return total;
}

int sendAll(int receiveSocket, Data data)
{
	//Сначала нам необходимо отправить размер отправляемых данных

	Data dataSize(data.getDataSize());

	//Посылаем размер данных
	if (sendPart(receiveSocket, dataSize.getCharString(), BUF_LEN) < 0)
	{
		throw runtime_error("Could't send data, check connection");
		return -1;
	}

	//Посылаем сами данные
	if (sendPart(receiveSocket, data.getCharString(), data.getDataSize()) < 0)
	{
		throw runtime_error("Could't send data, check connection");
		return -1;
	}

	return 1;
}

int sendIdenty(int receivedSocket, const char id)
{
	const char identy[1] = { id };
	return send(receivedSocket, identy, sizeof(char), 0);
}

int sendFile(int receivedSocket, Data eData)
{
    string filePath = eData.getCharString();
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

    #ifdef _WIN32
    size_t slashPos = filePath.rfind('\\');
    #else
    size_t slashPos = filePath.rfind('/');
    #endif

    slashPos == string::npos ? pathFileName = filePath : pathFileName = filePath.substr(slashPos + 1);

	if (sendAll(receivedSocket, pathFileName.c_str()) < 0)
    {
        cout << getStrTime() << "Connection with server was lost:" << strerror(errno) << endl;
        return 0;
    }

    file.seekg(0, std::ios_base::end);
    std::ifstream::pos_type endPos = file.tellg();
    file.seekg(0, std::ios_base::beg);
    int fileSize = static_cast<int>(endPos - file.tellg());

    try
    {
        char* dataBuf = new char[fileSize];

        file.read(dataBuf, fileSize);

        if (file)
            cout << getStrTime() << "All characters read successfully." << endl;
        else
            cout << getStrTime() << "Error: only " << file.gcount() << " could be read" << endl;

        if (sendAll(receivedSocket, dataBuf) < 0)
        {
            cout << getStrTime() << "Connection with server was lost:" << strerror(errno) << endl;
            delete dataBuf;
            return 0;
        }

        file.close();
        delete dataBuf;
    }
    catch(bad_alloc& ba)
    {
        cerr << "Out of memory." << ba.what() << endl;
        return -1;
    }

	return 1;
}
