#include "targetver.h"
// we need commctrl v6 for LoadIconMetric()
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")
// Need to link with Ws2_32.lib
#pragma comment(lib,"ws2_32.lib")

#include <Windows.h>
#include <memory>

#include "src/configurations/JsonConfigurationList.hpp"
#include "src/drivers/DisplayDriver.hpp"
#include "WindowsGui.hpp"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	int argc;
	WCHAR** argv = CommandLineToArgvW(lpCmdLine, &argc);

	std::optional<std::wstring> confNameToApply; // if set, we try to apply this config and exit app

	// Parse arguments before allocating/loading anything
	if (argv != NULL) {
		if (argc == 1) {
			confNameToApply.emplace(argv[0]);
		}
		else if (argc > 1) {
			MessageBox(NULL, L"Error starting application: invalid arguments\nRun without arguments or "
				"with 1 argument containing the desired configuration name surrounded by quotation marks.", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL);
			return -1;
		}
	}

	JsonConfigurationList config("ezconfig.json");
	std::unique_ptr<DisplayDriver> driver(DisplayDriver::getAvailableDriver());

	if (confNameToApply.has_value()) {
		auto optionalConf = config.getConfiguration(confNameToApply.value());
		if (optionalConf.has_value()) {
			driver->applyConfig(optionalConf.value());
			return 0;
		}
		else {
			MessageBox(NULL, L"Error applying configuration.\nConfiguration not found.", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL);
			return -1;
		}
	}

	WindowsGui gui(hInstance, config, *driver.get());

	return gui.msgLoop();
}