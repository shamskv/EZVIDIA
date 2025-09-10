#pragma once
#include <exception>
#include <string>

class ConfException : public std::exception {
 private:
  std::wstring message;

 public:
  ConfException(const std::wstring& msg) : message(msg) {};
  std::wstring msg() { return this->message; }
};