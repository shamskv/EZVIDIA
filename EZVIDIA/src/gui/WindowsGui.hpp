#pragma once
#include<memory>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define MAX_LOADSTRING 100

// Forward Declarations
class DisplayDriver;
class Settings;
class TcpServer;
class SaveConfDialog;

class WindowsGui {
private:
	HINSTANCE hInstance;
	WCHAR szTitle[MAX_LOADSTRING];
	WCHAR szWindowClass[MAX_LOADSTRING];

	bool actionLock = false;

	DisplayDriver& driver;
	Settings& settings;

	std::unique_ptr<TcpServer> tcpServer;

	ATOM MyRegisterClass(HINSTANCE, WCHAR*);
	static LRESULT CALLBACK MainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK NewConfProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK AboutProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static BOOL AddNotificationIcon(HWND hwnd, HINSTANCE hInst);
	static void ShowContextMenu(HWND hwnd, POINT pt, WindowsGui* thisPtr);

public:
	WindowsGui(HINSTANCE, Settings&, DisplayDriver&);
	~WindowsGui();
	int msgLoop(void);
	friend class SaveConfDialog;
};