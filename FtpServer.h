#ifndef FTP_FILE_SERVER_FTPSERVER_H
#define FTP_FILE_SERVER_FTPSERVER_H

#include <string>
#include <map>
#include <vector>
#include <atomic>
#include <arpa/inet.h>
#include <poll.h>

class FtpServer {
private:
    std::atomic_bool onSwitch = true;
    std::vector<pollfd> sockets;
    std::map<int, std::pair<bool, std::string> > headers;
    std::map<int, std::pair<FILE *, std::string> > files;
    sockaddr_in serverAddress{};
    int fileCounder = 0;
    int bufforSize = 64;
    int port = 8080;

public:
    FtpServer() = default;
    FtpServer(int p_bufforSize, int p_port);
    ~FtpServer();
    void turnSwitchOff();
    int createSocket();
    int bindSocket();
    int listenSocket(int backlog = 5);
    int receiveFiles();
    std::string createFileName(sockaddr_in p_socketAddres);
    int createFile(int fileDescriptor);
    int createHeader(int &index);
    int handleNewConnections(int &index);
    void closeFileAndSocket(int &index);
    int writeDataToFile(int &index, char *buffor, ssize_t recivedBytes);
    int handleExistingUploads(int &index);
    int SendAllBytes(int &index, char *buffor, int bytesToSend);
    int handleExistingDownloads(int &index);

};


#endif //FTP_FILE_SERVER_FTPSERVER_H
