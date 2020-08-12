#include "../include/configurations/ConfigManager.hpp"

void ConfigManager::addConfiguration(const GlobalConfiguration& conf) {
	std::lock_guard<std::mutex> lock(configurationLock);
	configMap.insert_or_assign(conf.name, conf);
	persist();
}

void ConfigManager::deleteConfiguration(const std::string& name) {
	std::lock_guard<std::mutex> lock(configurationLock);
	configMap.erase(name);
	persist();
}

bool ConfigManager::isConfigurationPresent(const std::string& name) {
	std::lock_guard<std::mutex> lock(configurationLock);
	return configMap.find(name) != configMap.end();
}

GlobalConfiguration ConfigManager::getConfiguration(const std::string& name) {
	std::lock_guard<std::mutex> lock(configurationLock);
	return configMap.at(name);
}

std::vector<std::string> ConfigManager::getAllConfigurationNames() {
	std::vector<std::string> names;

	for (auto it = configMap.begin(); it != configMap.end(); it++) {
		names.push_back(it->first);
	}

	return names;
}