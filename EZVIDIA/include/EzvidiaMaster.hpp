#pragma once
#include <Windows.h>
#include <memory>
#include "configurations/ConfigManager.hpp"

class EzvidiaMaster {
private:
	std::unique_ptr<ConfigManager> configManager;
	HINSTANCE hInst;
public:
	EzvidiaMaster(const HINSTANCE& hInstance, const std::string& configPath);
	void init();
	void saveCurrentConfig(const std::wstring& newName);
	void applySelectedConfig(const std::wstring& name);
	void deleteSelectedConfig(const std::wstring& name);
	std::vector<std::wstring> getConfigList();
	uint32_t getConfigNum();
};