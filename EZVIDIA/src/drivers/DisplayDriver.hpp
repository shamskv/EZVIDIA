#pragma once
#include "../configurations/GlobalConfiguration.hpp"
#include <optional>
#include <memory>

class DisplayDriver {
protected:
	enum DriverState { OK, FAIL };
	DriverState state = FAIL;
	virtual GlobalConfiguration _getConfig() = 0;
	virtual void _applyConfig(const GlobalConfiguration& conf) = 0;
public:
	std::optional<GlobalConfiguration> getConfig();
	void applyConfig(const GlobalConfiguration& conf);
	bool isOK();
	// static method to get instance
	static std::shared_ptr<DisplayDriver> getAvailableDriver();
};