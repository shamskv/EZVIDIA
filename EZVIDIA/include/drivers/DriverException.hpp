#pragma once
#include<exception>
#include<string>

class DriverException : public std::exception {
private:
	std::wstring message;
public:
	DriverException(const std::wstring& msg) : message(msg) {};
	std::wstring msg() { return this->message; }
};