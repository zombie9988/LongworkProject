#include "mainHeader.hpp"
#include "client.hpp"

using namespace std;

struct commandInfo
{
	string ip = "-1", port = "-1", command = "-1", path = "";
};

commandInfo processCommand(int argc, char* argv[])
{
	commandInfo info;

	for (size_t i = 0; i < argc; i++)
	{
		string arg = argv[i];
		if (arg == "-i")
		{
			info.ip = argv[i + 1];
		} 
		else if (arg == "-p")
		{
			info.port = argv[i + 1];
		}
		else if (arg == "-c")
		{
			info.command = argv[i + 1];
		}
		else if (arg == "-f")
		{
			info.path = argv[i + 1];
		}
	}

	return info;
}

int main(int argc, char* argv[])
{
	string ip;
	short port;
	commandInfo info;

	if (argc <= 1)
	{
		cout << "Enter server ip: ";
		cin >> ip;

		cout << "Enter server port: ";
		cin >> port;

		cout << endl;
	}
	else
	{
		info = processCommand(argc, argv);
		int receivedSocket = connectToServer(info.ip, stoi(info.port));
		if (receivedSocket < 0)  return -1;

		if (info.ip == "-1")
		{
			cout << "Bad IP address\n";
			return -1;
		}
		else if (info.port == "-1")
		{
			cout << "Bad port\n";
			return -1;
		}
		else if (info.command == "-1")
		{
			cout << "Bad command value\n";
			return -1;
		}
		else if (info.path == "")
		{
			cout << "Bad additional value\n";
			return -1;
		}

		return processRequest(receivedSocket, stoi(info.command), info.path);
	}

	int receivedSocket = connectToServer(ip, port);

	if (receivedSocket < 0)  return -1;

	if (processRequest(receivedSocket) < 0)
	{
		cout << "Bad Exit" << endl;
	}
	else
	{
		cout << "Good exit" << endl;
	}
}
