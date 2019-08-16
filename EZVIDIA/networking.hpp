#pragma once
#include "framework.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 48541

extern SOCKET globalSocket;

BOOL startUDPSocket();
void HandleUDPSocket(SOCKET s);