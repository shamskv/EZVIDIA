#include "..\include\drivers\NvapiDriver.hpp"
#include "../include/drivers/DriverException.hpp"
#include "../include/drivers/NvapiPathInfoWrapper.hpp"
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
	NvapiPathInfoWrapper pInfoWrapper;

	do {
		NvAPI_Status ret = NvAPI_DISP_GetDisplayConfig(&pInfoWrapper.nDisplays, pInfoWrapper.pInfo);
		if (ret != NVAPI_OK) {
			throw DriverException(L"Error calling NvAPI_DISP_GetDisplayConfig (code " + std::to_wstring(ret) + L")");
		}
	} while (pInfoWrapper.allocate());

	GlobalConfiguration gConf;
	uint32_t cloneGroup = 0;
	for (uint32_t i = 0; i < pInfoWrapper.nDisplays; i++) {
		for (uint32_t j = 0; j < pInfoWrapper.pInfo[i].targetInfoCount; j++) {
			DisplayConfiguration dConf;
			dConf.width = pInfoWrapper.pInfo[i].sourceModeInfo->resolution.width;
			dConf.height = pInfoWrapper.pInfo[i].sourceModeInfo->resolution.height;
			dConf.colorDepth = pInfoWrapper.pInfo[i].sourceModeInfo->resolution.colorDepth;
			dConf.posX = pInfoWrapper.pInfo[i].sourceModeInfo->position.x;
			dConf.posY = pInfoWrapper.pInfo[i].sourceModeInfo->position.y;

			dConf.displayId = pInfoWrapper.pInfo[i].targetInfo[j].displayId;
			dConf.refresh = pInfoWrapper.pInfo[i].targetInfo[j].details->refreshRate1K;
			dConf.rotation = pInfoWrapper.pInfo[i].targetInfo[j].details->rotation;
			dConf.scaling = pInfoWrapper.pInfo[i].targetInfo[j].details->scaling;
			dConf.tvFormat = pInfoWrapper.pInfo[i].targetInfo[j].details->tvFormat;

			dConf.cloneGroup = cloneGroup;

			if (pInfoWrapper.pInfo[i].sourceModeInfo->bGDIPrimary == 1) {
				gConf.primaryGroup = cloneGroup;
			}

			gConf.displays.push_back(dConf);
		}
		cloneGroup++;
	}

	return gConf;
}

void NvapiDriver::setConfig(const GlobalConfiguration& conf) {
}