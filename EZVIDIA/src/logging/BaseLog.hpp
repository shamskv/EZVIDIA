#pragma once
#include<sstream>
// Include all the policies available (just file for now, maybe a GUI Log in the future?)
#include"LogFilePolicy.hpp"

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

#define Log BaseLog<LogFilePolicy> // define the policy we will use
#define LOG(level) if(LogLevel::level > Log::globalLevel) ; else Log().getLogger(LogLevel::level) 