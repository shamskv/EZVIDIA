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

#include "readable_config.hpp"
#include <json.hpp>
#include <string>
#include <boost/optional/optional.hpp>
#include <Windows.h>
#include <nvapi.h>
#include <iostream>

using nlohmann::json;

json DisplayParameters::mapToJson() {
	json j;

	j["name"] = name;
	j["displayId"] = displayId;
	j["height"] = height;
	j["width"] = width;
	j["colorDepth"] = colorDepth;
	j["refresh"] = refresh;
	j["posX"] = posX;
	j["posY"] = posY;
	j["rotation"] = rotation;

	return j;
}

void DisplayParameters::populateFromJson(json j) {
	name = j["name"].get<std::string>();
	displayId = j["displayId"].get<unsigned long>();
	height = j["height"].get<int>();
	width = j["width"].get<int>();
	colorDepth = j["colorDepth"].get<int>();
	refresh = j["refresh"].get<int>();
	posX = j["posX"].get<int>();
	posY = j["posY"].get<int>();
	rotation = j["rotation"].get<int>();

	if (rotation == 0 || rotation == 180) {
		virtual_width = width;
		virtual_height = height;
	}
	else if (rotation == 90 || rotation == 270) {
		virtual_width = height;
		virtual_height = width;
	}
}

NV_DISPLAYCONFIG_PATH_INFO* DisplayParameters::toNVAPIPathInfo() {

	NV_DISPLAYCONFIG_PATH_INFO* pInfo;

	pInfo = (NV_DISPLAYCONFIG_PATH_INFO*)malloc(sizeof(NV_DISPLAYCONFIG_PATH_INFO));
	if (pInfo == NULL)
		return NULL;
	memset(pInfo, 0, sizeof(NV_DISPLAYCONFIG_PATH_INFO));

	pInfo->targetInfo = (NV_DISPLAYCONFIG_PATH_TARGET_INFO*)malloc(sizeof(NV_DISPLAYCONFIG_PATH_TARGET_INFO));
	if (pInfo->targetInfo == NULL)
		return NULL;

	pInfo->sourceModeInfo = (NV_DISPLAYCONFIG_SOURCE_MODE_INFO*)malloc(sizeof(NV_DISPLAYCONFIG_SOURCE_MODE_INFO));
	if (pInfo->sourceModeInfo == NULL)
		return NULL;
	memset(pInfo->targetInfo, 0, sizeof(NV_DISPLAYCONFIG_PATH_TARGET_INFO));

	pInfo->targetInfo->details = (NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO*)malloc(sizeof(NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO));
	if (pInfo->targetInfo->details == NULL)
		return NULL;
	memset(pInfo->targetInfo->details, 0, sizeof(NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO));

	pInfo->version = NV_DISPLAYCONFIG_PATH_INFO_VER;
	pInfo->targetInfo->details->version = NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO_VER;
	pInfo->targetInfoCount = 1; // Extended

	pInfo->targetInfo->displayId = displayId;
	pInfo->targetInfo->details->refreshRate1K = refresh;
	pInfo->sourceModeInfo->bGDIPrimary = 0;
	pInfo->sourceModeInfo->resolution.height = height;
	pInfo->sourceModeInfo->resolution.width = width;
	pInfo->sourceModeInfo->resolution.colorDepth = colorDepth;
	pInfo->sourceModeInfo->colorFormat = NV_FORMAT_UNKNOWN;
	pInfo->sourceModeInfo->spanningOrientation = NV_DISPLAYCONFIG_SPAN_NONE;
	pInfo->sourceModeInfo->position.x = posX;
	pInfo->sourceModeInfo->position.y = posY;

	switch (rotation) {
	case 0:
		pInfo->targetInfo->details->rotation = NV_ROTATE_0;
		break;
	case 90:
		pInfo->targetInfo->details->rotation = NV_ROTATE_90;
		break;
	case 180:
		pInfo->targetInfo->details->rotation = NV_ROTATE_180;
		break;
	case 270:
		pInfo->targetInfo->details->rotation = NV_ROTATE_270;
		break;
	default:
		pInfo->targetInfo->details->rotation = NV_ROTATE_0;
		break;
	}

	return pInfo;
}

DisplayParameters::DisplayParameters() {};
//DisplayParameters::DisplayParameters(std::string n, unsigned long dI, int h, int w, int cD, int r, int pX, int pY, int rotation) : name(n), displayId(dI), height(h), width(w), colorDepth(cD), refresh(r), posX(pX), posY(pY), rotation(rotation) {}
DisplayParameters::DisplayParameters(json j) { populateFromJson(j); }


