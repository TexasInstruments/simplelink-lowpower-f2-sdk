CC1352 AGAMA FPGA RF driver chnage
===================================
In order to run the TIMAC 15.4 on FPGA platform, we need the RF driver code.
This example demonstrates how to change the RF driver code

- In RFCC26X2_multiMode.c, all changes are related to power management.
- all changes are guard by TIMAC_AGAMA_FPGA. So far only two places are required.
- open the RFCC26X2_multiMode.c from latest CoreSDK (whatever version you plan to use )
- make corresponding changes and override the current RFCC26X2_multiMode.c

- from CCS, rebuild the project


