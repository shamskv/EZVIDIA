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
		if (std::find(disp.displayIds.begin(), disp.displayIds.end(), globalConf.primaryId) != disp.displayIds.end()) {
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

	// Other parameters are optional and have a default value (for compatibility with legacy config files)
	dispConf.scaling = getValueFromJson<uint32_t>(dcRoot, "scaling", std::optional<uint32_t>(0));
	dispConf.tvFormat = getValueFromJson<uint32_t>(dcRoot, "tvFormat", std::optional<uint32_t>(0));

	// DisplayIds should be in a JSON array or in a "displayId" object if dealing with a legacy configuration
	if (dcRoot.contains("displayIds") && dcRoot["displayIds"].is_array() && !dcRoot["displayIds"].empty()) {
		for (auto& id : dcRoot["displayIds"]) {
			if (!id.is_number_unsigned()) {
				throw ConfException(L"Invalid element in displayIds array");
			}
			dispConf.displayIds.push_back(id);
		}
	}
	else if (dcRoot.contains("displayId") && dcRoot["displayId"].is_number_unsigned()) {
		dispConf.displayIds.push_back(dcRoot["displayId"].get<uint32_t>());
	}
	else {
		throw ConfException(L"Missing/invalid displayId info");
	}

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
	for (auto& id : displayConfig.displayIds) {
		dcRoot["displayIds"].push_back(id);
	}

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