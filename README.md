# EZVIDIA
## Display configuration manager for NVIDIA GPUs

EZVIDIA is a command-line tool for **Windows** and **NVIDIA** GPUs that allows a user to create and apply multiple display configurations. It is mainly targeted at users with multiple displays that frequently have to change their display configuration (through NVIDIA's Control Panel).

### Why
My personal motivation behind this project was the hassle of changing from my "desk" setup, which contains 2 23" displays, to my "living room" setup, which only contains my living room TV. When doing this through NVIDIA's Control Panel, I would have to go through the following steps:
1. Open NVIDIA's Control Panel
2. Select the 'multiple displays' tab
3. Toggle the checkboxes for the displays I'm turning on/off
4. Check if the displays are correctly arranged and which is the primary display (when dealing with multiple displays)
5. Press 'Apply'
6. After the new configuration is applied you have around 15-20s to confirm it
7. (Optional) Change resolution/refresh rate of displays in the new configuration

Point 6 was especially annoying in my case, since I would have to rush to the other room before the timer ended.

Doing this everytime I wanted to use my PC on the living room got tiring, so I found out about NVAPI and decided to make this tool.

### How
This tool is written in C++ and makes use of NVIDIA's [NVAPI](https://developer.nvidia.com/nvapi). This SDK allows (between many other features) to control the display configuration, making this tool possible. All the source code I wrote is in this repo, you are free to fork and improve/expand upon it. None of the dependencies is included.

### How to use
After configuring all the desired configurations, the tool allows you to create a batch file for each configuration. Running this batch file simply calls the tool with the desired configuration as an argument, applying it instantly. You can create a shortcut to each batch file and place it in your start menu/desktop.
