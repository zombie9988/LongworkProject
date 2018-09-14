#include "pch.h"
#include "libs.h"

int runFile(string cmd)
{
    std::cout << getStrTime() << "Doing command: " << cmd << std::endl;

    return system(cmd.c_str());
}

int writeFile(Data &data, char* fileName)
{
    ofstream outFile(fileName, ios::binary);

    outFile.write(data.bufPointer, data.len);

    cout << getStrTime() << fileName << "Was written!" << endl;

    outFile.close();

    return 1;
}

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
