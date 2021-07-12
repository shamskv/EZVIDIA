#include "Updater.hpp"
//Internal dependencies
#include "../logging/Logger.hpp"
#include "../globals.hpp"
//Rest SDK
#include "cpprest/http_client.h"
#include <cpprest/filestream.h>
//STD lib stuff
#include<optional>
#include<array>
//Windows stuff
#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<debugapi.h>

using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

typedef std::array<int, 5> version_array_t;

namespace {
	std::optional<std::array<int, 5>> extractVersion(std::wstring versionString) {
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

	bool findAsset(json::value& v) {
		try {
			return v[U("name")].as_string() == U("EZVIDIA.exe");
		}
		catch (std::exception& e) {
			return false;
		}
	}
}

std::optional<VersionInfo> Updater::getLatestVersion() {
	// Build request URI and start the request.
	http_client client(U("http://api.github.com/"));

	http_request request(methods::GET);
	request.headers().add(header_names::accept, U("application/vnd.github.v3+json"));
	request.set_request_uri(U("/repos/") + Updater::owner + U("/") + Updater::repo + U("/releases/latest"));

	http_response response = client.request(request).get();

	if (response.status_code() == status_codes::OK) {
		try {
			auto responseBody = response.extract_json().get();
			VersionInfo info;
			info.tag = responseBody[U("tag_name")].as_string();
			info.notes = responseBody[U("body")].as_string();
			info.assetsUrl = responseBody[U("assets_url")].as_string();
			return info;
		}
		catch (std::exception& e) {
			LOG(ERR) << "Problem extracting tag + notes from latest release";
		}

		//for (auto& value : responseBody[U("assets")].as_array()) {
		//	http_client dlClient(value[U("browser_download_url")].as_string());
		//	auto dlBody = dlClient.request(methods::GET).get().body();

		//	auto outStream = fstream::open_ostream(U("dl_ezvidia.exe")).get();

		//	dlBody.read_to_end(outStream.streambuf()).get();
		//	outStream.close();
		//}
	}
	else {
		LOG(ERR) << "Received status code different than 200 when getting latest release: " << response.status_code();
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

bool Updater::downloadAndInstall(const VersionInfo& version) {
	// Build request URI and start the request.
	http_client client(version.assetsUrl);

	http_request request(methods::GET);
	request.headers().add(header_names::accept, U("application/vnd.github.v3+json"));

	http_response response = client.request(request).get();

	if (response.status_code() != status_codes::OK) {
		LOG(ERR) << "Received status code different than 200 when getting latest release assets: " << response.status_code();
		return false;
	}

	//Get download url
	std::wstring executableUrl{};
	try {
		auto assetArray = response.extract_json().get().as_array();
		auto targetAsset = std::find_if(assetArray.begin(), assetArray.end(), findAsset);
		if (targetAsset == assetArray.end()) {
			LOG(ERR) << "Couldn't find executable asset in asset list";
			return false;
		}
		executableUrl = (*targetAsset)[U("browser_download_url")].as_string();
	}
	catch (std::exception& e) {
		LOG(ERR) << "Problem extracting executable download url from assets list";
		return false;
	}
	//Download new exe
	try {
		http_client dlClient(executableUrl);
		http_response dlResponse = dlClient.request(methods::GET).get();
		if (response.status_code() != status_codes::OK) {
			LOG(ERR) << "Problem downloading executable url (request NOK)";
			return false;
		}
		auto outStream = fstream::open_ostream(U("EZVIDIA.exe.new")).get();
		dlResponse.body().read_to_end(outStream.streambuf()).get();
		outStream.close();
	}
	catch (std::exception& e) {
		LOG(ERR) << "Problem downloading executable url";
		return false;
	}

	return false;
}