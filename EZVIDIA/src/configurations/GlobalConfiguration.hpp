#pragma once
#include <stdint.h>

#include <string>
#include <vector>

#include "DisplayConfiguration.hpp"

class GlobalConfiguration {
 public:
  std::wstring name;
  std::vector<DisplayConfiguration> displays;
  uint32_t primaryGroup;
};