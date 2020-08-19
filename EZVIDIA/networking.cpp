//This file is part of EZVIDIA.
//
//EZVIDIA is free software : you can redistribute itand /or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//EZVIDIA is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with EZVIDIA. If not, see < https://www.gnu.org/licenses/>.

#include "networking.hpp"

BOOL startUDPSocket()
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[DEFAULT_BUFLEN], reply_buf[DEFAULT_BUFLEN];
	WSADATA wsa;

	//Initialise winsock
	//printf("\nInitialising Winsock...");
	logPrint("Initializing Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		logPrint("(ERROR) Failed to initialize Winsock. Code: " + std::to_string(WSAGetLastError()));
		return false;
	}

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		logPrint("(ERROR) Failed to create socket. Code: " + std::to_string(WSAGetLastError()));
		return false;
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(DEFAULT_PORT);

	//Bind
	if (bind(s, (struct sockaddr*) & server, sizeof(server)) == SOCKET_ERROR)
	{
		logPrint("(ERROR) Failed to bind scoket. Code: " + std::to_string(WSAGetLastError()));
		return false;
	}

	globalSocket = s;
	return true;
}

void HandleUDPSocket(SOCKET s) {

	struct sockaddr_in si_other;
	int slen, recv_len;
	char buf[DEFAULT_BUFLEN], reply_buf[DEFAULT_BUFLEN];

	slen = sizeof(si_other);

	while (1) {

		//Clean buffers
		memset(buf, '\0', DEFAULT_BUFLEN);
		memset(reply_buf, '\0', DEFAULT_BUFLEN);

		//Get UDP message
		if ((recv_len = recvfrom(s, buf, DEFAULT_BUFLEN - 1, 0, (struct sockaddr*) & si_other, &slen)) == SOCKET_ERROR)
		{
			logPrint("(ERROR) Socket error while waiting to receive message (this occurs on shutdown).");
			return;
		}
		logPrint("UDP message received: " + std::string(buf));

		//Parse UDP Message
		if (strncmp(buf, "LIST", 4) == 0) {
			std::string tmp = "";
			{
				std::lock_guard<std::mutex> lock(confMutex);
				for (auto& c : configList) {
					if (&c != &configList[0]) {
						tmp += ";;";
					}
					tmp += c.name;
				}
			}
			// If all the configurations give a list bigger than 512 chars then you gotta rename them
			if (tmp.length() >= DEFAULT_BUFLEN) {
				continue;
			}
			strcpy_s(reply_buf, tmp.c_str());
		}

		//No reply if trying to apply 
		else if (strncmp(buf, "APPLY ", 6) == 0) {
			if (configLock) {
				continue; //Ignore if recent config change
			}
			{
				std::lock_guard<std::mutex> lock(confMutex);
				strcpy_s(reply_buf, "NOK");
				for (auto& c : configList) {
					if (strlen(buf + 6) >= c.name.length()) {
						if (strncmp(buf + 6, c.name.c_str(), c.name.length()) == 0) {
							logPrint("Parsed apply request for configuration " + c.name);
							if (!safeApplyConfig(c)) {
								strcpy_s(reply_buf, "OK");
							}
							break;
						}
					}
				}
			}
		}
		else {
			strcpy_s(reply_buf, "INVALID");
		}

		//REPLY 
		if (sendto(s, reply_buf, strlen(reply_buf), 0, (struct sockaddr*) & si_other, slen) == SOCKET_ERROR)
		{
			logPrint("(ERROR) Socket error while trying to send message.");
			return;
		}
		logPrint("UDP message sent: " + std::string(reply_buf));
	}
}