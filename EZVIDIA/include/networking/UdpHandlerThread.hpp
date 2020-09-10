#pragma once
#include "../EzvidiaMaster.hpp"
#include <basetsd.h>
#include <thread>
#include <memory>

class UdpHandlerThread {
private:
	UINT_PTR serverSocket;
	unsigned short family;
	std::unique_ptr<std::thread> serverThreadPtr;
	std::weak_ptr<EzvidiaMaster> masterPtr;
	void threadFunction();
public:
	UdpHandlerThread(std::weak_ptr<EzvidiaMaster> master, unsigned short port);
	~UdpHandlerThread();
	UdpHandlerThread(const UdpHandlerThread& other) = delete;
	UdpHandlerThread& operator=(const UdpHandlerThread& other) = delete;
};