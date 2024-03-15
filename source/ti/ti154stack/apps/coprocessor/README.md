TI 15.4-Stack Coprocessor Example
============================

## Table of Contents

* [Example Summary](#ExampleSummary)
* [Peripherals Exercised](#PeripheralsExercised)
* [Resources & Jumper Settings](#Resources&JumperSettings)
* [Example Usage](#ExampleUsage)
* [Analyzing Network Traffic](#AnalyzingNetworkTraffic)
* [Project Configuration](#ProjectConfiguration)
  * [Multi-Page NV Configuration](#MultiPageNVConfiguration)
  * [IAR Configuration](#IARConfiguration)
  * [Building for CC2652P](#BuildingforCC2652P)

## <a name="ExampleSummary"></a>Example Summary

The coprocessor example application demonstrates how to implement a Media Access Control (MAC) coprocessor device designed to work with a host processor in a two-chip scenario. The coprocessor provides an interface to the TI 15.4-Stack protocol stack with full-function MAC capability through a serial interface. Through this interface the application, running on the host, allows for the addition of TI 15.4-Stack wireless functionality to systems that are not suited to single-chip solutions.

The example applications in TI 15.4-Stack are developed for the CC13x2 Launchpad platform. In addition, the Linux example applications for the external host (AM335x + MAC Coprocessor) are located in the [**TI 15.4-Stack Gateway SDK**](http://www.ti.com/tool/ti-15.4-stack-gateway-linux-sdk).

> The project names for CC1352 and CC2652 platforms are referred to as CC13x2 or CC26X2. Replace X with either **1** or **5** depending on the specific wireless MCU being used.

> Note that this also includes the CC1352P-X boards, where the X represents which board subset is used, and the power amplification range.

> See `ti_154stack_features.h` for the default application features that are enabled.

## <a name="PeripheralsExercised"></a>Peripherals Exercised

> To trigger various events, buttons can be used as well as the configurable user interface.
> The [Example Usage](#ExampleUsage) section of this document explains how to use the user interface, although both the button presses and the CUI perform the
> same actions.

* `CONFIG_UART_TX` `CONFIG_UART_RX` - UART transmit and receive lines available through the backchannel CC13x2 Launchpad USB connection. This is used as the serial connection to the host processor.

## <a name="Resources&JumperSettings"></a>Resources & Jumper Settings

The following hardware is required to run TI 15.4-
Stack Out of Box (OOB) example applications:

* Two [**CC13x2 Launchpad development kits**](http://www.ti.com/tool/launchxl-cc1352r1) or two [**CC26x2 Launchpad development kits**](http://www.ti.com/tool/launchxl-cc26x2r1)

> If you're using an IDE (such as CCS or IAR), please refer to `Board.html` in
your project directory for resources used and board-specific jumper settings.
Otherwise, you can find `Board.html` in the directory
`<SDK_INSTALL_DIR>/source/ti/boards/<BOARD>`.

Please refer to the following link for helpful SimpleLink Academy guides for ramping up
on TI 15.4-Stack: [TI 15.4-Stack SimpleLink Academy](https://dev.ti.com/tirex/explore/node?node=ABRXrYdFS1e-0P3PY6NmNg__pTTHBmu__LATEST).

For an in-depth overview of the TI 15.4-Stack application, please refer to the TI 15.4-Stack User Guide at
`<SDK_INSTALL_DIR>/docs/ti154stack/html/ti154stack/application-overview.html#application-overview`).

## <a name="ExampleUsage"></a>Example Usage

This example project implements a coprocessor device and as such does not directly provide a user interface.
After flashing the coprocessor application to the target device, the host collector application will output information
regarding the status of the coprocessor.

## <a name="AnalyzingNetworkTraffic"></a>Analyzing Network Traffic

TI 15.4-Stack provides the means to analyze over-the-air traffic by including a packet sniffer firmware image. With an additional CC13x2 Launchpad, users can set up a packet sniffer with the software provided in the SDK. More information about this can be found in the TI 15.4-Stack documentation under **Packet Sniffer**.

## <a name="ProjectConfiguration"></a>Project Configuration

> SysConfig generated files are dynamically generated upon build, and any manual changes to them will be overwritten.

### <a name="MultiPageNVConfiguration"></a>Multi-Page NV Configuration

By default, this project is configured to use four pages of NV. A maximum of five NV pages are supported. In order to modify the NV pages, update the following:

* `NVOCMP_NVPAGES=4` in the project-specific .opt file
* SysConfig NVS module:
   * Set Region Size based on the formula `NVOCMP_NVPAGES * 0x2000`
   * Set Region Base based on the formula `0x56000 - (NVOCMP_NVPAGES * 0x2000)`

A detailed description of the application architecture can be found within the TI 15.4-Stack Coprocessor Interface Guide
within the [**TI 15.4-Stack Gateway SDK**](http://www.ti.com/tool/ti-15.4-stack-gateway-linux-sdk) installation.
<LINUX_SDK_INSTALL_DIR>/docs/TI-15.4 Stack_CoP_Interface_Guide.pdf.

An overview of the application can be found in your installation within the
TI-15.4 Stack Getting Started Guide's Application Overview section: `<SDK_INSTALL_DIR>/docs/ti154stack/ti154stack-getting-started-guide.html`.

### <a name="IARConfiguration"></a>IAR Configuration

When using the CC13x2 SDK, the TI XDS110v3 USB Emulator must
be selected. For the CC13x2_LAUNCHXL, select TI XDS110 Emulator. In both cases,
select the cJTAG interface.

### <a name="BuildingforCC2652P"></a>Building for CC2652P

Note that to build the coprocessor for the CC2652P, you will need to use the CC1352P-4 coprocessor project. Additionally,
you will need to manually enable the `COPROCESSOR_CC2652P` symbol for the build. You can do this by removing the `x` in
front of the definition of this symbol in the `application/defines/coprocessor.opt` file.
