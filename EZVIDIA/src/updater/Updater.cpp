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

	auto findAsset(const std::wstring& name) {
		return [&](json::value& v) {
			try {
				return v[U("name")].as_string() == name;
			}
			catch (std::exception& e) {
				LOG(ERR) << "findAsset failed with exception " << e.what();
				return false;
			}
		};
	}

	template<class T>
	std::optional <T> convertResponse(http_response&);

	template<class T>
	std::optional <T> convertResponse(http_response& resp) = delete;

	template<>
	std::optional<json::value> convertResponse<json::value>(http_response& resp) {
		LOG(DEBUG) << "Extracting as json object";
		try {
			return resp.extract_json().get();
		}
		catch (std::exception& e) {
			LOG(ERR) << "Problem extracting json object from response body" << e.what();
			return std::nullopt;
		}
	}

	template<>
	std::optional<json::array> convertResponse<json::array>(http_response& resp) {
		LOG(DEBUG) << "Extracting as json array";
		try {
			return resp.extract_json().get().as_array();
		}
		catch (std::exception& e) {
			LOG(ERR) << "Problem extracting json array from response body: " << e.what();
			return std::nullopt;
		}
	}

	template<>
	std::optional<std::vector<unsigned char>> convertResponse<std::vector<unsigned char>>(http_response& resp) {
		LOG(DEBUG) << "Extracting as byte vector";
		try {
			return resp.extract_vector().get();
		}
		catch (std::exception& e) {
			LOG(ERR) << "Problem extracting byte vector from response body" << e.what();
			return std::nullopt;
		}
	}

	template<http::method& M, class T>
	std::optional<T> makeRequest(const std::wstring& url, const std::wstring& contentType) {
		LOG(DEBUG) << "Making request to " << url << " with method " << M.c_str();
		http_client client(url);
		http_request request(M);
		if (!contentType.empty()) {
			request.headers().add(header_names::accept, contentType);
		}
		http_response response = client.request(request).get();

		if (response.status_code() != status_codes::OK) {
			LOG(ERR) << "Received status code different than 200 (actual: " << response.status_code() << ") when fetching URL " << url << " with method " << M.c_str();
			return std::nullopt;
		}

		return convertResponse<T>(response);
	}

	template<class T>
	std::optional<T> downloadAsset(const json::array& assetArray, const std::wstring assetName) {
		auto assetIt = std::find_if(assetArray.begin(), assetArray.end(), findAsset(L"manifest.json"));

		if (assetIt == assetArray.end()) {
			LOG(ERR) << "Couldn't find " << assetName.c_str() << " in asset list";
			return std::nullopt;
		}

		std::wstring downloadUrl{};
		std::wstring contentType{};
		try {
			downloadUrl = assetIt->get(L"browser_download_url").as_string();
			contentType = assetIt->get(L"content_type").as_string();
		}
		catch (std::exception& e) {
			LOG(ERR) << "Problem getting download url/content type for " << assetName << " with exception: " << e.what();
			return std::nullopt;
		}

		method requestMethod = methods::GET;
		return makeRequest<requestMethod, T>(downloadUrl, contentType);
	}
}

