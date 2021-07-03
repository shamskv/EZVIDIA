#include "WindowsGui.hpp"
//Forward declarations
#include "../configurations/Settings.hpp"
#include "../drivers/DisplayDriver.hpp"
#include "../networking/TcpServer.hpp"
//Windows stuff
#include "../../resources/resource.h"
//Dependencies
#include <shellapi.h>
#include <CommCtrl.h>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>
#include "../utils/WindowsUtils.hpp"
#include "../utils/UpdaterUtils.hpp"
#include "../logging/Logger.hpp"
#include <cwctype>

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

WindowsGui::WindowsGui(HINSTANCE hInstance, Settings& settings, DisplayDriver& driver)
	: hInstance(hInstance), settings(settings), driver(driver) {
	// Do stuff related to windows idk what's happening here
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_EZVIDIASERVERDEV, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance, szWindowClass);

	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, this);

	// We try to turn on networking if the settings say so
	if (settings.networkTcpActive()) {
		LOG(DEBUG) << "GUI is activating TCP server because of settings";
		tcpServer = std::make_unique<TcpServer>(settings, driver);
	}
}

WindowsGui::~WindowsGui() = default;

int WindowsGui::msgLoop(void) {
	MSG msg;
	LOG(DEBUG) << "Entering GUI message loop";
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	LOG(DEBUG) << "Leaving GUI message loop";
	return (int)msg.wParam;
}

