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
}

std::optional<VersionInfo> Updater::getLatestVersionNumber() {
	const std::wstring owner(L"shamskv"), repo(L"EZVIDIA");

	// Build request URI and start the request.
	http_client client(U("http://api.github.com/"));

	http_request request(methods::GET);
	request.headers().add(header_names::accept, U("application/vnd.github.v3+json"));
	request.set_request_uri(U("/repos/") + owner + U("/") + repo + U("/releases/latest"));

	http_response response = client.request(request).get();

	if (response.status_code() == status_codes::OK) {
		try {
			auto responseBody = response.extract_json().get();
			VersionInfo info;
			info.tag = responseBody.at(U("tag_name")).as_string();
			info.notes = responseBody.at(U("body")).as_string();
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

	auto remoteVersionInfo = getLatestVersionNumber();
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