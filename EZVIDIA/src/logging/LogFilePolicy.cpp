#include "LogFilePolicy.hpp"

std::mutex LogFilePolicy::fileLock;
std::ofstream LogFilePolicy::outputFileStream;

void LogFilePolicy::write(std::string msg) {
	std::lock_guard lock(LogFilePolicy::fileLock);
	std::ofstream& ofs = LogFilePolicy::outputFileStream; // big name
	if (ofs.is_open() && ofs.good()) {
		ofs << msg;
		ofs.flush();
	}
}

void LogFilePolicy::init() {
	LogFilePolicy::outputFileStream.open(LOG_FILENAME, std::ofstream::app);
}