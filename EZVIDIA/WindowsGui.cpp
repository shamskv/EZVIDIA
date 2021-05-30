#include "WindowsGui.hpp"
//Forward declarations
#include "src/configurations/SynchronizedConfigurationList.hpp"
#include "src/drivers/DisplayDriver.hpp"
//Windows stuff
#include "resource.h"
//Dependencies
#include <shellapi.h>
#include <CommCtrl.h>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>
#include "WindowsUtils.hpp"

#define WMAPP_NOTIFYCALLBACK WM_APP+1

ATOM WindowsGui::MyRegisterClass(HINSTANCE, WCHAR*) {
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowsGui::MainProc;
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

WindowsGui::WindowsGui(HINSTANCE hInstance, SynchronizedConfigurationList& configList, DisplayDriver& driver)
	: hInstance(hInstance), configList(configList), driver(driver) {
	// Do stuff related to windows idk what's happening here
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_EZVIDIASERVERDEV, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance, szWindowClass);

	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, this);
}

int WindowsGui::msgLoop(void) {
	MSG msg;
	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

LRESULT WindowsGui::MainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	WindowsGui* thisPtr = reinterpret_cast<WindowsGui*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message) {
	case WM_CREATE:
	{
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		thisPtr = static_cast<WindowsGui*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)thisPtr);

		AddNotificationIcon(hWnd, thisPtr->hInstance);
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
			ShowContextMenu(hWnd, pt, thisPtr);
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
			if (wmId >= IDM_CONFIGNUM && wmId < IDM_CONFIGNUM + thisPtr->configList.getConfigNum()) {
				int confNum = wmId - IDM_CONFIGNUM;
				//GlobalConfig conf;
				//conf = configList.at(confnum);
				//int ret = safeApplyConfig(conf);
				//if (ret != 0 && ret != -2) { //TODO fix
				//	MessageBox(hWnd, L"Error applying configuration", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL);
				//}
				auto confToApply = thisPtr->configList.getConfiguration(confNum).value(); // TODO add some error checking here just in case
				thisPtr->driver.applyConfig(confToApply);
			}

			//Delete configs
			if (wmId >= IDM_DELCONFNUM && wmId < IDM_DELCONFNUM + thisPtr->configList.getConfigNum()) {
				int confNum = wmId - IDM_DELCONFNUM;
				std::wstring msg = L"Are you sure you want to delete configuration ";
				//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
				msg += thisPtr->configList.getConfiguration(confNum).value().name + L"?";
				//awaitingInput = true;
				thisPtr->actionLock = true;
				if (MessageBox(hWnd, msg.c_str(), L"Delete Configuration", MB_YESNO) == IDYES) {
					//configList.erase(configList.begin() + confnum);
					//saveConfigFile();
					thisPtr->configList.deleteConfiguration(confNum);
				}
				//awaitingInput = false;
				thisPtr->actionLock = false;
			}
		}

		// Parse the menu selections:
		switch (wmId) {
		case IDM_SAVECONF:
			//awaitingInput = true;
			thisPtr->actionLock = true;
			DialogBoxParam(thisPtr->hInstance, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, NewConfProc, (LPARAM)thisPtr);
			thisPtr->actionLock = false;
			break;
		case IDM_GENERATEBAT:
			thisPtr->actionLock = true;
			if (MessageBox(hWnd, L"Do you wish to create batch files for the current configurations?\nThe files will be placed in the same directory as the executable.", L"Generate batch files", MB_YESNO) == IDYES) {
				WindowsUtils::generateBatFiles(thisPtr->configList.getAllConfigurationNames());
			}
			thisPtr->actionLock = false;
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

LRESULT WindowsGui::NewConfProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	WindowsGui* thisPtr = reinterpret_cast<WindowsGui*>(GetWindowLongPtr(hDlg, GWLP_USERDATA));
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
			else if (thisPtr->configList.isConfigurationPresent(inputStr)) {
				MessageBox(hDlg, L"Chosen name already exists.", NULL, MB_OK | MB_ICONERROR);
				return (INT_PTR)FALSE;
			}

			auto optionalConf = thisPtr->driver.getConfig();
			if (optionalConf.has_value()) {
				optionalConf.value().name = inputStr;
				thisPtr->configList.addConfiguration(optionalConf.value());
			}
			else {
				MessageBox(hDlg, L"Error saving configuration.", NULL, MB_OK | MB_ICONERROR);
				return (INT_PTR)FALSE;
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

BOOL WindowsGui::AddNotificationIcon(HWND hwnd, HINSTANCE hInst) {
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

void WindowsGui::ShowContextMenu(HWND hwnd, POINT pt, WindowsGui* thisPtr) {
	HMENU hMenu = CreateMenu();
	HMENU hSubMenu = CreatePopupMenu();
	HMENU hDeleteMenu = CreatePopupMenu();
	std::vector<std::wstring> confNameList = thisPtr->configList.getAllConfigurationNames();

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