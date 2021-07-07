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
			return std::optional<version_array_t>();
		}

		version_array_t arr{ 0 };
		for (int i = 0; i < std::tuple_size<version_array_t>::value; i++) {
			versionStream >> arr[i];

			if (versionStream.fail()) {
				if (i == 0) return std::optional<version_array_t>(); // If we couldnt even read a number it's a fail
				else break;
			}

			if (versionStream.get() != '.') break;
		}

		return std::make_optional(std::move(arr));
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

std::wstring Updater::getLatestVersionNumber() {
	const std::wstring owner(L"shamskv"), repo(L"EZVIDIA");

	// Build request URI and start the request.
	http_client client(U("http://api.github.com/"));

	http_request request(methods::GET);
	request.headers().add(header_names::accept, U("application/vnd.github.v3+json"));
	request.set_request_uri(U("/repos/") + owner + U("/") + repo + U("/releases/latest"));

	http_response response = client.request(request).get();

	if (response.status_code() == status_codes::OK) {
		auto responseBody = response.extract_json().get();

		for (auto& value : responseBody[U("assets")].as_array()) {
			http_client dlClient(value[U("browser_download_url")].as_string());
			auto dlBody = dlClient.request(methods::GET).get().body();

			auto outStream = fstream::open_ostream(U("dl_ezvidia.exe")).get();

			dlBody.read_to_end(outStream.streambuf()).get();
			outStream.close();
		}

		return responseBody.at(U("tag_name")).as_string();
	}

	return std::wstring();
}

std::optional<std::wstring> Updater::checkUpdateAvailable() {
	std::wstring localVersionStr = EZVIDIA_VERSION;
	std::wstring remoteVersionStr = getLatestVersionNumber();

	auto localVersion = extractVersion(localVersionStr);
	if (!localVersion) return std::optional<std::wstring>();

	auto remoteVersion = extractVersion(remoteVersionStr);
	if (!remoteVersion) return std::optional<std::wstring>();

	if (compareVersions(*remoteVersion, *localVersion) >= 1) {
		LOG(DEBUG) << "New version found";
		return std::make_optional(std::move(remoteVersionStr));
	}

	return std::optional<std::wstring>();
}