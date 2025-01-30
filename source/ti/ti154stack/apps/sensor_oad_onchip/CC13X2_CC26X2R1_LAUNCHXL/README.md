TI 15.4-Stack Sensor OAD On-Chip Example
===================================

## Table of Contents

* [Example Summary](#ExampleSummary)
* [Peripherals Exercised](#PeripheralsExercised)
* [Resources & Jumper Settings](#Resources&JumperSettings)
* [Example Application Dataflow](#ExampleApplicationDataflow)
  * [Callbacks for Incoming Data Packets](#IncomingData)
* [Example Usage](#ExampleUsage)
* [Generating the Required Binary Images](#GeneratingtheRequiredBinaryImages)
* [Project Configuration](#ProjectConfiguration)
  * [Disabling Common User Interface](#DisablingCommonUserInterface)
  * [Multi-Page NV Configuration](#MultiPageNVConfiguration)
  * [Advanced OAD Configuration](#AdvancedOADConfiguration)
  * [CCFG Configuration](#CCFGConfiguration)
* [Reverting to Persistent Application](#RevertingtoPersistentApplication)

## <a name="ExampleSummary"></a>Example Summary

The Sensor OAD On-Chip example application demonstrates how to implement a sensor network device using TI 15.4-Stack with the On-Chip OAD configuration. TI 15.4-Stack based star network consists of two types of logical devices: the PAN-Coordinator and the network devices (e.g. the Collector and Sensor applications respectively).

The PAN-Coordinator is the device that starts the network and allows other devices to join the network. The network devices join the network through and always communicate with the PAN-Coordinator. The Linux Collector application is required to perform an OAD to sensor devices on the 15.4 network. Linux example applications for the external host (AM335x + MAC Coprocessor) are located in the [**TI 15.4-Stack Gateway SDK**](http://www.ti.com/tool/ti-15.4-stack-gateway-linux-sdk).

The difference between Off-Chip and On-Chip OAD is that in case of Off-Chip OAD, the application image running from the internal flash receives and stores the OAD update image onto the external flash. Subsequently when the OAD transfer is complete, the image is copied over the application image internal flash and ran.

In case of On-Chip OAD, there is another "always present/needed" persistent image along with the application image on the internal flash. When OAD process is started, the device boots into the persistent application context. It is the persistent application which receives the image update and overwrites the existing On-Chip application image. After OAD completes, if all checks including CRC and sign verification passes, the device boots into the new updated image - else, it boots back into the persistent application and requires a new OAD to be done.

> The project names for CC1352 and CC2652 platforms are referred to as CC13x2 or CC26x2. Replace x with either **1** or **5** depending on the specific wireless MCU being used.

> Note that this also includes the CC1352P-X boards, where the X represents which board subset is used, and the power amplification range.

## <a name="PeripheralsExercised"></a>Peripherals Exercised

> To trigger various events, buttons can be used as well as the configurable user interface.
> The [Example Usage](#ExampleUsage) section of this document explains how to use the user interface, although both the button presses and the UART perform the
> same actions.

* `CONFIG_LED_RED` - Turns on after the sensor connects to the collector.
* `CONFIG_BTN_LEFT` - Press to initialize the sensor application.
* `CONFIG_BTN_RIGHT` - Press to disassociate from the network.

> To erase NV flash, Hold `CONFIG_BTN_RIGHT` down, then press and release the reset button. Wait a second then release BTN-2. You should see a CUI message indicating NV erase. If BTN-2 is not held down constantly during the boot process the NVS flash will not be erased.

## <a name="Resources&JumperSettings"></a>Resources & Jumper Settings

The following hardware is required to run TI 15.4-Stack Out of Box (OOB) example applications:

* Two [**CC13x2 Launchpad development kits**](http://www.ti.com/tool/launchxl-cc1352r1) or two [**CC26x2 Launchpad development kits**](http://www.ti.com/tool/launchxl-cc26x2r1)

> If you're using an IDE (such as CCS or IAR), please refer to `Board.html` in
your project directory for resources used and board-specific jumper settings.
Otherwise, you can find `Board.html` in the directory
`<SDK_INSTALL_DIR>/source/ti/boards/<BOARD>`.

Please refer to the following link for helpful SimpleLink Academy guides for ramping up
on TI 15.4-Stack: [TI 15.4-Stack SimpleLink Academy](https://dev.ti.com/tirex/explore/node?node=ABRXrYdFS1e-0P3PY6NmNg__pTTHBmu__LATEST).

For an in-depth overview of the TI 15.4-Stack application, please refer to the TI 15.4-Stack User Guide at
`<SDK_INSTALL_DIR>/docs/ti154stack/html/ti154stack/application-overview.html#application-overview`).

## <a name="ExampleApplicationDataflow"></a>Example Application Dataflow

The sensor application has three processing loops each handling a different set of
events. These are as follows:

* Sensor_process: Sensor application event handling
	* Sensor event handling:
		* Start sensor scan for network (SENSOR_START_EVT)
		* Read sensor value and report to collector (SENSOR_READING_TIMEOUT_EVT)
		* Disassociate sensor (SENSOR_DISASSOC_EVT)
	* Triggers Jdllc_process and Ssf_processEvents
	* Triggers MAC callback handling via ApiMac_processIncoming
* Jdllc_process: Sensor logical link controller event handling
	* Trickle timer handling (PAN Advertisement/PAN Configuration message events)
	* Poll event handling (JDLLC_POLL_EVT)
	* Association request handling (JDLLC_ASSOCIATE_REQ_EVT)
	* Coordinator realignment handling (JDLLC_COORD_REALIGN)
	* Scan backoff handling (JDLLC_SCAN_BACKOFF)
	* State change handling
* Ssf_processEvents: External input handling
	* CUI input handling
	* Button press input handling
	* Triggers events in sensor/jdllc processing loops based on input

All three processing loops handle specialized tasks to service sensor functionality.
Additionally, ApiMac_processIncoming will trigger sensor and jdllc callbacks if
they are defined, acting as the trigger for several sensor and jdllc processing loop
events.

An overview of the sensor jdllc states and state transitions is as follows:

	               Jdllc_states_initWaiting
	                          | SENSOR_START_EVT, initiated by KEY_EVENT,
	                          | SENSOR_UI_INPUT_EVT, or by defining AUTO_START
	                          |
	         Existing         |          New
	         Network          |          Network
	            +-------------+-------------+
	            |                           |
	            V                           V
	  +--> Jdllc_states_                Jdllc_states_
	  |    initRestoring                joining
	  |         |                           |
	  |         V                           V
	  |    Jdllc_states_                Jdllc_states_
	  |    rejoined                     joined
	  |         |                           |
	  |         +-------------+-------------+
	  |                       | MAC reports sync loss (BCN mode) or
	  |                       | CONFIG_MAX_DATA_FAILURES consecutive data frames
	  |                       | fail to be ACK'ed by collector
	  | Orphan scan +         |
	  | Coord realign         V
	  +----------------- Jdllc_states_
	                     orphan

### <a name="IncomingData"></a>Callbacks for Incoming Data Packets

Aa described above, ApiMac_processIncoming processes all incoming messages from the MAC layer and calls the corresponding callback in the application layer. For incoming data packets, one of these two callbacks will be triggered:

* Data Indication Callback: Triggered when the MAC has successfully processed a valid data frame
* Comm Status Indication Callback: Triggered when a data frame is rejected due to a security failure. The status code in the ApiMac_mlmeCommStatusInd_t struct will contain more details regarding the error. Note that this is only applicable if MAC security is not disabled.

## <a name="ExampleUsage"></a>Example Usage

The out of box example for the sensor_oad_onchip project uses the Linux Collector to update the FW on a sensor device. The following steps walk through the process of performing an OAD:

The Linux collector requires the coprocessor device to operate. To setup the coprocessor device:

* Build a coprocessor example for your desired coprocessor device
* Load the generated coprocessor hex file onto the device using Uniflash

For the sensor device:

1. Add the On-Chip BIM image `<SDK_INSTALL_DIR>/examples/nortos/[LAUNCHPAD]/bim/hexfiles/bim_onchip/[Desired Security Level]/bim_onchip.hex` to the image list
2. Add your generated user application binary file to the image list and set the address to 0. Please refer to section [Generating the Required Binary Images](#GeneratingtheRequiredBinaryImages) in this document on how to build the images from the projects.
3. Add your generated persistent application hex file to the image list
4. Load all three images onto the device using Uniflash

Connect the coprocessor to a Linux machine and run the Linux Collector application. If the sensor has previously connected to the Linux Collector and then been reprogrammed (erasing its network parameters stored in flash), you will need to remove the nv-simulation.bin file before launching the application. More information can be found on using the Linux Collector in the Linux Gateway SDK documentation. To use the Linux Collector for OAD, make sure it is built without the `IS_HEADLESS` pre-defined symbol.

#### **Note**
> For the 2.4GHz OAD example applications, the OAD_BLOCK_SIZE has been changed from 128 down to 64. This was done because the max packet size is much smaller for a 2.4GHz network.
Modify the Makefile that resides in `example/collector` in the Linux Gateway SDK and uncomment `CFLAGS += -DOAD_BLOCK_SIZE=64` line.
After this step, rebuild the Collector example. Also, the prebuilt image for Collector working in 2.4GHz, `<SDK_INSTALL_DIR>/prebuilt/bin/host_collector_2_4g` can also be used.

When ran the Linux Collector will display the following UI on the terminal:

```shell
TI Collector
Nwk: Started


Info: Channel 0
cmd:
```

The available commands are:

```shell
sxx: Select a device. Example 's1'| 's0x1234'
o:   Toggle the permit join
l:   List devices
b:   Send a board type request to the selected device
t:   Send an LED toggle request to selected device
v:   Send a version request to selected device
u:   Send FW update request to selected device (Off-Chip OAD)
w:   Send FW update request to selected device (On-Chip OAD)
d:   Send disassociation request to selected device
fxx: Set FW file from configured OAD FW dir. Example 'f sensor_mac_oad_cc13x2lp_app.bin'
```

Enable permit join to allow sensors to connect to the network:

```shell
TI Collector
Nwk: Started


Info: PermitJoin-ON
cmd: o
```

Once joined to the network the sensor must be selected with the command `sx` (where x is the short address of the device - i.e. s1 or s0x0001). This will display:

```shell
TI Collector
Nwk: Started
Sensor 0x0001: Temp 25, RSSI -18


Info: Selected device 0x0001
cmd: s1
```

The current FW version running on the sensor can be requested using the `v` command. The next time the sensor polls it will respond and the FW version will be displayed. This will include application version (sv) and bim version (bv) that the OAD image is intended for.

```shell
TI Collector
Nwk: Started
Sensor 0x0001: FW Ver sv:0001 bv:01


Info: Sending 0x0001 FW Version Req
cmd: v
```

Note that the version of BIM must match the BIM's being used in the system.

Generate a v2 application binary (see [Generating the Required Binary Images](#GeneratingtheRequiredBinaryImages) section) and copy it to the Linux machine. Paste the '.bin' file in the following location: `<SDK_INSTALL_DIR>/prebuilt/bin/`.

The FW image file to update can be selected using the `f <path_to_File/file_name.bin>` command:

```shell
TI Collector
Nwk: Started
Sensor 0x0001: FW Ver sv:0001 bv:01


Info: OAD file ../../prebuilt/bin/sensor_oad_onchip_secure_CC1352R1_LAUNCHXL_tirtos_ccs.bin
cmd: f ../../prebuilt/bin/sensor_oad_onchip_secure_CC1352R1_LAUNCHXL_tirtos_ccs.bin
```

Entering just `f` will report the currently selected file.

A FW update of the selected sensor can be initiated with the `w` command. The next time the sensor polls it will process the request and if the FW update is accepted, the sensor will first reboot into the persistent image and start requesting OAD image blocks:

```shell
TI Collector
Nwk: Started
Sensor 0x0001: Temp 25, RSSI -18


Info: Sending 0x0001 FW Update Req
cmd: w
```

The sensor will display the OAD progress through the UART output:

```shell
 TI Sensor (Persistent App)
 Press Enter for Help
<      HELP      >

Sensor Status: Joined - Mode=NBCN, Addr=0001, PanId=ffff, Ch=0
OAD Status: Transferring Block 1 of 975
```

Once OAD is complete, the sensor will reboot. Depending on if the NV page was included in the image, the sensor may join the network.

```shell
 TI Sensor (Persistent App)
 Press Enter for Help
<      HELP      >

Sensor Status: Joined - Mode=NBCN, Addr=0001, PanId=ffff, Ch=0
OAD Status: Completed Successfully
```

After restarting:

```shell
 TI Sensor (User App)
 Press Enter for Help
<      HELP      >

Sensor Status: Joined - Mode=NBCN, Addr=0001, PanId=ffff, Ch=0
OAD Status: --
```

The Linux Collector can then verify that the new FW version is running on the sensor with the `v` command:

```shell
TI Collector
Nwk: Started
Sensor 0x0001: FW Ver sv:0002 bv:01


Info: Sending 0x0001 FW Version Req
cmd: v
```

## <a name="GeneratingtheRequiredBinaryImages"></a>Generating the Required Binary Images

The On-Chip OAD example requires three images to be flashed onto the sensor device: BIM, user application, and persistent application. The BIM can be built from the following project: `<SDK/DIR>/examples/nortos/CC13X2R1_LAUNCHXL/bim/bim_onchip`

The user application image is created as part of the post build process in the sensor_oad_onchip project: `<SDK_INSTALL_DIR>/examples/rtos/CC13X2R1_LAUNCHXL/ti154stack/sensor_oad_onchip_secure`

To update the version in the OAD header, open the project in your IDE and change the `SOFTWARE_VER` define in `application/sensor/oad/oad_image_header_app.c`. The two most significant bytes indicate the stack version, and the two least significant bytes indicate the application version.

When the project is built under the default build configuration, the resulting bin can be found in the following output directory:

* CCS: `<WORKSPACE_DIR>/sensor_oad_onchip_secure_CC13X2R1_LAUNCHXL_tirtos_ccs/Release/sensor_oad_onchip_secure_CC13X2R1_LAUNCHXL_tirtos_ccs.bin`
* IAR: `<WORKSPACE_DIR>/Release/sensor_oad_onchip_secure_src_CC13X2R1_LAUNCHXL_tirtos_iar`

#### **Note**
> The generated .out and .hex will not have the correct CRC, hence the BIM will not boot into the image. During development, the BIM can be built with the debug configuration to allow the BIM to ignore the incorrect CRC. Note that if a power-cycle occurs during an On-Chip OAD, ignoring the CRC check will cause the device to be bricked, which will require you to re-flash the device with a valid image.

> During normal operation when flashed for the first time, the application .bin and the bim .hex file need to be loaded separately. Ensure that BIM binary is flashed/downloaded before downloading the application using the debugger for debug purposes

> If you have enabled the persistent image authentication feature in the BIM (see the bim_onchip readme for more information), you will have to flash the `sensor_oad_onchip_persistent_secure_cc1352lp_tirtos_ccs.bin` file, not the .hex file. The .bin file is required since it has the populated security segment signature for BIM authentication. When flashing the persistent image .bin file, you must specify the binary load address as the starting address specified in the persistent application project's linker file (`IMG_A_FLASH_START`). The default persistent application starting address will depend on the available flash memory of the device.

The persistent image is built from the sensor_oad_onchip_persistent project:

* `<SDK_INSTALL_DIR>/examples/rtos/CC13X2R1_LAUNCHXL/ti154stack/sensor_oad_onchip_persistent_secure`

The built persistent image hex file can similarly be found in the output directory of your build configuration.

## <a name="ProjectConfiguration"></a>Project Configuration

The System Configuration (SysConfig) tool is a graphical interface for configuring your TI 15.4-Stack project. Based on the parameters configured in the SysConfig dashboard, C source files and header files are generated. Further advanced parameters can be located in `advanced_config.h`.

Some important settings in the TI 15.4-Stack module include:

| **Parameter**           | **SysConfig Location**            | **Description**                                                      |
|-------------------------|-----------------------------------|----------------------------------------------------------------------|
| Mode                    | Top of TI-15.4 Stack module       | Configures the mode of network operation                             |
| MAC Beacon Order        | MAC group within Network category | Configures how often the coordinator transmits a beacon              |
| MAC Super Frame Order   | MAC group within Network category | Configures the length of the active portion of the superframe        |
| Channel Mask            | Network category                  | Configures channels to be scanned                                    |
| Security Level          | Security category                 | Configures network security level                                    |
| Block Size              | Over-the-Air (OAD) category       | Configures the number of bytes sent in an OAD block                  |
| Block Request Rate      | Over-the-Air (OAD) category       | Configures interval in ms between block requests sent from the client|
| Block Request Poll Delay| Over-the-Air (OAD) category       | Configures delay in ms between sending the block request and block response poll request|

> SysConfig generated files are dynamically generated upon build, and any manual changes to them will be overwritten.

More information about the configuration and feature options can be found in the TI 15.4-Stack documentation under **Example Applications > Configuration Parameters**.

### <a name="DisablingCommonUserInterface"></a>Disabling Common User Interface

The common user interface (CUI) is a UART based interface that allows users to control and receive updates regarding the application. For various reasons, including reducing the memory footprint, the user is able to disable the common user interface (CUI). To disable the CUI, the following variable must be defined in the project-specific .opt file:

```
-DCUI_DISABLE
```

> Please Note: particular features that are dependent on the CUI will be unavailable when this feature is enabled.

### <a name="MultiPageNVConfiguration"></a>Multi-Page NV Configuration

By default, this project is configured to use two pages of NV. A maximum of five NV pages are supported. In order to modify the NV pages, update the following:

* `NVOCMP_NVPAGES=2` in the project-specific .opt file
* SysConfig NVS module:
   * Set Region Size based on the formula `NVOCMP_NVPAGES * 0x2000`
   * Set Region Base based on the formula `0x56000 - (NVOCMP_NVPAGES * 0x2000)`

### <a name="AdvancedOADConfiguration"></a>Advanced OAD Configuration

The 15.4 OAD protocol supports the following robustness features:

1. Timeouts: This is when a the data request for a block response is not answered. The data request delay from an OAD block request being sent is set by the `Block Request Poll Delay`. It is advised that this be set as short as possible to avoid unnecessary queuing of data in the Co-Processor. A timeout is typically caused by the Linux Collector taking too long to read the OAD Block from the FW Image file (due to CPU load). The number of timeouts before a retry is set by OAD_MAX_TIMEOUTS.
2. Retries: A retry is when the maximum number timeouts has expired before the OAD Block Response has been received. In his case the OAD Block Request is resent. The number of retires before an OAD Abort is set by OAD_MAX_RETRIES.
3. Aborts: The OAD is aborted after there are OAD_MAX_RETRIES block requests with no response. After an OAD abort the OAD is attempted to be resumed after OAD_BLOCK_AUTO_RESUME_DELAY ms, if the OAD abort again on the same block number the OAD is terminated.

The OAD is aborted if the device Orphans, when the device rejoins an OAD resume is attempted. If the device is reset / powered off during an OAD the device will attempt to resume when it rejoins. The block it resumes from is set to the first block of the page it was aborted from in case the flash page was corrupted by the power cycle.

Settings such as timeouts, retries, and aborts reside in oad_client.c. In non beacon and frequency hopping modes the default OAD parameters are:

**oad_client.c**
```c
#define OAD_MAX_TIMEOUTS              3
#define OAD_MAX_RETRIES               3
#define OAD_BLOCK_AUTO_RESUME_DELAY   5000
```

In Beacon Mode, data requests are configured to time out after 1 beacon interval. The default OAD settings are:

**oad_client.c**
```c
#define OAD_MAX_TIMEOUTS              ((uint8_t) ((BEACON_INTERVAL/OAD_BLOCK_REQ_RATE) + 1))
#define OAD_MAX_RETRIES               3
#define OAD_BLOCK_AUTO_RESUME_DELAY   BEACON_INTERVAL
```

Finally, the sensor_oad project has 3 defines related to the OAD feature, defined by default in the project's .opt file or in the project settings:

* `FEATURE_NATIVE_OAD`: This includes the 15.4 OAD client. This results in an application that supports the OAD messages needed to receive an OAD update over the 15.4 network.
* `FEATURE_BLE_OAD`: This allows the 15.4 application to revert to the persistent application to perform an OAD. For more information, see section [Reverting to Persistent Application](#RevertingtoPersistentApplication)
* `SECURITY`: This includes the OAD secure signing. This results in an application binary that is signed using an AES encryption of SHA2 hash of the image.

### <a name="CCFGConfiguration"></a>CCFG Configuration

For OAD applications, the Customer Configuration (CCFG) area must be configured in the BIM project. In order to modify the default CCFG values, update `ccfg_app.c` in the BIM On-Chip example and rebuild.

## <a name="RevertingtoPersistentApplication"></a>Reverting to Persistent Application

When the LEFT Button (`CONFIG_BTN_LEFT`) is held down and a reset occurs (RESET Button or cord unplug/plug in), the user application will enter into the `OAD_markSwitch` function. This invalidates the currently running application image itself as a bootable image and performs a soft reset of the device. This will cause the BIM to boot up like normal. However, the BIM will detect that the user application image is no longer valid. This will cause the BIM to boot into the persistent application.

In The [Example Usage](#ExampleUsage) section, the persistent application is a 15.4 image. However, this image can use any networking stack, such as BLE. The persistent application implements the OAD protocol, so whichever networking is stack used by the persistent application, you will have to perform On-Chip OAD according to the stack protocol.

For more information about performing a BLE OAD of a 15.4 image, see the BLE5-Stack documentation under **Over-the-Air Download (OAD) > Performing a BLE OAD**.
