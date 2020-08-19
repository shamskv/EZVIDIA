//This file is part of EZVIDIA.
//
//EZVIDIA is free software : you can redistribute itand /or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//EZVIDIA is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with EZVIDIA. If not, see < https://www.gnu.org/licenses/>.

# pragma once
#include <nvapi.h>
#include "readable_config.hpp"


class NVAPIController {
public:
	static int applyGlobalConfig(GlobalConfig& conf, bool test = false) {

		if (init()) {
			return -1;
		}

		int nDisplays, ret;
		nDisplays = conf.displayList.size();

		NV_DISPLAYCONFIG_PATH_INFO* pInfo = NULL;
		pInfo = (NV_DISPLAYCONFIG_PATH_INFO*)malloc(nDisplays * sizeof(NV_DISPLAYCONFIG_PATH_INFO));
		if (pInfo == NULL) {
			return -1;
		}

		for (int i = 0; i < nDisplays; i++) {
			pInfo[i] = *(conf.displayList[i].toNVAPIPathInfo());
			if (pInfo[i].targetInfo->displayId == conf.primaryId) {
				pInfo[i].sourceModeInfo->bGDIPrimary = 1;
			}
			else {
				pInfo[i].sourceModeInfo->bGDIPrimary = 0;
			}
		}

		//Validate first
		ret = NvAPI_DISP_SetDisplayConfig(nDisplays, pInfo, 1);
		if (ret) {
			return ret;
		}


		if (test) {
			ret = NvAPI_DISP_SetDisplayConfig(nDisplays, pInfo, 0);
		}
		else {
			ret = NvAPI_DISP_SetDisplayConfig(nDisplays, pInfo, 2);
		}

		return ret;
	}

	static boost::optional<GlobalConfig> getCurrentConfig() {

		NvU32 nDisplays = 0, ret = 0;
		NV_DISPLAYCONFIG_PATH_INFO* pInfo = NULL;

		if (init()) {
			return boost::none;
		}

		//Get number of displays
		ret = NvAPI_DISP_GetDisplayConfig(&nDisplays, NULL);
		if (ret != NVAPI_OK) {
			return boost::none;
		}

		//Retrieve display info
		pInfo = (NV_DISPLAYCONFIG_PATH_INFO*)malloc(nDisplays * sizeof(NV_DISPLAYCONFIG_PATH_INFO));
		if (pInfo == NULL) {
			return boost::none;
		}
		memset(pInfo, 0, nDisplays * sizeof(NV_DISPLAYCONFIG_PATH_INFO));
		for (int i = 0; i < nDisplays; i++) {
			pInfo[i].version = NV_DISPLAYCONFIG_PATH_INFO_VER;
		}
		ret = NvAPI_DISP_GetDisplayConfig(&nDisplays, pInfo);
		if (ret != NVAPI_OK) {
			return boost::none;
		}

		//Allocate space for more information
		for (int i = 0; i < nDisplays; i++) {
			if (pInfo[i].version == NV_DISPLAYCONFIG_PATH_INFO_VER1 || pInfo[i].version == NV_DISPLAYCONFIG_PATH_INFO_VER2) {
				pInfo[i].sourceModeInfo = (NV_DISPLAYCONFIG_SOURCE_MODE_INFO*)malloc(sizeof(NV_DISPLAYCONFIG_SOURCE_MODE_INFO));
			}
			else {
				return boost::none;
			}
			if (pInfo[i].sourceModeInfo == NULL) {
				return boost::none;
			}
			memset(pInfo[i].sourceModeInfo, 0, sizeof(NV_DISPLAYCONFIG_SOURCE_MODE_INFO));

			if (pInfo[i].targetInfoCount > 1) {
				return boost::none;
			}
			pInfo[i].targetInfo = (NV_DISPLAYCONFIG_PATH_TARGET_INFO*)malloc(sizeof(NV_DISPLAYCONFIG_PATH_TARGET_INFO));
			if (pInfo[i].targetInfo == NULL) {
				return boost::none;
			}
			memset(pInfo[i].targetInfo, 0, sizeof(NV_DISPLAYCONFIG_PATH_TARGET_INFO));
			pInfo[i].targetInfo->details = (NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO*)malloc(sizeof(NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO));
			if (pInfo[i].targetInfo->details == NULL) {
				return boost::none;
			}
			memset(pInfo[i].targetInfo->details, 0, sizeof(NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO));
			pInfo[i].targetInfo->details->version = NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO_VER;
		}

		ret = NvAPI_DISP_GetDisplayConfig(&nDisplays, pInfo);
		if (ret != NVAPI_OK) {
			return boost::none;
		}

		GlobalConfig conf;
		//Create the configuration
		for (int i = 0; i < nDisplays; i++) {
			DisplayParameters disp;
			disp.displayId = pInfo[i].targetInfo->displayId;
			disp.posX = pInfo[i].sourceModeInfo->position.x;
			disp.posY = pInfo[i].sourceModeInfo->position.y;
			disp.height = pInfo[i].sourceModeInfo->resolution.height;
			disp.width = pInfo[i].sourceModeInfo->resolution.width;
			disp.refresh = pInfo[i].targetInfo->details->refreshRate1K;
			disp.colorDepth = pInfo[i].sourceModeInfo->resolution.colorDepth;
			disp.rotation = pInfo[i].targetInfo->details->rotation * 90;
			if (disp.rotation == 0 || disp.rotation == 180) {
				disp.virtual_width = disp.width;
				disp.virtual_height = disp.height;
			}
			else if (disp.rotation == 90 || disp.rotation == 270) {
				disp.virtual_width = disp.height;
				disp.virtual_height = disp.width;
			}
			if (pInfo[i].sourceModeInfo->bGDIPrimary == 1) {
				conf.primaryId = disp.displayId;
			}

			conf.displayList.push_back(disp);
		}

		return conf;
	}

private:
	static bool isInit;


	static int init() {

		if (isInit) {
			return 0;
		}

		NvAPI_Status ret = NVAPI_OK;
		ret = NvAPI_Initialize();
		if (ret != NVAPI_OK)
		{
			return 1; // Initialization failed
		}

		isInit = true;
		return 0;
	}
};
