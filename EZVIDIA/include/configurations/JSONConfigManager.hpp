#include<string>
#include<json.hpp>
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
	T getValueFromJson(const nlohmann::json& root, const std::string& name);

public:
	JSONConfigManager(const std::string& fileName);
};