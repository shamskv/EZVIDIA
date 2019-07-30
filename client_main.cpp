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

#include <iostream>
#include <fstream>
#include <string>
#include "readable_config.hpp"
#include "nvapi_controller.hpp"
#include "user_text_intf.hpp"
#include <json.hpp>

bool NVAPIController::isInit = false;
std::string confName = "configs.json";
std::vector<GlobalConfig> configList;
std::vector<DisplayParameters> displayList;

int loadConfigFile() {

	std::ifstream myfilein;
	myfilein.open(confName);
	if (!myfilein) {
		return -1;
	}

	json proot;
	myfilein >> proot;

	try {
		if (proot["configList"].is_array()) {
			for (auto& config : proot["configList"]) {
				configList.push_back(GlobalConfig(config));
			}
		}

		if (proot["presetList"].is_array()) {
			for (auto& pre : proot["presetList"]) {
				displayList.push_back(DisplayParameters(pre));
			}
		}
	}
	catch(std::exception &e){
		configList.clear();
		displayList.clear();
		myfilein.close();
		return -2;
	}

	myfilein.close();
	return 0;
}

int saveConfigFile() {

	json root;

	root["configList"].array();
	for (auto& conf : configList) {
		root["configList"].push_back(conf.mapToJson());
	}

	root["presetList"].array();
	for (auto& pre : displayList) {
		root["presetList"].push_back(pre.mapToJson());
	}

	std::ofstream myfileout;
	myfileout.open(confName);
	if (!myfileout) {
		return -1;
	}
	myfileout << root.dump(3) << std::endl;
	myfileout.close();

	return 0;
}

int generateBatFiles(bool test = false) {
	if (configList.empty()) {
		return -1;
	}

	std::ofstream fileout;
	for (GlobalConfig& c : configList) {
		fileout.open((test? "EZVIDIA_TEST " : "EZVIDIA ") + c.name + ".bat");
		if (!fileout) {
			std::cout << "Failed to open batch file for " << c.name << std::endl;
			fileout.close();
			return -1;
		}
		fileout << "EZVIDIA.exe \"" << c.name << (test? "\" test" : "\"") << std::endl;
		fileout.close();
	}

	return 0;
}

