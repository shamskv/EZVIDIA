#pragma once
#include<mutex>
#include<vector>
#include<string>
#include<optional>
#include"GlobalConfiguration.hpp"

class Settings {
protected:
	std::vector<GlobalConfiguration> configVector;
	bool networkTcp;
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
};