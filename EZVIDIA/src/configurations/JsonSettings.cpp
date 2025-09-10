#include "JsonSettings.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include "ConfException.hpp"
#include "../utils/StringUtils.hpp"

namespace {
	// Declare the auxiliary functions used and the template
	DisplayConfiguration jsonToDisplayConfig(const nlohmann::json& dcRoot);
	GlobalConfiguration jsonToGlobalConfig(const nlohmann::json& gcRoot);
	nlohmann::json jsonFromDisplayConfig(const DisplayConfiguration& displayConfig);
	nlohmann::json jsonFromGlobalConfig(const GlobalConfiguration& globalConfig);

	template<typename T>
	bool isValueInJson(const nlohmann::json& root, const std::string& name);

	template<typename T>
	inline bool isValueInJson(const nlohmann::json& root, const std::string& name) = delete;

	template<>
	inline bool isValueInJson<uint32_t>(const nlohmann::json& root, const std::string& name) {
		return root.contains(name) && root[name].is_number_unsigned();
	}

	template<>
	inline bool isValueInJson<int32_t>(const nlohmann::json& root, const std::string& name) {
		return root.contains(name) && root[name].is_number();
	}

	template<>
	inline bool isValueInJson<std::string>(const nlohmann::json& root, const std::string& name) {
		return root.contains(name) && root[name].is_string();
	}

	template<>
	inline bool isValueInJson<bool>(const nlohmann::json& root, const std::string& name) {
		return root.contains(name) && root[name].is_boolean();
	}

	template<typename T>
	T getValueFromJson(const nlohmann::json& root, const std::string& name, std::optional<T> defaultValue = std::optional<T>()) {
		if (isValueInJson<T>(root, name)) {
			return root[name].get<T>();
		}
		else if (defaultValue.has_value()) {
			return defaultValue.value();
		}
		else {
			std::wstring errorMsg(L"Missing/invalid \"");
			errorMsg.append(StringUtils::stringToWideString(name));
			errorMsg.append(L"\" parameter.");
			throw ConfException(errorMsg);
		}
	}
}

// Class implementations
JsonSettings::JsonSettings(const std::string& fileName) : fileName(fileName) {
	this->init();
}

bool JsonSettings::persist() {
	std::ofstream out(fileName, std::ios_base::out);

	if (!out) {
		return false;
	}

	nlohmann::json root;
	// Schema version
	root["schema_version"] = 1;
	// Save config list
	root["configList"] = nlohmann::basic_json(nlohmann::detail::value_t::array);

	for (auto& entry : configVector) {
		root["configList"].push_back(jsonFromGlobalConfig(entry));
	}
	// Save other settings
	root["networkTcp"] = networkTcp;

	out << root.dump(3) << std::endl;

	return true;
}

bool JsonSettings::read() {
	std::ifstream in(fileName, std::ios_base::in);

	if (!in) {
		return false;
	}

	nlohmann::json fRoot;
	try {
		in >> fRoot;
		if (!fRoot.is_object()) {
			throw ConfException(L"Configuration file is not a JSON object.");
		}
	}
	catch (std::exception& e) {
		(e); //cmon warning
		throw ConfException(L"Problem reading configuration file.");
	}
	// If no schema present then it should be assumed to be a v1
	int schema_version = getValueFromJson(fRoot, "schema_version", std::optional<int>(1));

	if (schema_version == 1) {
		// Parse config list
		if (fRoot.contains("configList") && fRoot["configList"].is_array()) {
			for (auto& gConfig : fRoot["configList"]) {
				const GlobalConfiguration& conf = jsonToGlobalConfig(gConfig);
				if (std::find_if(configVector.begin(), configVector.end(), [&conf](GlobalConfiguration& gc) {return gc.name == conf.name; }) != configVector.end()) {
					// TODO log duplicate configuration here
					continue;
				}
				configVector.push_back(conf);
			}
		}
		else {
			configVector.clear();
			throw ConfException(L"Missing configList array.");
		}
		// Parse other settings
		networkTcp = getValueFromJson(fRoot, "networkTcp", std::optional<bool>(false));
	}
	else {
		throw ConfException(L"Invalid schema");
	}

	return true;
}

