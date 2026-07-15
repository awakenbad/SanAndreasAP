#pragma once
#include <atomic>
#include <chrono>
#include <mutex>
#include <queue>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

class APSocket
{
public:
    ~APSocket();
    bool connectToServer(const std::string& ip, int port);
    bool sendToServer(const std::string& msg);
    void closeConnection();

    void update();

    bool tryGetMessage(std::string& outMsg);

private:
    void recvLoop();
    void connectAttemptThreadFunc(std::string ip, int port);

    SOCKET sock = INVALID_SOCKET;
    bool connected = false;
    bool connecting = false;

    std::string m_ip;
    int m_port = 0;
    std::chrono::steady_clock::time_point m_lastConnectAttempt{};
    static constexpr std::chrono::seconds RECONNECT_INTERVAL{5};

    std::thread recvThread;
    std::thread connectThread;

    std::queue<std::string> incomingMessages;
    std::mutex queueMutex;
};

