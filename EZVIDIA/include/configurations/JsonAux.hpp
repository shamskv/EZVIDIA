#pragma once
#include<json.hpp>
#include"structs/GlobalConfiguration.hpp"

namespace JsonAux {
	GlobalConfiguration JsonToGlobalConfig(const nlohmann::json& gcRoot);
	nlohmann::json JsonFromGlobalConfig(const GlobalConfiguration& globalConfig);
};