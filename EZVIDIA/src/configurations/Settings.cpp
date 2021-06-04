#include "Settings.hpp"

bool Settings::addConfiguration(const GlobalConfiguration& conf) {
	std::lock_guard<std::mutex> lock(configurationLock);
	configVector.push_back(conf);
	return persist();
}

bool Settings::deleteConfiguration(const std::wstring& name) {
	std::lock_guard<std::mutex> lock(configurationLock);
	auto stfuCompiler = std::remove_if(configVector.begin(), configVector.end(), [&name](GlobalConfiguration& gc) {return name == gc.name; });
	return persist();
}

bool Settings::deleteConfiguration(const int& index) {
	std::lock_guard<std::mutex> lock(configurationLock);
	configVector.erase(configVector.begin() + index);
	return persist();
}

bool Settings::isConfigurationPresent(const std::wstring& name) {
	std::lock_guard<std::mutex> lock(configurationLock);
	return std::find_if(configVector.begin(), configVector.end(), [&name](GlobalConfiguration& gc) {return name == gc.name; }) != configVector.end();
}

std::optional<GlobalConfiguration> Settings::getConfiguration(const std::wstring& name) {
	std::lock_guard<std::mutex> lock(configurationLock);
	auto it = std::find_if(configVector.begin(), configVector.end(), [&name](GlobalConfiguration& gc) {return name == gc.name; });
	if (it != configVector.end()) {
		return std::optional<GlobalConfiguration>(*it);
	}
	else {
		return std::optional<GlobalConfiguration>();
	}
}

std::optional<GlobalConfiguration> Settings::getConfiguration(const int& index) {
	std::lock_guard<std::mutex> lock(configurationLock);
	if (index >= 0 && index < configVector.size()) {
		return std::optional<GlobalConfiguration>(configVector.at(index));
	}
	else {
		return std::optional<GlobalConfiguration>();
	}
}

std::vector<std::wstring> Settings::getAllConfigurationNames() {
	std::lock_guard<std::mutex> lock(configurationLock);
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
	if (!read()) {
		persist();
	}
}

size_t Settings::getConfigNum() {
	return configVector.size();
}