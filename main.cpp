#include <iostream>
#include <thread>

#include "FtpServer.h"

int main() {
    std::cout << "Type \"exit\" to stop the program" << std::endl << std::endl;

    auto server = FtpServer();
    if (server.createSocket())
        return 1;
    if (server.bindSocket())
        return 2;
    if (server.listenSocket())
        return 3;

    std::thread receiveFilesThread(&FtpServer::receiveFiles, &server);
    std::thread turnSwitchOffThread(&FtpServer::turnSwitchOff, &server);

    receiveFilesThread.join();
    turnSwitchOffThread.join();

    return 0;
}
