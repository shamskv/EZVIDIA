#include "DisplayDriver.hpp"
#include "NvapiDriver.hpp" // dangerous cycle here :)
#include "../logging/Logger.hpp"

std::optional<GlobalConfiguration> DisplayDriver::getConfig() {
	LOG(DEBUG) << "Generic get config";
	if (this->state == OK) {
		return std::make_optional(this->_getConfig());
	}
	else {
		return std::nullopt;
	}
}

bool DisplayDriver::applyConfig(const GlobalConfiguration& conf) {
	LOG(DEBUG) << "Generic apply config";
	if (this->state == OK) {
		return this->_applyConfig(conf);
	}
	return false;
}

bool DisplayDriver::isOK() {
	return this->state == OK;
}

std::unique_ptr<DisplayDriver> DisplayDriver::getAvailableDriver() {
	LOG(DEBUG) << "Selecting suitable driver...";

	auto nvidiaDriver = std::make_unique<NvapiDriver>();
	if (nvidiaDriver.get()->isOK()) {
		LOG(INFO) << "NVIDIA driver selected";
		return nvidiaDriver;
	}
	// We can add more drivers here (intel, windows, amd) one day... hopefully...
	LOG(ERR) << "No suitable driver found";
	return nvidiaDriver;
}