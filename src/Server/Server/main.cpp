#include "server.hpp"
#include "mainHeader.hpp"

using namespace std;

int main()
{
	string ip;

	//cout << "Enter IP address: ";
	//cin >> ip;
	ip = "127.0.0.1";

	int socketServer = startServer(ip.c_str());

	if (socketServer > 0)
	{
		while (startListenSocket(socketServer) >= 0);
	}
	else
	{
		return -1;
	}

	return 0;
}