namespace {
	// Auxiliary functions implementation
	GlobalConfiguration jsonToGlobalConfig(const nlohmann::json& gcRoot) {
		GlobalConfiguration globalConf;

		// Name
		globalConf.name = StringUtils::stringToWideString(getValueFromJson<std::string>(gcRoot, "name"));

		// Display List
		if (!gcRoot.contains("displayList") || !gcRoot["displayList"].is_array()) {
			throw ConfException(L"Configuration missing displayList array");
		}

		for (auto& dispRoot : gcRoot["displayList"]) {
			try {
				globalConf.displays.push_back(jsonToDisplayConfig(dispRoot));
			}
			catch (ConfException& e) {
				std::wstring errorMsg(L"Error occured while parsing the displayList for configuration \"");
				errorMsg.append(globalConf.name);
				errorMsg.append(L"\":\n");
				errorMsg.append(e.msg());
				throw ConfException(errorMsg);
			}
		}

		// Clone groups
		// If one of the displays in the configuration had a missing cloneGroup value (which is represented by a UINT32_MAX), we assume extended mode
		if (std::find_if(globalConf.displays.begin(), globalConf.displays.end(),
			[](const DisplayConfiguration& disp) {return disp.cloneGroup == UINT32_MAX; }) != globalConf.displays.end()) {
			uint32_t group = 0;
			for (auto& disp : globalConf.displays) {
				disp.cloneGroup = group++;
			}
		}

		// Primary Group (in configurations with primaryId, we change it to the group containing that id)
		globalConf.primaryGroup = getValueFromJson<uint32_t>(gcRoot, "primaryGroup", std::optional<uint32_t>(UINT32_MAX));
		if (globalConf.primaryGroup == UINT32_MAX) {
			uint32_t primaryId = getValueFromJson<uint32_t>(gcRoot, "primaryId", std::optional<uint32_t>(UINT32_MAX));
			for (auto& disp : globalConf.displays) {
				if (disp.displayId == primaryId) {
					globalConf.primaryGroup = disp.cloneGroup;
					break;
				}
			}
		}
		bool validPrimaryGroup = false;
		for (auto& disp : globalConf.displays) {
			if (disp.cloneGroup == globalConf.primaryGroup) {
				validPrimaryGroup = true;
			}
		}
		if (!validPrimaryGroup) {
			std::wstring errorMsg(L"Error occured while parsing primaryGroup/Id for configuration \"");
			errorMsg.append(globalConf.name);
			errorMsg.append(L"\":\n");
			errorMsg.append(L"Provided primaryGroup/Id doesn't match any of the cloneGroups/displayIds in DisplayList");
			throw ConfException(errorMsg);
		}

		return globalConf;
	}

	nlohmann::json jsonFromGlobalConfig(const GlobalConfiguration& globalConfig) {
		nlohmann::json gcRoot;

		gcRoot["name"] = StringUtils::wideStringToString(globalConfig.name);
		gcRoot["primaryGroup"] = globalConfig.primaryGroup;

		for (auto& displayConf : globalConfig.displays) {
			gcRoot["displayList"].push_back(jsonFromDisplayConfig(displayConf));
		}

		return gcRoot;
	}

	DisplayConfiguration jsonToDisplayConfig(const nlohmann::json& dcRoot) {
		DisplayConfiguration dispConf;
		// Some parameters HAVE to be in the configuration file
		dispConf.height = getValueFromJson<uint32_t>(dcRoot, "height");
		dispConf.width = getValueFromJson<uint32_t>(dcRoot, "width");
		dispConf.posX = getValueFromJson<int32_t>(dcRoot, "posX");
		dispConf.posY = getValueFromJson<int32_t>(dcRoot, "posY");
		dispConf.colorDepth = getValueFromJson<uint32_t>(dcRoot, "colorDepth");
		dispConf.refresh = getValueFromJson<uint32_t>(dcRoot, "refresh");
		dispConf.rotation = getValueFromJson<uint32_t>(dcRoot, "rotation");
		dispConf.displayId = getValueFromJson<uint32_t>(dcRoot, "displayId");

		// Other parameters are optional and have a default value (for compatibility with legacy config files)
		dispConf.scaling = getValueFromJson<uint32_t>(dcRoot, "scaling", std::optional<uint32_t>(0));
		dispConf.tvFormat = getValueFromJson<uint32_t>(dcRoot, "tvFormat", std::optional<uint32_t>(0));
		dispConf.cloneGroup = getValueFromJson<uint32_t>(dcRoot, "cloneGroup", std::optional<uint32_t>(UINT32_MAX));

		return dispConf;
	}

	nlohmann::json jsonFromDisplayConfig(const DisplayConfiguration& displayConfig) {
		nlohmann::json dcRoot;

		dcRoot["height"] = displayConfig.height;
		dcRoot["width"] = displayConfig.width;
		dcRoot["posX"] = displayConfig.posX;
		dcRoot["posY"] = displayConfig.posY;
		dcRoot["colorDepth"] = displayConfig.colorDepth;
		dcRoot["refresh"] = displayConfig.refresh;
		dcRoot["rotation"] = displayConfig.rotation;
		dcRoot["scaling"] = displayConfig.scaling;
		dcRoot["tvFormat"] = displayConfig.tvFormat;
		dcRoot["displayId"] = displayConfig.displayId;
		dcRoot["cloneGroup"] = displayConfig.cloneGroup;

		return dcRoot;
	}
}