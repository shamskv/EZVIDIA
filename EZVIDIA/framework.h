// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <shellapi.h>
#include <commctrl.h>
#include <strsafe.h>

#include <fstream>
#include <vector>
#include <locale>
#include <codecvt>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <json.hpp>
#include <atomic>
#include <thread>
#include <iomanip>
#include <ctime>
#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <nvapi.h>

#include "readable_config.hpp"
#include "nvapi_controller.hpp"
#include "networking.hpp"
#include "logging.hpp"
#include "ezvidia.hpp"
