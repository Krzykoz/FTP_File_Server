#include <string>
#include <map>
#include <vector>
#include <atomic>
#include <arpa/inet.h>
#include <poll.h>


class FtpServer
{
private:
    std::atomic_bool onSwitch = true;
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
    void turnSwitchOff();
    int createSocket();
    int bindSocket();
    int listenSocket(int backlog = 5);
    int receiveFiles();
    std::string createFileName(sockaddr_in p_socketAddres);
    int createFile(int fileDescriptor, sockaddr_in p_socketAddres);
    int handleNewConnections(int &index);
    void closeFileAndSocket(int &index);
    int writeDataToFile(int &index, char *buffor, int recivedBytes);
    int handleExistingConnections(int &index);
};
