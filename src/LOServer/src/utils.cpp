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
	string format = "[%H:%M:%S] ";

	strftime(buffer, 80, format.c_str(), timeinfo);

	string time = buffer;

	return time;
}

int receiveAll(int receivedSocket, Data &data)
{
	char buf[BUF_LEN];

	int received = recv(receivedSocket, buf, sizeof(char) * BUF_LEN, 0);

	if (received < 0) return 0;

	while (received != sizeof(char) * BUF_LEN)
	{
		int lastReceived = recv(receivedSocket, buf + received, sizeof(char) * BUF_LEN - received, 0);

		if (lastReceived < 0) return 0;

		received += lastReceived;
	}

	data.len = atoi(buf) + 1;

	received = recv(receivedSocket, data.setDataBuff(), sizeof(char)*data.len, 0);

	if (received < 0) return 0;

	while (received != data.len * sizeof(char))
	{
		int lastReceived = recv(receivedSocket, buf + received, sizeof(char) * BUF_LEN - received, 0);

		if (lastReceived < 0) return 0;

		received += recv(receivedSocket, data.bufPointer + received, sizeof(char)*data.len - received, 0);
	}

	return 1;
}

int sendall(int receivedSocket, const char *buf, int len)
{
	int total = 0;
	int n;

	while (total < len)
	{
		n = send(receivedSocket, buf + total, len - total, 0);
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

    #ifdef _WIN32
    size_t slashPos = filePath.rfind('\\');
    #else
    size_t slashPos = filePath.rfind('/');
    #endif

    slashPos == string::npos ? pathFileName = filePath : pathFileName = filePath.substr(slashPos + 1);

	if (sendall(receivedSocket, to_string(pathFileName.size()*sizeof(char)).c_str(), BUF_LEN) < 0)
    {
        cout << getStrTime() << "Connection with server was lost:" << strerror(errno) << endl;
        return 0;
    }

	if (sendall(receivedSocket, pathFileName.c_str(), (pathFileName.size() + 1) * sizeof(char)) < 0)
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
        char* dataBuf = new char[fileSize*sizeof(char) + 1];

        file.read(dataBuf, fileSize*sizeof(char));

        if (file)
            cout << getStrTime() << "All characters read successfully." << endl;
        else
            cout << getStrTime() << "Error: only " << file.gcount() << " could be read" << endl;

        if (sendall(receivedSocket, to_string(fileSize*sizeof(char)).c_str(), BUF_LEN) < 0)
        {
            cout << getStrTime() << "Connection with server was lost:" << strerror(errno) << endl;
            delete dataBuf;
            return 0;
        }

        if (sendall(receivedSocket, dataBuf, (fileSize * sizeof(char)) + 1) < 0)
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
