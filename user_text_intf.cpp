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

#include "user_text_intf.hpp"
#include <iostream>
#include <string>
#include <vector>
#include "readable_config.hpp"

void screenSpace() { std::cout << "\n\n"; }

int getUserInputRange(std::string prompt, int start, int end, int& input, bool skip) {

	std::string in;
	if (!skip) {
		std::cout << prompt;
	}

	while (true) {
		std::getline(std::cin, in);
		//Get the integer
		try {
			input = std::stoi(in);
		}
		catch (const std::exception& e) {
			std::cout << "Could not parse a number. Please try again: ";
			continue;
		}
		//Check if it is in the range
		if (input < start || input > end) {
			std::cout << "Option does not exist. Please try again: ";
			continue;
		}
		//It's good
		return 0;
	}
}

int getUserInputAllowed(std::string prompt, std::vector<const char*> allowed, std::string& input, bool skip) {

	std::string in;
	if (!skip) {
		std::cout << prompt;
	}

	while (true) {
		std::getline(std::cin, in);

		//Check if it is in the allowed array
		for (const char*& str : allowed) {
			if (!std::strcmp(in.c_str(), str)) {
				input = in;
				return 0;
			}
		}

		std::cout << "Invalid input. Please try again: ";
	}
}

bool getUserInputYesOrNo(std::string prompt, bool skip) {

	std::string in;
	if (!skip) {
		std::cout << prompt;
	}
	std::vector<const char*> yArray{ "Y","y","yes" }, nArray{ "N","n","no" };

	while (true) {
		std::getline(std::cin, in);

		//Check if it is in the yes array
		for (const char*& str : yArray) {
			if (!std::strcmp(in.c_str(), str)) {
				return true;
			}
		}

		//Check if it is in the no array
		for (const char*& str : nArray) {
			if (!std::strcmp(in.c_str(), str)) {
				return false;
			}
		}

		std::cout << "Invalid input. Please try again: ";
	}
}

int getUserInputNum(std::string prompt, bool skip) {

	std::string in;
	int input;
	if (!skip) {
		std::cout << prompt;
	}

	while (true) {
		std::getline(std::cin, in);
		//Get the integer
		try {
			input = std::stoi(in);
		}
		catch (const std::exception& e) {
			std::cout << "Could not parse a number. Please try again: ";
			continue;
		}
		//It's good
		return input;
	}
}

float getUserInputFloat(std::string prompt, bool skip) {

	std::string in;
	float input;
	if (!skip) {
		std::cout << prompt;
	}

	while (true) {
		std::getline(std::cin, in);
		//Get the integer
		try {
			input = std::stof(in);
		}
		catch (const std::exception& e) {
			std::cout << "Could not parse a number. Please try again: ";
			continue;
		}
		//It's good
		return input;
	}
}

int showMenu(std::vector<std::string>& options, bool top) {

	int i = 0, option = 0;
	std::cout << "\nYour current options are:\n";
	for (auto& s : options) {
		std::cout << " " << ++i << ". " << s << std::endl;
	}
	std::cout << (top ? " 0. Exit\n" : " 0. Return\n");
	getUserInputRange("Insert a number: ", 0, options.size(), option);
	return option;
}

void printConfigs(std::vector<GlobalConfig>& list) {
	int i = 0;
	std::cout << '\n';
	for (auto& c : list) {
		std::cout << " " << ++i << ". " << c.name << " (" << c.displayList.size() << " " << (c.displayList.size() == 1 ? "display)\n" : "displays)\n");
	}
	std::cout << '\n';
}

void printConfigDetails(GlobalConfig& conf) {
	int i = 0;
	std::cout << '\n';
	for (auto& d : conf.displayList) {
		std::cout << " " << ++i << ". " << d.name << " (" << d.width << "x" << d.height << "@" << float(d.refresh) / 1000 << ")";
		std::cout << " ([" << d.posX << ", " << d.posX + d.width << "], [" << d.posY << ", " << d.posY + d.height << "])" << std::endl;
	}
	std::cout << '\n';
}

void printDisplayList(std::vector<DisplayParameters>& list) {

	int i = 0;
	std::cout << '\n';
	for (auto& d : list) {
		std::cout << " " << ++i << ". \"" << d.name << "\" (" << d.width << "x" << d.height << "@" << float(d.refresh) / 1000 << ")" << std::endl;
	}
	std::cout << '\n';
}

