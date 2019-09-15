//This file is part of EZVIDIA.
//
//EZVIDIA is free software : you can redistribute itand /or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//EZVIDIA is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with EZVIDIA. If not, see < https://www.gnu.org/licenses/>.

// we need commctrl v6 for LoadIconMetric()
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")
// Need to link with Ws2_32.lib
#pragma comment(lib,"ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "framework.h"
#include "resource.h"


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

UINT const WMAPP_NOTIFYCALLBACK = WM_APP + 1;

bool NVAPIController::isInit = false;

std::string confName = "configs.json";
std::vector<GlobalConfig> configList;
std::atomic<bool> configLock = false;	//For the 5s delay
std::mutex confMutex;					//For writing/deleting configs

std::string logName = "ezvidia.log";
std::ofstream logStream;
std::mutex logMutex;

SOCKET globalSocket = 0;

bool globalError = false;
std::wstring globalErrorString = L"";
std::atomic<bool> awaitingInput = false;

CHAR dialogInput[MAX_LOADSTRING];

// Forward declarations of functions included in this code module:
BOOL				AddNotificationIcon(HWND);
void				ShowContextMenu(HWND, WPARAM, LPARAM);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    NewConfig(HWND, UINT, WPARAM, LPARAM);
int					safeApplyConfig(GlobalConfig&);
void				blockConfigs();
void				logPrint(std::string);
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Open log file
	logStream.open(logName);
	if (!logStream) {
		globalError = true;
		globalErrorString += L"Error opening log file\n";
	}

	// Open UDP Socket
	if (!startUDPSocket()) {
		globalError = true;
		globalErrorString += L"Error opening UDP socket\n";
	}
	{
		std::lock_guard<std::mutex> lock(confMutex);
		//Load config file
		if (!loadConfigFile()) {
			/*globalError = true;
			globalErrorString += L"Error opening config file\n";*/
			saveConfigFile();
		}
	}

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_EZVIDIASERVERDEV, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	
	std::thread t1(HandleUDPSocket, globalSocket);
	MSG msg;
	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (globalSocket) {
		closesocket(globalSocket);
	}
	logMutex.lock();
	logStream.close();
	logMutex.unlock();
	WSACleanup();
	t1.join();

	return (int)msg.wParam;
}

