#include"../../include/networking/UdpHandlerThread.hpp"
#include"../../include/networking/NetworkException.hpp"

#include<WinSock2.h>
#include<Ws2tcpip.h>

// Used in a separate thread to listen to for ezvidia commands
void UdpHandlerThread::threadFunction() {
}

// The constructor inits WSA, creates the socket and binds it to the desired port
UdpHandlerThread::UdpHandlerThread(std::weak_ptr<EzvidiaMaster> master, unsigned short port) : masterPtr(master) {
	SOCKET s = INVALID_SOCKET;
	WSADATA wsa;
	ADDRESS_FAMILY family;

	union {
		struct sockaddr_in ipv4;
		struct sockaddr_in6 ipv6;
	} sockaddrUnion;
	size_t sockaddrUnionSize;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		throw NetworkException(L"Error initiating WinSock2.");
	}

	// Open socket
	s = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		// No IPv6
		s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (s == INVALID_SOCKET) {
			throw NetworkException(L"Error opening socket.");
		}
		family = AF_INET;
		sockaddrUnionSize = sizeof(sockaddr_in);
		sockaddrUnion.ipv4.sin_family = AF_INET;
		sockaddrUnion.ipv4.sin_port = htons(port);
		sockaddrUnion.ipv4.sin_addr.S_un.S_addr = INADDR_ANY;
	}
	else {
		char* mode = 0;
		// Dual stack socket (captures both IPv6 and IPv4)
		setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, mode, sizeof(mode));
		family = AF_INET6;
		sockaddrUnionSize = sizeof(sockaddr_in6);
		sockaddrUnion.ipv6.sin6_family = AF_INET6;
		sockaddrUnion.ipv6.sin6_port = htons(port);
		sockaddrUnion.ipv6.sin6_addr = in6addr_any;
	}

	int ret = bind(s, reinterpret_cast<sockaddr*>(&sockaddrUnion), static_cast<int>(sockaddrUnionSize));
	if (ret == SOCKET_ERROR) {
		throw NetworkException(L"Error binding to port " + std::to_wstring(port) + L".");
	}

	this->serverSocket = s;
	this->family = family;
	this->serverThreadPtr = std::make_unique<std::thread>(&UdpHandlerThread::threadFunction, this);
}

// Close the socket, join the listening thread and WSACleanup
UdpHandlerThread::~UdpHandlerThread() {
	closesocket(this->serverSocket);
	this->serverThreadPtr.get()->join();
	WSACleanup();
}