bool identifyDisplaysByPosition(std::vector<DisplayParameters>& list) {

	std::sort(list.begin(), list.end(), DisplayParameters::comparePosX);

	std::cout << "Number of detected displays: " << list.size() << std::endl;
	std::cout << "Now we will go over the displays from LEFT to RIGHT (according to the positions in the current configuration)." << std::endl;
	std::cout << "Insert a name to identify each monitor." << std::endl << std::endl;

	for (int i = 0; i < list.size(); i++) {
		DisplayParameters& disp = list.at(i);
		disp.name = "";
		std::cout << "Monitor " << i + 1 << " of " << list.size() << " ";
		std::cout << "(" << disp.width << "x" << disp.height << "@" << float(disp.refresh) / 1000 << ")" << std::endl;
		while (disp.name.length() < 1) {
			std::cout << "Insert a name: ";
			getline(std::cin, disp.name);
		}
	}

	return 0;
}

void manageDisplayPresets(std::vector<DisplayParameters>& list) {

	int option = 0;
	std::string prompt_str;
	std::vector<std::string> menu_options{ "List current presets", "Add preset", "Delete preset (experimental)" };
	do {
		option = showMenu(menu_options);
		if (option == 1) {
			printDisplayList(list);
		}
		else if (option == 2) {
			int disp_num = 0;
			std::cout << "The current presets are: " << std::endl;
			printDisplayList(list);
			prompt_str = "Indicate the display you wish to add a new preset for (0 to exit): ";
			getUserInputRange(prompt_str, 0, list.size(), disp_num);

			if (disp_num == 0) { continue; }

			DisplayParameters new_disp(list.at((size_t)disp_num - 1));
			std::cout << "\nNow you need to select the resolution and refresh rate for the new preset.\n";
			std::cout << "The values inserted will not be validated so be dumb at your own risk.\n";

			prompt_str = "\nInsert the resolution width (the 1920 in 1920x1080): ";
			new_disp.width = getUserInputNum(prompt_str);
			prompt_str = "\nInsert the resolution height (the 1080 in 1920x1080): ";
			new_disp.height = getUserInputNum(prompt_str);
			prompt_str = "\nInsert the refresh rate (in Hz, up to 3 decimal places): ";
			new_disp.refresh = 1000 * getUserInputFloat(prompt_str); //Because we store a int that represents 1000*Hz

			list.push_back(new_disp);
			std::cout << "\nNew preset added.\n";
		}
		else if (option == 3) {
			if (list.size() < 2) {
				std::cout << "Can't delete anymore presets.\n";
				continue;
			}
			int disp_num = 0;
			std::cout << "WARNING: If you delete all the presets for a display you will need to perform the initial setup again.\n";
			std::cout << "The current presets are: " << std::endl;
			printDisplayList(list);
			prompt_str = "Indicate the preset you want to delete: ";
			getUserInputRange(prompt_str, 0, list.size(), disp_num);

			if (disp_num == 0) { continue; }

			prompt_str = "Are you sure you want to delete preset " + std::to_string(disp_num) + " ? (y/n)\n";
			if (getUserInputYesOrNo(prompt_str)) {
				list.erase(list.begin() + disp_num - 1);
			}
		}

	} while (option != 0);
}

