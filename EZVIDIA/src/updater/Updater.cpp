#include "Updater.hpp"
#include "cpprest/http_client.h"
#include <cpprest/filestream.h>

#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<debugapi.h>

using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

std::wstring Updater::getLatestVersionNumber() {
	std::wstring owner(L"shamskv"), repo(L"EZVIDIA");

	// Build request URI and start the request.
	http_client client(U("http://api.github.com/"));

	http_request request(methods::GET);
	request.headers().add(header_names::accept, U("application/vnd.github.v3+json"));
	request.set_request_uri(U("/repos/") + owner + U("/") + repo + U("/releases/latest"));

	http_response response = client.request(request).get();

	if (response.status_code() == status_codes::OK) {
		auto responseBody = response.extract_json().get();

		for (auto& value : responseBody[U("assets")].as_array()) {
			//OutputDebugStringW(value[U("name")].as_string().c_str());
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