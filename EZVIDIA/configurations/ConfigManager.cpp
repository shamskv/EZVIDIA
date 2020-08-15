#include "../include/configurations/ConfigManager.hpp"

bool ConfigManager::addConfiguration(const GlobalConfiguration& conf) {
	std::lock_guard<std::mutex> lock(configurationLock);
	configMap.insert_or_assign(conf.name, conf);
	return persist();
}

bool ConfigManager::deleteConfiguration(const std::wstring& name) {
	std::lock_guard<std::mutex> lock(configurationLock);
	configMap.erase(name);
	return persist();
}

bool ConfigManager::isConfigurationPresent(const std::wstring& name) {
	std::lock_guard<std::mutex> lock(configurationLock);
	return configMap.find(name) != configMap.end();
}

GlobalConfiguration ConfigManager::getConfiguration(const std::wstring& name) {
	std::lock_guard<std::mutex> lock(configurationLock);
	return configMap.at(name);
}

std::vector<std::wstring> ConfigManager::getAllConfigurationNames() {
	std::lock_guard<std::mutex> lock(configurationLock);
	std::vector<std::wstring> names;

	for (auto it = configMap.begin(); it != configMap.end(); it++) {
		names.push_back(it->first);
	}

	return names;
}

bool ConfigManager::refreshConfigurations() {
	return read();
}

void ConfigManager::init() {
	if (!read()) {
		persist();
	}
}

uint32_t ConfigManager::getConfigNum() {
	return configMap.size();
}