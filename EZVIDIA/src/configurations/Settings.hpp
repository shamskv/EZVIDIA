#pragma once
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "GlobalConfiguration.hpp"

class Settings {
 protected:
  std::vector<GlobalConfiguration> configVector;
  bool networkTcp;
  bool webServer;
  uint16_t webServerPort;
  std::mutex settingsLock;

  virtual bool persist() = 0;

  virtual bool read() = 0;

 public:
  bool addConfiguration(const GlobalConfiguration& conf);

  bool deleteConfiguration(const std::wstring& name);

  bool deleteConfiguration(const int& index);

  bool isConfigurationPresent(const std::wstring& name);

  std::optional<GlobalConfiguration> getConfiguration(const std::wstring& name);

  std::optional<GlobalConfiguration> getConfiguration(const int& index);

  std::vector<std::wstring> getAllConfigurationNames();

  bool refreshConfigurations();

  void init();

  size_t getConfigNum();

  bool networkTcpActive();

  bool setNetworkTcp(bool);

  bool webServerActive();

  bool setWebServer(bool);

  bool setWebServerPort(uint16_t port);

  uint16_t getWebServerPort();
};