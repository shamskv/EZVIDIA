# EZVIDIA
### Disclaimer
EZVIDIA has a small scope of features (nvidia only + change configurations through notification icon + remote network control) which won't grow very quickly (you can check the roadmap at the bottom of this document). If you are looking for a more feature-rich solution that supports multiple GPU manufacturers check out [Display Magician](https://github.com/terrymacdonald/DisplayMagician).

## Display configuration manager for NVIDIA GPUs

EZVIDIA is a tool for **Windows** and **NVIDIA** GPUs that allows a user to save and apply multiple display configurations. It offers two different methods to change configurations:
- Selecting the desired configuration from a popup menu:

	![Right-clicking the notification icon shows this menu](./menu.png)
- Sending a TCP message following a specific syntax. Works even if the computer is locked. This is implemented in an Android app (developed by [fchamicapereira](https://github.com/fchamicapereira)) available at [this repository](https://github.com/fchamicapereira/EZVIDIA_android_app). Here is a preview:

	<img src="./android.jpg" width="270">

### Requirements
- 64-bit Windows (Only tested in Windows 10)
- NVIDIA GPU (with installed drivers)
- Latest 64-bit Visual C++ Redistributable ([link](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads))

### How to use
Run the executable and look for a green notification icon in your task bar. To use the Android app, you will need to give activate the network control option. Right-clicking this icon gives the option to save the current display configuration and to pick any of the saved configurations.

With the tool running, you can use the [Android app](https://github.com/fchamicapereira/EZVIDIA_android_app), to change the display configuration even if you are away from the computer or the computer is locked.


### How to build
Built with MSVC 2019 (C++17)
- boost (algorithm, beast)
- NVAPI
- [JSON for Modern C++](https://github.com/nlohmann/json) by [NLohmann](https://github.com/nlohmann)


### Why
My personal motivation behind this project was the hassle of changing from my "desk" setup, which contains 2 23" displays, to my "living room" setup, which only contains my living room TV. When doing this through NVIDIA's Control Panel, I would have to go through the following steps:
1. Open NVIDIA's Control Panel
2. Select the 'multiple displays' tab
3. Toggle the checkboxes for the displays I'm turning on/off
4. Check if the displays are correctly arranged and which is the primary display (when dealing with multiple displays)
5. Press 'Apply'
6. After the new configuration is applied you have 20s to confirm it
7. (Optional) Change resolution/refresh rate of displays in the new configuration

Point 6 was especially annoying in my case, since I would have to rush to the other room before the timer ended.

Doing this everytime I wanted to use my PC on the living room got tiring, so I found out about NVAPI and decided to make this tool.

### Roadmap (priority order)
- Move to Windows Display API
- ...
- ...
- ...
- ...
- Update app (zip)
- Audio switching (using AudioDeviceCmdlets)
- Move to CMAKE
- Change TCP server to boost::asio (still not sure about this)
- Exception handling where necessary (WinMain)
- GUI adjustments (separators, etc)