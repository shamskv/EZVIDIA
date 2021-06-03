#include "TcpSocket.hpp"
#include<Ws2tcpip.h>

TcpSocket::TcpSocket(uint16_t port) : port(port) {
	WSADATA wsaData;
	SOCKADDR_IN socketAddr = {};
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		this->state = SocketState::FAIL;
		return;
	}

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		this->state = SocketState::FAIL;
		return;
	}

	socketAddr.sin_family = AF_INET;
	socketAddr.sin_port = htons(port);
	socketAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	iResult = bind(listenSocket, reinterpret_cast<sockaddr*>(&socketAddr), sizeof(socketAddr));
	if (iResult == SOCKET_ERROR) {
		this->state = SocketState::FAIL;
		int error = WSAGetLastError();
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

bool TcpSocket::ready() {
	return this->state == SocketState::OPEN && this->listenSocket != INVALID_SOCKET;
}

SOCKET TcpSocket::waitForClient() {
	if (this->state != SocketState::OPEN || listenSocket == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}
	return accept(listenSocket, NULL, NULL);
}

void TcpSocket::close() {
	if (listenSocket != INVALID_SOCKET) {
		closesocket(listenSocket);
		this->state = SocketState::CLOSED;
		listenSocket = INVALID_SOCKET;
	}
}