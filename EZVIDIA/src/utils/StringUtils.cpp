#include "StringUtils.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

std::wstring StringUtils::stringToWideString(std::string str) {
  if (str.empty()) {
    return std::wstring();
  }
  size_t len = str.length() + 1;
  std::wstring ret = std::wstring(len, 0);
  int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, &str[0],
                                 int(str.size()), &ret[0], int(len));
  ret.resize(size);

  return ret;
}

std::string StringUtils::wideStringToString(std::wstring wstr) {
  if (wstr.empty()) {
    return std::string();
  }
  int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0],
                                 int(wstr.size()), NULL, 0, NULL, NULL);
  std::string ret = std::string(size, 0);
  WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], int(wstr.size()),
                      &ret[0], size, NULL, NULL);

  return ret;
}