#pragma once
#include<sstream>

enum class LogLevel {
	ERR, WARN, INFO, DEBUG
};

template <typename OutputPolicy>
class BaseLog {
public:
	BaseLog();
	~BaseLog();
	std::wostringstream& getLogger(LogLevel);
	static void init(LogLevel);
private:
	BaseLog(const BaseLog&) = delete;
	BaseLog& operator=(const BaseLog&) = delete;
public:
	static LogLevel loggingLevel;
private:
	LogLevel msgLevel;
	std::wostringstream msgStream;
};