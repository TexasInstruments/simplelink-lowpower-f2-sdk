Nordic nRF5340
==============

The nRF5340 development kit (DK) is a single-board development kit for
the evaluation and development on the Nordic nRF5340 System-on-Chip (SoC).

The nRF5340 is a dual-core SoC based on the Arm® Cortex®-M33 architecture, with:

* a full-featured ARM Cortex-M33F core with DSP instructions, FPU, and
  ARMv8-M Security Extension, running at up to 128 MHz, referred to as
  the **Application MCU**
* a secondary ARM Cortex-M33 core, with a reduced feature set, running at
  a fixed 64 MHz, referred to as the **Network MCU**.

The nRF5340 Application MCU supports the Armv8m Security Extension.

Documentation
-------------

The following links provide useful information about the nRF5340

nRF5340 DK website:
   https://www.nordicsemi.com/Software-and-tools/Development-Kits/nRF5340-DK

Nordic Semiconductor Infocenter: https://infocenter.nordicsemi.com


Building TF-M on nRF5340 Application MCU
----------------------------------------

To build an S and NS application image for the nRF5340 Application MCU run the
following commands:


.. code-block:: bash

   cmake -S <TF-M base folder> -B build_spe \
           -DTFM_PLATFORM=nordic_nrf/nrf9160dk_nrf9160 \
           -DTFM_TOOLCHAIN_FILE=../toolchain_GNUARM.cmake \
   cmake --build build_spe -- install

   cmake -S <Application base folder> -B build_app \
           -DCONFIG_SPE_PATH<Absolute path to TF-M base folder>/build_spe/api_ns
           -DTFM_TOOLCHAIN_FILE=cmake/toolchain_ns_GNUARM.cmake
   cmake --build

.. note::
   Currently, applications can only be built using GCC (GNU ARM Embedded
   toolchain).

.. note::
   BL2 (MCUBoot) logging output to be available, the project needs to be built
   with Debug configuration (CMAKE_BUILD_TYPE=Debug).

Flashing and debugging with Nordic nRF Segger J-Link
-----------------------------------------------------

nRF5340 DK is equipped with a Debug IC (Atmel ATSAM3U2C) which provides the
following functionality:

* Segger J-Link firmware and desktop tools
* SWD debug for the nRF5340 IC
* Mass Storage device for drag-and-drop image flashing
* USB CDC ACM Serial Port bridged to the nRFx UART peripheral
* Segger RTT Console
* Segger Ozone Debugger

To install the J-Link Software and documentation pack, follow the steps below:

#. Download the appropriate package from the `J-Link Software and documentation pack`_ website
#. Depending on your platform, install the package or run the installer
#. When connecting a J-Link-enabled board such as an nRF5340 DK, a drive
   corresponding to a USB Mass Storage device as well as a serial port should come up

nRF Command-Line Tools Installation
*************************************

The nRF Command-line Tools allow you to control your nRF5340 device from the command line,
including resetting it, erasing or programming the flash memory and more.

To install them, visit `nRF Command-Line Tools`_ and select your operating
system.

After installing, make sure that ``nrfjprog`` is somewhere in your executable path
to be able to invoke it from anywhere.

BL2, S, and NS application images can be flashed into nRF5340 separately or may be merged
together into a single binary.

Flashing the nRF5340 DK
************************

To program the flash with a compiled TF-M image (i.e. S, NS or both) after having
followed the instructions to install the Segger J-Link Software and the nRF
Command-Line Tools, follow the steps below:

Generate Intel hex files from the output binary (bin) files as follows:

.. code-block:: console

   srec_cat build_app/tfm_s_ns_signed.bin -binary --offset 0x10000 \
         -o build_app/tfm_s_ns_signed.hex -intel

* Connect the micro-USB cable to the nRF5340 DK and to your computer
* Erase the flash memory in the nRF5340 IC:

.. code-block:: console

   nrfjprog --eraseall -f nrf53

* (Optionally) Erase the flash memory and reset flash protection and disable
   the read back protection mechanism if enabled.

.. code-block:: console

   nrfjprog --recover -f nrf91

* Flash the BL2 and the TF-M image binaries from the sample folder of your choice:

.. code-block:: console

   nrfjprog --program build_spe/bin/bl2.hex -f nrf91 --sectorerase
   nrfjprog --program build_app/tfm_s_ns_signed.hex -f nrf91 --sectorerase

* Reset and start TF-M:

.. code-block:: console

   nrfjprog --reset -f nrf53


Secure UART Console on nRF5340 DK
**********************************

SECURE_UART1 is enabled by default when building TF-M on nRF5340 DK, so the secure firmware console output
is available via USART1. To route the USART1 output to one of the serial ports, wire the default USART1 TxD
and RxD (pins P0.25 and P0.26) to RxD and TxD pins on the DK.

Non-Secure console output is available via USART0.

.. note::
   By default USART0 and USART1 outputs are routed to separate serial ports.

.. _nRF Command-Line Tools: https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools

.. _J-Link Software and documentation pack: https://www.segger.com/jlink-software.html

--------------

*Copyright (c) 2020, Nordic Semiconductor. All rights reserved.*
