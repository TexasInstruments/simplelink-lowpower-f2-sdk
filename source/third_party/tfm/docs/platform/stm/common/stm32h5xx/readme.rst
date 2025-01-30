-------
STM32H5
-------

TF-M is supported on STM32H5 family

https://www.st.com/en/microcontrollers-microprocessors/stm32h5-series.html


Directory content
^^^^^^^^^^^^^^^^^

- stm/common/stm32h5xx/stm32h5xx_hal:
   Content from https://github.com/STMicroelectronics/stm32h5xx_hal_driver

- stm/common/stm32h5xx/Device:
   Content from https://github.com/STMicroelectronics/cmsis_device_h5

- stm/common/stm32h5xx/bl2:
   stm32h5xx bl2 code specific from https://github.com/STMicroelectronics/STM32CubeH5.git (Projects/STM32H573I_DK/Applications/TFM)

- stm/common/stm32h5xx/secure:
   stm32h5xx Secure porting adaptation from https://github.com/STMicroelectronics/STM32CubeH5.git (Projects/STM32H573I_DK/Applications/TFM)

- stm/common/stm32h5xx/boards:
   Adaptation and tools specific to stm32 board using stm32h5xx soc from https://github.com/STMicroelectronics/STM32CubeH5.git (Projects/STM32H573I_DK/Applications/TFM)

- stm/common/stm32h5xx/CMSIS_Driver:
   Flash and uart driver for stm32h5xx platform

- stm/common/stm32h5xx/Native_Driver:
   Random generator and tickless implementation

Specific Software Requirements
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

STM32_Programmer_CLI is required.(see https://www.st.com/en/development-tools/stm32cubeprog.html)

Limitations to Consider When Using the Platform
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
MPU and ICACHE disabled in bl2 boot stage

Write software on target
^^^^^^^^^^^^^^^^^^^^^^^^
In build folder:

  - ``postbuild.sh``: Updates regression.sh and TFM_UPDATE.sh scripts according to flash_layout.h
  - ``regression.sh``: Sets platform option bytes config and erase platform
  - ``TFM_UPDATE.sh``: Writes bl2, secure, and non secure image in target


Connect board to USB and Execute the 3 scripts in following order to update platform:
postbuild.sh, regression.sh, TFM_UPDATE.sh

The virtual com port from STLINK is used for TFM log and serial port configuration should be:

  - Baud rate    = 115200
  - Data         = 8 bits
  - Parity       = none
  - Stop         = 1 bit
  - Flow control = none

-------------

*Copyright (c) 2023 STMicroelectronics. All rights reserved.*
*SPDX-License-Identifier: BSD-3-Clause*