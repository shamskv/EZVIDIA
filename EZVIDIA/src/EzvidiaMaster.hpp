#pragma once
#include <memory>
#include "configurations/VirtualConfigurationList.hpp"
#include "drivers/DriverManager.hpp"
#include "networking/UdpHandlerThread.hpp"

class EzvidiaMaster : public std::enable_shared_from_this<EzvidiaMaster> {
	using HINSTANCE = void*;
private:
	std::unique_ptr<VirtualConfigurationList> configManager;
	std::unique_ptr<DriverManager> driverManager;
public:
	EzvidiaMaster(const HINSTANCE& hInstance, const std::string& configPath);
	// Windows stuff
	HINSTANCE hInst;
	// Aux
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