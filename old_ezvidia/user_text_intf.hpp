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
#include <string>
#include <vector>
#include "readable_config.hpp"

void screenSpace();

int getUserInputRange(std::string prompt, int start, int end, int& input, bool skip = false);

int getUserInputAllowed(std::string prompt, std::vector<const char*> allowed, std::string& input, bool skip = false);

bool getUserInputYesOrNo(std::string prompt, bool skip = false);

int getUserInputNum(std::string prompt, bool skip = false);

float getUserInputFloat(std::string prompt, bool skip = false);

int showMenu(std::vector<std::string>& options, bool top = false);

void printConfigs(std::vector<GlobalConfig>& list);

void printConfigDetails(GlobalConfig& conf);

void printDisplayList(std::vector<DisplayParameters>& list);

bool identifyDisplaysByPosition(std::vector<DisplayParameters>& list);

void manageDisplayPresets(std::vector<DisplayParameters>& list);

void createConfig(std::vector<GlobalConfig>& conf_list, std::vector<DisplayParameters>& disp_list);

void manageConfigurations(std::vector<GlobalConfig>& conf_list, std::vector<DisplayParameters>& disp_list);