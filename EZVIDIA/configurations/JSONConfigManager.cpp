#include<json.hpp>
#include<fstream>
#include "../include/configurations/JSONConfigManager.hpp"

bool JSONConfigManager::persist() {
	return false;
}

bool JSONConfigManager::read() {
	std::ifstream in(fileName, std::ios_base::in);

	if (!in) {
		return false;
	}

	nlohmann::json fRoot;
	in >> fRoot;
	if (fRoot.contains("configList") && fRoot["configList"].is_array()) {
		for (auto& gConfig : fRoot["configList"]) {
			const GlobalConfiguration& conf = jsonToGlobalConfiguration(gConfig);
			if (configMap.find(conf.name) != configMap.end()) {
				// TODO log duplicate configuration here
				continue;
			}
			configMap.insert(std::pair<std::string, GlobalConfiguration>(conf.name, conf));
		}
	}
	else {
		configMap.clear();
		throw std::exception("Missing configList array.");
	}
	return true;
}

GlobalConfiguration JSONConfigManager::jsonToGlobalConfiguration(const nlohmann::json& gcroot) {
	GlobalConfiguration globalConf;

	// Name
	globalConf.name = getValueFromJson<std::string>(gcroot, "name");

	// Display List
	if (!gcroot.contains("displayList") || !gcroot["displayList"].is_array()) {
		throw std::exception("Configuration missing displayList array");
	}

	for (auto& dispRoot : gcroot["displayList"]) {
		try {
			globalConf.displays.push_back(jsonToDisplayConfiguration(dispRoot));
		}
		catch (std::exception& e) {
			std::string errorMsg("Error occured while parsing the displayList for configuration \"");
			errorMsg.append(globalConf.name);
			errorMsg.append("\":\n");
			errorMsg.append(e.what());
			throw std::exception(errorMsg.c_str());
		}
	}

	// Primary display
	globalConf.primaryDisplay = getValueFromJson<uint32_t>(gcroot, "primaryId");
	bool validPrimaryId = false;
	for (auto& disp : globalConf.displays) {
		if (std::find(disp.displayIds.begin(), disp.displayIds.end(), globalConf.primaryDisplay) != disp.displayIds.end()) {
			validPrimaryId = true;
		}
	}
	if (!validPrimaryId) {
		std::string errorMsg("Error occured while parsing primaryId for configuration \"");
		errorMsg.append(globalConf.name);
		errorMsg.append("\":\n");
		errorMsg.append("Provided primaryId doesn't match with any of the displays in displayList");
		throw std::exception(errorMsg.c_str());
	}

	return globalConf;
}

DisplayConfiguration JSONConfigManager::jsonToDisplayConfiguration(const nlohmann::json& dcroot) {
	DisplayConfiguration dispConf;
	// Some parameters HAVE to be in the configuration file
	dispConf.height = getValueFromJson<uint32_t>(dcroot, "height");
	dispConf.width = getValueFromJson<uint32_t>(dcroot, "width");
	dispConf.posX = getValueFromJson<int32_t>(dcroot, "posX");
	dispConf.posY = getValueFromJson<int32_t>(dcroot, "posY");
	dispConf.colorDepth = getValueFromJson<uint32_t>(dcroot, "colorDepth");
	dispConf.refresh = getValueFromJson<uint32_t>(dcroot, "refresh");
	dispConf.rotation = getValueFromJson<uint32_t>(dcroot, "refresh");

	// Other parameters are optional and have a default value (for compatibility with legacy config files)
	dispConf.scaling = getValueFromJson<uint32_t>(dcroot, "scaling", std::optional<uint32_t>(0));
	dispConf.tvFormat = getValueFromJson<uint32_t>(dcroot, "tvFormat", std::optional<uint32_t>(0));

	// DisplayIds should be in a JSON array or in a "displayId" object if dealing with a legacy configuration
	if (dcroot.contains("displayIds") && dcroot["displayIds"].is_array() && !dcroot["displayIds"].empty()) {
		for (auto& id : dcroot["displayIds"]) {
			if (!id.is_number_unsigned()) {
				throw std::exception("Invalid element in displayIds array");
			}
			dispConf.displayIds.push_back(id);
		}
	}
	else if (dcroot.contains("displayId") && dcroot["displayId"].is_number_unsigned()) {
		dispConf.displayIds.push_back(dcroot["displayId"].get<uint32_t>());
	}
	else {
		throw std::exception("Missing/invalid displayId info");
	}

	return dispConf;
}

JSONConfigManager::JSONConfigManager(const std::string& fileName) : fileName(fileName) {}