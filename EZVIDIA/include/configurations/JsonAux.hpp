#pragma once
#include<json.hpp>
#include"structs/GlobalConfiguration.hpp"

namespace JsonAux {
	GlobalConfiguration globalConfigFromJson(const nlohmann::json& gcRoot);
	nlohmann::json jsonFromGlobalConfig(const GlobalConfiguration& globalConfig);
};