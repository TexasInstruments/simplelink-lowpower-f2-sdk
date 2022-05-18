/******************************************************************************
*  Filename:       aux_compa_doc.h
*
*  Copyright (c) 2015 - 2022, Texas Instruments Incorporated
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*
*  1) Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2) Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*
*  3) Neither the name of the ORGANIZATION nor the names of its contributors may
*     be used to endorse or promote products derived from this software without
*     specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
*  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
*  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
*  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
*  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
*  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
*  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
*  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/
//! \addtogroup auxcompa_api
//! @{
//! \section intro_auxcompa Introduction
//!
//! The COMPA module provides the low-level APIs to manage the
//! high-performance continuous-time comparator located in the AUX domain.
//!
//! The COMPA driverlib module supports the following features:
//! - Operating the COMPA peripheral in MCU active power mode
//! - Selecting the source connected to the positive and reference input of COMPA
//! - Registering/unregistering a callback on the interrupt triggered by COMPA's
//!   output, as well as clearing the interrupt flag
//! - Enabling and disabling the COMPA module
//!
//! COMPA supports the following inputs:
//! - Positive input from:
//!   - Analog-capable I/O pins
//! - Reference input from:
//!   - Analog-capable I/O pins
//!   - Internal chip voltages
//!   - Reference DAC
//!
//! The COMPA driverlib module has the following limitation:
//! - Cannot operate the COMPA peripheral in standby mode
//! - Cannot setup COMPA as a wakeup source from standby
//!
//! COMPA output is routed to the event bus where it holds a dedicated interrupt
//! line.
//!
//! On devices equipped with sensor controller, a CPU application that uses COMPA
//! must configure ownership of the module through AUX hardware semaphores. When
//! using the DAC as reference, ownership of both DAC and COMPA modules is
//! required. The table below shows the conventional matching between AUX
//! semaphores IDs and AUX peripherals:
//! | SMPH_ID | Resource Allocation |
//! |---------|---------------------|
//! | 1       | TDC                 |
//! | 2       | ADC + COMPB         |
//! | 3       | COMPA + ISRC        |
//! | 4       | Reference DAC       |
//! | 5       | Timer2              |
//! | 6       | Unassigned by TI    |
//! | 7       | Unassigned by TI    |
//!
//! COMPA operations are tightly integrated with the reference DAC present in the
//! AUX domain, which can be connected to the reference input of COMPA.
//! When using the DAC as reference for COMPA, it is discouraged to use the
//! RTOS-based DAC driver. The RTOS-based DAC driver has been designed to support
//! the specific use case of driving external loads, therefore it is not compatible
//! with the COMPA driverlib module. When using the DAC as reference for COMPA,
//! use the DAC driverlib module instead, which provides more flexibility and
//! can adapt to this particular use case.
//!
//! \section excode_auxcompa Code example
//! The following code example demonstrates how to set up COMPA with:
//! - an analog IO pin connected to its positive input
//! - the reference DAC connected to its reference input
//! in order to trigger an interrupt on a rising edge of its output.
//! \code
//! #include "inc/hw_types.h"
//! #include "inc/hw_ints.h"
//! #include "inc/hw_memmap.h"
//! #include "inc/hw_adi_4_aux.h"
//! #include "inc/hw_aux_aiodio.h"
//! #include "driverlib/aux_compa.h"
//! #include "driverlib/aux_dac.h"
//! #include "driverlib/aon_batmon.h"
//! #include "driverlib/ioc.h"
//!
//! #define DAC_CLOCK_FREQ_24MHZ        0U
//! #define MICROVOLTS_PER_MILLIVOLT    1000
//! #define CALIBRATION_VDDS            3000
//! #define REF_DAC_UVOLT               1500000
//!
//! static volatile uint32_t vdds = 0;
//!
//! void aux_compaISR(void)
//! {
//!     // Handle interrupt here
//!
//!     // Clear interrupt flag
//!     AUXCOMPAIntClear();
//! }
//!
//! void test_compa_interrupt_with_dacref(uint32_t in_iothDacOutput)
//! {
//!     uint32_t dacMax = 0;
//!     uint32_t dacMin = 0;
//!     uint32_t vdds = 0;
//!     uint32_t tempReg;
//!     bool auxSmphIsAcquired = false;
//!
//!     //
//!     // DIO setup
//!     //
//!
//!     // DIO 26 is setup to be used as a regular GPIO with output disabled, no pull and input buffer disabled
//!     // Internally DIO 26 is hardwired to AUXIO23
//!     IOCPortConfigureSet(IOID_26, IOC_PORT_GPIO, IOC_IOMODE_NORMAL | IOC_NO_IOPULL | IOC_INPUT_DISABLE );
//!     GPIO_setOutputEnableDio(GPIO_DIO_26_MASK, GPIO_OUTPUT_DISABLE);
//!
//!     //
//!     // AUX semaphores acquisition in case the device is equipped with sensor controller
//!     //
//!
//!     // Try acquiring AUX semaphore 3 belonging to COMPA+ISRC
//!     auxSmphIsAcquired = AUXSMPHTryAcquire(AUX_SMPH_3);
//!
//!     // Try acquiring AUX semaphore 4 belonging to the DAC
//!     auxSmphIsAcquired = AUXSMPHTryAcquire(AUX_SMPH_4);
//!
//!     //
//!     // DAC setup
//!     //
//!
//!     // Use BATMON (7 bits) to measure VDDS for DAC scaling (LaunchPads don't have exactly 3.3V on VDDS).
//!     // BatMon uses [3.8] format (8 fractional bits) which is converted to mV.
//!     AONBatMonEnable();
//!
//!     //wait for a BATMON measurement to be ready
//!     while(!AONBatMonNewBatteryMeasureReady()) {}
//!     vdds = (1000 * AONBatMonBatteryVoltageGet()) >> 8;
//!
//!     // Use VDDS as voltage reference for the DAC
//!     AUXDACSetVref(AUXDAC_VREF_SEL_VDDS);
//!
//!     // Retrieve the Min and Max values relative to the set voltage ref
//!     dacMin = AUXDACCalcMin() * MICROVOLTS_PER_MILLIVOLT;
//!     dacMax = ((AUXDACCalcMax() * vdds) / CALIBRATION_VDDS) * MICROVOLTS_PER_MILLIVOLT;
//!
//!     AUXDACSetSampleClock(DAC_CLOCK_FREQ_24MHZ);
//!
//!     // Set code to 1.5V and then enable the DAC with "not connected" output pin
//!     uint8_t dacCode = (uint8_t) AUXDACCalcCode (REF_DAC_UVOLT, dacMin, dacMax);
//!     AUXDACSetCode(dacCode);
//!
//!     AUXDACEnable(0x0);
//!
//!     //
//!     // COMPA setup
//!     //
//!
//!     AUXCOMPASelectGpioInput(AUXCOMPA_GPIO_IN_AUX23);
//!
//!     AUXCOMPASelectDacRef();
//!
//!     AUXCOMPAEnable();
//!
//!     // wait for reference DAC and COMPA to stabilize their outputs before enabling COMPA interrupt
//!
//!     AUXCOMPAIntRegister(AUXCOMPA_POL_RISE, aux_compaISR);
//!
//!     // Wait for interrupt to trigger
//!
//!     // When finished, release AUX semaphores, if sensor controller is present on the device
//!     AUXSMPHRelease(AUX_SMPH_3);
//!     AUXSMPHRelease(AUX_SMPH_4);
//! }
//! \endcode
//!
//! @}
