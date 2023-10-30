# SIMPLELINK CC13XX/CC26XX SDK ENG
This repository contains an engineering subset of TI's
[TI's SimpleLink Low Power F2][cc13xx_cc26xx_sdk]
(previously CC13XX/CC26XX) Software Development Kit (SDK)
and was specifically created for use with open source projects such
as [Matter][matter_repo].

An overview of the full SDK is hosted on [TI Resource Explorer][ti_rex].


## Features

- Update POSIX Timer to resolve double timer callback calls
- Update SHA2 implementation to disable Power management in driver
- Update BLE Util layer to recover from message queue failure gracefully
- Checked-in Pre-built drivers for 7.10.01.24
- Resolve incorrect DMM library reference

## Supported Devices

The following device(s) are currently supported:
* [CC2652R7][cc2652r7]
* [CC1354P10][cc1354p10]
* [CC2674P10][cc2674p10]
* [CC2674R10][cc2674r10]

## Components
The software components contained are:
* BLE stack with FreeRTOS supported
* DMM (Dynamic Multi-protocol Manager) with FreeRTOS supported
* FreeRTOS build with TI-POSIX extensions
* Device drivers and tools (TI Drivers, rflib, Sysconfig files)

## More Information

For licensing information, please refer to [licensing].

For technical support, please consider creating a post on TI's [E2E
forum][e2e].  Additionally, we welcome any feedback.

[cc13xx_cc26xx_sdk]: https://www.ti.com/tool/download/SIMPLELINK-CC13XX-CC26XX-SDK
[matter_repo]: https://github.com/project-chip/connectedhomeip
[cc2652r7]: https://www.ti.com/product/CC2652R7
[cc1354p10]: https://www.ti.com/tool/LP-EM-CC1354P10?keyMatch=CC1354P10-6
[cc2674p10]: https://www.ti.com/product/CC2674P10
[cc2674r10]: https://www.ti.com/product/CC2674R10
[licensing]: todo:link_to_ti_text_license
[e2e]: https://e2e.ti.com/support/wireless-connectivity/zigbee-and-thread
[ti_rex]: https://dev.ti.com/
