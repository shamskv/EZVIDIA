#include "LogFilePolicy.hpp"

std::mutex LogFilePolicy::fileLock;
std::wofstream LogFilePolicy::outputFileStream;

void LogFilePolicy::write(std::wstring msg) {
	std::lock_guard lock(LogFilePolicy::fileLock);
	std::wofstream& ofs = LogFilePolicy::outputFileStream; // big name
	if (ofs.is_open() && ofs.good()) {
		ofs << msg;
		ofs.flush();
	}
}

void LogFilePolicy::init() {
	LogFilePolicy::outputFileStream.open(LOG_FILENAME, std::wofstream::app);
}