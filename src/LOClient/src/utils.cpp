#include "utils.hpp"

int writeFile(Data &data, char* fileName)
{
    ofstream outFile(fileName, ios::binary);

    outFile.write(data.bufPointer, data.len);

    cout << fileName << " Was written!" << endl;

    outFile.close();

    return 1;
}

int receiveAll(int receivedSocket, Data &data)
{
	char buf[1024];

	int received = recv(receivedSocket, buf, sizeof(char) * 1024, 0);

	if (received < 0) return 0;

	while (received != sizeof(char) * 1024)
	{
		int lastReceived = recv(receivedSocket, buf + received, sizeof(char) * 1024 - received, 0);

		if (lastReceived < 0) return 0;

		received += lastReceived;
	}

	data.len = atoi(buf) + 1;

	received = recv(receivedSocket, data.setDataBuff(), sizeof(char)*data.len, 0);

	if (received < 0) return 0;

	while (received != data.len * sizeof(char))
	{
		int lastReceived = recv(receivedSocket, buf + received, sizeof(char) * 1024 - received, 0);

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
