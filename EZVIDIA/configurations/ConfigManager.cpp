#include "../include/configurations/ConfigManager.hpp"

bool ConfigManager::addConfiguration(const GlobalConfiguration& conf) {
	std::lock_guard<std::mutex> lock(configurationLock);
	configVector.push_back(conf);
	return persist();
}

bool ConfigManager::deleteConfiguration(const std::wstring& name) {
	std::lock_guard<std::mutex> lock(configurationLock);
	auto stfuCompiler = std::remove_if(configVector.begin(), configVector.end(), [&name](GlobalConfiguration& gc) {return name == gc.name; });
	return persist();
}

bool ConfigManager::deleteConfiguration(const int& index) {
	std::lock_guard<std::mutex> lock(configurationLock);
	configVector.erase(configVector.begin() + index);
	return persist();
}

bool ConfigManager::isConfigurationPresent(const std::wstring& name) {
	std::lock_guard<std::mutex> lock(configurationLock);
	return std::find_if(configVector.begin(), configVector.end(), [&name](GlobalConfiguration& gc) {return name == gc.name; }) != configVector.end();
}

std::optional<GlobalConfiguration> ConfigManager::getConfiguration(const std::wstring& name) {
	std::lock_guard<std::mutex> lock(configurationLock);
	auto it = std::find_if(configVector.begin(), configVector.end(), [&name](GlobalConfiguration& gc) {return name == gc.name; });
	if (it != configVector.end()) {
		return std::optional<GlobalConfiguration>(*it);
	}
	else {
		return std::optional<GlobalConfiguration>();
	}
}

std::optional<GlobalConfiguration> ConfigManager::getConfiguration(const int& index) {
	std::lock_guard<std::mutex> lock(configurationLock);
	if (index >= 0 && index < configVector.size()) {
		return std::optional<GlobalConfiguration>(configVector.at(index));
	}
	else {
		return std::optional<GlobalConfiguration>();
	}
}

std::vector<std::wstring> ConfigManager::getAllConfigurationNames() {
	std::lock_guard<std::mutex> lock(configurationLock);
	std::vector<std::wstring> names;

	for (auto& conf : configVector) {
		names.push_back(conf.name);
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

size_t ConfigManager::getConfigNum() {
	return configVector.size();
}