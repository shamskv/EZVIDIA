#include "BaseLog.hpp"
// Include all the policies available, this is necessary to compile the template specializations at the end of the file
#include"LogFilePolicy.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {
	// All with the same number of chars so the log is nice on the eyes :)
	std::string logLevelToString(LogLevel lvl) {
		switch (lvl) {
		case LogLevel::ERR:
			return "ERROR";
			break;
		case LogLevel::WARN:
			return " WARN";
			break;
		case LogLevel::INFO:
			return " INFO";
			break;
		case LogLevel::DEBUG:
			return "DEBUG";
			break;
		default:
			return "?????";
			break;
		}
	}

	void getCurrentTimeStamp(char* buffer, size_t size) {
		SYSTEMTIME systime;
		GetSystemTime(&systime);

		sprintf_s(buffer, size, "%04u-%02u-%02u %02u:%02u:%02u:%03u", systime.wYear, systime.wMonth, systime.wDay,
			systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
	}
}

template <typename OutputPolicy>
LogLevel BaseLog<OutputPolicy>::loggingLevel = LogLevel::INFO;

template <typename OutputPolicy>
BaseLog<OutputPolicy>::BaseLog() = default;

template <typename OutputPolicy>
BaseLog<OutputPolicy>::~BaseLog() {
	this->msgStream << std::endl;
	OutputPolicy::write(this->msgStream.str());
}

template<typename OutputPolicy>
std::wostringstream& BaseLog<OutputPolicy>::getLogger(LogLevel lvl) {
	this->msgLevel = lvl;
	this->msgStream << "[" << logLevelToString(lvl).c_str() << "]";
	char buf[200] = { 0 };
	getCurrentTimeStamp(buf, 200);
	this->msgStream << "[" << buf << "]";
	this->msgStream << "[TID " << GetCurrentThreadId() << "]";

	this->msgStream << " ";
	return this->msgStream;
}

template<typename OutputPolicy>
void BaseLog<OutputPolicy>::init(LogLevel lvl) {
	BaseLog<OutputPolicy>::loggingLevel = lvl;
	OutputPolicy::init();
}

template class BaseLog<LogFilePolicy>;