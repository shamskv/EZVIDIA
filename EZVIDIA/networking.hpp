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

#pragma once
#include "framework.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 48541

extern SOCKET globalSocket;

BOOL startUDPSocket();
void HandleUDPSocket(SOCKET s);