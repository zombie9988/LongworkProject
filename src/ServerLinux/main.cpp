#include "header.hpp"
#include "runFile.hpp"


//функция, которая принимает все отправленные байты.
char* receiveAll(int receivedSocket)
{
    //Сначала инициализируем бфер на 1024 байта
    //Это буфер, в который мы будем передавать размер передаваемого пакета
    //то есть по омему представлению мы сначала говорим сколько байт мы будем передавать
    //Для этого и нужен первый receive
    char buf[1024];

    //В received записывается, сколько байт он принял, сами же байты
    //идут в буфер
    int received = recv(receivedSocket, buf, sizeof(char)*1024, 0);

    //Так как соединение не особо стабильное, нужно удостовериться
    //Что все байты переданы
    while(received != sizeof(char)*1024)
    {
        received += recv(receivedSocket, buf + received, sizeof(char)*1024 - received, 0);
    }

    //Из полученных данных, узнаем какой у нас размер данных
    int dataBufSize = atoi(buf) + 1;

    //Под данные выделяем необходимое число байтов
    char* dataBuf = new char[dataBufSize];

    //Получаем первую пачку данных
    received = recv(receivedSocket, dataBuf, sizeof(char)*dataBufSize, 0);

    //Получаем все остальное, если осталось
    while(received != dataBufSize*sizeof(char))
    {
        received += recv(receivedSocket, dataBuf + received, sizeof(char)*dataBufSize - received, 0);
    }

    //Возвращаем данные в виде набора байтов
    return dataBuf;
}

//Функция по конвертации времени в крсивый формат, ничего интересного
string getStrTime()
{
    time_t seconds = time(NULL);
    tm* timeinfo = localtime(&seconds);
    char buffer[80];
    const char* format = "[%H:%M:%S] ";

    strftime(buffer, 80, format, timeinfo);

    string time = buffer;

    return time;
}

//Функция, стартующая сервер
void startServer(const char* ip)
{
    int socketServer = socket(AF_INET, SOCK_STREAM, 0);

    if (socketServer < 0)
    {
        cout << "Socket start error!" << endl;
        return;
    }

    sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(1337);
    addr.sin_addr.s_addr = inet_addr(ip);

    if(bind(socketServer, (sockaddr*)&addr, sizeof(addr)))
    {
        cout << "Bind fail!" << strerror(errno) << endl;
        close(socketServer);
        return;
    }

    if(listen(socketServer, 0x100))
    {
        cout << "listen fail!" << strerror(errno) << endl;
        close(socketServer);
        return;
    }

    char* cmd;
    bool stopServer = false;
    int socketClient;
    cout << "Waiting for connect..." << endl;

    while(!stopServer){

        sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);

        if(listen(socketServer, 0x100))
        {
            cout << "Listen failed!" << strerror(errno) << endl;
            return;
        }

        bool doListen = true;
        hostent* host;

        while(doListen)
        {
            socketClient = accept(socketServer, (sockaddr*)&clientAddr, &clientAddrSize);

            if(socketClient >= 0)
            {
                host = gethostbyaddr((char*)&clientAddr.sin_addr.s_addr, 4, AF_INET);
                cout << getStrTime() << "User: " << host->h_name << " connected successfully!" << endl;
                doListen = false;
            }
        }

        char jobIdentifier = '0';
        char* buf = new char[1024];
        int receiveArg = -1;

        //Вот с этого момента мы начинаем прослушивать сокет, с которого идет запрос
        while (true)
        {
            //Пока мы приняли меньше чем 0 байт мы продолжаем слушать
            while(receiveArg < 0)
            {
                receiveArg = recv(socketClient, buf, sizeof(char), 0);
            }

            //Обработка ошибки, на случай, если отключится пользователь
            if (receiveArg == 0)
            {
                cout << getStrTime() << "User: " << host->h_name << " disconnected!" << endl;
                break;
            }

            //Получаем идентификатор необходимого действия
            jobIdentifier = buf[0];
            buf[0] = '0';
            receiveArg = -1;
            string command;

            //В зависимости от него, смотрим, какую задачу выполнять
            switch (jobIdentifier)
            {
            case '1':
                cout << "Launh file!" << endl;
                cmd = receiveAll(socketClient);
                command = cmd;
                runFile(command);
                delete cmd;
                jobIdentifier = '0';
                break;
            case '2':
                cout << "Succeed second identy!" << endl;
                jobIdentifier = '0';
                break;
            case '3':
                cout << "Succeed third identy!" << endl;
                jobIdentifier = '0';
                break;
            case '4':
                cout << "Succeed fourth identy!" << endl;
                jobIdentifier = '0';
                break;
            }

            if (jobIdentifier == '5')
            {
                cout << getStrTime() << "User: " << host->h_name << " disconnected!" << endl;
                break;
            }
        }
    }
    close(socketServer);
    close(socketClient);
    return;
}

int main()
{
    string ip;

    cout << "Enter IP address: ";
    cin >> ip;
    startServer(ip.c_str());

    return 0;
}
