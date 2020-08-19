#include<string>
#include<json.hpp>
#include <optional>
#include "ConfigManager.hpp"

class JsonConfigManager : public ConfigManager {
private:
	std::string fileName;
	bool persist();
	bool read();

public:
	JsonConfigManager(const std::string& fileName);
};