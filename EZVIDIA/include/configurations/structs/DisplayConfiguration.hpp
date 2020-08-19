#pragma once
#include<stdint.h>
#include<vector>

class DisplayConfiguration {
public:
	// Global parameters, should be used in a future AMD implementation
	uint32_t height;
	uint32_t width;
	uint32_t colorDepth;
	uint32_t refresh;
	int32_t posX;
	int32_t posY;
	uint32_t rotation;

	// NVIDIA specific parameters
	uint32_t scaling;
	uint32_t tvFormat;
	std::vector<uint32_t> displayIds; // multiple displayIds for clone mode
};