#include<json.hpp>
#include<fstream>
#include "../include/configurations/JsonConfigManager.hpp"
#include "../include/configurations/JsonAux.hpp"
#include "../include/configurations/ConfException.hpp"

bool JsonConfigManager::persist() {
	std::ofstream out(fileName, std::ios_base::out);

	if (!out) {
		return false;
	}

	nlohmann::json root;
	root["configList"] = nlohmann::basic_json(nlohmann::detail::value_t::array);

	for (auto& entry : configVector) {
		root["configList"].push_back(JsonAux::fromGlobalConfig(entry));
	}

	out << root.dump(3) << std::endl;

	return true;
}

bool JsonConfigManager::read() {
	std::ifstream in(fileName, std::ios_base::in);

	if (!in) {
		return false;
	}

	nlohmann::json fRoot;
	in >> fRoot;
	if (fRoot.contains("configList") && fRoot["configList"].is_array()) {
		for (auto& gConfig : fRoot["configList"]) {
			const GlobalConfiguration& conf = JsonAux::toGlobalConfig(gConfig);
			if (std::find_if(configVector.begin(), configVector.end(), [&conf](GlobalConfiguration& gc) {return gc.name == conf.name; }) != configVector.end()) {
				// TODO log duplicate configuration here
				continue;
			}
			configVector.push_back(conf);
		}
	}
	else {
		configVector.clear();
		throw ConfException(L"Missing configList array.");
	}
	return true;
}

JsonConfigManager::JsonConfigManager(const std::string& fileName) : fileName(fileName) {}