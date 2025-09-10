#include "WindowsUtils.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <fstream>

#include "../logging/Logger.hpp"
#include "StringUtils.hpp"

#define PATH_BUFFER_SIZE 300

int WindowsUtils::generateBatFiles(const std::vector<std::wstring>& configs) {
  LOG(INFO) << "Generating .bat files";
  WCHAR path[PATH_BUFFER_SIZE];
  GetModuleFileName(NULL, path, PATH_BUFFER_SIZE);

  std::ofstream fileout;
  for (auto& name : configs) {
    std::wstring targetFile(L"EZVIDIA " + name + L".bat");
    fileout.open(targetFile);
    if (!fileout) {
      LOG(ERR) << "Failed to open file " << targetFile
               << " while generating .bat files";
      fileout.close();
      return -1;
    }
    fileout << StringUtils::wideStringToString(std::wstring(path)) << " \""
            << StringUtils::wideStringToString(name) << "\"" << std::endl;
    fileout.close();
  }

  return 0;
}