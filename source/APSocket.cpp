#include "APSocket.h"

APSocket::~APSocket()
{
	closeConnection();
}

bool APSocket::connectToServer(const std::string& ip, int port)
{
	m_ip = ip;
	m_port = port;
	m_lastConnectAttempt = std::chrono::steady_clock::now();

	if (connected || connecting) return false;

	if (sock != INVALID_SOCKET || recvThread.joinable() || connectThread.joinable())
	{
		closeConnection();
	}

	connecting = true;
	connectThread = std::thread(&APSocket::connectAttemptThreadFunc, this, ip, port);
	return true;
}

void APSocket::connectAttemptThreadFunc(std::string ip, int port)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		connecting = false;
		return;
	}

	SOCKET newSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (newSock == INVALID_SOCKET)
	{
		WSACleanup();
		connecting = false;
		return;
	}

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

	if (connect(newSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		closesocket(newSock);
		WSACleanup();
		connecting = false;
		return;
	}

	sock = newSock;
	connected = true;
	connecting = false;
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		incomingMessages.push("STATUS:connection restored");
	}
	recvThread = std::thread(&APSocket::recvLoop, this);
}

void APSocket::update()
{
	if (connected || connecting) return;

	auto now = std::chrono::steady_clock::now();
	if (now - m_lastConnectAttempt < RECONNECT_INTERVAL) return;

	connectToServer(m_ip, m_port);
}

bool APSocket::sendToServer(const std::string& msg)
{
	if (!connected) return false;
	return send(sock, msg.c_str(), (int)msg.size(), 0) != SOCKET_ERROR;
}

void APSocket::closeConnection()
{
	if (connectThread.joinable()) {
		connectThread.join();
	}

	if (sock != INVALID_SOCKET) {
		shutdown(sock, SD_BOTH);
		closesocket(sock);
		sock = INVALID_SOCKET;
	}

	if (recvThread.joinable()) {
		recvThread.join();
	}

	connected = false;
	WSACleanup();
}

bool APSocket::tryGetMessage(std::string& outMsg)
{
	std::lock_guard<std::mutex> lock(queueMutex);
	if (incomingMessages.empty()) return false;
	outMsg = incomingMessages.front();
	incomingMessages.pop();
	return true;
}

void APSocket::recvLoop()
{
	char buffer[512];
	std::string leftover;

	while (connected) {
		int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
		if (bytesReceived <= 0) {
			connected = false;
			{
				std::lock_guard<std::mutex> lock(queueMutex);
				incomingMessages.push("STATUS:connection lost, will retry");
			}
			break;
		}

		buffer[bytesReceived] = '\0';
		leftover += buffer;

		size_t pos;
		while ((pos = leftover.find('\n')) != std::string::npos) {
			std::string line = leftover.substr(0, pos);
			leftover.erase(0, pos + 1);

			std::lock_guard<std::mutex> lock(queueMutex);
			incomingMessages.push(line);
		}
	}
}
