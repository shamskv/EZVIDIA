#include<string>
#include<json.hpp>
#include <optional>
#include "Settings.hpp"

class JsonSettings : public Settings {
private:
	std::string fileName;
	bool persist();
	bool read();

public:
	JsonSettings(const std::string& fileName);
};