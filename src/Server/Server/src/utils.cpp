#include "utils.hpp"

string runFile(string cmd)
{
	string result = "";

	std::cout << getStrTime() << "Doing command: " << cmd << std::endl;

	char buff[128];

	FILE* pipe = _popen(cmd.c_str(), "r");

	if (!pipe)
	{
		fclose(pipe);
		return result;
	}

	while (!feof(pipe))
		if (fgets(buff, 128, pipe) != NULL)
			result += buff;

	fclose(pipe);

	if (result == "")
		result = "0";

	return result;
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
	//��������� ������ � data
	Data fileSize;

	//��������� ������ ������
	if (receivePart(receivedSocket, fileSize, BUF_LEN) < 0)
	{
		//throw runtime_error("Could't receive data, check connection");
		return -1;
	}

	//��������� ���� ������
	if (receivePart(receivedSocket, data, atoi(fileSize.getCharString())) < 0)
	{
		//throw runtime_error("Could't receive data, check connection");
		return -1;
	}

	return 1;
}

int sendPart(int receivedSocket, const char* buf, int len)
{
	int total = 0;

	while (total < len)
	{
		int n = send(receivedSocket, buf + total, len - total, 0);
		
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
	//������� ��� ���������� ��������� ������ ������������ ������

	Data dataSize(data.getDataSize());

	//�������� ������ ������
	if (sendPart(receiveSocket, dataSize.getCharString(), BUF_LEN) < 0)
	{
		//throw runtime_error("Could't send data, check connection");
		return -1;
	}

	//�������� ���� ������
	if (sendPart(receiveSocket, data.getCharString(), data.getDataSize()) < 0)
	{
		//throw runtime_error("Could't send data, check connection");
		return -1;
	}

	return 1;
}

int sendIdenty(int receivedSocket, const char id)
{
	const char identy[1] = { id };
	return send(receivedSocket, identy, sizeof(char), 0);
}
