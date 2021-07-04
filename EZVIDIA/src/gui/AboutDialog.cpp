#include "AboutDialog.hpp"
#include "../logging/Logger.hpp"
#include "../../resources/resource.h"
#include <shellapi.h>

LRESULT AboutDialog::callbackProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_INITDIALOG:
		LOG(DEBUG) << "Initializing About dialog";
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		SetDlgItemText(hDlg, IDC_ABOUT, L"yadadadadaa");
		return (INT_PTR)FALSE;

	case WM_COMMAND:
		return handleCommand(hDlg, wParam);
	}
	return (INT_PTR)FALSE;
}

LRESULT AboutDialog::handleCommand(HWND hDlg, WPARAM wParam) {
	if (LOWORD(wParam) == IDOK) {
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;
	}
	else if (LOWORD(wParam) == IDM_ABOUT_URL) {
		ShellExecute(hDlg, L"open",
			L"https://github.com/shamskv/ezvidia",
			NULL, NULL, SW_SHOWNORMAL);
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)FALSE;
}

INT_PTR AboutDialog::show(HINSTANCE hInst, HWND hWnd, LPARAM lParam) {
	return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, callbackProc, lParam);
}