#pragma once
#include "../configurations/GlobalConfiguration.hpp"
#include "VirtualDriver.hpp"
#include <memory>

class DriverManager {
private:
	std::unique_ptr<VirtualDriver> driver;
public:
	DriverManager();
	void applyConfiguration(const GlobalConfiguration& conf);
	GlobalConfiguration getCurrentConfig();
};