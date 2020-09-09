#include "..\include\networking\UdpHandlerThread.hpp"
#include "..\include\networking\NetworkException.hpp"

#include<WinSock2.h>
#include<Ws2tcpip.h>

UdpHandlerThread::UdpHandlerThread() {
	SOCKET s = INVALID_SOCKET;
	WSADATA wsa;
	ADDRESS_FAMILY family;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		throw NetworkException(L"Error initiating WinSock2.");
	}

	// Open socket
	s = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		// No IPv6
		s = socket(family, SOCK_DGRAM, IPPROTO_UDP);
		if (s == INVALID_SOCKET) {
			throw NetworkException(L"Error opening socket.");
		}
		family = AF_INET;
	}
	else {
		int mode = 0;
		setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, (char*)mode, sizeof(mode));
		family = AF_INET6;
	}

	SOCKADDR_STORAGE sockaddr;
}

UdpHandlerThread::~UdpHandlerThread() {
}