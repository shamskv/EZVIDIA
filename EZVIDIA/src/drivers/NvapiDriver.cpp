#include "NvapiDriver.hpp"
#include "DriverException.hpp"
#include "NvapiPathInfoWrapper.hpp"
#include <nvapi.h>
#include <unordered_map>

#pragma warning (disable:26812)
NvapiDriver::NvapiDriver() {
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
	NvapiPathInfoWrapper pInfoWrapper;

	// assign a pInfo index for each clone group
	std::unordered_map<uint32_t, uint32_t> groupIndexMap;
	uint32_t idx = 0;
	for (auto& disp : conf.displays) {
		if (groupIndexMap.emplace(disp.cloneGroup, idx).second) {
			idx++;
		}
	}
	// count how many displays are in each index
	std::vector<uint32_t> sizePerIdx(idx, 0);
	for (auto& disp : conf.displays) {
		sizePerIdx[groupIndexMap[disp.cloneGroup]]++;
	}

	// Allocate memory
	pInfoWrapper.nDisplays = idx;
	pInfoWrapper.allocate();
	for (uint32_t i = 0; i < pInfoWrapper.nDisplays; i++) {
		pInfoWrapper.pInfo[i].targetInfoCount = sizePerIdx[i];
	}
	pInfoWrapper.allocate();

	// Fill pInfo
	for (auto& disp : conf.displays) {
		int i = groupIndexMap[disp.cloneGroup];
		int j = --sizePerIdx[i];

		// This means if two displays in the same clone group have different source modes it will silently pick one (undefined)
		pInfoWrapper.pInfo[i].sourceModeInfo->resolution.width = disp.width;
		pInfoWrapper.pInfo[i].sourceModeInfo->resolution.height = disp.height;
		pInfoWrapper.pInfo[i].sourceModeInfo->resolution.colorDepth = disp.colorDepth;
		pInfoWrapper.pInfo[i].sourceModeInfo->position.x = disp.posX;
		pInfoWrapper.pInfo[i].sourceModeInfo->position.y = disp.posY;

		pInfoWrapper.pInfo[i].targetInfo[j].displayId = disp.displayId;
		pInfoWrapper.pInfo[i].targetInfo[j].details->refreshRate1K = disp.refresh;
		pInfoWrapper.pInfo[i].targetInfo[j].details->rotation = static_cast<NV_ROTATE>(disp.rotation);
		pInfoWrapper.pInfo[i].targetInfo[j].details->scaling = static_cast<NV_SCALING>(disp.scaling);
		pInfoWrapper.pInfo[i].targetInfo[j].details->tvFormat = static_cast<NV_DISPLAY_TV_FORMAT>(disp.tvFormat);
	}
	pInfoWrapper.pInfo[groupIndexMap[conf.primaryGroup]].sourceModeInfo->bGDIPrimary = 1;

	NvAPI_Status ret = NvAPI_DISP_SetDisplayConfig(pInfoWrapper.nDisplays, pInfoWrapper.pInfo, NV_DISPLAYCONFIG_SAVE_TO_PERSISTENCE);
	if (ret != NVAPI_OK) {
		throw DriverException(L"Error calling NvAPI_DISP_SetDisplayConfig (code " + std::to_wstring(ret) + L")");
	}
}