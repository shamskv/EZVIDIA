#pragma once
#include<exception>
#include<string>

class NetworkException : public std::exception {
private:
	std::wstring message;
public:
	NetworkException(const std::wstring& msg) : message(msg) {};
	std::wstring msg() { return this->message; }
};