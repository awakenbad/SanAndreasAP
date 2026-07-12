#include "APSocket.h"

#include <CMessages.h>

bool APSocket::connectToServer(const std::string& ip, int port)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
	{
		return false;
	}
	
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		WSACleanup();
		return false;
	}

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

	if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) 
	{
		closesocket(sock);
		WSACleanup();
		connected = false;
		return false;
	}
	
	recvThread = std::thread(&APSocket::recvLoop, this);
	recvThread.detach();
	connected = true;
	return true;
}

void APSocket::sendToServer(const std::string& msg)
{
	if (!connected) return;
	send(sock, msg.c_str(), (int)msg.size(), 0);
}

void APSocket::closeConnection()
{
	if (connected) 
	{
		closesocket(sock);
		WSACleanup();
		connected = false;
	}
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
			//Log("Socket received line: %s", line.c_str());
			CMessages::AddMessageJumpQ(line.c_str(), 10000, 0);
		}
	}
}