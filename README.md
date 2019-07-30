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

## Tutorial
### 1. Place the executable inside an empty folder
Pretty straightforward. You can download the .exe from this repo's Releases or compile it yourself if you prefer. **Do not change the executable's name.**

### 2. Name your displays
Run the executable and you should arrive at the main menu. Select the first option to perform the **initial setup**. During this setup you will be asked to change your current display configuration so it includes all your displays on a horizontal grid. The following picture shows an example of how you should set your configuration.

![](./pictures/horizontal_grid.png)

The goal of this step is for you to name your displays, since I could not find a way to retrieve manufacturer/model information. You will be asked to name each display from **left to right**.

![](./pictures/naming.png)

### 3. (Optional) Create more presets
After naming the displays you are asked if you want to create more presets. The tool will save the resolution and refresh rate you are currently using for each display, but sometimes you may wish for a display to be at a different resolution or refresh rate depending on the configuration. If you wish to do so, here you can create more presets for each display. Each preset stores a resolution and refresh rate combination for that display. In the following picture I add a 1080p@120Hz preset to my LG display.

![](./pictures/new_preset.png)

After you are done adding presets, exit the preset menu (by selecting option 0).

### 4. Creating your configurations
You should have reached the end of the initial setup:

![](./pictures/setup_done.png)

After pressing Enter you will return to the main menu and a *configs.json* file will be created in the current directory. This file must always be in the same folder as the executable.

Now, from the main manu, select the second option (Manage configurations) to access the configuration management submenu. The first option in this submenu allows you to list all configurations (you will only have one initially) and check the display presets/positions in it:

![](./pictures/config_manage.png)

The previous configuration is just a default one created with all your displays, but now you can create your own configurations.

When creating a configuration, the first step is to select your primary display (the one that Windows recognizes as primary). After selecting this display, you can finish the configuration or add more displays.

#### 4.1 One display
One display configurations are very straightforward:

![](./pictures/one_disp.png)

#### 4.2 Two or more displays
If you want to add another display, you will need to pick its position relative to the 
