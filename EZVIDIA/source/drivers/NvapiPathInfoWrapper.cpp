#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <memory>
#include "../../include/drivers/NvapiPathInfoWrapper.hpp"
#include "../../include/drivers/DriverException.hpp"

NvapiPathInfoWrapper::NvapiPathInfoWrapper() {
}

// Returns true or false depending on if it had to allocate more memory
bool NvapiPathInfoWrapper::allocate() {
	if (this->nDisplays <= 0) {
		return false;
	}

	// First pass (allocate according nDisplays * pInfo and respective source mode infos
	if (this->pInfo == nullptr) {
		this->pInfo = (NV_DISPLAYCONFIG_PATH_INFO*)calloc(this->nDisplays, sizeof(NV_DISPLAYCONFIG_PATH_INFO));
		if (this->pInfo == nullptr) {
			throw DriverException(L"Error allocating memory (pInfo calloc).");
		}
		for (uint32_t i = 0; i < this->nDisplays; i++) {
			this->pInfo[i].version = NV_DISPLAYCONFIG_PATH_INFO_VER;
			this->pInfo[i].sourceModeInfo = (NV_DISPLAYCONFIG_SOURCE_MODE_INFO*)calloc(1, sizeof(NV_DISPLAYCONFIG_SOURCE_MODE_INFO));
			if (this->pInfo[i].sourceModeInfo == nullptr) {
				throw DriverException(L"Error allocating memory (sourceModeInfo calloc).");
			}
		}
		return true;
	}

	// Second pass (allocate targetInfo according to targetInfoCount)
	bool changed = false;
	for (uint32_t i = 0; i < this->nDisplays; i++) {
		if (this->pInfo[i].targetInfo == nullptr && this->pInfo[i].targetInfoCount > 0) {
			this->pInfo[i].targetInfo = (NV_DISPLAYCONFIG_PATH_TARGET_INFO*)calloc(this->pInfo[i].targetInfoCount, sizeof(NV_DISPLAYCONFIG_PATH_TARGET_INFO));
			if (this->pInfo[i].targetInfo == nullptr) {
				throw DriverException(L"Error allocating memory (targetInfo calloc).");
			}
			for (uint32_t j = 0; j < this->pInfo[i].targetInfoCount; j++) {
				this->pInfo[i].targetInfo[j].details = (NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO*)calloc(1, sizeof(NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO));
				if (this->pInfo[i].targetInfo[j].details == nullptr) {
					throw DriverException(L"Error allocating memory (targetInfo.details calloc).");
				}
				this->pInfo[i].targetInfo[j].details->version = NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO_VER;
			}
			changed = true;
		}
	}
	return changed;
}

void NvapiPathInfoWrapper::destroy() {
	if (this->pInfo != nullptr) {
		for (uint32_t i = 0; i < this->nDisplays; i++) {
			if (this->pInfo[i].sourceModeInfo != nullptr) {
				free(this->pInfo[i].sourceModeInfo);
			}
			if (this->pInfo[i].targetInfo != nullptr) {
				for (uint32_t j = 0; j < this->pInfo[i].targetInfoCount; j++) {
					if (this->pInfo[i].targetInfo[j].details != nullptr) {
						free(this->pInfo[i].targetInfo[j].details);
					}
				}
				free(this->pInfo[i].targetInfo);
			}
		}
		free(this->pInfo);
	}
	this->pInfo = nullptr;
	this->nDisplays = 0;
}

NvapiPathInfoWrapper::~NvapiPathInfoWrapper() {
	this->destroy();
}