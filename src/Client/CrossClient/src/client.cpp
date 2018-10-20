#include "../include/client.hpp"
#include "../include/utils.hpp"

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

	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

#ifdef _WIN32
	if (clientSocket < 0)
	{
		cout << "Socket start error - " << WSAGetLastError() << endl;

		
		return -1;
	}

#endif
	struct sockaddr_in addr;

	addr.sin_port   = htons(port);
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

	if (sendAll(receivedSocket, cmd.c_str()) < 0)
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

	if (sendAll(receivedSocket, pathFileName.c_str()) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        return -1;
    }

    file.seekg(0, std::ios_base::end);
    std::ifstream::pos_type endPos = file.tellg();
    file.seekg(0, std::ios_base::beg);

    int fileSize = (int)(endPos - file.tellg());

	Data fileData;

	fileData.createBuffer(fileSize);
    file.read(fileData.getBuffer(), fileSize);

    if (file)
        std::cout << "All characters read successfully.";
    else
        std::cout << "Error: only " << file.gcount() << " could be read";

    if (sendAll(receivedSocket, fileData.getBuffer()) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        return -1;
    }

    file.close();

	return 1;
}

int getFile(int receivedSocket)
{
    string filePath;
    Data eData, data, resData;

    cout << "Enter path to file: " << endl;

    getline(cin, filePath);

    if (sendAll(receivedSocket, filePath.c_str()) < 0)
    {
        cout << "Connection with server was lost: " << strerror(errno) << endl;
        return -1;
    }

	Data resultData;
	resultData.createBuffer(RESULT_LEN);

	if (receiveAll(receivedSocket, resultData) < 0)
	{
		cout << "Connection with server was lost: " << strerror(errno) << endl;
		return -1;
	}

	if (resultData.getCharString() == "badPath ")
	{
		cout << "Bad file path!" << endl;
		system("pause");
		return 1;
	}

	if (receiveAll(receivedSocket, eData) < 0)
	{
		cout << "Connection with server was lost: " << strerror(errno) << endl;
		return -1;
	}

	cout << "Getting file by name: " << eData.getCharString() << endl;

	if (receiveAll(receivedSocket, data) < 0)
	{
		cout << "Connection with server was lost: " << strerror(errno) << endl;
		return -1;
	}

	Data result;

	if (receiveAll(receivedSocket, result) < 0)
	{
		cout << "Connection with server was lost: " << strerror(errno) << endl;
		return -1;
	}

	if (result.getCharString() == "good")
	{
		writeFile(data, eData.getCharString());
		cout << "File: " << eData.getCharString() << " was written!" << endl;
		system("pause");
	}
	else
	{
		cout << "File: " << eData.getCharString() << " wasn't written" << endl;
		system("pause");
		return 0;
	}

	return 1;
}

int deleteFile(int receivedSocket)
{
    string filePath;

    cout << "Enter path to file: " << endl;
    getline(cin, filePath);

    if (sendAll(receivedSocket, filePath) < 0)
    {
        cout << "Connection with server was lost:" << strerror(errno) << endl;
        return -1;
    }

	Data result;
	receiveAll(receivedSocket, result);
	string pathRes = result.getCharString();

	if (pathRes == "badPath ")
	{
		cout << "Bad file path!" << endl;
		system("pause");
		return 0;
	}

	cout << "File was deleted!" << endl;
	system("pause");
  
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

		(cin >> option).get();

		switch (option)
		{
		case 1:
			CLEAR
			if ((sent = sendIdenty(receivedSocket, '1')) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                return -1;
            }

			if ((runApplication(receivedSocket)) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                return -1;
            }

			CLEAR
			break;

		case 2:
			CLEAR
			if ((sent = sendIdenty(receivedSocket, '2')) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                
                return -1;
            }

			if ((sendFile(receivedSocket)) < 0)
            {
                cout << "Connection with server was lost: " << endl;
                
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

			if((deleteFile(receivedSocket)) < 0)
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
