#include "VirtualConfigurationList.hpp"

bool VirtualConfigurationList::addConfiguration(const GlobalConfiguration& conf) {
	std::lock_guard<std::mutex> lock(configurationLock);
	configVector.push_back(conf);
	return persist();
}

bool VirtualConfigurationList::deleteConfiguration(const std::wstring& name) {
	std::lock_guard<std::mutex> lock(configurationLock);
	auto stfuCompiler = std::remove_if(configVector.begin(), configVector.end(), [&name](GlobalConfiguration& gc) {return name == gc.name; });
	return persist();
}

bool VirtualConfigurationList::deleteConfiguration(const int& index) {
	std::lock_guard<std::mutex> lock(configurationLock);
	configVector.erase(configVector.begin() + index);
	return persist();
}

bool VirtualConfigurationList::isConfigurationPresent(const std::wstring& name) {
	std::lock_guard<std::mutex> lock(configurationLock);
	return std::find_if(configVector.begin(), configVector.end(), [&name](GlobalConfiguration& gc) {return name == gc.name; }) != configVector.end();
}

std::optional<GlobalConfiguration> VirtualConfigurationList::getConfiguration(const std::wstring& name) {
	std::lock_guard<std::mutex> lock(configurationLock);
	auto it = std::find_if(configVector.begin(), configVector.end(), [&name](GlobalConfiguration& gc) {return name == gc.name; });
	if (it != configVector.end()) {
		return std::optional<GlobalConfiguration>(*it);
	}
	else {
		return std::optional<GlobalConfiguration>();
	}
}

std::optional<GlobalConfiguration> VirtualConfigurationList::getConfiguration(const int& index) {
	std::lock_guard<std::mutex> lock(configurationLock);
	if (index >= 0 && index < configVector.size()) {
		return std::optional<GlobalConfiguration>(configVector.at(index));
	}
	else {
		return std::optional<GlobalConfiguration>();
	}
}

std::vector<std::wstring> VirtualConfigurationList::getAllConfigurationNames() {
	std::lock_guard<std::mutex> lock(configurationLock);
	std::vector<std::wstring> names;

	for (auto& conf : configVector) {
		names.push_back(conf.name);
	}

	return names;
}

bool VirtualConfigurationList::refreshConfigurations() {
	return read();
}

void VirtualConfigurationList::init() {
	if (!read()) {
		persist();
	}
}

size_t VirtualConfigurationList::getConfigNum() {
	return configVector.size();
}