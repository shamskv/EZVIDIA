# EZVIDIA
### Disclaimer
EZVIDIA has a small scope of features (listed below) which won't grow very quickly (you can check the roadmap at the bottom of this document). If you are looking for a more feature-rich solution that supports multiple GPU manufacturers check out [Display Magician](https://github.com/terrymacdonald/DisplayMagician).

## Display configuration manager for NVIDIA GPUs

EZVIDIA is a tool for **Windows** and **NVIDIA** GPUs that allows a user to save and apply multiple display configurations. Its most noteworthy features/limitations are:
- Nvidia GPUs only (for now)
- Quickly change configurations (two clicks)
- Can create 1 .bat file per configuration, you can add more commands to the file in order to combine this solution with other stuff in 1 action.
- Network control (older versions use UDP, latest one uses TCP and can be turned on/off)
- A [separate Android app](https://github.com/fchamicapereira/EZVIDIA_android_app)  which implements the UDP interface. It will be updated to the TCP interface soon (tm). If you don't want to wait, try using some TCP controller app and implement the protocol detailed below.
- Configurations are stored in a human readable format, so you can edit or even create one by hand for some odd reason. (I don't advise it)

Some pictures:

![Right-clicking the notification icon shows this menu](./menu.png)

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

### Network protocol
#### TCP
The TCP interface follows a connectionless approach. Each time a TCP connection is established the program receives data until reading a delimiter ('\n'), replies accordingly and immediately shuts down the connection. Two commands are available:

	>>"LIST\n"
	<<"conf1;conf2;conf3;conf4"

	>>"APPLY conf1\n"
	<<"OK"
	>>"APPLY gibberish\n"
	<<"NOK"

	>>"GIBBERISH"
    >>"COMMAND"
    >>"\n"
	<<"INVALID"

#### UDP (old)
The tool listen for two different types of UDP messages on port 48541. A LIST message, which prompts a reply with all the current configurations:

	>>"LIST"
	<<"conf1;;conf2;;conf3;;conf4"
	
And an APPLY message, which prompts the tool to apply the desired configuration:

	>>"APPLY conf1"
	<<"OK"
	>>"APPLY gibberish"
	<<"NOK"


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