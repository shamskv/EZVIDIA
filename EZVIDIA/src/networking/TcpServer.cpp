#include "TcpServer.hpp"
#include "../utils/StringUtils.hpp"
#include <boost/algorithm/string.hpp>

void TcpServer::serverThread() {
	int ret;
	SOCKET clientSocket;
	char buf[DEFAULT_BUFLEN], reply_buf[DEFAULT_BUFLEN];

	while ((clientSocket = this->socket_.waitForClient()) != INVALID_SOCKET) {
		memset(buf, '\0', DEFAULT_BUFLEN);
		memset(reply_buf, '\0', DEFAULT_BUFLEN);

		ret = recv(clientSocket, buf, DEFAULT_BUFLEN, 0);
		if (ret > 0) {
			if (strncmp(buf, "LIST", 4) == 0) {
				std::string tmp = "";
				for (auto& confName : this->config_.getAllConfigurationNames()) {
					tmp += tmp.empty() ? "" : ";;";
					tmp += StringUtils::wideStringToString(confName);
				}
				if (tmp.length() < DEFAULT_BUFLEN) {
					strcpy_s(reply_buf, tmp.c_str());
				}
				else {
					strcpy_s(reply_buf, "RESPONSE BUFFER SIZE EXCEEDED");
				}
			}

			//No reply if trying to apply
			else if (strncmp(buf, "APPLY ", 6) == 0) {
				strcpy_s(reply_buf, "NOK");
				if (strlen(buf) > 6) {
					std::string targetConf(buf + 6);
					boost::trim(targetConf);
					std::wstring targetConfW = StringUtils::stringToWideString(targetConf);
					auto optionalConf = this->config_.getConfiguration(targetConfW);
					if (optionalConf.has_value()) {
						if (this->driver_.applyConfig(optionalConf.value())) {
							strcpy_s(reply_buf, "OK");
						}
					}
				}
			}
			else {
				strcpy_s(reply_buf, "INVALID");
			}

			//REPLY
			if (send(clientSocket, reply_buf, static_cast<int>(strlen(reply_buf)), 0) == SOCKET_ERROR) {
				// TODO log the error
			}
		}

		// Connection-less
		closesocket(clientSocket);
	}
}

TcpServer::TcpServer(SynchronizedConfigurationList& config, DisplayDriver& driver) :
	config_(config), driver_(driver), socket_(TCP_PORT) {
	if (!socket_.ready()) {
		this->state_ = ServerState::DOWN;
		return;
	}

	this->state_ = ServerState::UP;
	thread_ = std::thread(&TcpServer::serverThread, this);
}

TcpServer::~TcpServer() {
	this->socket_.close();
	if (thread_.joinable()) {
		thread_.join(); // pls don't be stuck somehow, maybe implement timeout?
	}
}