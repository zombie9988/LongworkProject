#include "utils.hpp"

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

// ���� ������ ������, �� ���������� _c_buffer, ����� �������

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