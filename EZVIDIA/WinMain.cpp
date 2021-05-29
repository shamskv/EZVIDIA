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
	JsonConfigurationList config("ezconfig.json");
	std::unique_ptr<DisplayDriver> driver(DisplayDriver::getAvailableDriver());

	WindowsGui gui(hInstance, config, *driver.get());

	return gui.msgLoop();
}