BOOL AddNotificationIcon(HWND hwnd)
{
	NOTIFYICONDATA nid = { sizeof(nid) };
	nid.hWnd = hwnd;
	// add the icon, setting the icon, tooltip, and callback message.
	// the icon will be identified with the GUID
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP;
	nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
	LoadIconMetric(hInst, MAKEINTRESOURCE(IDI_SMALL), LIM_SMALL, &nid.hIcon);
	LoadString(hInst, IDS_APP_TITLE, nid.szTip, ARRAYSIZE(nid.szTip));
	Shell_NotifyIcon(NIM_ADD, &nid);

	// NOTIFYICON_VERSION_4 is prefered
	nid.uVersion = NOTIFYICON_VERSION_4;
	return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

void ShowContextMenu(HWND hwnd, POINT pt)
{
	HMENU hMenu = CreateMenu();
	HMENU hSubMenu = CreatePopupMenu();
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	//Applying configs
	{
		ULONG i = 0;
		std::lock_guard<std::mutex> lock(confMutex);
		if (configList.size() > 0) {
			for (auto& conf : configList) {
				AppendMenu(hSubMenu, MF_STRING, IDM_CONFIGNUM + i++, converter.from_bytes(conf.name).c_str());
			}
		}
		else {
			AppendMenu(hSubMenu, MF_STRING | MF_GRAYED, NULL, L"(No saved configurations)");
		}
	}
	//Deleting configs
	AppendMenu(hSubMenu, MF_SEPARATOR, 0, L"");
	HMENU hDeleteMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, IDM_SAVECONF, L"Save current configuration");
	{
		ULONG i = 0;
		std::lock_guard<std::mutex> lock(confMutex);
		for (auto& conf : configList) {
			AppendMenu(hDeleteMenu, MF_STRING, IDM_DELCONFNUM + i++, converter.from_bytes(conf.name).c_str());
		}
		if (configList.size() > 0) {
			AppendMenu(hSubMenu, MF_STRING | MF_POPUP, (UINT)hDeleteMenu, L"Delete configuration");
		}
	}
	AppendMenu(hSubMenu, MF_SEPARATOR, 0, L"");
	AppendMenu(hSubMenu, MF_STRING, IDM_EXIT, L"Exit");
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, L"");
	bool res = SetMenu(hwnd, hMenu);

	if (hMenu)
	{
		HMENU hSubMenu = GetSubMenu(hMenu, 0);
		if (hSubMenu)
		{
			// our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
			SetForegroundWindow(hwnd);

			// respect menu drop alignment
			UINT uFlags = TPM_RIGHTBUTTON;
			if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0)
			{
				uFlags |= TPM_RIGHTALIGN;
			}
			else
			{
				uFlags |= TPM_LEFTALIGN;
			}

			TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, NULL);
		}
		DestroyMenu(hMenu);
	}
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		if (globalError) {
			std::wstring errorMessage = std::wstring(L"Error(s) on start up:\n") + globalErrorString;
			MessageBox(hWnd, errorMessage.c_str(), L"Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
			PostQuitMessage(0);
			break;
		}

		AddNotificationIcon(hWnd);
	}
	break;

	case WMAPP_NOTIFYCALLBACK:
	{
		switch (LOWORD(lParam)) {
		case NIN_SELECT:
		{
			//MessageBox(hWnd, L"left click", L"Title", MB_OK);
		}
		break;
		case WM_CONTEXTMENU:
		{
			POINT const pt = { LOWORD(wParam), HIWORD(wParam) };
			if (!globalError && !awaitingInput) {
				ShowContextMenu(hWnd, pt);
			}
		}
		break;
		}
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		{
			std::lock_guard<std::mutex> lock(confMutex);
			//Applying configs
			if (wmId >= IDM_CONFIGNUM && wmId < IDM_CONFIGNUM + configList.size()) {
				int confnum = wmId - IDM_CONFIGNUM;
				GlobalConfig conf;
				conf = configList.at(confnum);
				int ret = safeApplyConfig(conf);
				if (ret != 0 && ret != -2) { //TODO fix
					MessageBox(hWnd, L"Error applying configuration", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL);
				}
			}

			//Delete configs
			if (wmId >= IDM_DELCONFNUM && wmId < IDM_DELCONFNUM + configList.size()) {
				int confnum = wmId - IDM_DELCONFNUM;
				std::wstring msg = L"Are you sure you want to delete configuration ";
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
				msg += converter.from_bytes(configList.at(confnum).name) + L"?";
				awaitingInput = true;
				if (MessageBox(hWnd, msg.c_str(), L"Delete Configuration", MB_YESNO) == IDYES) {
					configList.erase(configList.begin()+confnum);
					saveConfigFile();
				}
				awaitingInput = false;
			}
		}



		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_SAVECONF:
			awaitingInput = true;
			if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, NewConfig) == IDOK) {
				std::lock_guard<std::mutex> lock(confMutex);
				//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
				boost::optional<GlobalConfig> opt_conf = NVAPIController::getCurrentConfig();
				if (opt_conf.has_value()) {
					GlobalConfig conf = opt_conf.get();
					conf.name = std::string(dialogInput);
					//conf.name = converter.to_bytes(std::wstring(dialogInput));
					configList.push_back(conf);
					saveConfigFile();
				}
				else {
					MessageBox(hWnd, L"Problem retrieving configuration.", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL);
				}
			}
			awaitingInput = false;
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK NewConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowText(hDlg, L"Insert the configuration name:");
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			if (!GetDlgItemTextA(hDlg, IDC_EDIT1, dialogInput, MAX_LOADSTRING)) {
				dialogInput[0] = '\0';
			}

			std::string input(dialogInput);
			boost::trim(input);
			if (input.find(";") != std::string::npos) {
				MessageBox(hDlg, L"The character \';\' is not allowed.", NULL, MB_OK | MB_ICONERROR);
				return (INT_PTR)FALSE;
			}
			else if (input.length() <= 0){
				MessageBox(hDlg, L"Type something.", NULL, MB_OK | MB_ICONERROR);
				return (INT_PTR)FALSE;
			}
			else if (input.length() > 50) {
				MessageBox(hDlg, L"No more than 50 characters.", NULL, MB_OK | MB_ICONERROR);
				return (INT_PTR)FALSE;
			}
			else {
				for (auto& c : configList) {
					if (!c.name.compare(input)) {
						MessageBox(hDlg, L"Chosen name already exists.", NULL, MB_OK | MB_ICONERROR);
						return (INT_PTR)FALSE;
					}
				}
			}
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

int safeApplyConfig(GlobalConfig& conf) {

	if (!configLock) {
		std::thread waitThread(blockConfigs);
		waitThread.detach();

		logPrint("Applying configuration " + conf.name + "...");
		int ret = NVAPIController::applyGlobalConfig(conf);
		return ret;
	}
	else {
		logPrint("Skipping apply configuration request...");
		return -2;
	}
}

void blockConfigs() {
	if (configLock) return;
	configLock = true;
	logPrint("Blocking configs...");
	boost::asio::io_context io;
	boost::asio::steady_timer t(io, boost::asio::chrono::seconds(5));
	t.wait();
	logPrint("Unblocking configs...");
	configLock = false;
	return;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EZVIDIASERVERDEV));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_EZVIDIASERVERDEV);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	//ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}