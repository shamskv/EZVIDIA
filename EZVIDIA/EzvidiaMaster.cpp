#include "include/EzvidiaMaster.hpp"
#include "include/configurations/JsonConfigManager.hpp"
#include "include/configurations/structs/GlobalConfiguration.hpp"

EzvidiaMaster::EzvidiaMaster(const HINSTANCE& hInstance, const std::string& configPath) : hInst(hInstance) {
	configManager = std::make_unique<JsonConfigManager>(configPath);
	this->blockInput = false;
}
void EzvidiaMaster::init() {
	configManager.get()->init();
}

void EzvidiaMaster::saveCurrentConfig(const std::wstring& newName) {
	// TODO get config from our driver interface
	DisplayConfiguration dConf;
	dConf.height = 1080;
	dConf.width = 1920;
	dConf.posX = 0;
	dConf.posY = 0;
	dConf.colorDepth = 32;
	dConf.refresh = 60000;
	dConf.rotation = 0;
	dConf.scaling = 0;
	dConf.tvFormat = 0;
	dConf.displayIds.push_back(12345);
	GlobalConfiguration gConf;
	gConf.name = newName;
	gConf.primaryId = 12345;
	gConf.displays.push_back(dConf);

	configManager.get()->addConfiguration(gConf);
}

void EzvidiaMaster::applySelectedConfig(const std::wstring& name) {
	auto optConf = configManager.get()->getConfiguration(name);

	if (!optConf.has_value()) {
		return;
	}

	auto conf = optConf.value();
	// TODO apply the config
}

void EzvidiaMaster::applySelectedConfig(const int& index) {
	auto optConf = configManager.get()->getConfiguration(index);

	if (!optConf.has_value()) {
		return;
	}

	auto conf = optConf.value();
	// TODO apply the config
}

void EzvidiaMaster::deleteSelectedConfig(const std::wstring& name) {
	configManager.get()->deleteConfiguration(name);
}

void EzvidiaMaster::deleteSelectedConfig(const int& index) {
	configManager.get()->deleteConfiguration(index);
}

bool EzvidiaMaster::isConfigNameAvailable(const std::wstring& name) {
	return !configManager.get()->isConfigurationPresent(name);
}

std::wstring EzvidiaMaster::getConfigNameByIndex(const int& index) {
	auto optConf = configManager.get()->getConfiguration(index);

	if (!optConf.has_value()) {
		return L"";
	}

	return optConf.value().name;
}

std::vector<std::wstring> EzvidiaMaster::getConfigList() {
	return configManager.get()->getAllConfigurationNames();
}

size_t EzvidiaMaster::getConfigNum() {
	return configManager.get()->getConfigNum();
}