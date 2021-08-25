#pragma once
#include<string>
#include<optional>

class HttpClient {
private:
	inline static std::string redirectUrl;
	template<class T>
	static std::optional<T> makeGetRequestSslNoRedirect(const std::string& url, const std::string& contentType);
public:
	template<class T>
	static std::optional<T> makeGetRequestSsl(const std::string& url, const std::string& contentType);
};