#pragma once
#include<fstream>
#include<mutex>
#include<string>

class LogFilePolicy {
private:
	LogFilePolicy();
	static std::mutex fileLock;
	static std::ofstream outputFileStream;
public:
	static void write(std::string);
	static void init(std::string);
};