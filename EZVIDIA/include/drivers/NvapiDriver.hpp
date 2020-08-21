#pragma once
#include "VirtualDriver.hpp"

class NvapiDriver : public VirtualDriver {
public:
	NvapiDriver();
	GlobalConfiguration getConfig();
	void setConfig(const GlobalConfiguration& conf);
};