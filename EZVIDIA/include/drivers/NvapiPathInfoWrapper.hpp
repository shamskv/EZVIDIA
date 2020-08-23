#pragma once
#include <nvapi.h>

class NvapiPathInfoWrapper {
public:
	NvU32 nDisplays = 0;
	NV_DISPLAYCONFIG_PATH_INFO* pInfo = nullptr;

	NvapiPathInfoWrapper();

	bool allocate();
	void destroy();

	~NvapiPathInfoWrapper();

	NvapiPathInfoWrapper(const NvapiPathInfoWrapper& other) = delete;
	NvapiPathInfoWrapper& operator=(const NvapiPathInfoWrapper& other) = delete;
};