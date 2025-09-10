#include "Updater.hpp"
//Internal dependencies
#include "../logging/Logger.hpp"
#include "../globals.hpp"
#include "../utils/StringUtils.hpp"
#include "HttpClient.hpp"
//json
#include <nlohmann/json.hpp>
//STD lib stuff
#include<optional>
#include<array>
//Windows stuff
#define WIN32_LEAN_AND_MEAN
#include<Windows.h>

typedef std::array<int, 5> version_array_t;

namespace {
	std::optional<version_array_t> extractVersion(std::wstring versionString) {
		LOG(DEBUG) << "Extracting version from string " << versionString.c_str();
		std::wistringstream versionStream(versionString);

		if (versionStream.get() != 'v') {
			return std::nullopt;
		}

		version_array_t arr{ 0 };
		for (int i = 0; i < std::tuple_size<version_array_t>::value; i++) {
			versionStream >> arr[i];

			if (versionStream.fail()) {
				if (i == 0) return std::nullopt; // If we couldnt even read a number it's a fail
				else break;
			}

			if (versionStream.get() != '.') break;
		}

		return arr;
	}

	int compareVersions(version_array_t v1, version_array_t v2) {
		for (int i = 0; i < std::tuple_size<version_array_t>::value; i++) {
			if (v1[i] > v2[i]) {
				return 1;
			}
			else if (v1[i] < v2[i]) {
				return -1;
			}
		}
		return 0;
	}

	auto findAsset(const std::string& name) {
		return [&](const nlohmann::json& v) {
			try {
				return v["name"].get<std::string>() == name;
			}
			catch (std::exception& e) {
				LOG(ERR) << "findAsset failed with exception " << e.what();
				return false;
			}
		};
	}

	template<class T>
	std::optional<T> downloadAsset(const nlohmann::json& assetArray, const std::string assetName) {
		auto assetIt = std::find_if(assetArray.begin(), assetArray.end(), findAsset(assetName));

		if (assetIt == assetArray.end()) {
			LOG(ERR) << "Couldn't find " << assetName.c_str() << " in asset list";
			return std::nullopt;
		}

		std::string downloadUrl{};
		std::string contentType{};
		try {
			downloadUrl = assetIt->at("browser_download_url").get<std::string>();
			contentType = assetIt->at("content_type").get<std::string>();
		}
		catch (std::exception& e) {
			LOG(ERR) << "Problem getting download url/content type for " << assetName.c_str() << " with exception: " << e.what();
			return std::nullopt;
		}

		return HttpClient::makeGetRequestSsl<T>(downloadUrl, contentType);
	}
}

std::optional<VersionInfo> Updater::getLatestVersion() {
	// Build request URI and start the request.
	std::string versionUrl("https://api.github.com/repos/" + Updater::owner + "/" + Updater::repo + "/releases/latest");
	std::string versionContentType("application/vnd.github.v3+json");

	auto optionalVersionJson = HttpClient::makeGetRequestSsl<nlohmann::json>(versionUrl, versionContentType);
	if (!optionalVersionJson) {
		return std::nullopt;
	}

	try {
		VersionInfo info;
		nlohmann::json& versionJson = optionalVersionJson.value();
		info.tag = StringUtils::stringToWideString(versionJson["tag_name"].get<std::string>());
		info.notes = StringUtils::stringToWideString(versionJson["body"].get<std::string>());
		info.assetsUrl = versionJson["assets_url"].get<std::string>();
		return info;
	}
	catch (std::exception& e) {
		LOG(ERR) << "Problem extracting tag + notes from latest release. exception msg: " << e.what();
	}

	return std::nullopt;
}

std::optional<VersionInfo> Updater::checkUpdateAvailable() {
	std::wstring localVersionStr = EZVIDIA_VERSION;

	auto remoteVersionInfo = getLatestVersion();
	if (!remoteVersionInfo) return std::nullopt;

	auto localVersion = extractVersion(localVersionStr);
	if (!localVersion) return std::nullopt;

	auto remoteVersion = extractVersion(remoteVersionInfo->tag);
	if (!remoteVersion) return std::nullopt;

	if (compareVersions(*remoteVersion, *localVersion) >= 1) {
		LOG(DEBUG) << "Comparison between remote and local version returned >=1";
		return remoteVersionInfo;
	}
	else {
		LOG(DEBUG) << "Comparison between remote and local version returned <1";
	}

	return std::nullopt;
}

// For this to work, the latest release needs a manifest.json
bool Updater::downloadAndInstall(const VersionInfo& version) {
	std::string assetArrayContentType("application/vnd.github.v3+json");
	auto optionalAssetArray = HttpClient::makeGetRequestSsl<nlohmann::json>(version.assetsUrl, assetArrayContentType);

	if (!optionalAssetArray) {
		LOG(ERR) << "Problem downloading asset array with url " << version.assetsUrl.c_str();
		return false;
	}
	nlohmann::json& assetArray = optionalAssetArray.value();
	if (!assetArray.is_array()) {
		LOG(ERR) << "Downloaded asset array payload isn't a JSON array";
		return false;
	}

	auto optionalZip = downloadAsset<std::vector<char>>(assetArray, "ezvidia.zip");

	if (!optionalZip) {
		LOG(ERR) << "Couldn't find ezvidia.zip in asset array";
		return false;
	}

	std::ofstream zipOutputFile("ezvidia.zip", std::ios::out | std::ios::binary);
	if (zipOutputFile) {
		zipOutputFile.write(optionalZip.value().data(), optionalZip.value().size());
		if (!zipOutputFile) {
			LOG(ERR) << "Unknown error writing to ezvidia.zip";
			return false;
		}
	}
	zipOutputFile.close();

	return true;
}