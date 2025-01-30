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

## <a name="ExampleSummary"></a>Example Summary

The Sensor OAD Off-Chip example application demonstrates how to implement a sensor network device using TI 15.4-Stack with the ff-Chip OAD configuration. TI 15.4-Stack based star network consists of two types of logical devices: the PAN-Coordinator and the network devices (e.g. the Collector and Sensor applications respectively).

The PAN-Coordinator is the device that starts the network and allows other devices to join the network. The network devices join the network through and always communicate with the PAN-Coordinator. The Linux Collector application is required to perform an OAD to sensor devices on the 15.4 network. Linux example applications for the external host (AM335x + MAC Coprocessor) are located in the [**TI 15.4-Stack Gateway SDK**](http://www.ti.com/tool/ti-15.4-stack-gateway-linux-sdk).

The difference between Off-Chip and On-Chip OAD is that in case of Off-Chip OAD, the application image running from the internal flash receives and stores the OAD update image onto the external flash. Subsequently when the OAD transfer is complete, the image is copied over the application image internal flash and ran.

In case of On-Chip OAD for MCUboot, there is only one application image on the internal flash, the same as previous off-chip examples. After the OAD completes, if all checks including verification passes, the device overwrites the old image with the new updated image - otherwise, it stays on the previous application image.

> Currently only Thor devices support using MCUboot.

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

* Two [**CC13x4 Launchpad development kits**](http://www.ti.com/tool/launchxl-cc1352r1) or two [**CC26x3/CC26x4 Launchpad development kits**](http://www.ti.com/tool/launchxl-cc26x2r1)

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

Please refer to section [Generating the Required Binary Images](#GeneratingtheRequiredBinaryImages) in this document on how to build the images from the projects.

The Linux collector requires the coprocessor device to operate. To setup the coprocessor device:

* Load `<SDK_INSTALL_DIR>/examples/rtos/CC1352R1_LAUNCHXL/ti154stack/hexfiles/coprocessor_cc1352lp_tirtos_ccs.hex` onto the device using Uniflash

For the sensor device:

1. Add the Off-Chip MCUboot image to the image list
2. Add the user application binary image with header to the image list at the primary slot location (0x6000 unless otherwise changed during image generation)
3. Load the images onto the device using Uniflash

Connect the coprocessor to a Linux machine and run the Linux Collector application. If the sensor has previously connected to the Linux Collector and then been reprogrammed (erasing its network parameters stored in flash), you will need to remove the nv-simulation.bin file before launching the application. More information can be found on using the Linux Collector in the Linux Gateway SDK documentation. To use the Linux Collector for OAD, make sure it is built without the `IS_HEADLESS` pre-defined symbol.

#### **Note**
> For the 2.4GHz OAD example applications, the default OAD_BLOCK_SIZE has been changed from 128 down to 64.
Modify the Makefile that resides in `example/collector` in the Linux Gateway SDK and uncomment `CFLAGS += -DOAD_BLOCK_SIZE=64` line.
After this step, fully rebuild the Collector example. Also, the prebuilt image for Collector working in 2.4GHz, `<SDK_INSTALL_DIR>/prebuilt/bin/host_collector_2_4g` can also be used.

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
fxx: Set FW file from configured OAD FW dir. Example 'f oad_img_v2.bin'
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
Sensor 0x0001: FW Ver sv: 1.0 bv:0x96


Info: Sending 0x0001 FW Version Req
cmd: v
```

Note that the version of BIM must match the BIM's being used in the system.

Generate a v2 application binary (see [Generating the Required Binary Images](#GeneratingtheRequiredBinaryImages) section) and copy it to the Linux machine. Paste the '.bin' file in the following location: `<SDK_INSTALL_DIR>/prebuilt/bin/`.

The FW image file to update can be selected using the `f <path_to_File/file_name.bin>` command:

```shell
TI Collector
Nwk: Started
Sensor 0x0001: FW Ver sv: 1.0 bv:0x96


Info: OAD file ../../prebuilt/bin/sensor_oad_onchip_secure_CC1314R10_LAUNCHXL_tirtos_ccs.bin
cmd: f ../../prebuilt/bin/sensor_oad_onchip_secure_CC1314R10_LAUNCHXL_tirtos_ccs.bin
```

Entering just `f` will report the currently selected file.

A FW update of the selected sensor can be initiated with the `w` command. The next time the sensor polls it will process the request and if the FW update is accepted, the sensor will start requesting OAD image blocks:

```shell
TI Collector
Nwk: Started
Sensor 0x0001: Temp 25, RSSI -18


Info: Sending 0x0001 FW Update Req
cmd: w
```

The sensor will display the OAD progress through the UART output:

```shell
  TI Sensor (onchip) [MCUBOOT]
 Press Enter for Help
<      HELP      >

Sensor Status: Joined - Mode=NBCN, Addr=0001, PanId=ffff, Ch=0
OAD Status: Transferring Block 1 of 975
```

Once OAD is complete, the sensor will reboot. The sensor should rejoin the network.

After restarting:

```shell
  TI Sensor (onchip) [MCUBOOT]
 Press Enter for Help
<      HELP      >

Sensor Status: Rejoined - Mode=NBCN, Addr=0001, PanId=ffff, Ch=0
OAD Status: --
```

The Linux Collector can then verify that the new FW version is running on the sensor with the `v` command:

```shell
TI Collector
Nwk: Started
Sensor 0x0001: FW Ver sv: 1.0 bv:0x96


Info: Sending 0x0001 FW Version Req
cmd: v
```

## <a name="GeneratingtheRequiredBinaryImages"></a>Generating the Required Binary Images

For OAD projects on Thor boards, MCUboot is used as the bootloader. To build MCUboot to work with the Thor OAD examples out of the box, first import the project from `examples/nortos/[Desired Board]/mcuboot_app/mcuboot`. After importing the project, we need to make a few changes; by default, MCUboot uses up all the on-chip flash for both images. We need to leave 0x4000 bytes of on-chip flash open at 0xFC000 for persistent memory between OAD images to store network and OAD information. The basic sensor OAD application also takes up a lot less space than half of the device's flash, so to cut down on OAD time we allocate 0x26000 bytes to each slot. 

To configure MCUboot with these settings, open the MCUboot sysconfig file in the MCUboot project and set the following configurables:
* For the `Upgrade Mode` configurable, choose `Overwrite`. This enables overwrite mode, which is what our OAD process uses.
* For the `Enable External Flash` configurable:
    * If using an off-chip OAD project, choose true (checked)
    * Otherwise if using an on-chip OAD project, choose false (unchecked).
* In the `Image 1` module, if using a CC13X4 platform, modify the following configurables:
    * Primary image - Base address:     0x00006000
    * Primary image - Image size:       0x00026000
    * Secondary image - Base address:   0x0002C000 for on-chip OAD, 0x0 for off-chip OAD
    * Secondary image - Image size:     0x00026000

After configuring the bootloader, we can now take a look at the Sensor OAD project. By default, binary images will be built with the following post-build step, using a prebuilt binary of the MCUboot image tool:

`<SDK_INSTALL_DIR>/tools/common/mcuboot/imgtool sign --header-size 0x80 --align 4 --slot-size 0x26000 --pad --version 1.0.0 --pad-header --key <SDK_INSTALL_DIR>/source/third_party/mcuboot/root-ec-p256.pem ${ProjName}-noheader.bin ${ProjName}.bin`

* To take a look at the source code for the image tool, find it at to `<SDK_INSTALL_DIR>/source/third_party/mcuboot/scripts/imgtool.py`.

This will generate an image with an MCUboot header size of 128 bytes, a slot size of 0x26000 bytes, and mark it as version 1. The pad option ensures a trailer will be inserted at the end of the image marking it as upgradeable, so an image generated like this but with a higher version number placed in the secondary slot (0x81000 by default) will be booted into after a reset. Specifically, MCUboot will compare the two version numbers, and run whichever one is higher. If the higher numbered image is in the secondary slot in flash, then it will erase the primary slot and write over it with the secondary slot. 

If you wish to change either the slot size or the base of the slot, be sure to update `PRIMARY_SLOT_BASE` and `SLOT_SIZE`, respectively. If you change the slot size, also make sure to update the post-build step parameter `slot-size` and the `SLOT_SIZE` define in the linker file to match. These changes will also need to be reflected in the bootloader itself, in `flash_map_backend/flash_map_backend.c`. 

For further information related to MCUboot and potential configurations, check out https://docs.mcuboot.com/. 

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
   * Set Region Base based on the formula `0x100000 - (NVOCMP_NVPAGES * 0x2000)`

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
* `FEATURE_BLE_OAD`: This allows the 15.4 application to revert to the persistent application to perform an OAD. Currently unused for MCUboot.
* `SECURITY`: This includes the OAD secure signing. This results in an application binary that is signed using an AES encryption of SHA2 hash of the image. Note that this is currently not used for the MCUboot OAD process, as MCUboot has it's own signing process.

### <a name="CCFGConfiguration"></a>CCFG Configuration

For OAD applications, the Customer Configuration (CCFG) area must be configured in the MCUboot project.
