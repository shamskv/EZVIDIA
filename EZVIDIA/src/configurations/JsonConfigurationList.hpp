#include<string>
#include<json.hpp>
#include <optional>
#include "SynchronizedConfigurationList.hpp"

class JsonConfigurationList : public SynchronizedConfigurationList {
private:
	std::string fileName;
	bool persist();
	bool read();

public:
	JsonConfigurationList(const std::string& fileName);
};