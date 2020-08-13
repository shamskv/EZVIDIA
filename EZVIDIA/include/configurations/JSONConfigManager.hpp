#include<string>
#include<json.hpp>
#include <optional>
#include "ConfigManager.hpp"

class JSONConfigManager : ConfigManager {
private:
	std::string fileName;
	bool persist();
	bool read();
	// These functions are used internally to help reading/writing the file
	GlobalConfiguration jsonToGlobalConfiguration(const nlohmann::json& gcroot);
	DisplayConfiguration jsonToDisplayConfiguration(const nlohmann::json& dcroot);
	template<typename T>
	T getValueFromJson(const nlohmann::json& root, const std::string& name, const std::optional<T> defaultValue = std::optional<T>());

public:
	JSONConfigManager(const std::string& fileName);
};

template<typename T>
inline T JSONConfigManager::getValueFromJson(const nlohmann::json& root, const std::string& name, const std::optional<T> defaultValue) = delete;

template<>
inline uint32_t JSONConfigManager::getValueFromJson<uint32_t>(const nlohmann::json& root, const std::string& name, const std::optional<uint32_t> defaultValue) {
	if (root.contains(name) && root[name].is_number_unsigned()) {
		return root[name].get<uint32_t>();
	}
	else if (defaultValue.has_value()) {
		return defaultValue.value();
	}
	else {
		char errorMsg[30];
		sprintf_s(errorMsg, "Missing/invalid \"%s\" parameter.", name.c_str());
		throw std::exception(errorMsg);
	}
}

template<>
inline int32_t JSONConfigManager::getValueFromJson<int32_t>(const nlohmann::json& root, const std::string& name, const std::optional<int32_t> defaultValue) {
	if (root.contains(name) && root[name].is_number_integer()) {
		return root[name].get<int32_t>();
	}
	else if (defaultValue.has_value()) {
		return defaultValue.value();
	}
	else {
		char errorMsg[30];
		sprintf_s(errorMsg, "Missing/invalid \"%s\" parameter.", name.c_str());
		throw std::exception(errorMsg);
	}
}

template<>
inline std::string JSONConfigManager::getValueFromJson<std::string>(const nlohmann::json& root, const std::string& name, const std::optional<std::string> defaultValue) {
	if (root.contains(name) && root[name].is_string()) {
		return root[name].get<std::string>();
	}
	else if (defaultValue.has_value()) {
		return defaultValue.value();
	}
	else {
		char errorMsg[30];
		sprintf_s(errorMsg, "Missing/invalid \"%s\" parameter.", name.c_str());
		throw std::exception(errorMsg);
	}
}