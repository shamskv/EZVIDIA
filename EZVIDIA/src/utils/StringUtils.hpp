#pragma once
#include <string>

namespace StringUtils {
std::wstring stringToWideString(std::string str);
std::string wideStringToString(std::wstring wstr);
}  // namespace StringUtils