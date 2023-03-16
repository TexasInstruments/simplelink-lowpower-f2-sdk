Overview
--------
Proprietary RF examples are intended to demonstrate various use cases of the 
RF Driver in order to provide a starting point for customers to use as is for testing 
or extend to suit their application use cases.


Changelog
---------

### Version 3.10.00
- added support for CC1352P_4_LAUNCHXL
- added support for custom boards in SysConfig

### Version 3.20.00

- Added support for LP-CC2652RSIP and LP-CC2652PSIP modules

### Version 3.30.00

- Added support for limited migration between devices/boards in SysConfig

### Version 3.30.01

- Added support for LP_CC1312R7, LP_CC1352P7_1, LP_CC1352P7_4, and LP_CC2652R7

### Version 4.10.00

- Added TI Clang compiler support

### Version 4.20.00

- Added support for CC1311P3, CC2651P3, and CC2651R3

### Version 4.30.00

- GPIO driver has replaced pin driver 
- TIRTOS7 is used for CC13X2X7, CC26X1, and CC13X1 devices

### Version 4.40.00

- Added support for FreeRTOS
- Added CC2652PSIP suppport
- TIRTOS7 is used for CC13X2 and CC26X2 devices

### Version 5.10.00

- Added CC2651R3SIPA support

### Version 5.20.00

- Removal of TI ARM CCS compiler support (TI Clang still supported)

### Version 5.30.00

- Support for CC1312PSIP device
- NoRTOS application supports CC13X4 devices

### Version 5.40.00

- SPI support for AT interface in rfDiagnostics example 

### Version 6.10.00

- Secure non-secure rfPacketTx/Rx examples, tfm_rfPacketRx/Tx, added for CC13x4 devices