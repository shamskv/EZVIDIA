#pragma once
#include<vector>
#include<stdint.h>
#include<string>
#include"DisplayConfiguration.hpp"

class GlobalConfiguration {
public:
	std::wstring name;
	std::vector<DisplayConfiguration> displays;
	uint32_t primaryId;
};