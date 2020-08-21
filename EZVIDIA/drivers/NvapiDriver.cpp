#include "..\include\drivers\NvapiDriver.hpp"
#include "../include/drivers/DriverException.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <nvapi.h>

NvapiDriver::NvapiDriver() {
#pragma warning (suppress:26812)
	NvAPI_Status ret = NvAPI_Initialize();
	if (ret != NVAPI_OK) {
		throw DriverException(L"Error initializing NvAPI.");
	}
}

GlobalConfiguration NvapiDriver::getConfig() {
	return GlobalConfiguration();
}

void NvapiDriver::setConfig(const GlobalConfiguration& conf) {
}