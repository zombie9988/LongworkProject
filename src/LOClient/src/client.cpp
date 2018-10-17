#include "client.hpp"
#include "utils.hpp"

int connectToServer(string ip, string port)
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);

	if (WSAStartup(wVersionRequested, &wsaData))
	{
		cout << "WSAStartup error: " << WSAGetLastError() << endl;
		return -1;
	}

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (clientSocket < 0)
	{
		cout << "Socket start error - " << WSAGetLastError() << endl;

		WSACleanup();
		return -1;
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
		cout << "Invalid address!" << endl;
		WSACleanup();
		return -1;
	}

	if (connect(clientSocket, (sockaddr*)&addr, sizeof(addr)))
	{
		cout << "Connect error - " <<  WSAGetLastError();
		WSACleanup();
		return -1;
	}

	system("cls");
	cout << "Connected!" << endl;

	return clientSocket;

}

int sendIdenty(int receivedSocket, const char id)
{
	const char identy[1] = { id };
	return send(receivedSocket, identy, sizeof(char), 0);
}

int runApplication(int receivedSocket)
{
	string cmd;

	cout << "Enter command:" << endl;

	while (cmd == "")
	{
		getline(cin, cmd);
	}

	if (sendall(receivedSocket, to_string(cmd.size()*sizeof(char)).c_str(), BUF_LEN) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        return -1;
    }

	if (sendall(receivedSocket, cmd.c_str(), (cmd.size() + 1) * sizeof(char)) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        return -1;
    }

	return 1;
}

int sendFile(int receivedSocket)
{
	string filePath;
	string pathFileName;
    ifstream file;

	cout << "Enter path to file: " << endl;

    do
    {
        getline(cin, filePath);

        file.open (filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

        if (!file.is_open())
            cout << "Bad file path!" << endl;
    }
    while (!file.is_open());

    size_t slashPos = filePath.rfind('\\');

    slashPos == string::npos ? pathFileName = filePath : pathFileName = filePath.substr(slashPos + 1);

	if (sendall(receivedSocket, to_string(pathFileName.size()*sizeof(char)).c_str(), BUF_LEN) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        return -1;
    }

	if (sendall(receivedSocket, pathFileName.c_str(), (pathFileName.size() + 1) * sizeof(char)) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        return -1;
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
            std::cout << "All characters read successfully.";
        else
            std::cout << "Error: only " << file.gcount() << " could be read";

        stringstream ss;
        ss << fileSize*sizeof(char);
        string myString = ss.str();

        if (sendall(receivedSocket, to_string(fileSize*sizeof(char)).c_str(), BUF_LEN) < 0)
        {
            cout << "Connection with server was lost:" << strerror(errno) << endl;
            delete dataBuf;
            return -1;
        }

        if (sendall(receivedSocket, dataBuf, (fileSize * sizeof(char)) + 1) < 0)
        {
            cout << "Connection with server was lost:" << strerror(errno) << endl;
            delete dataBuf;
            return -1;
        }

        file.close();
        delete dataBuf;
	}
    catch(bad_alloc& ba)
    {
        cerr << "Out of memory. " << ba.what() << endl;
        return -1;
    }

	return 1;
}

int getFile(int receivedSocket)
{
    string filePath;
    Data eData, data, resData;

    cout << "Enter path to file: " << endl;

    getline(cin, filePath);

    if (sendall(receivedSocket, to_string(filePath.size()*sizeof(char)).c_str(), BUF_LEN) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        return -1;
    }

    if (sendall(receivedSocket, filePath.c_str(), (filePath.size() + 1) * sizeof(char)) < 0)
    {
        cout << "Connection with server was lost: " << strerror(errno) << endl;
        return -1;
    }

    try
    {
        char* filePathResult = new char[9];
        recv(receivedSocket, filePathResult, 9, 0);
        string pathRes = filePathResult;

        if(pathRes == "badPath ")
        {
            cout << "Bad file path!" << endl;
            system("pause");
            delete filePathResult;
            return 1;
        }

        if (!receiveAll(receivedSocket, eData))
        {
            cout << "Connection with server was lost!" << endl;
        }

        cout << "Getting file by name: " << eData.bufPointer << endl;

        if (!receiveAll(receivedSocket, data))
        {
            cout << "Connection with server was lost!" << endl;
        }

        char buf[5];

        int received = recv(receivedSocket, buf, sizeof(char) * 5, 0);

        string result = buf;

        if (result == "good")
        {
            writeFile(data, eData.bufPointer);
            cout << "File: " << eData.bufPointer << " was written!" << endl;
            system("pause");
        }
        else
        {
            cout << "File: " << eData.bufPointer << " wasn't written" << endl;
            return 0;
        }
    }
    catch(bad_alloc& ba)
    {
        cerr << "Out of memory. " << ba.what() << endl;
        return -1;
    }

	return 1;
}

int deleteFile(int receivedSocket)
{
    string filePath;

    cout << "Enter path to file: " << endl;

    getline(cin, filePath);

    if (sendall(receivedSocket, to_string(filePath.size()*sizeof(char)).c_str(), BUF_LEN) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        return -1;
    }

    if (sendall(receivedSocket, filePath.c_str(), (filePath.size() + 1) * sizeof(char)) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        return -1;
    }

    try
    {
        char* filePathResult = new char[9];
        recv(receivedSocket, filePathResult, 9, 0);
        string pathRes = filePathResult;

        if(pathRes == "badPath ")
        {
            cout << "Bad file path!" << endl;
            system("pause");
            delete filePathResult;
            return 0;
        }

        cout << "File was deleted!" << endl;
        system("pause");
    }
    catch(bad_alloc& ba)
    {
       cerr << "Out of memory. " << ba.what() << endl;
       return -1;
    }

	return 1;
}

int processRequest(int receivedSocket)
{
    while (true)
	{
		cout << "1. Run Application" << endl;
		cout << "2. Send File" << endl;
		cout << "3. Get file" << endl;
		cout << "4. Delete file" << endl << endl;
		cout << "0. Exit" << endl << endl;

		int option = 0;
		int sent;
		Data data;

		(cin >> option).get();

		switch (option)
		{
		case 1:
			system("cls");
			if ((sent = sendIdenty(receivedSocket, '1')) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                WSACleanup();
                return -1;
            }

			if ((runApplication(receivedSocket)) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                WSACleanup();
                return -1;
            }

			system("cls");
			break;

		case 2:
			system("cls");
			if ((sent = sendIdenty(receivedSocket, '2')) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                WSACleanup();
                return -1;
            }

			if ((sendFile(receivedSocket)) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                WSACleanup();
                return -1;
            }

			system("cls");
			break;

		case 3:
			system("cls");
			if ((sent = sendIdenty(receivedSocket, '3')) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                WSACleanup();
                return -1;
            }

			if (getFile(receivedSocket) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                WSACleanup();
                return -1;
            }

			system("cls");
			break;

		case 4:
			system("cls");
			if ((sent = sendIdenty(receivedSocket, '4')) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                WSACleanup();
                return -1;
            }

			if((deleteFile(receivedSocket)) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                WSACleanup();
                return -1;
            }

			system("cls");
			break;
		case 0:
			system("cls");
			sendIdenty(receivedSocket, '5');
			system("cls");
			return 0;

		default:
			system("cls");
			cout << "Choose one of the point\n" << endl;
			break;
		}
	}
}
