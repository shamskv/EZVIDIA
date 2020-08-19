#pragma once
#include "../configurations/structs/GlobalConfiguration.hpp"

class VirtualDriver {
public:
	virtual GlobalConfiguration getConfig() = 0;
	virtual void setConfig(const GlobalConfiguration& conf) = 0;
};