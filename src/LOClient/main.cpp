#include "includes.hpp"
#include "client.hpp"

using namespace std;

int main()
{
	string ip;
	string port;

	cout << "Enter server ip: ";
	cin >> ip;
    //ip = "192.168.1.253";

	cout << "Enter server port: ";
	cin >> port;

	cout << endl;

	int receivedSocket = connectToServer(ip, port);

	if (receivedSocket < 0)  return -1;

	//Здесь начинается обратботка запросов
	if  (processRequest(receivedSocket) < 0)
	{
		cout << "Bad Exit" << endl;
	} 
	else 
	{
		cout << "Good exit" << endl;
	}
}
