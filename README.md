# PixelDisplay Driver
This is the firmware for the PixelDisplay co-processor (MKV10Z16VLC7) that drives inexpensive RGB LED matrix panels.

### Why Was This Created?
Although alternate solutions exist for driving a small matrix of RGB LEDs, they were found to be either too expensive or too resource intensive. By creating a co-processor to offload the LED driving work, the main project processor if freed up to only worry about the raw color data to display.

## Software
This project was created using [Kinetis Design Studio](https://www.nxp.com/support/developer-resources/evaluation-and-development-boards/freedom-development-boards/wireless-connectivy/kinetis-design-studio-integrated-development-environment-ide:KDS_IDE) so you will need to install it if you wish to compile it yourself. You will also need to install the [Kinetis SDK](https://www.nxp.com/support/developer-resources/nxp-designs/software-development-kit-for-kinetis-mcus:KINETIS-SDK?&code=KINETIS-SDK&nodeId=0152109D3F1E8C1EF7&fpsp=1&tab=Design_Tools_Tab) and the [Kinetis SDK Project Generator Tool](https://www.nxp.com/support/developer-resources/nxp-designs/software-development-kit-for-kinetis-mcus:KINETIS-SDK?&code=KINETIS-SDK&nodeId=0152109D3F1E8C1EF7&fpsp=1&tab=Design_Tools_Tab). Refer to this [Getting Started Guide](https://www.nxp.com/support/developer-resources/evaluation-and-development-boards/freedom-development-boards/mcu-boards/freedom-development-platform-for-kinetis-kv1x-family-16-kb-and-32-kb-flash-mcus:FRDM-KV10Z?tab=In-Depth_Tab) for information on the compilation process.

First you will need to generate a new project by using the Kinetis SDK Project Generator Tool. Setup the new project using the parameters shown below (package MKV10Z16VLC7).
[Image]

Open the project folder, delete the 'main.c' and 'main.h' files and copy in the source code from this repository. Now start up Kinetis Design Studio and import the projects found in the folder you just generated. There should actually be two separate projects to import (ksdk_platform_libMKV10Z16VLC7 and PixelDispay_Driver_MKV10Z16VLC7). Make sure to build the platform project first, and then go on to build the main driver project.

## Hardware
All chip flashing was done through Kinetis Design Studio using the the [FRDM-KV10Z](https://www.nxp.com/support/developer-resources/evaluation-and-development-boards/freedom-development-boards/mcu-boards/freedom-development-platform-for-kinetis-kv1x-family-16-kb-and-32-kb-flash-mcus:FRDM-KV10Z) development board.
