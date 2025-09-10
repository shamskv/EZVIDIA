#include "TcpServer.hpp"

#include <boost/algorithm/string.hpp>

#include "../logging/Logger.hpp"
#include "../utils/StringUtils.hpp"

namespace {
int recvUntilDelimiterOrTimeout(SOCKET, char*, int, int, char);
}

void TcpServer::serverThread() {
  int ret;
  SOCKET clientSocket;
  char buf[DEFAULT_BUFLEN], reply_buf[DEFAULT_BUFLEN];
  LOG(DEBUG) << "TCP Server thread started";

  while ((clientSocket = this->socket_.waitForClient()) != INVALID_SOCKET) {
    LOG(INFO) << "New TCP connection";
    memset(buf, '\0', DEFAULT_BUFLEN);
    memset(reply_buf, '\0', DEFAULT_BUFLEN);

    ret =
        recvUntilDelimiterOrTimeout(clientSocket, buf, DEFAULT_BUFLEN, 0, '\n');
    if (ret > 0) {
      LOG(INFO) << "TCP message received: " << std::string(buf).c_str();
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
        } else {
          LOG(DEBUG) << "LIST response size is exceeds buffer";
          strcpy_s(reply_buf, "RESPONSE BUFFER SIZE EXCEEDED");
        }
      }

      // No reply if trying to apply
      else if (strncmp(buf, "APPLY ", 6) == 0) {
        LOG(DEBUG) << "TCP message interpreted as a APPLY request";
        strcpy_s(reply_buf, "NOK");
        if (strlen(buf) > 6) {
          std::string targetConf(buf + 6);
          boost::trim(targetConf);
          std::wstring targetConfW =
              StringUtils::stringToWideString(targetConf);
          LOG(DEBUG) << "APPLY request target conf name parsed as "
                     << targetConfW;
          auto optionalConf = this->config_.getConfiguration(targetConfW);
          if (optionalConf.has_value()) {
            LOG(DEBUG) << "Target conf name was succesfully found ";
            if (this->driver_.applyConfig(optionalConf.value())) {
              strcpy_s(reply_buf, "OK");
            }
          }
        }
      } else {
        LOG(ERR) << "TCP message unrecognized";
        strcpy_s(reply_buf, "INVALID");
      }

      // REPLY
      LOG(DEBUG) << "Replying to TCP client with message: "
                 << std::string(reply_buf).c_str();
      if (send(clientSocket, reply_buf, static_cast<int>(strlen(reply_buf)),
               0) == SOCKET_ERROR) {
        LOG(ERR) << "Error replying to TCP client. Code: " << WSAGetLastError();
      }
    } else {
      LOG(ERR) << "Unable to receive TCP message. Custom recv returned: "
               << ret;
    }

    // Connection-less
    closesocket(clientSocket);
  }
  LOG(DEBUG) << "TCP Server thread finished";
}

TcpServer::TcpServer(Settings& config, DisplayDriver& driver)
    : config_(config), driver_(driver), socket_(TCP_PORT) {
  LOG(INFO) << "Starting TCP server...";
  if (!socket_.ready()) {
    LOG(DEBUG) << "TCP Server failed to start (socket not ready)";
    this->state_ = ServerState::DOWN;
    return;
  }

  this->state_ = ServerState::UP;
  thread_ = std::thread(&TcpServer::serverThread, this);
}

TcpServer::~TcpServer() {
  LOG(DEBUG) << "Shutting down TCP server...";
  this->socket_.close();
  if (thread_.joinable()) {
    thread_.join();  // pls don't be stuck somehow, maybe implement timeout?
  }
  LOG(INFO) << "TCP Server shut down";
}

bool TcpServer::up() {
  return this->state_ == ServerState::UP && this->socket_.ready() &&
         this->thread_.joinable();
}

namespace {
int recvUntilDelimiterOrTimeout(SOCKET socket, char* buffer, int bufferSize,
                                int flags, char delimiter) {
  int byteCount = 0, readCount = 0;
  do {
    WSAPOLLFD pollfd = {};
    pollfd.fd = socket;
    pollfd.events = POLLIN;

    LOG(DEBUG) << "Entering Poll with " << byteCount << " and " << readCount
               << " readCount";
    int ret = WSAPoll(&pollfd, 1, 10000);
    if (ret < 0) {
      LOG(DEBUG) << "WSAPoll returned less than zero. Code: "
                 << WSAGetLastError();
      return -1;
    } else if (ret == 0) {
      LOG(DEBUG) << "WSAPoll returned zero";
      return -2;
    } else {
      LOG(DEBUG) << "WSAPoll returned greater than zero";
      int n = recv(socket, buffer + byteCount, bufferSize - byteCount, flags);
      if (n > 0) {
        for (int i = byteCount; i < byteCount + n && i < bufferSize; i++) {
          if (buffer[i] == delimiter) {
            LOG(DEBUG) << "Found delimiter at position " << i;
            buffer[i] = '\0';
            return i;
          }
        }
        LOG(DEBUG) << "Successful recv but no delimiter";
        byteCount += n;
        readCount++;
      } else if (n == 0) {
        LOG(DEBUG) << "recv returned zero";
        return n;
      } else {
        LOG(DEBUG) << "recv returned less than zero. Code: "
                   << WSAGetLastError();
      }
    }
  } while (byteCount < bufferSize && readCount < 10);

  LOG(DEBUG) << "Exceeded number of recvs from TCP connection. readCount "
             << readCount << " byteCount " << byteCount;
  return -3;
}
}  // namespace