void createConfig(std::vector<GlobalConfig>& conf_list, std::vector<DisplayParameters>& disp_list) {

	int option = 0;
	std::string prompt_str;
	GlobalConfig conf;

	//Name the configuration
	conf.name = "";
	while (conf.name.length() < 1) {
		std::cout << "Insert a name for this configuration: ";
		getline(std::cin, conf.name);
		for (auto& c : conf_list) {
			if (!conf.name.compare(c.name)) {
				std::cout << "Configuration with that name already exists.\n";
				conf.name = "";
				break;
			}
		}
	}

	//Get the primary display
	DisplayParameters preset;
	int disp_num = 0;
	unsigned long ref_id = 0;
	std::cout << "\nYour presets are:\n";
	printDisplayList(disp_list);
	prompt_str = "Select the preset for the primary display in this configuration (0 to cancel): ";
	getUserInputRange(prompt_str, 0, disp_list.size(), disp_num);
	if (disp_num == 0) return;
	preset = disp_list.at(disp_num - 1);
	if (!conf.addDisplaytoConfig(preset, ref_id, 0, 0)) {
		std::cout << "Problem adding main display to config.\n";
		return;
	}
	conf.primaryId = preset.displayId;

	//Add more displays
	std::vector<DisplayParameters> allowed_presets;
	int rel_pos = 0, align = 0, i = 0, ref_num = 0;
	while (true) {
		//Check which presets we can add
		allowed_presets.clear();
		for (auto& d : disp_list) {
			if (!conf.isDisplayInConfig(d)) {
				allowed_presets.push_back(d);
			}
		}
		if (allowed_presets.empty()) break;
		screenSpace();
		std::cout << "You can now add more displays to the configuration.";
		printDisplayList(allowed_presets);
		prompt_str = "Select the preset you wish to add (0 to finish): ";
		getUserInputRange(prompt_str, 0, allowed_presets.size(), disp_num);
		if (disp_num == 0) break;
		preset = allowed_presets.at(disp_num - 1);

		//Ref display
		screenSpace();
		std::cout << "Select a display which will be adjacent to the display you are adding (so you can pick its position).\n";
		std::cout << "Each display shows its X/Y coordinates, note that the Y coordinate grows downwards.\n";
		printConfigDetails(conf);
		prompt_str = "Pick the adjacent display (0 to cancel): ";
		getUserInputRange(prompt_str, 0, conf.displayList.size(), ref_num);
		if (ref_num == 0) continue;
		DisplayParameters& ref_disp = conf.displayList.at(ref_num - 1);

		//relative position
		screenSpace();
		std::cout << "Pick a position relative to \"" << ref_disp.name << "\".\n\n 1. Up\n 2. Down\n 3. Left\n 4. Right\n\n";
		prompt_str = "Insert a number (0 to cancel): ";
		getUserInputRange(prompt_str, 0, 4, rel_pos);
		if (rel_pos == 0) continue;
		rel_pos--;

		//aligment
		if ((rel_pos == 0 || rel_pos == 1) && ref_disp.width != preset.width) {
			screenSpace();
			std::cout << "The selected displays don't have matching horizontal resolutions. It is necessary to choose an aligment.\n";
			std::cout << " 1. Align the left edges\n 2. Align the right edges\n 3. Align centrally\n";
			prompt_str = "Pick an aligment (0 to cancel): ";
			getUserInputRange(prompt_str, 0, 3, align);
			if (align == 0) continue;
			align--;
		}
		else if ((rel_pos == 2 || rel_pos == 3) && ref_disp.height != preset.height) {
			screenSpace();
			std::cout << "The selected displays don't have matching vertical resolutions. It is necessary to choose an aligment.\n";
			std::cout << " 1. Align the upper edges\n 2. Align the bottom edges\n 3. Align centrally\n";
			prompt_str = "Pick an aligment (0 to cancel): ";
			getUserInputRange(prompt_str, 0, 3, align);
			if (align == 0) continue;
			align--;
		}

		if (!conf.addDisplaytoConfig(preset, ref_disp.displayId, rel_pos, align)) {
			std::cout << "ERROR adding display to configuration.\n";
		}
	}

	screenSpace();
	std::cout << "Configuration \"" << conf.name << "\" created.\n";
	conf_list.push_back(conf);
}

void manageConfigurations(std::vector<GlobalConfig>& conf_list, std::vector<DisplayParameters>& disp_list) {

	int option = 0;
	std::string prompt_str;
	std::vector<std::string> menu_options{ "List current configurations", "Add configuration", "Delete configuration" };
	do {
		option = showMenu(menu_options);
		if (option == 1) {
			int conf_num = 0;
			printConfigs(conf_list);
			while (true) {
				prompt_str = "Select a configuration if you want more details (0 to return): ";
				getUserInputRange(prompt_str, 0, conf_list.size(), conf_num);
				if (conf_num == 0) break;
				printConfigDetails(conf_list.at(conf_num - 1));
			}
		}
		else if (option == 2) {
			createConfig(conf_list, disp_list);
		}
		else if (option == 3) {
			screenSpace();
			if (conf_list.size() < 2) {
				std::cout << "Can't delete the last configuration.\n";
				continue;
			}
			int conf_num = 0;
			std::cout << "The current configurations are: " << std::endl;
			printConfigs(conf_list);
			prompt_str = "Indicate the configuration you want to delete (0 to cancel): ";
			getUserInputRange(prompt_str, 0, conf_list.size(), conf_num);
			if (conf_num == 0) { continue; }
			prompt_str = "Are you sure you want to delete configuration " + std::to_string(conf_num) + " ? (y/n): ";
			if (getUserInputYesOrNo(prompt_str)) {
				conf_list.erase(conf_list.begin() + conf_num - 1);
			}
		}

	} while (option != 0);
}