LRESULT WindowsGui::MainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	WindowsGui* thisPtr = reinterpret_cast<WindowsGui*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message) {
	case WM_CREATE:
	{
		LOG(DEBUG) << "Main window received CREATE signal";
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
			LOG(DEBUG) << "Main window received CONTEXTMENU (right-click) signal";
			POINT const pt = { LOWORD(wParam), HIWORD(wParam) };
			if (!thisPtr->actionLock) {
				ShowContextMenu(hWnd, pt, thisPtr);
			}
			else {
				LOG(DEBUG) << "CONTEXTMENU signal ignored (actionLock)";
			}
		}
		break;
		}
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		//Applying configs
		if (wmId >= IDM_CONFIGNUM && wmId < IDM_CONFIGNUM + thisPtr->settings.getConfigNum()) {
			int confNum = wmId - IDM_CONFIGNUM;
			LOG(DEBUG) << "Main window received APPLYCONFIG signal with num " << confNum;
			auto confToApply = thisPtr->settings.getConfiguration(confNum).value(); // TODO add some error checking here just in case
			LOG(INFO) << "Applying configuration " << confToApply.name;
			thisPtr->driver.applyConfig(confToApply);
			break;
		}

		//Delete configs
		if (wmId >= IDM_DELCONFNUM && wmId < IDM_DELCONFNUM + thisPtr->settings.getConfigNum()) {
			int confNum = wmId - IDM_DELCONFNUM;
			std::wstring msg = L"Are you sure you want to delete configuration ";
			msg += thisPtr->settings.getConfiguration(confNum).value().name + L"?";
			thisPtr->actionLock = true;
			if (MessageBox(hWnd, msg.c_str(), L"Delete Configuration", MB_YESNO) == IDYES) {
				LOG(INFO) << "Deleting configuration " << thisPtr->settings.getConfiguration(confNum).value().name;
				thisPtr->settings.deleteConfiguration(confNum);
			}
			thisPtr->actionLock = false;
		}

		// Parse the menu selections:
		switch (wmId) {
		case IDM_SAVECONF:
			thisPtr->actionLock = true;
			DialogBoxParam(thisPtr->hInstance, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, NewConfProc, (LPARAM)thisPtr);
			thisPtr->actionLock = false;
			break;
		case IDM_GENERATEBAT:
			thisPtr->actionLock = true;
			if (MessageBox(hWnd, L"Do you wish to create batch files for the current configurations?\nThe files will be placed in the same directory as the executable.", L"Generate batch files", MB_YESNO) == IDYES) {
				WindowsUtils::generateBatFiles(thisPtr->settings.getAllConfigurationNames());
			}
			thisPtr->actionLock = false;
			break;
		case IDM_NETWORK_ON:
			thisPtr->settings.setNetworkTcp(true);
			thisPtr->tcpServer = std::make_unique<TcpServer>(thisPtr->settings, thisPtr->driver);
			if (!thisPtr->tcpServer.get()->up()) {
				LOG(ERR) << "Error starting TCP server from GUI, resetting object";
				thisPtr->tcpServer.reset();
			}
			break;
		case IDM_NETWORK_OFF:
			thisPtr->settings.setNetworkTcp(false);
			thisPtr->tcpServer.reset();
			break;
		case IDM_UPDATE:
			MessageBox(NULL, UpdaterUtils::getLatestVersionNumber().c_str(), L"Version", MB_OK | MB_APPLMODAL);
			break;
		case IDM_EXIT:
			LOG(DEBUG) << "EXIT signal received by main window";
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
		LOG(DEBUG) << "Initializing New conf dialog";
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

			auto check_invalid_char = [](wchar_t c) {return !std::iswprint(c); };
			std::wstring inputStr(input);
			boost::trim(inputStr);
			if (inputStr.find(L";") != std::wstring::npos) {
				LOG(ERR) << "New configuration name rejected (contains ;)";
				MessageBox(hDlg, L"The character \';\' is not allowed.", NULL, MB_OK | MB_ICONERROR);
				return (INT_PTR)FALSE;
			}
			else if (std::find_if(inputStr.begin(), inputStr.end(), check_invalid_char) != inputStr.end()) {
				LOG(ERR) << "New configuration name rejected (misc invalid character)";
				MessageBox(hDlg, L"Invalid character detected.", NULL, MB_OK | MB_ICONERROR);
				return (INT_PTR)FALSE;
			}
			else if (inputStr.length() <= 0) {
				LOG(ERR) << "New configuration name rejected (empty)";
				MessageBox(hDlg, L"Type something.", NULL, MB_OK | MB_ICONERROR);
				return (INT_PTR)FALSE;
			}
			else if (inputStr.length() > 50) {
				LOG(ERR) << "New configuration name rejected (more than 50 chars)";
				MessageBox(hDlg, L"No more than 50 characters.", NULL, MB_OK | MB_ICONERROR);
				return (INT_PTR)FALSE;
			}
			else if (thisPtr->settings.isConfigurationPresent(inputStr)) {
				LOG(ERR) << "New configuration name rejected (already exists)";
				MessageBox(hDlg, L"Chosen name already exists.", NULL, MB_OK | MB_ICONERROR);
				return (INT_PTR)FALSE;
			}

			auto optionalConf = thisPtr->driver.getConfig();
			if (optionalConf.has_value()) {
				LOG(INFO) << "Saving new configuration with name " << inputStr;
				optionalConf.value().name = inputStr;
				thisPtr->settings.addConfiguration(optionalConf.value());
			}
			else {
				LOG(ERR) << "Error saving new configuration (driver couldn't obtain config)";
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

void WindowsGui::ShowContextMenu(HWND hwnd, POINT pt, WindowsGui * thisPtr) {
	HMENU hMenu = CreateMenu();
	HMENU hSubMenu = CreatePopupMenu();
	HMENU hDeleteMenu = CreatePopupMenu();
	HMENU hOptionsMenu = CreatePopupMenu();
	std::vector<std::wstring> confNameList = thisPtr->settings.getAllConfigurationNames();

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
	}
	// Build options menu
	if (confNameList.size() > 0) {
		AppendMenu(hOptionsMenu, MF_STRING, IDM_GENERATEBAT, L"Generate batch files");
	}
	else {
		AppendMenu(hOptionsMenu, MF_STRING | MF_GRAYED, NULL, L"Generate batch files");
	}
	if (thisPtr->settings.networkTcpActive() && thisPtr->tcpServer && thisPtr->tcpServer->up()) {
		AppendMenu(hOptionsMenu, MF_STRING | MF_CHECKED, IDM_NETWORK_OFF, L"Network control (TCP)");
	}
	else if (thisPtr->settings.networkTcpActive()) {
		AppendMenu(hOptionsMenu, MF_STRING | MF_CHECKED, IDM_NETWORK_OFF, L"Network control (TCP) (ERROR)");
	}
	else {
		AppendMenu(hOptionsMenu, MF_STRING, IDM_NETWORK_ON, L"Network control (TCP)");
	}
	AppendMenu(hOptionsMenu, MF_STRING, IDM_UPDATE, L"Check for updates");

	AppendMenu(hSubMenu, MF_STRING | MF_POPUP, (UINT_PTR)hOptionsMenu, L"Options");
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