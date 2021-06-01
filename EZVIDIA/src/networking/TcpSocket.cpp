#include "TcpSocket.hpp"
#include<Ws2tcpip.h>

TcpSocket::TcpSocket(uint32_t port) {
	WSADATA wsaData;
	struct sockaddr_in6 socketAddr;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		this->state = SocketState::FAIL;
		return;
	}

	listenSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		this->state = SocketState::FAIL;
		return;
	}

	// Dual stack socket
	char* mode = 0;
	setsockopt(listenSocket, IPPROTO_IPV6, IPV6_V6ONLY, mode, sizeof(mode));
	socketAddr.sin6_family = AF_INET6;
	socketAddr.sin6_port = htons(port);
	socketAddr.sin6_addr = in6addr_any;

	iResult = bind(listenSocket, reinterpret_cast<sockaddr*>(&socketAddr), sizeof(socketAddr));
	if (iResult == SOCKET_ERROR) {
		this->state = SocketState::FAIL;
		return;
	}

	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		this->state = SocketState::FAIL;
		return;
	}

	this->state = SocketState::OPEN;
}

TcpSocket::~TcpSocket() {
	if (listenSocket != INVALID_SOCKET) {
		closesocket(listenSocket);
	}
	WSACleanup();
}

SOCKET TcpSocket::waitForClient() {
	if (this->state != SocketState::OPEN || listenSocket == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}
	accept(listenSocket, NULL, NULL);
}