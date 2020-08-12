#include<json.hpp>
#include<fstream>
#include "../include/configurations/JSONConfigManager.hpp"

bool JSONConfigManager::persist() {
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
}

GlobalConfiguration JSONConfigManager::jsonToGlobalConfiguration(const nlohmann::json& gcroot) {
	return GlobalConfiguration();
}

DisplayConfiguration JSONConfigManager::jsonToDisplayConfiguration(const nlohmann::json& dcroot) {
	DisplayConfiguration dispConf;
	// Some parameters HAVE to be in the configuration file
	if (dcroot.contains("height") && dcroot["height"].is_number_unsigned()) {
		dispConf.height = dcroot["height"].get<uint32_t>();
	}
	else {
		throw std::exception("Missing/invalid height parameter.");
	}
	if (dcroot.contains("width") && dcroot["width"].is_number_unsigned()) {
		dispConf.width = dcroot["width"].get<uint32_t>();
	}
	else {
		throw std::exception("Missing/invalid width parameter.");
	}
	if (dcroot.contains("width") && dcroot["width"].is_number_unsigned()) {
		dispConf.width = dcroot["width"].get<uint32_t>();
	}
	else {
		throw std::exception("Missing/invalid width parameter.");
	}
	if (dcroot.contains("posX") && dcroot["posX"].is_number_integer()) {
		dispConf.width = dcroot["posX"].get<int32_t>();
	}
	else {
		throw std::exception("Missing/invalid width parameter.");
	}
}

JSONConfigManager::JSONConfigManager(const std::string& fileName) : fileName(fileName) {}

template<typename T>
inline T JSONConfigManager::getValueFromJson(const nlohmann::json& root, const std::string& name) {
	return T();
}

template<>
inline uint32_t JSONConfigManager::getValueFromJson<uint32_t>(const nlohmann::json& root, const std::string& name) {
	if (root.contains(name) && root[name].is_number_unsigned()) {
		return root[name].get<uint32_t>();
	}
	else {
		char errorMsg[30];
		sprintf(errorMsg, "Missing/invalid \"%s\" parameter.", name.c_str());
		throw std::exception(errorMsg);
	}
}

template<>
inline int32_t JSONConfigManager::getValueFromJson<int32_t>(const nlohmann::json& root, const std::string& name) {
	if (root.contains(name) && root[name].is_number_integer()) {
		return root[name].get<int32_t>();
	}
	else {
		char errorMsg[30];
		sprintf(errorMsg, "Missing/invalid \"%s\" parameter.", name.c_str());
		throw std::exception(errorMsg);
	}
}

template<>
inline std::string JSONConfigManager::getValueFromJson<std::string>(const nlohmann::json& root, const std::string& name) {
	if (root.contains(name) && root[name].is_string()) {
		return root[name].get<std::string>();
	}
	else {
		char errorMsg[30];
		sprintf(errorMsg, "Missing/invalid \"%s\" parameter.", name.c_str());
		throw std::exception(errorMsg);
	}
}