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
#include "include/EzvidiaMaster.hpp"
#include "include/configurations/ConfException.hpp"
#include "include/drivers/DriverException.hpp"
#include <boost/algorithm/string.hpp>

#define MAX_LOADSTRING 100
#define WMAPP_NOTIFYCALLBACK WM_APP+1

// Forward declarations of functions included in this code module:
BOOL				AddNotificationIcon(HWND, HINSTANCE);
void				ShowContextMenu(HWND, POINT, EzvidiaMaster*);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    NewConfig(HWND, UINT, WPARAM, LPARAM);
//int					safeApplyConfig(GlobalConfig&);
//void				blockConfigs();
//int					generateBatFiles();
//void				logPrint(std::string);
ATOM                MyRegisterClass(HINSTANCE, WCHAR*);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);

	WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
	WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

	//{
	//	std::lock_guard<std::mutex> lock(confMutex);
	//	//Load config file
	//	if (!loadConfigFile()) {
	//		/*globalError = true;
	//		globalErrorString += L"Error opening config file\n";*/
	//		saveConfigFile();
	//	}
	//}

	//int argc;
	//LPWSTR* argv;
	//argv = CommandLineToArgvW(lpCmdLine, &argc);

	////MessageBox(NULL, lpCmdLine, std::to_wstring(argc).c_str(), MB_OK | MB_ICONERROR | MB_APPLMODAL);

	//if (argc == 1 && *lpCmdLine != 0) {
	//	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	//	std::string confArg = converter.to_bytes(argv[0]);
	//	for (auto& c : configList) {
	//		if (!c.name.compare(confArg)) {
	//			int ret = NVAPIController::applyGlobalConfig(c);
	//			if (ret != 0) {
	//				MessageBox(NULL, L"Error applying configuration.", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL);
	//			}
	//			return 0;
	//		}
	//	}
	//	MessageBox(NULL, L"Configuration not found.", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL);
	//	return -1;
	//}
	//else if (argc > 1 && *lpCmdLine != 0) {
	//	return -1;
	//}

	//// Open log file
	//logStream.open(logName);
	//if (!logStream) {
	//	globalError = true;
	//	globalErrorString += L"Error opening log file\n";
	//}

	//// Open UDP Socket
	//if (!startUDPSocket()) {
	//	globalError = true;
	//	globalErrorString += L"Error opening UDP socket\n";
	//}

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_EZVIDIASERVERDEV, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance, szWindowClass);

	// Instantiate master and check for problems
	std::shared_ptr<EzvidiaMaster> masterPtr;
	try {
		masterPtr = std::make_unique<EzvidiaMaster>(hInstance, "ezconfig.json");
	}
	catch (ConfException& e) {
		MessageBox(NULL, e.msg().c_str(), L"Configuration exception", MB_OK | MB_ICONERROR);
		return -1;
	}
	catch (DriverException& e) {
		MessageBox(NULL, e.msg().c_str(), L"Driver exception", MB_OK | MB_ICONERROR);
		return -1;
	}

	// Perform application initialization:
	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, masterPtr.get());

	if (!hWnd) {
		return FALSE;
	}

	//ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//std::thread t1(HandleUDPSocket, globalSocket);
	MSG msg;
	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//if (globalSocket) {
	//	closesocket(globalSocket);
	//}
	//logMutex.lock();
	//logStream.close();
	//logMutex.unlock();
	//WSACleanup();
	//t1.join();

	return (int)msg.wParam;
}

