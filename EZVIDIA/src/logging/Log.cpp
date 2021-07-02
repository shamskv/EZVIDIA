#include "Log.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {
	std::string logLevelToString(LogLevel lvl) {
		switch (lvl) {
		case LogLevel::ERR:
			return "ERROR";
			break;
		case LogLevel::WARNING:
			return "WARNING";
			break;
		case LogLevel::INFO:
			return "INFO";
			break;
		case LogLevel::DEBUG:
			return "DEBUG";
			break;
		default:
			return "???";
			break;
		}
	}

	std::string getCurrentTimeStamp() {
		SYSTEMTIME systime;
		GetSystemTime(&systime);

		char buf[200];
		sprintf_s(buf, "%u-%u-%u %u:%u:%u.%u", systime.wYear, systime.wMonth, systime.wDay,
			systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);

		return std::string(buf);
	}
}

template <typename OutputPolicy>
LogLevel Log<OutputPolicy>::globalLevel = LogLevel::INFO;

template <typename OutputPolicy>
Log<OutputPolicy>::Log() = default;

template <typename OutputPolicy>
Log<OutputPolicy>::~Log() {
	if (this->messageLevel <= Log<OutputPolicy>::globalLevel) {
		this->msgStream << std::endl;
		OutputPolicy::write(this->msgStream.str());
	}
}

template<typename OutputPolicy>
std::ostringstream& Log<OutputPolicy>::err() {
	return this->level(LogLevel::ERR);
}

template<typename OutputPolicy>
std::ostringstream& Log<OutputPolicy>::warn() {
	return this->level(LogLevel::WARNING);
}

template<typename OutputPolicy>
std::ostringstream& Log<OutputPolicy>::info() {
	return this->level(LogLevel::INFO);
}

template<typename OutputPolicy>
std::ostringstream& Log<OutputPolicy>::debug() {
	return this->level(LogLevel::DEBUG);
}

template<typename OutputPolicy>
std::ostringstream& Log<OutputPolicy>::level(LogLevel lvl) {
	this->messageLevel = lvl;
	this->msgStream << "[" << logLevelToString(lvl) << "] ";
	this->msgStream << "[" << getCurrentTimeStamp() << "] ";
	this->msgStream << "[ TID " << GetCurrentThreadId() << "] ";

	return this->msgStream;
}

template class Log<LogFilePolicy>;