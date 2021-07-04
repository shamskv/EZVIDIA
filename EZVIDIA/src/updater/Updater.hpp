#pragma once
#include <string>
#include <optional>

class Updater {
private:
	static bool isUpdate(std::string current, std::string candidate);
public:
	static std::wstring getLatestVersionNumber();
	static std::optional<std::string> checkUpdateAvailable();
};