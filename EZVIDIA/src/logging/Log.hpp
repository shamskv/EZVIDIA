#pragma once
#include<sstream>
// Include all the policies available (just file for now, maybe a GUI Log in the future?)
#include"LogFilePolicy.hpp"

enum class LogLevel {
	ERR, WARNING, INFO, DEBUG
};

template <typename OutputPolicy>
class Log {
public:
	Log();
	~Log();
	std::ostringstream& err();
	std::ostringstream& warn();
	std::ostringstream& info();
	std::ostringstream& debug();
private:
	Log(const Log&) = delete;
	Log& operator=(const Log&) = delete;
	std::ostringstream& level(LogLevel = LogLevel::DEBUG);
public:
	static LogLevel globalLevel;
private:
	LogLevel messageLevel;
	std::ostringstream msgStream;
};