std::optional<VersionInfo> Updater::getLatestVersion() {
	// Build request URI and start the request.
	std::wstring versionUrl(L"http://api.github.com/repos/" + Updater::owner + L"/" + Updater::repo + L"/releases/latest");
	std::wstring versionContentType(L"application/vnd.github.v3+json");
	method versionMethod = methods::GET;

	auto versionJsonOpt = makeRequest<methods::GET, json::value>(versionUrl, versionContentType);
	//http_client client(U("http://api.github.com/"));

	//http_request request(methods::GET);
	//request.headers().add(header_names::accept, U("application/vnd.github.v3+json"));
	//request.set_request_uri(U("/repos/") + Updater::owner + U("/") + Updater::repo + U("/releases/latest"));

	//http_response response = client.request(request).get();

	//if (response.status_code() == status_codes::OK) {
	//	try {
	//		auto responseBody = response.extract_json().get();
	//		VersionInfo info;
	//		info.tag = responseBody[U("tag_name")].as_string();
	//		info.notes = responseBody[U("body")].as_string();
	//		info.assetsUrl = responseBody[U("assets_url")].as_string();
	//		return info;
	//	}
	//	catch (std::exception& e) {
	//		LOG(ERR) << "Problem extracting tag + notes from latest release. exception " << e.what();
	//	}

	//	//for (auto& value : responseBody[U("assets")].as_array()) {
	//	//	http_client dlClient(value[U("browser_download_url")].as_string());
	//	//	auto dlBody = dlClient.request(methods::GET).get().body();

	//	//	auto outStream = fstream::open_ostream(U("dl_ezvidia.exe")).get();

	//	//	dlBody.read_to_end(outStream.streambuf()).get();
	//	//	outStream.close();
	//	//}
	//}
	//else {
	//	LOG(ERR) << "Received status code different than 200 when getting latest release: " << response.status_code();
	//}

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
	// Build request URI and start the request.
	http_client client(version.assetsUrl);

	http_request request(methods::GET);
	request.headers().add(header_names::accept, U("application/vnd.github.v3+json"));

	http_response response = client.request(request).get();

	if (response.status_code() != status_codes::OK) {
		LOG(ERR) << "Received status code different than 200 when getting latest release assets: " << response.status_code();
		return false;
	}

	auto assetArray = response.extract_json().get().as_array();
	auto manifestAsset = std::find_if(assetArray.begin(), assetArray.end(), findAsset(L"manifest.json"));

	if (manifestAsset == assetArray.end()) {
		LOG(ERR) << "Couldn't find manifest asset in asset list";
		return false;
	}

	////Get download urls
	//std::wstring executableUrl{};
	//std::wstring checksumUrl{};
	//try {
	//	//Executable
	//	static const wchar_t executableName[] = L"EZVIDIA.exe";
	//	auto executableAsset = std::find_if(assetArray.begin(), assetArray.end(), findAsset<executableName>);
	//	if (executableAsset == assetArray.end()) {
	//		LOG(ERR) << "Couldn't find executable asset in asset list";
	//		return false;
	//	}
	//	executableUrl = (*executableAsset)[U("browser_download_url")].as_string();
	//	//Checksum
	//	static const wchar_t checksumName[] = L"checksum.txt";
	//	auto checksumAsset = std::find_if(assetArray.begin(), assetArray.end(), findAsset<checksumName>);
	//	if (checksumAsset == assetArray.end()) {
	//		LOG(ERR) << "Couldn't find checksum asset in asset list";
	//		return false;
	//	}
	//	checksumUrl = (*checksumAsset)[U("browser_download_url")].as_string();
	//}
	//catch (std::exception& e) {
	//	LOG(ERR) << "Problem extracting download urls from assets list. exception " << e.what();
	//	return false;
	//}
	////Download checksum
	//std::vector<unsigned char> checksum;
	//try {
	//	http_client dlClient(checksumUrl);
	//	http_response dlResponse = dlClient.request(methods::GET).get();
	//	if (response.status_code() != status_codes::OK) {
	//		LOG(ERR) << "Problem downloading checksum (request NOK)";
	//		return false;
	//	}
	//	checksum = dlResponse.extract_vector().get();
	//}
	//catch (std::exception& e) {
	//	LOG(ERR) << "Problem downloading checksum. exception " << e.what();
	//	return false;
	//}
	////Download executable
	//try {
	//	http_client dlClient(executableUrl);
	//	http_response dlResponse = dlClient.request(methods::GET).get();
	//	if (response.status_code() != status_codes::OK) {
	//		LOG(ERR) << "Problem downloading executable url (request NOK)";
	//		return false;
	//	}
	//	auto outStream = fstream::open_ostream(U("EZVIDIA.exe.new")).get();
	//	dlResponse.body().read_to_end(outStream.streambuf()).get();
	//	outStream.close();
	//}

	//catch (std::exception& e) {
	//	LOG(ERR) << "Problem downloading executable url. exception " << e.what();
	//	return false;
	//}

	return false;
}