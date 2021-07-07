#pragma once
#include <string>
#include <optional>

class Updater {
public:
	static std::wstring getLatestVersionNumber();
	static std::optional<std::wstring> checkUpdateAvailable();
};