#include "AboutDialog.hpp"
#include "../logging/Logger.hpp"
#include "../../resources/resource.h"
#include <shellapi.h>
#include "../globals.hpp"

LRESULT AboutDialog::callbackProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_INITDIALOG:
		LOG(DEBUG) << "Initializing About dialog";
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		SetDlgItemText(hDlg, IDC_ABOUT, getAboutText().c_str());
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
		ShellExecute(hDlg, L"open", getHomepageUrl(), NULL, NULL, SW_SHOWNORMAL);
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)FALSE;
}

std::wstring AboutDialog::getAboutText() {
	std::wstringstream ss;
	ss << EZVIDIA_VERSION << std::endl << std::endl;
	ss << "EZVIDIA is an open source program distributed under the GNU General Public License v3.0." << std::endl;
	ss << "If you have any feedback or problem leave an issue at the GitHub repository." << std::endl << std::endl;
	ss << "Developed by shamskv.";

	return ss.str();
}

LPCWSTR AboutDialog::getHomepageUrl() {
	return L"https://github.com/shamskv/ezvidia";
}

INT_PTR AboutDialog::show(HINSTANCE hInst, HWND hWnd, LPARAM lParam) {
	return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, callbackProc, lParam);
}