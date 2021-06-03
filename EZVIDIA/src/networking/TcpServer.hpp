#pragma once
#include "TcpSocket.hpp"
#include "../configurations/SynchronizedConfigurationList.hpp"
#include "../drivers/DisplayDriver.hpp"
#include <thread>
#include <atomic>

#define TCP_PORT 48541
#define DEFAULT_BUFLEN 1024

class TcpServer {
private:
	TcpSocket socket_;
	std::thread thread_;

	SynchronizedConfigurationList& config_;
	DisplayDriver& driver_;

	enum class ServerState { UP, DOWN, INIT };
	std::atomic<ServerState> state_ = ServerState::INIT;

	void serverThread();
public:
	TcpServer(SynchronizedConfigurationList&, DisplayDriver&);
	~TcpServer();

	TcpServer(const TcpServer&) = delete;
	TcpServer& operator=(const TcpServer&) = delete;
};