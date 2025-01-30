TI 15.4-Stack Sensor OAD Off-Chip Example
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
  * [Turbo OAD Configuration](#TurboOADConfiguration)
  * [CCFG Configuration](#CCFGConfiguration)
* [Support for Multiple OAD Files](#SupportforMultipleOADFiles)
* [Reverting to Factory Image](#RevertingtoFactoryImage)

## <a name="ExampleSummary"></a>Example Summary

The Sensor OAD Off-Chip example application demonstrates how to implement a sensor network device using TI 15.4-Stack with the Off-Chip OAD configuration. TI 15.4-Stack based star network consists of two types of logical devices: the PAN-Coordinator and the network devices (e.g. the Collector and Sensor applications respectively).

The PAN-Coordinator is the device that starts the network and allows other devices to join the network. The network devices join the network through and always communicate with the PAN-Coordinator. The Linux Collector application is required to perform an OAD to sensor devices on the 15.4 network. Linux example applications for the external host (AM335x + MAC Coprocessor) are located in the [**TI 15.4-Stack Gateway SDK**](http://www.ti.com/tool/ti-15.4-stack-gateway-linux-sdk).

In the Off-Chip OAD configuration, the application image running from the internal flash receives and stores the OAD update image onto the external flash. Subsequently when the OAD transfer is complete, the image is copied over the application image internal flash and ran.

The external flash also contains a factory backup image to revert to in case of corruption to the user application. The factory image is not required to be a 15.4 image and can be of any networking stack. More information of using a BLE factory image to perform an OAD of a 15.4 image is described the section [Reverting to Factory Image](#RevertingtoFactoryImage)"

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

The out of box example for the sensor_oad_offchip project uses the Linux Collector to update the FW on a sensor device. The following steps walk through the process of performing an OAD:

The Linux collector requires the coprocessor device to operate. To setup the coprocessor device:

* Build a coprocessor example for your desired coprocessor device
* Load the generated coprocessor hex file onto the device using Uniflash

For the sensor device:
1. Add the Off-Chip BIM image `<SDK_INSTALL_DIR>/examples/nortos/[LAUNCHPAD]/bim/hexfiles/bim_offchip/[Desired Security Level]/bim_offchip.hex` to the image list
2. Add your user-generated binary file to the image list and set the address to 0. Please refer to section [Generating the Required Binary Images](#GeneratingtheRequiredBinaryImages) in this document on how to build the images from the projects.
3. Load both images onto the device using Uniflash

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


Info: OAD file ../../prebuilt/bin/sensor_oad_offchip_secure_CC1352R1_LAUNCHXL_tirtos_ccs.bin
cmd: f ../../prebuilt/bin/sensor_oad_offchip_secure_CC1352R1_LAUNCHXL_tirtos_ccs.bin
```

Entering just `f` will report the currently selected file.

A FW update of the selected sensor can be initiated with the `u` command. The next time the sensor polls, it will process the request and if the FW update is accepted, the sensor will start requesting OAD image blocks:

```shell
TI Collector
Nwk: Started
Sensor 0x0001: Temp 25, RSSI -18


Info: Sending 0x0001 FW Update Req
cmd: w
```

The sensor will display the OAD progress through the UART output:

```shell
 TI Sensor
 Press Enter for Help
<      HELP      >

Sensor Status: Joined - Mode=NBCN, Addr=0001, PanId=ffff, Ch=0
OAD Status: Transferring Block 1 of 975
```

Once OAD is complete, the sensor will reboot. Depending on if the NV page was included in the image, the sensor may join the network.

```shell
 TI Sensor
 Press Enter for Help
<      HELP      >

Sensor Status: Joined - Mode=NBCN, Addr=0001, PanId=ffff, Ch=0
OAD Status: Completed Successfully
```

After restarting:

```shell
 TI Sensor
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

The Off-Chip OAD example requires two images to be flashed onto the sensor device (the BIM and user application). The BIM can be built from the following project: `<SDK/DIR>/examples/nortos/CC13X2R1_LAUNCHXL/bim/bim_offchip`

The user application image is created as part of the post build process in the sensor_oad_offchip project: `<SDK_INSTALL_DIR>/examples/rtos/CC13X2R1_LAUNCHXL/ti154stack/sensor_oad_offchip_secure`

To update the version in the OAD header, open the project in your IDE and change the `SOFTWARE_VER` define in `application/sensor/oad/oad_image_header_app.c`. The two most significant bytes indicate the stack version, and the two least significant bytes indicate the application version.

When the project is built under the default build configuration, the resulting bin can be found in the following output directory:

* CCS: `<WORKSPACE_DIR>/sensor_oad_offchip_secure_CC13X2R1_LAUNCHXL_tirtos_ccs/Release/sensor_oad_offchip_secure_CC13X2R1_LAUNCHXL_tirtos_ccs.bin`
* IAR: `<WORKSPACE_DIR>/Release/sensor_oad_offchip_secure_src_CC13X2R1_LAUNCHXL_tirtos_iar`

#### **Note**
> The generated .out and .hex will not have the correct CRC, hence the BIM will not boot into the image. During development, the BIM can be built with the debug configuration to allow the BIM to ignore the incorrect CRC.

> During normal operation when flashed for the first time, the application .bin and the bim .hex file need to be loaded separately. Ensure that BIM binary is flashed/downloaded before downloading the application using the debugger for debug purposes

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
* `SECURITY`: This includes the OAD secure signing. This results in an application binary that is signed using an AES encryption of SHA2 hash of the image.

### <a name="TurboOADConfiguration"></a>Turbo OAD Configuration

Turbo OAD enables the support of using delta software updates in the OAD protocol. Delta updates are software updates that contain only the changed or updated content in the new software image. This results on average around 5x to 10x faster software updates for typical small or medium sized software updates.

Turbo OAD can be enabled in the project in SysConfig (TI 15.4 Stack > Over-the-Air Download > Turbo OAD > Enabled Turbo OAD). When creating the initial device image, no path to the old image needs to be specified. This image will setup the device to support receiving delta images (the device still supports receiving regular OAD images as well).

For the creation of delta updates however, a path to the binary running on the sensor must be provided to create the delta image. This will create a delta image (.dim) file in the build configuration output directory. This file can be uploaded to the Linux Gateway for distributing the delta image. The process of sending a delta update is the same process as described in the [Example Usage](#ExampleUsage) section.

Additional advanced configuration options are provided, which are discussed in the Turbo OAD SysConfig Module.

### <a name="CCFGConfiguration"></a>CCFG Configuration

For OAD applications, the Customer Configuration (CCFG) area must be configured in the BIM project. In order to modify the default CCFG values, update `ccfg_app.c` in the BIM Off-Chip example and rebuild.

## <a name="SupportforMultipleOADFiles"></a>Support for Multiple OAD Files

The OAD protocol supports multiple OAD images by using an Image ID that is sent when the Collector initiates the OAD and then in each OAD block request / response. This insures that the device always receives a block from the correct FW image, especially in the case where a device loses power or orphans and it is not known when it will come back on line. When an OAD image file is selected on the collector it is assigned a new image ID and added to a table, when a block request is received the image ID in the block request is used to find the correct FW image file. This insures that a device will always get a block from the correct image, no matter how long it is off line.

## <a name="RevertingtoFactoryImage"></a>Reverting to Factory Image

When the LEFT Button (`CONFIG_BTN_LEFT`) is held down and a reset occurs (RESET Button or cord unplug/plug in), the user application will enter into the `OAD_markSwitch` function. This invalidates the currently running application image itself as a bootable image and performs a soft reset of the device. This will cause the BIM to boot up like normal. However, the BIM will detect that the user application image is no longer valid. This will cause the BIM to copy the current Factory Image that is stored in External Flash to Internal Flash and perform a soft reset. Now when the BIM boots up again, the current running image (Factory Image) will be valid and the BIM will let the current image to start execution.

In order for this functionality to be useful however, one first needs to have a Factory Image saved into External Flash. Luckily, when the BIM boots up and there is no current Factory Image and the current internal image is valid, a copy of the current internal image will be made effectively creating a backup of the current image. This is useful, but will not provide any special functionality other than redundancy. Instead you can first flash the LP with a BLE Simple Peripheral OAD application and BIM so that it creates a backup of that image. Then using BLE OAD send the Sensor 15.4 Image to the LP. Now, whenever you want, you can switch to the Factory Image and upgrade to any image you want.

For more information about performing a BLE OAD of a 15.4 image, see the BLE5-Stack documentation under **Over-the-Air Download (OAD) > Performing a BLE OAD**.

**Note: BLE OAD is not supported for CC1312 devices.**
