#include "WindowsUtils.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include "src/utils/StringUtils.hpp"

#define PATH_BUFFER_SIZE 300

int WindowsUtils::generateBatFiles(const std::vector<std::wstring>& configs) {
	WCHAR path[PATH_BUFFER_SIZE];
	GetModuleFileName(NULL, path, PATH_BUFFER_SIZE);

	std::ofstream fileout;
	for (auto& name : configs) {
		fileout.open(L"EZVIDIA " + name + L".bat");
		if (!fileout) {
			fileout.close();
			return -1;
		}
		fileout << StringUtils::wideStringToString(std::wstring(path))
			<< " \"" << StringUtils::wideStringToString(name) << "\"" << std::endl;
		fileout.close();
	}

	return 0;
}