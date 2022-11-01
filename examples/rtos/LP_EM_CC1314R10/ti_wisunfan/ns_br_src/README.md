TI Wi-SUN FAN Border Router, Router Node, and CoAP Node Examples
================================================================

Example Summary
---------------

TI Wi-SUN FAN Border Router, Router Node, and CoAP Node Examples (`ns_br`, `ns_node`, `ns_coap_node` in the installed `<SDK_install_path>/examples/rtos/<board>/ti_wisunfan`) can be used to demonstrate Wi-SUN FAN 1.0 networks using TI's SimpleLink Connectivity devices. The Border Router and CoAP Node examples should be run on `CC1312R7` or `CC1352P7_1` (704 KB FLASH and 144 KB RAM) devices while the Router Node example is to be run on `CC1312R1`, `CC1352R1`, or `CC1352P1` (352 KB Flash & 80 KB RAM) devices.

The Border Router and Router Node examples run as Network Co-Processors (NCP) for Wi-SUN FAN stack operation, with a host communicating with the NCPs, while the CoAP application runs as a fully embedded implementation of a CoAP server on the Wi-SUN FAN stack. Refer to Example Usage section in this readme for more information.

Software layer architecture of these examples are as defined by the Wi-SUN FAN v1.0, with application code utilizing the services provided by UDP protocol.The UDP packets are sent/received using the services of 6LoWPAN and underlying MAC, PHY constructs.

6LoWPAN, RPL, IPv6, ICMPv6, UDP layers have been adapted from ARM mbed-os nanostack. (https://github.com/ARMmbed/mbed-os/tree/master/connectivity/nanostack)

Refer to the src projects variant to be able to access the source code of the Wi-SUN stack and NCP interface. MAC layer is TI proprietary software and hence will be included as a library only. 


Steps for building the examples
--------------------------------

Currently the projects are provided for CCS and IAR tool chain. 
No additional steps need to be taken while importing the projects. CCS will recognize the SDK installed automatically.


Peripherals Exercised
---------------------

> UART is used for communicating with the host for NCP examples (Border Router and Router Node), on which TI Wi-SUN pySpinel python scripts are run. UART is also available to view the connection status and IP of the CoAP example. The following UART settings are set by the example code, during initialization:
```
    Baud-rate:     115200
    Data bits:          8
    Stop bits:          1
    Parity:          None
    Flow Control:    None
```

Configuration using SysConfig
-----------------------------

Out of the box, the TI WI-SUN FAN Border Router, Router Node, and CoAP Node examples are configured to use the following configuration. To configure settings different from the Out of Box settings, System Configuration Tool (SysConfig) should be used.
* Double click on .syscfg file from the application in the IDE to launch the GUI of SysConfig. CCS IDE has SysConfig Tool integrated into it and thus, will automatically open the file.
* Following are the Out of Box Settings for some the parameters. Please refer to the SysConfig file for the full list of parameters configurable through SysConfig GUI.
Channel Function - Frequency Hopping ( Fixed channel mode is alo supported)
Region - currently testing has been done for NA region alone
Phy Mode ID - Mode 2, 50 Kbps, Mod Index 1
PAN ID - 0xABCD
Unicast Channel Mask - all 129 channels as defined by Wi-SUN FAN PHY spec
Async Channel Mask - all 129 channels as defined by Wi-SUN FAN PHY spec
Broadcast Channel Mask (applicable on Border Router side alone) - all 129 channels as defined by Wi-SUN FAN PHY spec
Network Name - Wi-SUN Network
Security - Enabled (we dont support disabling security for debug purposes at this point)
* On modifying any of the above parameters, save the changes and proceed to build the example.

Example Usage
-------------

#### Border Router and Router Node

Both Border Router and Router node are configured to work as Network Co-Processors and TI Wi-SUN FAN pySpinel scripts sould be run on the host to configure/interact/ping nodes in the network.
For information on running these scripts, refer to the readme at https://github.com/TexasInstruments/ti-wisunfan-pyspinel.

While Sysconfig can be used to configure different parameters like network name, Pan ID, Broadcast and Unicast Dwell time, Broadcast interval, channel lists for for async, unicast, broadcast transmissions, the same can be configured in run-time using the NCP interface from the host. However, these configurations should be set before bringing up the Wi-SUN network interface and the starting the Wi-SUN stack.

Some parameters like PHY Region, PHY ID, PHY Channel spacing and Channel 0 center frequency, HW address can only be configured through SysConfig and not through the NCP interface. These parameters, however, can be read through the NCP interface.

Additionally, to enable field trials where it might be difficult to have a host attached to the device to send NCP commands to bring up network interface (ifconfig up) and Wi-SUN stack (wisun start), an autostart mode is supported. In this mode, when the device is powered up, the NCP interface in initialized and automatically the network interface is brought up and Wi-SUN stack is started without having to receive the NCP commands. To enable this mode of operation define the macro WISUN_AUTO_START in project properties. Out of the Box, this macro is disabled.

On running the Border Router or Router example in NWP mode, the Red LED will turn on and then the Green LED will blink very briefly to indicate the NCP interface is ready to receive commands. On sending the command ifconfig up, the Red LED continues to stay on and on sending the command wisunstack start, the Red LED will turn off. Similar behavior is seen with auto start mode as well, but without having to receive the commands over NCP interface. In Auto start case, when an error occurs, the Red and Green LEDs will blink continuously. On running the router as a standalone embedded application, the Green LED will blink as the node joins the network and stays on, once it has succesfully joined the network. 



#### Border Router and CoAP Node

The Border Router and CoAP Node examples are a pair of applications demonstrating a basic CoAP client-server model on Wi-SUN FAN. The Border Router is configured to work as a Network Co-Processor, and acts as the CoAP client, while the CoAP node is a standalone embedded application acting as a server.

Setup the Border Router as you would in the Border Router - Router Node example, forming the Wi-SUN network. Next, start the CoAP node by simply flashing and booting the CoAP application. The CoAP node automatically looks for a Wi-SUN network to join, and upon joining, starts up the CoAP server. Note that the CoAP node uses the network paramters set in SysConfig, and cannot be dynamically adjusted at runtime. Make sure the network settings of the Border Router and CoAP Node match.

As the CoAP node joins the network, the green LED of the LaunchPad will flash. The frequency of the flashing LED will decrease as the joining process proceeds. One the green LED stops flashing and stays as a solid green light, the device has successfully joined the network and formed the CoAP server. The connection state of the device can also be seen via the UART output of the LaunchPad. Once the CoAP node has connected to the network, the UART output will display the local and global IP addresses.

After the CoAP Node joining and server setup is complete, the Border Router is able to interface with the server as a CoAP client using the 'coap' command using the pySpinel CLI. The CoAP server currently supports a single CoAP resource named "led" for simple LED state reading and writing. This is implemented in application/application.c, and supports read of LED states using GET CoAP requests and setting of LED states using PUT or POST commands. Please refer to the help text of the 'coap' command for details on supported CoAP commands (type 'help coap' in the CLI). In order to find the IP address of the CoAP node, either look at the UART output of the CoAP node or perform a multicast ping of the network from the border router (`ping ff03::1`).