int main(int argc, char** argv) {

	std::string str_input, prompt_str;
	std::vector<std::string> menu_options{ "Initial setup (warning: this will delete your current config file)",
		"Manage configurations", "Manage presets", "Create batch files", "Create test batch files (configurations are not applied permanently)" };
	int int_input, ret, option = 0;

	std::cout << "Looking for configs.json file in current directory..." << std::endl;
	ret = loadConfigFile();
	if (ret == 0) {
		std::cout << "Configurations loaded successfully." << std::endl;
	}
	else if (ret == -1) {
		std::cout << "Configuration file not found." << std::endl;
	}
	else if (ret == -2) {
		std::cout << "Configuration file with invalid syntax." << std::endl;
	}

	if (argc == 1) {
		std::cout << "\nWelcome to EZVIDIA!" << std::endl;
		do {
			option = showMenu(menu_options, true);
			if (option == 1) {

				if (configList.size() > 0 || displayList.size() > 0) {
					prompt_str = "Are you sure you want to run the initial setup? Your current configs will be deleted.\n";
					prompt_str += "Type yes to continue (y/n): ";
					if (!getUserInputYesOrNo(prompt_str)) {
						continue;
					}
				}

				configList.clear();
				displayList.clear();

				screenSpace();
				prompt_str = "Please set your current display configuration so that all the monitors you wish to use are active and side-by-side.\n";
				prompt_str += "So if have 4 monitors overall, turn them all on and place them on a 4x1 grid. Do NOT clone any of the displays.\n";
				prompt_str += "This will help identifying your monitors, since we haven't found a way to get model/manufacturer info.\n";
				prompt_str += "The resolutions and refresh rates in the current configuration will be the ones used by default.\n";
				prompt_str += "Type yes after you have done this (y/n): ";
				bool asked = false;
				while (!getUserInputYesOrNo(prompt_str, asked)) {
					std::cout << "\nType yes when you wish to continue (y/n): ";
					asked = true;
				}
				ret = NVAPIController::getDisplayList(displayList);
				if (ret != 0) {
					screenSpace();
					std::cout << "Problem getting display information, code: " << ret << std::endl;
					std::cout << "Press Enter to exit...";
					getchar();
					return -1;
				}
				screenSpace();
				identifyDisplaysByPosition(displayList);

				screenSpace();
				prompt_str = "Will any of the displays use different resolution/refresh rate modes between configurations?\n";
				prompt_str += "For example, you may wish to run the same display at 1080p@120Hz in one configuration and 4k@60Hz in another.\n";
				prompt_str += "To achieve this functionality you will need to configure a new preset for that display.\n";
				prompt_str += "Your current presets are:\n";
				std::cout << prompt_str;
				printDisplayList(displayList);
				prompt_str = "Type yes if you wish to add new presets (y/n): ";
				if (getUserInputYesOrNo(prompt_str)) {
					manageDisplayPresets(displayList);
				}

				GlobalConfig conf;
				conf.name = "DEFAULT_CONFIG";
				unsigned long prev = 0;
				for (auto& d : displayList) {
					if (conf.addDisplaytoConfig(d, prev, 3, 1)) {
						prev = d.displayId;
					}
				}
				conf.primaryId = prev;
				configList.push_back(conf);

				screenSpace();
				std::cout << "The initial setup is done. You should now create new configurations from the \"Manage configurations\" option in the menu.";
				std::cout << "We have added a default configuration for technical purposes. ";
				std::cout << "You can delete this configuration after adding a new one.\n";
				std::cout << "Press Enter to create the new configuration file and return to the menu.\n";
				fflush(stdin);
				getchar();
				fflush(stdin);
				if (!saveConfigFile()) {
					std::cout << "Configuration file created succesfully.\n";
				}
				else {
					std::cout << "Failed to create file (could not open " << confName << " for write operation).\n";
				}

			}
			else if (option == 2) {
				if (configList.size() > 0) {
					manageConfigurations(configList, displayList);
					if (!saveConfigFile()) {
						std::cout << "\nChanges saved succesfully.\n";
					}
					else {
						std::cout << "\nFailed to save changes (could not open " << confName << " for write operation).\n";
					}
				}
				else {
					std::cout << "\nNo configurations loaded. Perform the initial setup.\n";
				}
			}
			else if (option == 3) {
				if (displayList.size() > 0) {
					manageDisplayPresets(displayList);
					if (!saveConfigFile()) {
						std::cout << "\nChanges saved succesfully.\n";
					}
					else {
						std::cout << "\nFailed to save changes (could not open " << confName << " for write operation).\n";
					}
				}
				else {
					std::cout << "\nNo presets loaded. Perform the initial setup.\n";
				}
			}
			else if (option == 4) {
				if (configList.size() > 0) {
					if (!generateBatFiles()) {
						std::cout << "Batch files successfully generated.\n";
					}
					else {
						std::cout << "Problem generating batch files.\n";

					}
				}
				else {
					std::cout << "\nNo configurations loaded. Perform the initial setup.\n";
				}
			}
			else if (option == 5) {
				if (configList.size() > 0) {
					if (!generateBatFiles(true)) {
						std::cout << "Test batch files successfully generated.\n";
					}
				}
				else {
					std::cout << "\nNo configurations loaded. Perform the initial setup.\n";
				}
			}
		} while (option != 0);
	}
	else if (argc == 2) {
		//If we loaded configurations we try to apply the asked conf
		if (!configList.empty()) {

			bool applied = false;
			int ret;
			std::cout << "Applying configuration " << argv[1] << "..." << std::endl;
			for (GlobalConfig& conf : configList) {
				if (!strcmp(conf.name.c_str(), argv[1])) {
					ret = NVAPIController::applyGlobalConfig(conf);
					applied = true;
					if (ret) {
						std::cout << "Problem applying configuration, code: " << ret << std::endl;
						std::cout << "Press Enter to exit...";
						getchar();
						return -1;
					}
					break;
				}
			}
			if (!applied) {
				std::cout << "Could not find configuration " << argv[1] << std::endl;
				std::cout << "Press Enter to exit...";
				getchar();
				return -1;
			}
		}
		else {
			std::cout << "Trying to apply configuration but no configurations were loaded (no config file/error in config file)." << std::endl;
			std::cout << "Press Enter to exit...";
			getchar();
			return -1;
		}
	}
	else if (argc == 3) {
		if (strcmp(argv[2], "test")) {
			std::cout << "Invalid arguments." << std::endl;
			std::cout << "Press Enter to exit...";
			getchar();
			return -1;
		}
		if (!configList.empty()) {

			bool applied = false;
			int ret;
			std::cout << "Applying configuration (test mode)..." << argv[1] << "..." << std::endl;
			for (GlobalConfig& conf : configList) {
				if (!strcmp(conf.name.c_str(), argv[1])) {
					ret = NVAPIController::applyGlobalConfig(conf, true);
					applied = true;
					if (ret) {
						std::cout << "Problem applying configuration, code: " << ret << std::endl;
						std::cout << "Press Enter to exit...";
						getchar();
						return -1;
					}
					break;
				}
			}
			if (!applied) {
				std::cout << "Could not find configuration " << argv[1] << std::endl;
				std::cout << "Press Enter to exit...";
				getchar();
				return -1;
			}
		}
		else {
			std::cout << "Trying to apply configuration but no configurations were loaded (no config file/error in config file)." << std::endl;
			std::cout << "Press Enter to exit...";
			getchar();
			return -1;
		}
	}
	else {
		std::cout << "Invalid arguments." << std::endl;
		std::cout << "Press Enter to exit...";
		getchar();
		return -1;
	}
	return 0;
}