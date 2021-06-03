#include "TcpServer.hpp"
#include "../utils/StringUtils.hpp"

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
				for (auto& confName : this->config_->getAllConfigurationNames()) {
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
				//for (auto& c : configList) {
				//	if (strlen(buf + 6) >= c.name.length()) {
				//		if (strncmp(buf + 6, c.name.c_str(), c.name.length()) == 0) {
				//			logPrint("Parsed apply request for configuration " + c.name);
				//			if (!safeApplyConfig(c)) {
				//				strcpy_s(reply_buf, "OK");
				//			}
				//			break;
				//		}
				//	}
				//}
			}
			else {
				strcpy_s(reply_buf, "INVALID");
			}

			//REPLY
			//if (sendto(s, reply_buf, strlen(reply_buf), 0, (struct sockaddr*)&si_other, slen) == SOCKET_ERROR) {
			//	return;
			//}
		}
	}
}

TcpServer::TcpServer(SynchronizedConfigurationList* config, DisplayDriver* driver) :
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