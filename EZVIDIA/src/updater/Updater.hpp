#pragma once
#include <optional>
#include <string>

class VersionInfo {
 public:
  std::wstring tag;
  std::wstring notes;
  std::string assetsUrl;
};

class Updater {
 private:
  inline static const std::string repo = std::string("EZVIDIA");
  inline static const std::string owner = std::string("shamskv");

 private:
  static std::optional<VersionInfo> getLatestVersion();

 public:
  static std::optional<VersionInfo> checkUpdateAvailable();
  static bool downloadAndInstall(const VersionInfo&);
};