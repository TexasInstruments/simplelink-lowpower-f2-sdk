Wireless Solar Management System Base and Node Examples
=======================================================

Example Summary
---------------

The Wireless Solar Management System (WSMS) examples (`wsms_ipv6base`, `wsms_ipv6node` in the installed `<SDK_install_path>/examples/rtos/<board>/wsms` path) can be used to demonstrate IPv6-based WSMS networks using TI's SimpleLink Connectivity devices. The ipv6base and ipv6node examples should be run on `CC1312R7` or `CC1352P7_1` (704 KB FLASH and 144 KB RAM) devices.

The ipv6base examples run as Network Co-Processors (NCP) with a host communicating with the NCPs, while the node application runs as a fully embedded implementation of a CoAP server on the WSMS IPv6 stack. Refer to Example Usage section in this readme for more information.

6LoWPAN, RPL, IPv6, ICMPv6, UDP layers have been adapted from ARM mbed-os nanostack. (https://github.com/ARMmbed/mbed-os/tree/master/connectivity/nanostack)

Note that the MAC layer is TI proprietary software and hence will be included as a library only. 


Steps for building the examples
--------------------------------

Currently the projects are provided for CCS and IAR tool chain. 
No additional steps need to be taken while importing the projects. CCS will recognize the SDK installed automatically.


Peripherals Exercised
---------------------

> UART is used for communicating with the host for the ipv6base NCP example, on which pySpinel python scripts are run. UART is also available to view the connection status and IP of the ipv6node example. The following UART settings are set by the example code, during initialization:
```
    Baud-rate:     115200
    Data bits:          8
    Stop bits:          1
    Parity:          None
    Flow Control:    None
```

Configuration using SysConfig
-----------------------------

Out of the box, the ipv6base and ipv6node examples are configured to use the following configuration. To configure settings different from the Out of Box settings, System Configuration Tool (SysConfig) should be used.
* Double click on .syscfg file from the application in the IDE to launch the GUI of SysConfig. CCS IDE has SysConfig Tool integrated into it and thus, will automatically open the file.
* Following are the Out of Box Settings for some the parameters. Please refer to the SysConfig file for the full list of parameters configurable through SysConfig GUI.
* On modifying any of the above parameters, save the changes and proceed to build the example.

Secure Boot Configuration
-------------------------

All projects can be configured to be built as a secure boot project. This will configure the project to be built into a `<project_name>.bin` file, meant to be securely booted into via MCUBoot. To configure a project to be build in this mode, change the project build configuration of the imported project to the SecureBoot configurationbefore building the project. This can be done in both CCS and IAR IDEs. A sysconfig option for secure boot configuration is present in current projects, but is only used to inform users to change the project build configuration.

Similar to OAD projects, the secure boot project must be booted into with the MCUBoot bootloader already flashed onto the device. MCUBoot can be built from the following project: `<SDK_INSTALL_DIR>/examples/nortos/<board_type>/mcuboot_app/mcuboot`. Configure MCUboot using the MCUBoot project sysconfig file as follows:

* For the `Upgrade Mode` configurable, choose `Overwrite`
* For the `Enable External Flash` configurable, choose false (unchecked)
* In the `Image 1` module:
    * If using a CC13X2X7 platform, modify the following configurables:
        * Primary image - Base address:     0x00000000
        * Primary image - Image size:       0x000A8000
        * Secondary image - Base address:   0x000A8000
        * Secondary image - Image size      0x00000000
    * If using a CC13X4 platform, modify the following configurables:
        * Primary image - Base address:     0x00006000
        * Primary image - Image size:       0x000A8000
        * Secondary image - Base address:   0x000AE000
        * Secondary image - Image size      0x00000000

Once both MCUBoot is configured and flashed on the device, the secure boot project image (.bin file) must be loaded onto the device. This file can either be flashed to the device with CCS/TI-flash or with a serial bootloader application. Currently, the 15.4 stack Linux Gateway provides a serial bootloader application to faciliate this. Please refer to the 15.4 documentation on how to download and use the serial bootloader.

Whether flashing normally or using the serial bootloader application, you must specify a specific starting address depending on the platform used.
    * If using a CC13X2X7 platform, you must load the .bin file to address 0x0.
    * If using a CC13X4 platform, you must load the .bin file to address 0x6000.

With both MCUBoot and the secure boot image loaded, the device should successfully boot into the WSMS application via MCUBoot.

Example Usage
-------------

The ipv6base and ipv6node examples are a pair of applications demonstrating a basic CoAP client-server model on WSMS. The ipv6base example is configured to work as a Network Co-Processor, and acts as the CoAP client, while the ipv6node is a standalone embedded application acting as a server. pySpinel scripts should be run on the host to configure/interact/ping nodes in the network. For information on running these scripts, refer to the readme at https://github.com/TexasInstruments/ti-wisunfan-pyspinel.

Sysconfig can be used to configure different parameters like network name, Pan ID, Broadcast and Unicast Dwell time, Broadcast interval, channel lists for for async, unicast, broadcast transmissions. The same can be configured in run-time using the NCP interface from the host. However, these configurations should be set before bringing up the network interface and the starting the stack.

Some parameters like PHY Region, PHY ID, PHY Channel spacing and Channel 0 center frequency, HW address can only be configured through SysConfig and not through the NCP interface. These parameters, however, can be read through the NCP interface.

Additionally, to enable field trials where it might be difficult to have a host attached to the device to send NCP commands to bring up network interface and stack, an autostart mode is supported. In this mode, when the device is powered up, the NCP interface in initialized and automatically the network interface is brought up and stack is started without having to receive the NCP commands. To enable this mode of operation define the macro WISUN_AUTO_START in project properties. Out of the Box, this macro is disabled.

Setup the ipv6base example by starting the network interface (interface up) and starting the stack (wisunstack start), forming the network. Next, start the ipv6node by simply flashing and booting the ipv6node application. The ipv6node automatically looks for a WSMS network to join, and upon joining, starts up the CoAP server. Note that the ipv6node uses the network paramters set in SysConfig, and cannot be dynamically adjusted at runtime. Make sure the network settings of the ipv6base and ipv6node.

As the ipv6node joins the network, the red LED of the LaunchPad will flash. The frequency of the flashing LED will decrease as the joining process proceeds. One the red LED stops flashing and the green LED turns on, the device has successfully joined the network and formed the CoAP server. The connection state of the device can also be seen via the UART output of the LaunchPad. Once the ipv6node has connected to the network, the UART output will display the local and global IP addresses.

After the ipv6node joining and server setup is complete, the ipv6base is able to interface with the server as a CoAP client using the 'coap' command using the pySpinel CLI. The CoAP server currently supports a single CoAP resource named "led" for simple LED state reading and writing. This is implemented in application/application.c, and supports read of LED states using GET CoAP requests and setting of LED states using PUT or POST commands. Please refer to the help text of the 'coap' command for details on supported CoAP commands (type 'help coap' in the CLI). In order to find the IP address of the CoAP node, either look at the UART output of the CoAP node or perform a multicast ping of the network from the ipv6base (`ping ff03::1`).

An optional PAN ID filter list feature can be enabled by building the coap node example with the `COAP_PANID_LIST` predefined symbol. You can then use the `setpanidlistjson`, `setpanidlist`, `getpanidlist`, and `panrediscovery` commands with pySpinel to configure what PANs coap nodes are allowed to join. See the pySpinel README file for more details.
