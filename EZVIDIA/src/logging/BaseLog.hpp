#pragma once
#include<sstream>

enum class LogLevel {
	ERR, WARNING, INFO, DEBUG
};

template <typename OutputPolicy>
class BaseLog {
public:
	BaseLog();
	~BaseLog();
	std::ostringstream& err();
	std::ostringstream& warn();
	std::ostringstream& info();
	std::ostringstream& debug();
	std::ostringstream& getLogger(LogLevel);
private:
	BaseLog(const BaseLog&) = delete;
	BaseLog& operator=(const BaseLog&) = delete;
public:
	static LogLevel globalLevel;
private:
	LogLevel messageLevel;
	std::ostringstream msgStream;
};