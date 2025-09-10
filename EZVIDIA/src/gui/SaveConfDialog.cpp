#include "SaveConfDialog.hpp"
#include "../logging/Logger.hpp"
#include "../../resources/resource.h"
#include <cwctype>
#include "WindowsGui.hpp"
#include "../utils/StringUtils.hpp"
//Forward declarations
#include "../configurations/Settings.hpp"
#include "../drivers/DisplayDriver.hpp"
#include "../networking/TcpServer.hpp"

LRESULT SaveConfDialog::callbackProc(HWND hDlg, UINT message, WPARAM wParam,
                                     LPARAM lParam) {
  WindowsGui* thisPtr =
      reinterpret_cast<WindowsGui*>(GetWindowLongPtr(hDlg, GWLP_USERDATA));
  switch (message) {
    case WM_INITDIALOG:
      LOG(DEBUG) << "Initializing New conf dialog";
      SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
      SetWindowText(hDlg, L"Insert the configuration name:");
      SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
      return (INT_PTR)FALSE;

    case WM_COMMAND:
      return handleCommand(hDlg, wParam, thisPtr);
  }
  return (INT_PTR)FALSE;
}

LRESULT SaveConfDialog::handleCommand(HWND hDlg, WPARAM wParam,
                                      WindowsGui* thisPtr) {
  if (LOWORD(wParam) == IDOK) {
    TCHAR input[100];
    if (!GetDlgItemText(hDlg, IDC_EDIT1, input, MAX_LOADSTRING)) {
      input[0] = '\0';
    }

    auto check_invalid_char = [](wchar_t c) { return !std::iswprint(c); };
    std::wstring inputStr(input);
    StringUtils::trim(inputStr);
    if (inputStr.find(L";") != std::wstring::npos) {
      LOG(ERR) << "New configuration name rejected (contains ;)";
      MessageBox(hDlg, L"The character \';\' is not allowed.", NULL,
                 MB_OK | MB_ICONERROR);
      return (INT_PTR)FALSE;
    } else if (std::find_if(inputStr.begin(), inputStr.end(),
                            check_invalid_char) != inputStr.end()) {
      LOG(ERR) << "New configuration name rejected (misc invalid character)";
      MessageBox(hDlg, L"Invalid character detected.", NULL,
                 MB_OK | MB_ICONERROR);
      return (INT_PTR)FALSE;
    } else if (inputStr.length() <= 0) {
      LOG(ERR) << "New configuration name rejected (empty)";
      MessageBox(hDlg, L"Type something.", NULL, MB_OK | MB_ICONERROR);
      return (INT_PTR)FALSE;
    } else if (inputStr.length() > 50) {
      LOG(ERR) << "New configuration name rejected (more than 50 chars)";
      MessageBox(hDlg, L"No more than 50 characters.", NULL,
                 MB_OK | MB_ICONERROR);
      return (INT_PTR)FALSE;
    } else if (thisPtr->settings.isConfigurationPresent(inputStr)) {
      LOG(ERR) << "New configuration name rejected (already exists)";
      MessageBox(hDlg, L"Chosen name already exists.", NULL,
                 MB_OK | MB_ICONERROR);
      return (INT_PTR)FALSE;
    }

    auto optionalConf = thisPtr->driver.getConfig();
    if (optionalConf.has_value()) {
      LOG(INFO) << "Saving new configuration with name " << inputStr;
      optionalConf.value().name = inputStr;
      thisPtr->settings.addConfiguration(optionalConf.value());
    } else {
      LOG(ERR)
          << "Error saving new configuration (driver couldn't obtain config)";
      MessageBox(hDlg, L"Error saving configuration.", NULL,
                 MB_OK | MB_ICONERROR);
      return (INT_PTR)FALSE;
    }

    EndDialog(hDlg, LOWORD(wParam));
    return (INT_PTR)TRUE;
  } else if (LOWORD(wParam) == IDCANCEL) {
    EndDialog(hDlg, LOWORD(wParam));
    return (INT_PTR)TRUE;
  }
  return (INT_PTR)FALSE;
}

INT_PTR SaveConfDialog::show(HINSTANCE hInst, HWND hWnd, LPARAM lParam) {
  return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, callbackProc,
                        lParam);
}