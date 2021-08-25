#pragma once
#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<string>

class AboutDialog {
private:
	static LRESULT CALLBACK callbackProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT handleCommand(HWND, WPARAM);
	static std::wstring getAboutText();
	static LPCWSTR getHomepageUrl();
public:
	static INT_PTR show(HINSTANCE, HWND, LPARAM);
};