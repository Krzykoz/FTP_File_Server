
#include <unistd.h> // read(), write(), close()
#include <arpa/inet.h>
#include <poll.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>

#include <thread>
#include <atomic>

class FtpServer
{
private:
    std::vector<pollfd> sockets;
    std::map<int, std::pair<FILE *, std::string> > files;
    sockaddr_in serverAddress;
    int fileCounder = 0;
    int bufforSize = 64;
    int port = 8080;

public:
    FtpServer() = default;
    FtpServer(int p_bufforSize, int p_port);
    ~FtpServer();
    int createSocket();
    int bindSocket();
    int listenSocket(int backlog);
    int receiveFiles(std::atomic_bool &onSwitch);
    std::string createFileName(sockaddr_in p_socketAddres);
    int createFile(int fileDescriptor, sockaddr_in p_socketAddres);
    int handleNewConnections(int &index);
    void closeFileAndSocket(int &index);
    int writeDataToFile(int &index, char *buffor, int recivedBytes);
    int handleExistingConnections(int &index);
};

FtpServer::FtpServer(int p_bufforSize, int p_port) : bufforSize(p_bufforSize), port(p_port) {}

FtpServer::~FtpServer()
{
    close(sockets.front().fd);
    sockets.erase(sockets.begin());
    for (int index = 0; index < sockets.size(); index++){
        std::remove(files.at(sockets[index].fd).second.c_str());
        closeFileAndSocket(index);
    }
}

int FtpServer::createSocket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        std::cerr << "Socket creation failed";
        return 1;
    }
    else
    {
        std::cout << "Socket successfully created"
                  << std::endl;
        pollfd poll = {sockfd, POLLIN};
        sockets.push_back(poll);
        return 0;
    }
}

int FtpServer::bindSocket()
{
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port);
    if ((bind(sockets.front().fd, (sockaddr *)&serverAddress, sizeof(serverAddress))) != 0)
    {
        std::cerr << "Socket bind failed";
        return 1;
    }
    else
        std::cout << "Socket successfully binded" << std::endl;
    return 0;
}

int FtpServer::listenSocket(int backlog = 5)
{
    if ((listen(sockets.front().fd, backlog)) != 0)
    {
        std::cerr << "Listen failed";
        return 1;
    }
    else
    {
        std::cout << "Server listening" << std::endl;
        return 0;
    }
}

std::string FtpServer::createFileName(sockaddr_in p_socketAddres)
{
    char ipAddress[INET6_ADDRSTRLEN];

    inet_ntop(serverAddress.sin_family, (sockaddr *)&p_socketAddres, ipAddress, sizeof(ipAddress));

    std::stringstream fileName;
    fileName << ipAddress << "_" << fileCounder << ".tmp";
    fileCounder++;

    return fileName.str();
}

int FtpServer::createFile(int fileDescriptor, sockaddr_in p_socketAddres)
{
    std::string fileName = this->createFileName(serverAddress);
    FILE *file = fopen(fileName.c_str(), "wb");
    if (file == 0)
    {
        close(fileDescriptor);
        fclose(file);
        std::cerr << "File creation failed";
        return 1;
    }
    std::cout << "Created File: " << fileName << std::endl;
    files.insert(std::pair<int, std::pair<FILE *, std::string> >(fileDescriptor, std::pair<FILE *, std::string>(file, fileName)));

    return 0;
}

int FtpServer::handleNewConnections(int &index)
{

    socklen_t socketLenght = sizeof(serverAddress);
    auto connfd = accept(sockets[index].fd, (sockaddr *)&serverAddress, &socketLenght);
    if (connfd < 0)
    {
        std::cerr << "Server accept failed";
        return 1;
    }
    else
    {
        std::cout << "Server accepted the client" << std::endl;

        pollfd poll = {connfd, POLLIN};
        sockets.push_back(poll);

        if (this->createFile(connfd, serverAddress))
            return 1;
    }

    return 0;
}

void FtpServer::closeFileAndSocket(int &index)
{
    close(sockets[index].fd);
    fclose(files.at(sockets[index].fd).first);
    files.erase(sockets[index].fd);
    sockets.erase(sockets.begin() + index);
    index--;

}

int FtpServer::writeDataToFile(int &index, char *buffor, int recivedBytes)
{
    if (fwrite(buffor, 1, bufforSize, files.at(sockets[index].fd).first) <= 0)
    {
        std::remove(files.at(sockets[index].fd).second.c_str());
        closeFileAndSocket(index);
        std::cerr << "File write failed";
        return 1;
    }

    return 0;
}

int FtpServer::handleExistingConnections(int &index)
{
    char buffor[bufforSize];

    int recivedBytes = recv(sockets[index].fd, buffor, bufforSize, 0);

    if (recivedBytes == -1)
    {
        std::remove(files.at(sockets[index].fd).second.c_str());
        closeFileAndSocket(index);
        std::cerr << "Socket failed";
        return 1;
    }
    else if (recivedBytes == 0) // koniec pliku
    {
        closeFileAndSocket(index);
        std::cout << "File transfer succesfull" << std::endl;
        return 0;
    }

    if (writeDataToFile(index, buffor, recivedBytes))
        return 1;

    return 0;
}

int FtpServer::receiveFiles(std::atomic_bool &onSwitch)
{
    while (onSwitch)
    {
        if (poll(&sockets.front(), sockets.size(), 0))
        {
            for (int index = 0; index < sockets.size(); index++)
                if (sockets[index].revents & POLLIN)
                {
                    if (sockets[index].fd == sockets.front().fd)
                    {
                        if (this->handleNewConnections(index))
                            return 1;
                    }
                    else
                    {
                        if (this->handleExistingConnections(index))
                            return 1;
                    }
                }
        }
    }

    std::cout << "Program turned off manualy" << std::endl;
    return 0;
}

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
