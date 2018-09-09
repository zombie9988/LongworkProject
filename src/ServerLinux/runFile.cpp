#include "runFile.hpp"

int runFile(string cmd)
{
    cout << getStrTime() << "Doing command: " << cmd << endl;

    return system(cmd.c_str());
}
