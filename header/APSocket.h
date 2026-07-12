#pragma once
#include <mutex>
#include <queue>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

class APSocket
{
public:
    bool connectToServer(const std::string& ip, int port);
    void sendToServer(const std::string& msg);
    void closeConnection();

    bool tryGetMessage(std::string& outMsg);

private:
    void recvLoop();

    SOCKET sock = INVALID_SOCKET;
    bool connected = false;

    std::thread recvThread;

    std::queue<std::string> incomingMessages;
    std::mutex queueMutex;
};

