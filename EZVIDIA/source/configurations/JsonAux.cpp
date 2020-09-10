#include "../../include/configurations/JsonAux.hpp"
#include "../../include/configurations/ConfException.hpp"
#include "../../include/utils/StringUtils.hpp"

#include<optional>

// Declare the auxiliary functions used and the template
namespace {
	DisplayConfiguration JsonToDisplayConfig(const nlohmann::json& dcRoot);
	nlohmann::json JsonFromDisplayConfig(const DisplayConfiguration& displayConfig);
	template<typename T>
	T getValueFromJson(const nlohmann::json& root, const std::string& name, const std::optional<T> defaultValue = std::optional<T>());

	template<typename T>
	inline T getValueFromJson(const nlohmann::json& root, const std::string& name, const std::optional<T> defaultValue) = delete;

	template<>
	inline uint32_t getValueFromJson<uint32_t>(const nlohmann::json& root, const std::string& name, const std::optional<uint32_t> defaultValue) {
		if (root.contains(name) && root[name].is_number_unsigned()) {
			return root[name].get<uint32_t>();
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

	template<>
	inline int32_t getValueFromJson<int32_t>(const nlohmann::json& root, const std::string& name, const std::optional<int32_t> defaultValue) {
		if (root.contains(name) && root[name].is_number_integer()) {
			return root[name].get<int32_t>();
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

	template<>
	inline std::string getValueFromJson<std::string>(const nlohmann::json& root, const std::string& name, const std::optional<std::string> defaultValue) {
		if (root.contains(name) && root[name].is_string()) {
			return root[name].get<std::string>();
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
};

GlobalConfiguration JsonAux::JsonToGlobalConfig(const nlohmann::json& gcRoot) {
	GlobalConfiguration globalConf;

	// Name
	globalConf.name = StringUtils::stringToWideString(getValueFromJson<std::string>(gcRoot, "name"));

	// Display List
	if (!gcRoot.contains("displayList") || !gcRoot["displayList"].is_array()) {
		throw ConfException(L"Configuration missing displayList array");
	}

	for (auto& dispRoot : gcRoot["displayList"]) {
		try {
			globalConf.displays.push_back(JsonToDisplayConfig(dispRoot));
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

nlohmann::json JsonAux::JsonFromGlobalConfig(const GlobalConfiguration& globalConfig) {
	nlohmann::json gcRoot;

	gcRoot["name"] = StringUtils::wideStringToString(globalConfig.name);
	gcRoot["primaryGroup"] = globalConfig.primaryGroup;

	for (auto& displayConf : globalConfig.displays) {
		gcRoot["displayList"].push_back(JsonFromDisplayConfig(displayConf));
	}

	return gcRoot;
}

namespace {
	DisplayConfiguration JsonToDisplayConfig(const nlohmann::json& dcRoot) {
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

	nlohmann::json JsonFromDisplayConfig(const DisplayConfiguration& displayConfig) {
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