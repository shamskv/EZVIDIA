#pragma once
#include <WinSock2.h>
#include<stdint.h>

class TcpSocket {
private:
	SOCKET listenSocket = INVALID_SOCKET;
	enum class SocketState { OPEN, FAIL, CLOSED };
	SocketState state = SocketState::FAIL;
	uint16_t port;
public:
	TcpSocket(uint16_t);
	~TcpSocket();

	bool ready();
	SOCKET waitForClient();
	void close();

	TcpSocket(const TcpSocket&) = delete;
	TcpSocket& operator=(const TcpSocket&) = delete;
};