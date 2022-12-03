#include <iostream>
#include <thread>
#include <atomic>

#include "FtpServer.hpp"

void turnSwitchOff(std::atomic_bool &onSwitch)
{
    while (true)
    {
        std::string consoleInput;
        std::cin >> consoleInput;
        if (consoleInput == "exit")
        {
            onSwitch = false;
            break;
        }
    }
}

int main()
{
    std::atomic_bool onSwitch{true};

    std::cout << "Type \"exit\" to stop the program" << std::endl
              << std::endl;

    auto server = FtpServer();
    if (server.createSocket())
        return 1;
    if (server.bindSocket())
        return 2;
    if (server.listenSocket())
        return 3;

    std::thread receiveFilesThread(&FtpServer::receiveFiles, &server, std::ref(onSwitch));
    std::thread turnSwitchOffThread(&turnSwitchOff, std::ref(onSwitch));

    receiveFilesThread.join();
    turnSwitchOffThread.join();

    return 0;
}
