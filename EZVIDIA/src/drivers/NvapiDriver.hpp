#pragma once
#include "DisplayDriver.hpp"

class NvapiDriver : public DisplayDriver {
public:
	NvapiDriver();
	GlobalConfiguration _getConfig() override;
	void _applyConfig(const GlobalConfiguration& conf) override;
};