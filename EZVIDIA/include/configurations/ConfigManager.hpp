#pragma once
#include<map>
#include<mutex>
#include<vector>
#include<string>
#include"structs/GlobalConfiguration.hpp"

class ConfigManager {
protected:
	std::map<std::string, GlobalConfiguration> configMap;
	std::mutex configurationLock;

	virtual bool persist() = 0;

	virtual bool read() = 0;

public:
	void addConfiguration(const GlobalConfiguration& conf);

	void deleteConfiguration(const std::string& name);

	bool isConfigurationPresent(const std::string& name);

	// Return a copy of the configuration (it's not heavy and we don't want to risk it being deleted before we use it)
	GlobalConfiguration getConfiguration(const std::string& name);

	std::vector<std::string> getAllConfigurationNames();
};