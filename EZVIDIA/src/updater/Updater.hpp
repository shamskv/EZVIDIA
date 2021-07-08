#pragma once
#include <string>
#include <optional>

class VersionInfo {
public:
	std::wstring tag;
	std::wstring notes;
};

class Updater {
private:
	static std::optional<VersionInfo> getLatestVersionNumber();
public:
	static std::optional<VersionInfo> checkUpdateAvailable();
};