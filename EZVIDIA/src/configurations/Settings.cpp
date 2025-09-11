#include "Settings.hpp"

bool Settings::addConfiguration(const GlobalConfiguration& conf) {
  std::lock_guard<std::mutex> lock(settingsLock);
  configVector.push_back(conf);
  return persist();
}

bool Settings::deleteConfiguration(const std::wstring& name) {
  std::lock_guard<std::mutex> lock(settingsLock);
  auto stfuCompiler = std::remove_if(
      configVector.begin(), configVector.end(),
      [&name](GlobalConfiguration& gc) { return name == gc.name; });
  return persist();
}

bool Settings::deleteConfiguration(const int& index) {
  std::lock_guard<std::mutex> lock(settingsLock);
  configVector.erase(configVector.begin() + index);
  return persist();
}

bool Settings::isConfigurationPresent(const std::wstring& name) {
  std::lock_guard<std::mutex> lock(settingsLock);
  return std::find_if(configVector.begin(), configVector.end(),
                      [&name](GlobalConfiguration& gc) {
                        return name == gc.name;
                      }) != configVector.end();
}

std::optional<GlobalConfiguration> Settings::getConfiguration(
    const std::wstring& name) {
  std::lock_guard<std::mutex> lock(settingsLock);
  auto it = std::find_if(
      configVector.begin(), configVector.end(),
      [&name](GlobalConfiguration& gc) { return name == gc.name; });
  if (it != configVector.end()) {
    return std::optional<GlobalConfiguration>(*it);
  } else {
    return std::optional<GlobalConfiguration>();
  }
}

std::optional<GlobalConfiguration> Settings::getConfiguration(
    const int& index) {
  std::lock_guard<std::mutex> lock(settingsLock);
  if (index >= 0 && index < configVector.size()) {
    return std::optional<GlobalConfiguration>(configVector.at(index));
  } else {
    return std::optional<GlobalConfiguration>();
  }
}

std::vector<std::wstring> Settings::getAllConfigurationNames() {
  std::lock_guard<std::mutex> lock(settingsLock);
  std::vector<std::wstring> names;

  for (auto& conf : configVector) {
    names.push_back(conf.name);
  }

  return names;
}

bool Settings::refreshConfigurations() {
  return read();
}

void Settings::init() {
  read();
  persist();
}

size_t Settings::getConfigNum() {
  return configVector.size();
}

bool Settings::networkTcpActive() {
  return networkTcp;
}

bool Settings::setNetworkTcp(bool value) {
  networkTcp = value;

  return persist();
}

bool Settings::webServerActive() {
  return webServer;
}

bool Settings::setWebServer(bool value) {
  webServer = value;
  return persist();
}