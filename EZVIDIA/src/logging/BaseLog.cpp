#include "BaseLog.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {
	// All with the same number of chars so the log is nice on the eyes :)
	std::string logLevelToString(LogLevel lvl) {
		switch (lvl) {
		case LogLevel::ERR:
			return "  ERROR";
			break;
		case LogLevel::WARNING:
			return "WARNING";
			break;
		case LogLevel::INFO:
			return "    INFO";
			break;
		case LogLevel::DEBUG:
			return "   DEBUG";
			break;
		default:
			return "????????";
			break;
		}
	}

	std::string getCurrentTimeStamp() {
		SYSTEMTIME systime;
		GetSystemTime(&systime);

		char buf[200];
		sprintf_s(buf, "%04u-%02u-%02u %02u:%02u:%02u:%03u", systime.wYear, systime.wMonth, systime.wDay,
			systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);

		return std::string(buf);
	}
}

template <typename OutputPolicy>
LogLevel BaseLog<OutputPolicy>::globalLevel = LogLevel::INFO;

template <typename OutputPolicy>
BaseLog<OutputPolicy>::BaseLog() = default;

template <typename OutputPolicy>
BaseLog<OutputPolicy>::~BaseLog() {
	//if (this->messageLevel <= BaseLog<OutputPolicy>::globalLevel) {
	this->msgStream << std::endl;
	OutputPolicy::write(this->msgStream.str());
	//}
}

template<typename OutputPolicy>
std::ostringstream& BaseLog<OutputPolicy>::err() {
	return this->getLogger(LogLevel::ERR);
}

template<typename OutputPolicy>
std::ostringstream& BaseLog<OutputPolicy>::warn() {
	return this->getLogger(LogLevel::WARNING);
}

template<typename OutputPolicy>
std::ostringstream& BaseLog<OutputPolicy>::info() {
	return this->getLogger(LogLevel::INFO);
}

template<typename OutputPolicy>
std::ostringstream& BaseLog<OutputPolicy>::debug() {
	return this->getLogger(LogLevel::DEBUG);
}

template<typename OutputPolicy>
std::ostringstream& BaseLog<OutputPolicy>::getLogger(LogLevel lvl) {
	this->messageLevel = lvl;
	this->msgStream << "[" << logLevelToString(lvl) << "]";
	this->msgStream << "[" << getCurrentTimeStamp() << "]";
	this->msgStream << "[TID " << GetCurrentThreadId() << "]";

	this->msgStream << " ";
	return this->msgStream;
}

template class BaseLog<LogFilePolicy>;