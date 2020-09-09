#pragma once
#include <basetsd.h>

class UdpHandlerThread {
private:
	UINT_PTR serverSocket;
public:
	UdpHandlerThread();
	~UdpHandlerThread();
	UdpHandlerThread(const UdpHandlerThread& other) = delete;
	UdpHandlerThread& operator=(const UdpHandlerThread& other) = delete;
};