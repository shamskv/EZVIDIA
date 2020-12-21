#include<string>
#include<json.hpp>
#include <optional>
#include "VirtualConfigurationList.hpp"

class JsonConfigurationList : public VirtualConfigurationList {
private:
	std::string fileName;
	bool persist();
	bool read();

public:
	JsonConfigurationList(const std::string& fileName);
};