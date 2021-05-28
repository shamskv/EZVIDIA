#include "DisplayDriver.hpp"
#include "NvapiDriver.hpp"

std::optional<GlobalConfiguration> DisplayDriver::getConfig() {
	if (this->state == OK) {
		return std::make_optional(this->_getConfig());
	}
	else {
		return std::nullopt;
	}
}

void DisplayDriver::applyConfig(const GlobalConfiguration& conf) {
	if (this->state == OK) {
		this->_applyConfig(conf);
	}
}

bool DisplayDriver::isOK() {
	return this->state == OK;
}

std::unique_ptr<DisplayDriver> DisplayDriver::getAvailableDriver() {
	auto nvidiaDriver = std::make_unique<NvapiDriver>();
	if (nvidiaDriver.get()->isOK()) {
		return nvidiaDriver;
	}
	// We can add more drivers here (intel, windows, amd) one day... hopefully...
	return nvidiaDriver;
}