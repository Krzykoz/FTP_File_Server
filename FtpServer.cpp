#include "FtpServer.h"

#include <unistd.h> // read(), write(), close()
#include <arpa/inet.h>
#include <poll.h>
#include <atomic>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

FtpServer::FtpServer(int p_bufforSize, int p_port) : bufforSize(p_bufforSize), port(p_port) {}

FtpServer::~FtpServer() {
    close(sockets.front().fd);
    sockets.erase(sockets.begin());
    for (int index = 0; index < sockets.size(); index++) {
        std::remove(files.at(sockets[index].fd).second.c_str());
        closeFileAndSocket(index);
    }
}

void FtpServer::turnSwitchOff() {
    while (true) {
        std::string consoleInput;
        std::cin >> consoleInput;
        if (consoleInput == "exit") {
            onSwitch = false;
            break;
        }
    }
}

int FtpServer::createSocket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "Socket creation failed";
        return 1;
    } 

    std::cout << "Socket successfully created"
              << std::endl;
    pollfd poll = {sockfd, POLLIN};
    sockets.push_back(poll);
    return 0;
}

int FtpServer::bindSocket() {
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port);

    if ((bind(sockets.front().fd, (sockaddr *) &serverAddress, sizeof(serverAddress))) != 0) {
        std::cerr << "Socket bind failed";
        return 1;
    }

    std::cout << "Socket successfully binded" << std::endl;
    return 0;
}

int FtpServer::listenSocket(int backlog) {
    if ((listen(sockets.front().fd, backlog)) != 0) {
        std::cerr << "Listen failed";
        return 1;
    }
    std::cout << "Server listening" << std::endl;
    return 0;
}

std::string FtpServer::createFileName(sockaddr_in p_socketAddres) {
    char ipAddress[INET6_ADDRSTRLEN];

    inet_ntop(serverAddress.sin_family, (sockaddr *) &p_socketAddres, ipAddress, sizeof(ipAddress));

    std::stringstream fileName;
    fileName << ipAddress << "_" << fileCounder << ".tmp";
    fileCounder++;

    return fileName.str();
}

int FtpServer::createFile(int fileDescriptor) {
    std::string fileName = this->createFileName(serverAddress);
    FILE *file = fopen(fileName.c_str(), "wb");
    if (file == nullptr) {
        close(fileDescriptor);
        fclose(file);
        std::cerr << "File creation failed";
        return 1;
    }
    std::cout << "Created File: " << fileName << std::endl;
    files.insert(std::pair<int, std::pair<FILE *, std::string> >(fileDescriptor,
                                                                 std::pair<FILE *, std::string>(file, fileName)));

    return 0;
}

int FtpServer::handleNewConnections(int &index) {
    socklen_t socketLenght = sizeof(serverAddress);
    auto connfd = accept(sockets[index].fd, (sockaddr *) &serverAddress, &socketLenght);
    if (connfd < 0) {
        std::cerr << "Server accept failed";
        return 1;
    }
    std::cout << "Server accepted the client" << std::endl;

    pollfd poll = {connfd, POLLIN};
    sockets.push_back(poll);

    if (this->createFile(connfd)) return 1;

    return 0;
}

void FtpServer::closeFileAndSocket(int &index) {
    close(sockets[index].fd);
    fclose(files.at(sockets[index].fd).first);
    files.erase(sockets[index].fd);
    sockets.erase(sockets.begin() + index);
    index--;

}

int FtpServer::writeDataToFile(int &index, char *buffor, ssize_t recivedBytes) {
    if (fwrite(buffor, 1, recivedBytes, files.at(sockets[index].fd).first) <= 0) {
        std::remove(files.at(sockets[index].fd).second.c_str());
        closeFileAndSocket(index);
        std::cerr << "File write failed";
        return 1;
    }

    return 0;
}

int FtpServer::handleExistingConnections(int &index) {
    char buffor[bufforSize];

    ssize_t recivedBytes = recv(sockets[index].fd, buffor, bufforSize, 0);

    if (recivedBytes == -1) {
        std::remove(files.at(sockets[index].fd).second.c_str());
        closeFileAndSocket(index);
        std::cerr << "Socket failed";
        return 1;
    } else if (recivedBytes == 0) // koniec pliku
    {
        closeFileAndSocket(index);
        std::cout << "File transfer succesfull" << std::endl;
        return 0;
    }

    if (writeDataToFile(index, buffor, recivedBytes))
        return 1;

    return 0;
}

int FtpServer::receiveFiles() {
    while (onSwitch) {
        if (poll(&sockets.front(), sockets.size(), 0)) {
            for (int index = 0; index < sockets.size(); index++)
                if (sockets[index].revents & POLLIN) {
                    if (sockets[index].fd == sockets.front().fd) {
                        if (this->handleNewConnections(index))
                            return 1;
                    } else {
                        if (this->handleExistingConnections(index))
                            return 1;
                    }
                }
        }
    }

    std::cout << "Program turned off manualy" << std::endl;
    return 0;
}