BOOL AddNotificationIcon(HWND hwnd, HINSTANCE hInst) {
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

void ShowContextMenu(HWND hwnd, POINT pt, EzvidiaMaster* master) {
	HMENU hMenu = CreateMenu();
	HMENU hSubMenu = CreatePopupMenu();
	HMENU hDeleteMenu = CreatePopupMenu();
	std::vector<std::wstring> confNameList = master->getConfigList();

	// Menu parts that list the configurations
	if (confNameList.size() > 0) {
		long long i = 0;
		for (auto& confName : confNameList) {
			AppendMenu(hSubMenu, MF_STRING, long long(IDM_CONFIGNUM) + i, confName.c_str());
			AppendMenu(hDeleteMenu, MF_STRING, long long(IDM_DELCONFNUM) + i, confName.c_str());
			i++;
		}
	}
	else {
		AppendMenu(hSubMenu, MF_STRING | MF_GRAYED, NULL, L"(No saved configurations)");
	}
	AppendMenu(hSubMenu, MF_SEPARATOR, 0, L"");
	AppendMenu(hSubMenu, MF_STRING, IDM_SAVECONF, L"Save current configuration");
	// If we have configurations, add these options
	if (confNameList.size() > 0) {
		AppendMenu(hSubMenu, MF_STRING | MF_POPUP, (UINT_PTR)hDeleteMenu, L"Delete configuration");
		AppendMenu(hSubMenu, MF_SEPARATOR, 0, L"");
		AppendMenu(hSubMenu, MF_STRING, IDM_GENERATEBAT, L"Generate batch files");
	}
	AppendMenu(hSubMenu, MF_SEPARATOR, 0, L"");
	AppendMenu(hSubMenu, MF_STRING, IDM_EXIT, L"Exit");
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, L"");
	bool res = SetMenu(hwnd, hMenu);

	if (hMenu) {
		HMENU hSubMenu = GetSubMenu(hMenu, 0);
		if (hSubMenu) {
			// our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
			SetForegroundWindow(hwnd);

			// respect menu drop alignment
			UINT uFlags = TPM_RIGHTBUTTON;
			if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0) {
				uFlags |= TPM_RIGHTALIGN;
			}
			else {
				uFlags |= TPM_LEFTALIGN;
			}

			TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, NULL);
		}
		DestroyMenu(hMenu);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	EzvidiaMaster* master = reinterpret_cast<EzvidiaMaster*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message) {
	case WM_CREATE:
	{
		//if (globalError) {
		//	std::wstring errorMessage = std::wstring(L"Error(s) on start up:\n") + globalErrorString;
		//	MessageBox(hWnd, errorMessage.c_str(), L"Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
		//	PostQuitMessage(0);
		//	break;
		//}

		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		master = reinterpret_cast<EzvidiaMaster*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)master);

		AddNotificationIcon(hWnd, (HINSTANCE)master->hInst);
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
			//if (!globalError && !awaitingInput) {
			ShowContextMenu(hWnd, pt, master);
		}
		break;
		}
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		{
			//Applying configs
			if (wmId >= IDM_CONFIGNUM && wmId < IDM_CONFIGNUM + master->getConfigNum()) {
				int confNum = wmId - IDM_CONFIGNUM;
				//GlobalConfig conf;
				//conf = configList.at(confnum);
				//int ret = safeApplyConfig(conf);
				//if (ret != 0 && ret != -2) { //TODO fix
				//	MessageBox(hWnd, L"Error applying configuration", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL);
				//}
				master->applySelectedConfig(confNum);
			}

			//Delete configs
			if (wmId >= IDM_DELCONFNUM && wmId < IDM_DELCONFNUM + master->getConfigNum()) {
				int confNum = wmId - IDM_DELCONFNUM;
				std::wstring msg = L"Are you sure you want to delete configuration ";
				//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
				msg += master->getConfigNameByIndex(confNum) + L"?";
				//awaitingInput = true;
				master->blockInput = true;
				if (MessageBox(hWnd, msg.c_str(), L"Delete Configuration", MB_YESNO) == IDYES) {
					//configList.erase(configList.begin() + confnum);
					//saveConfigFile();
					master->deleteSelectedConfig(confNum);
				}
				//awaitingInput = false;
				master->blockInput = false;
			}
		}

		// Parse the menu selections:
		switch (wmId) {
		case IDM_SAVECONF:
			//awaitingInput = true;
			master->blockInput = true;
			DialogBoxParam((HINSTANCE)master->hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, NewConfig, (LPARAM)master);
			//std::lock_guard<std::mutex> lock(confMutex);
			//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			//boost::optional<GlobalConfig> opt_conf = NVAPIController::getCurrentConfig();
			//if (opt_conf.has_value()) {
			//	GlobalConfig conf = opt_conf.get();
			//	conf.name = std::string(dialogInput);
			//	//conf.name = converter.to_bytes(std::wstring(dialogInput));
			//	configList.push_back(conf);
			//	saveConfigFile();
			//}
			//else {
			//	MessageBox(hWnd, L"Problem retrieving configuration.", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL);
			//}
			//awaitingInput = false;
			master->blockInput = false;
			break;
		case IDM_GENERATEBAT:
			master->blockInput = true;
			//if (MessageBox(hWnd, L"Do you wish to create batch files for the current configurations?\nThe files will be placed in the same directory as the executable.", L"Generate batch files", MB_YESNO) == IDYES) {
			//	generateBatFiles();
			//}
			master->blockInput = false;
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
	EzvidiaMaster* master = reinterpret_cast<EzvidiaMaster*>(GetWindowLongPtr(hDlg, GWLP_USERDATA));
	switch (message) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		SetWindowText(hDlg, L"Insert the configuration name:");
		SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
		return (INT_PTR)FALSE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			TCHAR input[100];
			if (!GetDlgItemText(hDlg, IDC_EDIT1, input, MAX_LOADSTRING)) {
				input[0] = '\0';
			}

			std::wstring inputStr(input);
			boost::trim(inputStr);
			if (inputStr.find(L";") != std::wstring::npos) {
				MessageBox(hDlg, L"The character \';\' is not allowed.", NULL, MB_OK | MB_ICONERROR);
				return (INT_PTR)FALSE;
			}
			else if (inputStr.length() <= 0) {
				MessageBox(hDlg, L"Type something.", NULL, MB_OK | MB_ICONERROR);
				return (INT_PTR)FALSE;
			}
			else if (inputStr.length() > 50) {
				MessageBox(hDlg, L"No more than 50 characters.", NULL, MB_OK | MB_ICONERROR);
				return (INT_PTR)FALSE;
			}
			else if (!master->isConfigNameAvailable(inputStr)) {
				MessageBox(hDlg, L"Chosen name already exists.", NULL, MB_OK | MB_ICONERROR);
				return (INT_PTR)FALSE;
			}

			master->saveCurrentConfig(inputStr);

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

//int safeApplyConfig(GlobalConfig& conf) {
//	if (!configLock) {
//		std::thread waitThread(blockConfigs);
//		waitThread.detach();
//
//		logPrint("Applying configuration " + conf.name + "...");
//		int ret = NVAPIController::applyGlobalConfig(conf);
//		return ret;
//	}
//	else {
//		logPrint("Skipping apply configuration request...");
//		return -2;
//	}
//}

//void blockConfigs() {
//	if (configLock) return;
//	configLock = true;
//	logPrint("Blocking configs...");
//	boost::asio::io_context io;
//	boost::asio::steady_timer t(io, boost::asio::chrono::seconds(5));
//	t.wait();
//	logPrint("Unblocking configs...");
//	configLock = false;
//	return;
//}

//int generateBatFiles() {
//	if (configList.empty()) {
//		return -1;
//	}
//
//	std::ofstream fileout;
//	for (auto& c : configList) {
//		fileout.open("EZVIDIA " + c.name + ".bat");
//		if (!fileout) {
//			fileout.close();
//			return -1;
//		}
//		fileout << "EZVIDIA.exe \"" << c.name << "\"" << std::endl;
//		fileout.close();
//	}
//
//	return 0;
//}

ATOM MyRegisterClass(HINSTANCE hInstance, WCHAR* szWindowClass) {
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