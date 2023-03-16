#########
stm32l5xx
#########
'platform/ext/stm/stm32l5xx' folder contains code that has been imported from
other projects available on https://github.com/STMicroelectronics.

***********
Sub-folders
***********

stm32l5xx_hal
=============
Content from https://github.com/STMicroelectronics/stm32l5xx_hal_driver.git

Device
======
Content from https://github.com/STMicroelectronics/cmsis_device_l5.git

bl2
===
stm32l5xx bl2 code specific from https://github.com/STMicroelectronics/STM32CubeL5.git
(Projects/STM32L562E-DK/Applications/TFM)

Secure
======
stm32l5xx Secure porting adaptation from https://github.com/STMicroelectronics/STM32CubeL5.git
(Projects/STM32L562E-DK/Applications/TFM)

boards
======
Adaptation and tools specific to stm32 board using stm32l5xx soc
from https://github.com/STMicroelectronics/STM32CubeL5.git
(Projects/STM32L562E-DK/Applications/TFM)

CMSIS_Driver
============
Flash and uart driver for stm32l5xx platform

Native Driver
=============
random generator and tickless implementation

***********************
Documentation on boards
***********************
Following links provide information on the supported boards:
https://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-mpu-eval-tools/stm32-mcu-mpu-eval-tools/stm32-discovery-kits/stm32l562e-dk.html

https://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-mpu-eval-tools/stm32-mcu-mpu-eval-tools/stm32-nucleo-boards/nucleo-l552ze-q.html

***************************************
Building and running software on target
***************************************

Specific Software Requirements
==============================

STM32_Programmer_CLI is required.(see https://www.st.com/en/development-tools/stm32cubeprog.html)

Configuration and Build
=======================
GNUARM compilation is available for targets  STM_NUCLEO_L552ZE_Q and STM_DISCO_L562QE
and build the selected configuration as follow.

``>cmake --build ./  -- install``

Write software on target
========================
In install folder:

  - ``postbuild.sh``: Updates regression.sh and TFM_UPDATE.sh scripts according to flash_layout.h.
  - ``regression.sh``: Sets platform option bytes config and erase platform.
  - ``TFM_UPDATE.sh``: Writes bl2, secure, and non secure image in target.


Connect board to USB and Execute the 3 scripts in following order to update platform:
postbuild.sh, regression.sh, TFM_UPDATE.sh

The virtual com port from STLINK is used for TFM log and serial port configuration should be:

  - Baud rate    = 115200
  - Data         = 8 bits
  - Parity       = none
  - Stop         = 1 bit
  - Flow control = none

-------------

*Copyright (c) 2019 STMicroelectronics. All rights reserved.*
*SPDX-License-Identifier: BSD-3-Clause*

