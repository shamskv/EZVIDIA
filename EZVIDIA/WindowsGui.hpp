#pragma once
#include<memory>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define MAX_LOADSTRING 100

// Forward Declarations
class DisplayDriver;
class SynchronizedConfigurationList;

class WindowsGui {
private:
	HINSTANCE hInstance;
	WCHAR szTitle[MAX_LOADSTRING];
	WCHAR szWindowClass[MAX_LOADSTRING];

	bool actionLock = false;

	DisplayDriver& driver;
	SynchronizedConfigurationList& configList;

	ATOM MyRegisterClass(HINSTANCE, WCHAR*);
	static LRESULT CALLBACK MainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK NewConfProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static BOOL AddNotificationIcon(HWND hwnd, HINSTANCE hInst);
	static void ShowContextMenu(HWND hwnd, POINT pt, WindowsGui* thisPtr);

public:
	WindowsGui(HINSTANCE, SynchronizedConfigurationList&, DisplayDriver&);
	int msgLoop(void);
};