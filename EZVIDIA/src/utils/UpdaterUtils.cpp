#include "UpdaterUtils.hpp"
#include "cpprest/http_client.h"

using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features

std::wstring UpdaterUtils::getLatestVersionNumber() {
	std::wstring owner(L"shamskv"), repo(L"EZVIDIA");

	// Build request URI and start the request.
	http_client client(U("http://api.github.com/"));

	http_request request(methods::GET);
	request.headers().add(header_names::accept, U("application/vnd.github.v3+json"));
	request.set_request_uri(U("/repos/") + owner + U("/") + repo + U("/releases/latest"));

	http_response response = client.request(request).get();

	if (response.status_code() == status_codes::OK) {
		auto responseBody = response.extract_json().get();
		return responseBody.at(U("tag_name")).as_string();
	}

	return std::wstring();
}