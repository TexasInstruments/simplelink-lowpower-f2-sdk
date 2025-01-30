TI 15.4-Stack Collector SM Example
=======================

## Table of Contents

* [Example Summary](#ExampleSummary)
* [Peripherals Exercised](#PeripheralsExercised)
* [Resources & Jumper Settings](#Resources&JumperSettings)
* [Example Application Dataflow](#ExampleApplicationDataflow)
  * [Callbacks for Incoming Data Packets](#IncomingData)
* [Example Usage](#ExampleUsage)
* [Analyzing Network Traffic](#AnalyzingNetworkTraffic)
* [Project Configuration](#ProjectConfiguration)
  * [Disabling Common User Interface](#DisablingCommonUserInterface)
  * [Multi-Page NV Configuration](#MultiPageNVConfiguration)
  * [IAR Configuration](#IARConfiguration)
  * [SBL Binary File Generation When Using Thor](#sbl-binary-file-generation-when-using-thor)

## <a name="ExampleSummary"></a>Example Summary

The collector secure manager (SM) example application demonstrates how to implement a Personal Area Network (PAN) Coordinator using TI 15.4-Stack with secure commissioning. A SM-enabled TI 15.4-Stack based star network consists of two types of logical devices: the SM-enabled PAN-Coordinator and the SM-enabled network devices, e.g. the Collector SM and Sensor SM applications, respectively. These devices function like their regular counterparts, but include provisions for authentication and secure device-key generation.

The SM-enabled PAN-Coordinator initiates the secure manager process and controls whether a device is accepted into the network or rejected and disassociated in the event of a failure. To be accepted, a network device must be verified using a mutually supported authentication method, and successfully generate a unique device key to be used for unicast communication. Broadcast communication will still be encrypted with the pre-shared key.

Key refreshments are also handled by the SM-enabled PAN-Coordinator and occur on a timeout basis.

The example applications in TI 15.4-Stack are developed for the CC13x2 Launchpad platform. In addition, the Linux example applications for the external host (AM335x + MAC Coprocessor) are located in the [**TI 15.4-Stack Gateway SDK**](http://www.ti.com/tool/ti-15.4-stack-gateway-linux-sdk).

> The project names for CC1352 and CC2652 platforms are referred to as CC13x2 or CC26x2. Replace x with either **1** or **5** depending on the specific wireless MCU being used.

> Note that this also includes the CC1352P-X boards, where the X represents which board subset is used, and the power amplification range.

> Although the application code for the collector SM example is the same for each platform, there are different example projects, named as collector_sm_x. Replace x with the device name of the specific Launchpad used.

## <a name="PeripheralsExercised"></a>Peripherals Exercised

> To trigger various events, buttons can be used as well as the configurable user interface.
> The [Example Usage](#ExampleUsage) section of this document explains how to use the user interface, although both the button presses and the UART perform the
> same actions.

* `CONFIG_LED_RED` - Set when the collector application is initialized. Flashes when the network is open for joining.
* `CONFIG_BTN_LEFT` - Press to initialize the collector application.
* `CONFIG_BTN_RIGHT` - Press to allow new devices to join the network. While the network is open for joining, `CONFIG_LED_RED` will flash. Press again to disallow joining.

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

The collector application has three processing loops each handling a different set of
events. These are as follows:

* Collector_process: Collector application event handling
	* Collector event handling
		* Network start (COLLECTOR_START_EVT)
		* Permit join toggle (COLLECTOR_OPEN_NWK_EVT/COLLECTOR_CLOSE_NWK_EVT)
		* Generate tracking message (COLLECTOR_TRACKING_TIMEOUT_EVT)
		* Generate config message (COLLECTOR_CONFIG_EVT)
	* Triggers Cllc_process and Csf_processEvents
	* Triggers MAC callback handling via ApiMac_processIncoming
* Cllc_process: Collector logical link controller event handling
	* Trickle timer handling (PAN Advertisement/PAN Configuration message events)
	* State change processing
	* Join permit timeout handling
* Csf_processEvents: External input handling
	* CUI input handling
	* Button press input handling
	* Triggers events in collector/cllc processing loops based on input

All three processing loops handle specialized tasks to service collector functionality.
Additionally, ApiMac_processIncoming will trigger collector and cllc callbacks if
they are defined, acting as the trigger for several collector and cllc processing loop
events.

An overview of the collector cllc states and state transitions is as follows:

	          Cllc_states_initWaiting
	                     | COLLECTOR_START_EVT, initiated by CSF_KEY_EVENT,
	                     | COLLECTOR_UI_INPUT_EVT, or by defining AUTO_START
	                     |
	    Existing         |          New
	    Network          |          Network
		   +-------------+-------------+
		   |                           |
		   V                           V
	  Cllc_states_                Cllc_states_
	  initRestoringCoordinator    startingCoordinator
		   |                           |
		   V                           V
	  Cllc_states_                Cllc_states_
	  restored                    started
		   |                           |
		   +-------------+-------------+
	                     | CSF_KEY_EVENT/COLLECTOR_UI_INPUT_EVT triggers
	                     | COLLECTOR_OPEN_NWK_EVT or COLLECTOR_CLOSE_NWK_EVT,
	                     | triggering Cllc_setJoinPermit
	                     |
	    Permit join      |          Permit join
	    Off              |          On
		   +-------------+-------------+
		   |                           |
		   V                           V
	  Cllc_states_                Cllc_states_
	  joiningNotAllowed           joiningAllowed

### <a name="IncomingData"></a>Callbacks for Incoming Data Packets

Aa described above, ApiMac_processIncoming processes all incoming messages from the MAC layer and calls the corresponding callback in the application layer. For incoming data packets, one of these two callbacks will be triggered:

* Data Indication Callback: Triggered when the MAC has successfully processed a valid data frame
* Comm Status Indication Callback: Triggered when a data frame is rejected due to a security failure. The status code in the ApiMac_mlmeCommStatusInd_t struct will contain more details regarding the error. Note that this is only applicable if MAC security is not disabled.

## <a name="ExampleUsage"></a>Example Usage

This example project implements a SM-enabled collector device: the PAN-Coordinator for the secure network. This device creates a TI 15.4-Stack secure network, commissions sensor devices, collects sensor information sent by devices running the sensor SM example application, and tracks if the devices are on the network or not by periodically sending tracking request messages. This example assumes a second Launchpad is running the default sensor SM application code.

The example output can be viewed through the UART terminal.

* Open a serial session (e.g. [PuTTY](http://www.putty.org/ "PuTTY's
Homepage"), etc.) to the appropriate COM port with the following settings.

* Note that if you are using Tera Term, by default, the Backspace key will be replaced with the delete key.
If you go to Setup > Keyboard There is a section called `Transmitting DEL by:`
Make sure that the backspace character is checked as well.

> The COM port can be determined via Device Manager in Windows or via
`ls /dev/tty*` in Linux.

* Upon example start, the connection will have the following settings:
```
    Baud-rate:     115200
    Data bits:          8
    Stop bits:          1
    Parity:          None
    Flow Control:    None
```
and initially display the following text on the UART terminal.

Main Menu:
```
 TI Collector
 Press Enter for Help
<      HELP      >

Security Status: --
Status: Waiting...
Device Status: --
Number of Joined Devices: --
```

The configurable user interface is provided to allow you to make changes at runtime, as follows:
```
    0xFFFF

<      SET PANID      >

Security Status: --
Status: Waiting...
Device Status: --
Number of Joined Devices: --
```

```
 00 00 0F 00 00 00 00 00 00 00 00 00 00 00 00 00 00

<    SET CHAN MASK    >

Security Status: --
Status: Waiting...
Device Status: --
Number of Joined Devices: --
```

```
 12 34 56 78 9A BC DE F0 00 00 00 00 00 00 00 00

<     SET NWK KEY     >

Security Status: --
Status: Waiting...
Device Status: --
Number of Joined Devices: --
```

```
  DEFAULT CODE

<     AUTH METHOD     >

Security Status: --
Status: Waiting...
Device Status: --
Number of Joined Devices: --
```

Note that these changes will only take effect if the collector is in a waiting state.
Keys 0-F can be used to change the value when in edit mode, and left/right keys can be used for navigating the digits.
Once the collector is started, the settings can only be changed if it is restarted.

> If the **AUTO_START** symbol is defined in your application, then the application will automatically configure itself on startup.
> This is not enabled by default with the project, but it can be configured as described below.

**AUTO_START** can be defined by removing the x in the .opt file under the defines folder
: `-DxAUTO_START > -DAUTO_START`

> If **AUTO_START** is defined, the collector will display `Starting...` instead of `Waiting...`
After the collector has been started, permit join must be turned on, and this can be done either with the right button press on the collector, or through the user interface as shown below.

> If **AUTO_START** is not defined, then the collector will not open a network unless it is started with the left button press, and permit join is turned on with the right button press.
Alternatively, the configurable user interface will allow you to start the collector, and open the network for sensors to join using the following menu options under the `NETWORK ACTIONS` tab:

To FORM Network:
```
 TI Collector

<       FORM NWK      >

Security Status: --
Status: Waiting...
Device Status: --
Number of Joined Devices: --
```

To OPEN Network:

```
 TI Collector

<       OPEN NWK      >

Security Status: --
Status: Waiting...
Device Status: --
Number of Joined Devices: --
```

To CLOSE Network:

```
 TI Collector

<       CLOSE NWK     >

Security Status: --
Status: Waiting...
Device Status: --
Number of Joined Devices: --
```

* Start the application by pressing `CONFIG_BTN_LEFT` or selecting `FORM NWK` under the `NETWORK ACTIONS` tab.
```
 TI Collector
 Press Enter for Help
<      HELP      >

Security Status: --
Status: Starting...
Device Status: --
Number of Joined Devices: 0
```

After starting, sensor specific application information will be displayed, such as the devices current state.
```
 TI Collector

<   NETWORK ACTIONS   >

Status: Started--Mode=NBCN, Addr=0xaabb, PanId=0x0001, Ch=0, PermitJoin=Off
Device Status: --
Number of Joined Devices: 0
```

Now turn on permit join using `CONFIG_BTN_RIGHT` or by selecting `OPEN NWK` under the `NETWORK ACTIONS` tab.
Once the network is started, and sensors begin to join, each of the status lines will update accordingly.
The network coordinator (Collector) will update the Security Status line with the Commissioning progress:

Security Status: Commissioning Started: 0x0001
Security Status: Commissioned: 0x0001

and in the case of any commissioning failures:
Security Status: Commissioning Failed: 0x0001

```
 TI Collector

<   NETWORK ACTIONS   >

Security Status: Commissioned: 0x0001
Status: Started--Mode=NBCN, Addr=0xaabb, PanId=0x0001, Ch=0, PermitJoin=On
Device Status: Sensor - Addr=0001, Temp=29, RSSI=-14
Number of Joined Devices: 1
```

* Key refreshments will occur based on a timeout period set on the collector SM application.
The Security Status line will be updated with one of the following:
```
    Key Refreshment Started: 0x1
    Key Refreshed: 0x1
```

* The network coordinator (Collector) needs to keep two unique keys per device (Seed Key and Device Key). Due to the limit of NV space as of today, all of the keys cannot be stored to the NV space. The NV storage only keeps the sensor device information, not key information.
For real products, it is recommended to store all the keys to internal flash or external flash. Note that the key may need to be encrypted if an external flash space is used for key storage.
The key information in the RAM memory will be lost after power cycle or reset. So, to improve the out of box experience for the secure commission feature, the collector is expected to work normally even after reset or power cycle and following solution has been applied:
The collector recommissions all the devices that have been in its network before it experienced a power glitch. In FH mode of operation, the sensor quickly loses sync with coordinator when the coordinator experiences a power glitch and hence, might not receive the packets for recommissioning sent by coordinator and hence the collector prints the following message.
```
    Commissioning Postponed: 0x1
```
In this case, it is expected to wait for a time where the sensor recognizes it got orphaned and joins the network back, when commissioning will happen again and communication between sensor and coordinator is restored.
As far as the sensor is concerned, the device stores the key info of the coordinator alone and hence, the same is stored in its NV.

## <a name="AnalyzingNetworkTraffic"></a>Analyzing Network Traffic

TI 15.4-Stack provides the means to analyze over-the-air traffic by including a packet sniffer firmware image. With an additional CC13x2 or CC26x2 Launchpad, users can set up a packet sniffer with the software provided in the SDK. More information about this can be found in the TI 15.4-Stack documentation under **Packet Sniffer**.

## <a name="ProjectConfiguration"></a>Project Configuration

The System Configuration (SysConfig) tool is a graphical interface for configuring your TI 15.4-Stack project. Based on the parameters configured in the SysConfig dashboard, C source files and header files are generated. Further advanced parameters can be located in `advanced_config.h`.

Some important settings in the TI 15.4-Stack module include:

| **Parameter**              | **SysConfig Location**            | **Description**                                               |
|----------------------------|-----------------------------------|---------------------------------------------------------------|
| Mode                       | Top of TI-15.4 Stack module       | Configures the mode of network operation                      |
| MAC Beacon Order           | MAC group within Network category | Configures how often the coordinator transmits a beacon       |
| MAC Super Frame Order      | MAC group within Network category | Configures the length of the active portion of the superframe |
| Channel Mask               | Network category                  | Configures channels to be scanned                             |
| Security Level             | Security category                 | Configures network security level                             |
| Pre-Shared Network Key     | Security category                 | Configures pre-shared network key                             |
| Authentication Methods     | Security category                 | Configures authentication methods supported by the collector  |
| Default Authentication Code| Security category                 | Configures pre-defined code used for authentication           |
| Key Refresh Timeout        | Security category                 | Configures time in secs between sensor key refreshments       |

> SysConfig generated files are dynamically generated upon build, and any manual changes to them will be overwritten.

More information about the configuration and feature options can be found in the TI 15.4-Stack documentation under **Example Applications > Configuration Parameters**.

### <a name="DisablingCommonUserInterface"></a>Disabling Common User Interface

The common user interface (CUI) is a UART based interface that allows users to control and receive updates regarding the application. For various reasons, including reducing the memory footprint, the user is able to disable the common user interface (CUI). To disable the CUI, the following variable must be defined in the project-specific .opt file:

```
-DCUI_DISABLE
```

> Please Note: particular features that are dependent on the CUI will be unavailable when this feature is enabled.

### <a name="MultiPageNVConfiguration"></a>Multi-Page NV Configuration

By default, this project is configured to use four pages of NV. A maximum of five NV pages are supported. In order to modify the NV pages, update the following:

* `NVOCMP_NVPAGES=4` in the project-specific .opt file
* SysConfig NVS module:
   * Set Region Size based on the formula `NVOCMP_NVPAGES * 0x2000`
   * Set Region Base based on the formula `0x56000 - (NVOCMP_NVPAGES * 0x2000)`

A detailed description of the application architecture can be found in your installation within the
TI-15.4 Stack Getting Started Guide's Application Overview section: `<SDK_INSTALL_DIR>/docs/ti154stack/ti154stack-getting-started-guide.html`.

### <a name="IARConfiguration"></a>IAR Configuration

When using the CC13x2 SDK, the TI XDS110v3 USB Emulator must
be selected. For the CC13x2_LAUNCHXL, select TI XDS110 Emulator. In both cases,
select the cJTAG interface.

In order to build from flash, within the IAR Project options > Build Actions
Update the "Pre-build command line" and change the "NO_ROM=0" to "NO_ROM=1".

### <a name="ThorSBLConfiguration"></a>SBL Binary File Generation When Using Thor

When generating a binary file for a Thor device take care not to generate a file containing
both the code and the CCFG, as the CCFG is located at 0x5000000 in flash. This will result
in a massive binary file, as binary file will include padding for all the space between the 
application code and the CCFG. Instead, you should generate two binary files - One containing
the CCFG, the other containing all other code. To do this, add 
`${CG_TOOL_ROOT}/bin/tiarmobjcopy ${ProjName}.out --output-target binary ${ProjName}-code.bin --remove-section=.ccfg`
and 
`${CG_TOOL_ROOT}/bin/tiarmobjcopy ${ProjName}.out --output-target binary ${ProjName}-ccfg.bin --only-section=.ccfg`
to your post build steps to generate two separate binary files. When using the SBL tool, flash the binary file containing 
the code first, then flash the CCFG. If you flash the CCFG beforehand, you may disable the Serial BootLoader.

You can also flash using a hex file, if you don't want to deal with split binary files.
