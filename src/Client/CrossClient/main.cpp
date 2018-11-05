#include "mainHeader.hpp"
#include "client.hpp"

using namespace std;

int main()
{
	string ip;
	short port;

	/*cout << "Enter server ip: ";
	cin >> ip;

	cout << "Enter server port: ";
	cin >> port;

	cout << endl;*/

	ip = "127.0.0.1";
	port = PORT;

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
