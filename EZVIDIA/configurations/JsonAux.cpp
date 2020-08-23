#include "..\include\configurations\JsonAux.hpp"
#include "..\include\configurations\confException.hpp"
#include <Windows.h>

GlobalConfiguration JsonAux::toGlobalConfig(const nlohmann::json& gcRoot) {
	GlobalConfiguration globalConf;

	// Name
	globalConf.name = JsonAux::stringToWideString(getValueFromJson<std::string>(gcRoot, "name"));

	// Display List
	if (!gcRoot.contains("displayList") || !gcRoot["displayList"].is_array()) {
		throw ConfException(L"Configuration missing displayList array");
	}

	for (auto& dispRoot : gcRoot["displayList"]) {
		try {
			globalConf.displays.push_back(JsonAux::toDisplayConfig(dispRoot));
		}
		catch (ConfException& e) {
			std::wstring errorMsg(L"Error occured while parsing the displayList for configuration \"");
			errorMsg.append(globalConf.name);
			errorMsg.append(L"\":\n");
			errorMsg.append(e.msg());
			throw ConfException(errorMsg);
		}
	}

	// Primary display
	globalConf.primaryId = getValueFromJson<uint32_t>(gcRoot, "primaryId");
	bool validPrimaryId = false;
	for (auto& disp : globalConf.displays) {
		if (disp.displayId == globalConf.primaryId) {
			validPrimaryId = true;
		}
	}
	if (!validPrimaryId) {
		std::wstring errorMsg(L"Error occured while parsing primaryId for configuration \"");
		errorMsg.append(globalConf.name);
		errorMsg.append(L"\":\n");
		errorMsg.append(L"Provided primaryId doesn't match with any of the displays in displayList");
		throw ConfException(errorMsg);
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

	return globalConf;
}

DisplayConfiguration JsonAux::toDisplayConfig(const nlohmann::json& dcRoot) {
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

nlohmann::json JsonAux::fromGlobalConfig(const GlobalConfiguration& globalConfig) {
	nlohmann::json gcRoot;

	gcRoot["name"] = JsonAux::wideStringToString(globalConfig.name);
	gcRoot["primaryId"] = globalConfig.primaryId;

	for (auto& displayConf : globalConfig.displays) {
		gcRoot["displayList"].push_back(JsonAux::fromDisplayConfig(displayConf));
	}

	return gcRoot;
}

nlohmann::json JsonAux::fromDisplayConfig(const DisplayConfiguration& displayConfig) {
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

std::wstring JsonAux::stringToWideString(std::string str) {
	if (str.empty()) {
		return std::wstring();
	}
	size_t len = str.length() + 1;
	std::wstring ret = std::wstring(len, 0);
	int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, &str[0], int(str.size()), &ret[0], int(len));
	ret.resize(size);

	return ret;
}

std::string JsonAux::wideStringToString(std::wstring wstr) {
	if (wstr.empty()) {
		return std::string();
	}
	int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], int(wstr.size()), NULL, 0, NULL, NULL);
	std::string ret = std::string(size, 0);
	WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], int(wstr.size()), &ret[0], size, NULL, NULL);

	return ret;
}