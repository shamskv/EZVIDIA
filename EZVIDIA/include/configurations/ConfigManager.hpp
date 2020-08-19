#pragma once
#include<mutex>
#include<vector>
#include<string>
#include<optional>
#include"structs/GlobalConfiguration.hpp"

class ConfigManager {
protected:
	std::vector<GlobalConfiguration> configVector;
	std::mutex configurationLock;

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
};