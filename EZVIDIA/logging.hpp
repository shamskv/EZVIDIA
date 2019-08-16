#pragma once
#include "framework.h"

extern std::string logName;
extern std::ofstream logStream;
extern std::mutex logMutex;

void logPrint(std::string msg);