#include "TcpServer.hpp"
#include "../utils/StringUtils.hpp"
#include <boost/algorithm/string.hpp>
#include "../logging/Logger.hpp"

namespace {
	int recvUntilDelimiterOrTimeout(SOCKET, char*, int, int, char);
}

void TcpServer::serverThread() {
	int ret;
	SOCKET clientSocket;
	char buf[DEFAULT_BUFLEN], reply_buf[DEFAULT_BUFLEN];

	while ((clientSocket = this->socket_.waitForClient()) != INVALID_SOCKET) {
		LOG(INFO) << "New TCP connection";
		memset(buf, '\0', DEFAULT_BUFLEN);
		memset(reply_buf, '\0', DEFAULT_BUFLEN);

		ret = recvUntilDelimiterOrTimeout(clientSocket, buf, DEFAULT_BUFLEN, 0, '\n');
		if (ret > 0) {
			LOG(INFO) << "Message received: " << std::string(buf);
			if (strncmp(buf, "LIST", 4) == 0) {
				LOG(DEBUG) << "TCP message interpreted as a LIST request";
				std::string tmp = "";
				for (auto& confName : this->config_.getAllConfigurationNames()) {
					tmp += tmp.empty() ? "" : ";";
					tmp += StringUtils::wideStringToString(confName);
				}
				if (tmp.length() < DEFAULT_BUFLEN) {
					LOG(DEBUG) << "LIST response size is valid";
					strcpy_s(reply_buf, tmp.c_str());
				}
				else {
					LOG(DEBUG) << "LIST response size is exceeds buffer";
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

TcpServer::TcpServer(Settings& config, DisplayDriver& driver) :
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

bool TcpServer::up() {
	return this->state_ == ServerState::UP && this->socket_.ready() && this->thread_.joinable();
}

namespace {
	int recvUntilDelimiterOrTimeout(SOCKET socket, char* buffer, int bufferSize, int flags, char delimiter) {
		int byteCount = 0, readCount = 0;
		do {
			WSAPOLLFD pollfd = {};
			pollfd.fd = socket;
			pollfd.events = POLLIN;

			//wchar_t text_buffer[500] = { 0 };
			//swprintf(text_buffer, _countof(text_buffer), L"Entering Poll with %d byteCount and %d readCount.\n", byteCount, readCount);
			//OutputDebugString(text_buffer);
			LOG(DEBUG) << "Entering Poll with " << byteCount << " and " << readCount << " readCount";
			int ret = WSAPoll(&pollfd, 1, 10000);
			if (ret < 0) {
				/*OutputDebugString(L"WSAPoll returned less than zero\n");*/
				LOG(DEBUG) << "WSAPoll returned less than zero";
				return -1;
			}
			else if (ret == 0) {
				//OutputDebugString(L"WSAPoll returned zero\n");
				LOG(DEBUG) << "WSAPoll returned zero";
				return -2;
			}
			else {
				//OutputDebugString(L"WSAPoll returned greater than zero\n");
				LOG(DEBUG) << "WSAPoll returned greater than zero";
				int n = recv(socket, buffer + byteCount, bufferSize - byteCount, flags);
				if (n > 0) {
					for (int i = byteCount; i < byteCount + n && i < bufferSize; i++) {
						if (buffer[i] == delimiter) {
							/*OutputDebugString(L"Found delimiter\n");*/
							LOG(DEBUG) << "Found delimiter at position " << i;
							buffer[i] = '\0';
							return i;
						}
					}
					//OutputDebugString(L"Successful read but no delimiter\n");
					LOG(DEBUG) << "Successful read but no delimiter";
					byteCount += n;
					readCount++;
				}
				else {
					//OutputDebugString(L"recv returned zero or less\n");
					LOG(DEBUG) << "recv returned zero or less";
					return n;
				}
			}
		} while (byteCount < bufferSize && readCount < 10);

		LOG(DEBUG) << "Exceeded number of recvs from TCP connection. readCount " << readCount << " byteCount " << byteCount;
		return -3;
	}
}