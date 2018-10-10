#include "../include/utils.hpp"

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