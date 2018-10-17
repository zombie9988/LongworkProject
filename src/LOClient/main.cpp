#include "includes.hpp"
#include "client.hpp"

using namespace std;

int main()
{
	string ip;
	string port;

	cout << "Enter server ip: ";
	cin >> ip;

	cout << "Enter server port: ";
    cin >> port;

	cout << endl;

	int receivedSocket = connectToServer(ip, port);

	if (receivedSocket < 0)  return -1;

	if  (processRequest(receivedSocket) < 0)
	{
		cout << "Bad Exit" << endl;
	}
	else
	{
		cout << "Good exit" << endl;
	}
}
