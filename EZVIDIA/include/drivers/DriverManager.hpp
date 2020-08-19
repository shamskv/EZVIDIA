#pragma once
#include "../configurations/structs/GlobalConfiguration.hpp"

class DriverManager {
public:
	DriverManager();
	void applyConfiguration(const GlobalConfiguration& conf);
	GlobalConfiguration getCurrentConfig();
};