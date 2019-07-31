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

#pragma once
#include <json.hpp>
#include <string>
#include <boost/optional/optional.hpp>
#include <Windows.h>
#include <nvapi.h>

using nlohmann::json;

class DisplayParameters {
public:
	std::string name;
	unsigned long displayId;
		
	int height; //resolution
	int width;

	int colorDepth;
		
	int refresh; //refresh rate

	int posX;	//position of bottom-left corner
	int posY;

	int rotation;
	int virtual_height;
	int virtual_width;

	json mapToJson();

	NV_DISPLAYCONFIG_PATH_INFO* toNVAPIPathInfo();

	void populateFromJson(json j);

	static bool comparePosX(DisplayParameters& i, DisplayParameters& j) {
		return i.posX < j.posX;
	}

	DisplayParameters();
	DisplayParameters(std::string n, unsigned long dI, int h, int w, int cD, int r, int pX, int pY, int rotation);
	DisplayParameters(json j);
	
};

class GlobalConfig {
public:
	std::string name;
	unsigned long primaryId;
	std::vector<DisplayParameters> displayList;

	json mapToJson();

	void populateFromJson(json j);

	bool isDisplayInConfig(DisplayParameters& disp);

	boost::optional<DisplayParameters> getDisplayFromId(unsigned long ref_id);

	bool addDisplaytoConfig(DisplayParameters disp, unsigned long ref_id, int rel_pos, int align);

	GlobalConfig();
	GlobalConfig(json j);
};