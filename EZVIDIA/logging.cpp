#include "logging.hpp"

void logPrint(std::string msg) {
	auto t = std::time(nullptr);
	struct tm newtime;
	localtime_s(&newtime, &t);
	logMutex.lock();
	logStream << "[" << std::put_time(&newtime, "%d-%m-%Y %H:%M:%S") << "] " << msg << std::endl;
	logStream.flush();
	logMutex.unlock();
}