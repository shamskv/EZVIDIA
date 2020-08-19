#pragma once
#include <Windows.h>
#include <memory>
#include "configurations/ConfigManager.hpp"

class EzvidiaMaster {
private:
	std::unique_ptr<ConfigManager> configManager;
public:
	EzvidiaMaster(const HINSTANCE& hInstance, const std::string& configPath);
	// Windows stuff
	HINSTANCE hInst;
	// Aux
	void init();
	bool blockInput;
	// Operations
	void saveCurrentConfig(const std::wstring& newName);
	void applySelectedConfig(const std::wstring& name);
	void applySelectedConfig(const int& index);
	void deleteSelectedConfig(const std::wstring& name);
	void deleteSelectedConfig(const int& index);
	bool isConfigNameAvailable(const std::wstring& name);
	std::wstring getConfigNameByIndex(const int& index);
	std::vector<std::wstring> getConfigList();
	size_t getConfigNum();
};