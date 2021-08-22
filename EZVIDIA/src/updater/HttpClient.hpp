#pragma once
#include<string>
#include<optional>

class HttpClient {
public:
	template<class T>
	static std::optional<T> makeGetRequest(const std::string& host, const std::string& target, const std::string& contentType);
};