json GlobalConfig::mapToJson() {
	json j;

	j["name"] = name;
	j["primaryId"] = primaryId;
	j["displayList"].array();

	for (DisplayParameters dp : displayList) {
		j["displayList"].push_back(dp.mapToJson());
	}

	return j;
}

void GlobalConfig::populateFromJson(json j) {

	if (j["name"].empty() || j["primaryId"].empty() || j["displayList"].empty() || !j["displayList"].is_array()) {
		throw std::exception("Invalid config"); //TODO throw exception?
	}

	name = j["name"].get<std::string>();
	primaryId = j["primaryId"].get<unsigned long>();
	displayList.clear();
	for (json display : j["displayList"]) {
		displayList.push_back(DisplayParameters(display));
	}
}

bool GlobalConfig::isDisplayInConfig(DisplayParameters& disp) {
	for (auto& d : displayList) {
		if (disp.displayId == d.displayId) {
			return true;
		}
	}
	return false;
}

boost::optional<DisplayParameters> GlobalConfig::getDisplayFromId(unsigned long ref_id) {
	for (auto& d : displayList) {
		if (d.displayId == ref_id) {
			return d;
		}
	}
	return boost::none;
}
//REL_POS
//0 - UP
//1 - DOWN
//2 - LEFT
//3 - RIGHT
//ALIGN
//0 - UP/LEFT
//1 - BOTTOM/RIGHT
//2 - CENTER
bool GlobalConfig::addDisplaytoConfig(DisplayParameters disp, unsigned long ref_id, int rel_pos, int align) {
	if (isDisplayInConfig(disp) || rel_pos < 0 || rel_pos > 3 || align < 0 || align > 2) {
		return false;
	}

	//If it's the first display
	if (displayList.empty()) {
		disp.posX = 0;
		disp.posY = 0;
		displayList.push_back(disp);
		return true;
	}
	boost::optional<DisplayParameters> ref_opt = getDisplayFromId(ref_id);
	if (!ref_opt.has_value()) {
		throw std::exception("reference display should be in config");
	}

	DisplayParameters ref = ref_opt.get();

	//If it's up or down
	if (rel_pos == 0 || rel_pos == 1) {
		//Horizontal alignement
		if (align == 0) {
			disp.posX = ref.posX;
		}
		else if (align == 1) {
			disp.posX = ref.posX + ref.virtual_width - disp.virtual_width;
		}
		else if (align == 2) {
			disp.posX = ref.posX + (ref.virtual_width - disp.virtual_width) / 2;
		}
		//Vertical coordinates 
		if (rel_pos == 0) {
			disp.posY = ref.posY - disp.virtual_height;
		}
		else if (rel_pos == 1) {
			disp.posY = ref.posY + ref.virtual_height;
		}
	}

	//left or right
	if (rel_pos == 2 || rel_pos == 3) {
		//Vertical aligment
		if (align == 0) {
			disp.posY = ref.posY;
		}
		else if (align == 1) {
			disp.posY = ref.posY + ref.virtual_height - disp.virtual_height;
		}
		else if (align == 2) {
			disp.posY = ref.posY + (ref.virtual_height - disp.virtual_height) / 2;
		}
		//Horizontal coordinates
		if (rel_pos == 2) {
			disp.posX = ref.posX - disp.virtual_width;
		}
		else if (rel_pos == 3) {
			disp.posX = ref.posX + ref.virtual_width;
		}

	}

	//Calculate overlaps
	int x1_start, x1_end, y1_start, y1_end;
	int x2_start, x2_end, y2_start, y2_end;

	x1_start = disp.posX;
	x1_end = disp.posX + disp.virtual_width;
	y1_start = disp.posY;
	y1_end = disp.posY + disp.virtual_height;

	for (auto& d : displayList) {
		x2_start = d.posX;
		x2_end = d.posX + d.virtual_width;
		y2_start = d.posY;
		y2_end = d.posY + d.virtual_height;

		if (x1_end > x2_start && x2_end > x1_start) {
			if (y1_end > y2_start && y2_end > y1_start) {
				std::cout << "Overlap with " << d.name << std::endl;
				return false;
			}
		}
	}

	displayList.push_back(disp);
	return true;
}

GlobalConfig::GlobalConfig() {};
GlobalConfig::GlobalConfig(json j) { populateFromJson(j); }