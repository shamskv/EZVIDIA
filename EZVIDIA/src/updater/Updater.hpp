#pragma once
#include <string>
#include <optional>

class VersionInfo {
public:
	std::wstring tag;
	std::wstring notes;
	std::wstring assetsUrl;
};

class Updater {
private:
	inline static const std::wstring repo = std::wstring(L"EZVIDIA");
	inline static const std::wstring owner = std::wstring(L"shamskv");
private:
	static std::optional<VersionInfo> getLatestVersion();
public:
	static std::optional<VersionInfo> checkUpdateAvailable();
	static bool downloadAndInstall(const VersionInfo&);
};