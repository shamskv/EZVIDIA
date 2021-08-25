#pragma once
#include "DisplayDriver.hpp"

class NvapiDriver : public DisplayDriver {
public:
	NvapiDriver();
	GlobalConfiguration _getConfig() override;
	bool _applyConfig(const GlobalConfiguration& conf) override;
};