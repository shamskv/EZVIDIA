#pragma once
#include<string>
#include<json.hpp>
#include <optional>
#include "ConfigManager.hpp"
#include "ConfException.hpp"

class JsonAux {
public:
	static GlobalConfiguration toGlobalConfig(const nlohmann::json& gcRoot);
	static nlohmann::json fromGlobalConfig(const GlobalConfiguration& globalConfig);
private:
	static DisplayConfiguration toDisplayConfig(const nlohmann::json& dcRoot);
	static nlohmann::json fromDisplayConfig(const DisplayConfiguration& displayConfig);
	template<typename T>
	static T getValueFromJson(const nlohmann::json& root, const std::string& name, const std::optional<T> defaultValue = std::optional<T>());
	//String conversion
	static std::wstring stringToWideString(std::string str);
	static std::string wideStringToString(std::wstring wstr);
};

template<typename T>
inline T JsonAux::getValueFromJson(const nlohmann::json& root, const std::string& name, const std::optional<T> defaultValue) = delete;

template<>
inline uint32_t JsonAux::getValueFromJson<uint32_t>(const nlohmann::json& root, const std::string& name, const std::optional<uint32_t> defaultValue) {
	if (root.contains(name) && root[name].is_number_unsigned()) {
		return root[name].get<uint32_t>();
	}
	else if (defaultValue.has_value()) {
		return defaultValue.value();
	}
	else {
		std::wstring errorMsg(L"Missing/invalid \"");
		errorMsg.append(JsonAux::stringToWideString(name));
		errorMsg.append(L"\" parameter.");
		throw ConfException(errorMsg);
	}
}

template<>
inline int32_t JsonAux::getValueFromJson<int32_t>(const nlohmann::json& root, const std::string& name, const std::optional<int32_t> defaultValue) {
	if (root.contains(name) && root[name].is_number_integer()) {
		return root[name].get<int32_t>();
	}
	else if (defaultValue.has_value()) {
		return defaultValue.value();
	}
	else {
		std::wstring errorMsg(L"Missing/invalid \"");
		errorMsg.append(JsonAux::stringToWideString(name));
		errorMsg.append(L"\" parameter.");
		throw ConfException(errorMsg);
	}
}

template<>
inline std::string JsonAux::getValueFromJson<std::string>(const nlohmann::json& root, const std::string& name, const std::optional<std::string> defaultValue) {
	if (root.contains(name) && root[name].is_string()) {
		return root[name].get<std::string>();
	}
	else if (defaultValue.has_value()) {
		return defaultValue.value();
	}
	else {
		std::wstring errorMsg(L"Missing/invalid \"");
		errorMsg.append(JsonAux::stringToWideString(name));
		errorMsg.append(L"\" parameter.");
		throw ConfException(errorMsg);
	}
}