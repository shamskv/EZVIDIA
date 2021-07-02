#include "TcpSocket.hpp"
#include<Ws2tcpip.h>
#include"../logging/Logger.hpp"

TcpSocket::TcpSocket(uint16_t port) : port(port) {
	WSADATA wsaData;
	SOCKADDR_IN socketAddr = {};
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		LOG(ERR) << "WSAStartup failed. Code: " << WSAGetLastError();
		this->state = SocketState::FAIL;
		return;
	}

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		LOG(ERR) << "socket() failed. Code: " << WSAGetLastError();
		this->state = SocketState::FAIL;
		return;
	}

	socketAddr.sin_family = AF_INET;
	socketAddr.sin_port = htons(port);
	socketAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	iResult = bind(listenSocket, reinterpret_cast<sockaddr*>(&socketAddr), sizeof(socketAddr));
	if (iResult == SOCKET_ERROR) {
		LOG(ERR) << "bind() failed. Code: " << WSAGetLastError();
		this->state = SocketState::FAIL;
		return;
	}

	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		LOG(ERR) << "listen() failed. Code: " << WSAGetLastError();
		this->state = SocketState::FAIL;
		return;
	}

	LOG(DEBUG) << "TcpSocket constructed succesfully";
	this->state = SocketState::OPEN;
}

TcpSocket::~TcpSocket() {
	if (listenSocket != INVALID_SOCKET) {
		LOG(DEBUG) << "Closing TCPsocket (destructor)";
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
		LOG(DEBUG) << "Closing TCPsocket (explicit)";
		closesocket(listenSocket);
		this->state = SocketState::CLOSED;
		listenSocket = INVALID_SOCKET;
	}
}