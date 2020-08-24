#include "..\include\drivers\DriverManager.hpp"
#include "..\include\drivers\NvapiDriver.hpp"
#include "..\include\drivers\DriverException.hpp"

DriverManager::DriverManager() {
	try {
		driver = std::make_unique<NvapiDriver>();
		return;
	}
	catch (DriverException& e) {
		// TODO Log that nvidia failed
		e;
	}

	throw DriverException(L"Failed to find suitable driver.");
}

void DriverManager::applyConfiguration(const GlobalConfiguration& conf) {
	driver.get()->setConfig(conf);
}

GlobalConfiguration DriverManager::getCurrentConfig() {
	return driver.get()->getConfig();
}