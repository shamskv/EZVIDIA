#pragma once
#include<fstream>
#include<mutex>
#include<string>

#define LOG_FILENAME "ezvidia.log"

class LogFilePolicy {
private:
	LogFilePolicy();
	static std::mutex fileLock;
	static std::wofstream outputFileStream;
public:
	static void write(std::wstring);
	static void init();
};