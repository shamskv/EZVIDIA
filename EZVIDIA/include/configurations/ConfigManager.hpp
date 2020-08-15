#pragma once
#include<map>
#include<mutex>
#include<vector>
#include<string>
#include"structs/GlobalConfiguration.hpp"

class ConfigManager {
protected:
	std::map<std::wstring, GlobalConfiguration> configMap;
	std::mutex configurationLock;

	virtual bool persist() = 0;

	virtual bool read() = 0;

public:
	bool addConfiguration(const GlobalConfiguration& conf);

	bool deleteConfiguration(const std::wstring& name);

	bool isConfigurationPresent(const std::wstring& name);

	GlobalConfiguration getConfiguration(const std::wstring& name);

	std::vector<std::wstring> getAllConfigurationNames();

	bool refreshConfigurations();

	void init();

	uint32_t getConfigNum();
};