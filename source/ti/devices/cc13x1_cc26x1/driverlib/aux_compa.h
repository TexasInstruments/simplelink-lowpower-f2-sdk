/******************************************************************************
*  Filename:       aux_compa.h
*
*  Description:     Defines and prototypes for the AUX high-performance
*                   continuous-time comparator.
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

//*****************************************************************************
//
//! \addtogroup aux_group
//! @{
//! \addtogroup auxcompa_api
//! @{
//
//*****************************************************************************

#ifndef __AUX_COMPA_H__
#define __AUX_COMPA_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include "../inc/hw_types.h"
#include "../inc/hw_ints.h"
#include "../inc/hw_memmap.h"
#include "../inc/hw_adi_4_aux.h"
#include "../inc/hw_aux_evctl.h"
#include "../inc/hw_aux_anaif.h"
#include "../inc/hw_aux_sysif.h"
#include "../inc/hw_aon_pmctl.h"
#include "interrupt.h"
#include "aon_event.h"
#include "adi.h"

//*****************************************************************************
//
// Defines that can be used to select an external reference through gpio
//
//*****************************************************************************
#define AUXCOMPA_GPIO_REF_AUX19     ADI_4_AUX_MUX4_COMPA_REF_AUXIO19
#define AUXCOMPA_GPIO_REF_AUX20     ADI_4_AUX_MUX4_COMPA_REF_AUXIO20
#define AUXCOMPA_GPIO_REF_AUX21     ADI_4_AUX_MUX4_COMPA_REF_AUXIO21
#define AUXCOMPA_GPIO_REF_AUX22     ADI_4_AUX_MUX4_COMPA_REF_AUXIO22
#define AUXCOMPA_GPIO_REF_AUX23     ADI_4_AUX_MUX4_COMPA_REF_AUXIO23
#define AUXCOMPA_GPIO_REF_AUX24     ADI_4_AUX_MUX4_COMPA_REF_AUXIO24
#define AUXCOMPA_GPIO_REF_AUX25     ADI_4_AUX_MUX4_COMPA_REF_AUXIO25
#define AUXCOMPA_GPIO_REF_AUX26     ADI_4_AUX_MUX4_COMPA_REF_AUXIO26
#define AUXCOMPA_GPIO_REF_NC        ADI_4_AUX_MUX4_COMPA_REF_NC

//*****************************************************************************
//
// Defines that can be used to select an internal voltage reference
//
//*****************************************************************************
#define AUXCOMPA_INT_REF_ADCVREFP   ADI_4_AUX_MUX0_COMPA_REF_ADCVREFP
#define AUXCOMPA_INT_REF_VDDS       ADI_4_AUX_MUX0_COMPA_REF_VDDS
#define AUXCOMPA_INT_REF_VSS        ADI_4_AUX_MUX0_COMPA_REF_VSS
#define AUXCOMPA_INT_REF_DCOUPL     ADI_4_AUX_MUX0_COMPA_REF_DCOUPL
#define AUXCOMPA_INT_REF_NC         ADI_4_AUX_MUX0_COMPA_REF_NC

//*****************************************************************************
//
// Defines that can be used to select an external input through gpio
//
//*****************************************************************************
#define AUXCOMPA_GPIO_IN_AUX19      ADI_4_AUX_MUX1_COMPA_IN_AUXIO19
#define AUXCOMPA_GPIO_IN_AUX20      ADI_4_AUX_MUX1_COMPA_IN_AUXIO20
#define AUXCOMPA_GPIO_IN_AUX21      ADI_4_AUX_MUX1_COMPA_IN_AUXIO21
#define AUXCOMPA_GPIO_IN_AUX22      ADI_4_AUX_MUX1_COMPA_IN_AUXIO22
#define AUXCOMPA_GPIO_IN_AUX23      ADI_4_AUX_MUX1_COMPA_IN_AUXIO23
#define AUXCOMPA_GPIO_IN_AUX24      ADI_4_AUX_MUX1_COMPA_IN_AUXIO24
#define AUXCOMPA_GPIO_IN_AUX25      ADI_4_AUX_MUX1_COMPA_IN_AUXIO25
#define AUXCOMPA_GPIO_IN_AUX26      ADI_4_AUX_MUX1_COMPA_IN_AUXIO26
#define AUXCOMPA_GPIO_IN_NC         ADI_4_AUX_MUX1_COMPA_IN_NC

//*****************************************************************************
//
// Defines that can be used to select the edge polarity on which an interrupt
// is triggered
//
//*****************************************************************************
#define AUXCOMPA_POL_RISE   0x00
#define AUXCOMPA_POL_FALL   0x01

//*****************************************************************************
//
// API Functions and prototypes
//
//*****************************************************************************

//*****************************************************************************
//
//! \brief Disables COMPA.
//!
//! This function disables COMPA and it must be called before re-enabling the
//! module using \ref AUXCOMPAEnable().
//
//*****************************************************************************
__STATIC_INLINE void AUXCOMPADisable(void)
{
    ADI8BitsClear(AUX_ADI4_BASE, ADI_4_AUX_O_COMP, ADI_4_AUX_COMP_COMPA_EN_M);
}

//*****************************************************************************
//
//! \brief Enables COMPA.
//!
//! This function enables COMPA. This function must be called after having
//! selected an input and reference for COMPA.
//
//*****************************************************************************
__STATIC_INLINE void AUXCOMPAEnable(void)
{
    ADI8BitsSet(AUX_ADI4_BASE, ADI_4_AUX_O_COMP, ADI_4_AUX_COMP_COMPA_EN_M);
}

//*****************************************************************************
//
//! \brief Selects an AUXIO pin as analog input of COMPA.
//!
//! This function selects one the available AUXIOs as analog input for COMPA.
//! This function must be called before enabling COMPA using \ref AUXCOMPAEnable().
//!
//! \param auxio
//!     COMPA analog input:
//!     - \ref AUXCOMPA_GPIO_IN_AUX19
//!     - \ref AUXCOMPA_GPIO_IN_AUX20
//!     - \ref AUXCOMPA_GPIO_IN_AUX21
//!     - \ref AUXCOMPA_GPIO_IN_AUX22
//!     - \ref AUXCOMPA_GPIO_IN_AUX23
//!     - \ref AUXCOMPA_GPIO_IN_AUX24
//!     - \ref AUXCOMPA_GPIO_IN_AUX25
//!     - \ref AUXCOMPA_GPIO_IN_AUX26
//!     - \ref AUXCOMPA_GPIO_IN_NC
//
//*****************************************************************************
__STATIC_INLINE void AUXCOMPASelectGpioInput(uint8_t auxio)
{
    ADI8RegWrite(AUX_ADI4_BASE, ADI_4_AUX_O_MUX1, auxio);
}

//*****************************************************************************
//
//! \brief Selects an AUXIO pin as analog reference of COMPA.
//!
//! This function selects one of the available AUXIOs as analog reference for
//! COMPA. The function makes sure that any previously connected reference is
//! disconnected from the COMPA_REF line, before connecting the AUXIO reference.
//! This function must be called before enabling COMPA using \ref AUXCOMPAEnable().
//!
//! \param auxio
//!     COMPA external analog reference:
//!     - \ref AUXCOMPA_GPIO_REF_AUX19
//!     - \ref AUXCOMPA_GPIO_REF_AUX20
//!     - \ref AUXCOMPA_GPIO_REF_AUX21
//!     - \ref AUXCOMPA_GPIO_REF_AUX22
//!     - \ref AUXCOMPA_GPIO_REF_AUX23
//!     - \ref AUXCOMPA_GPIO_REF_AUX24
//!     - \ref AUXCOMPA_GPIO_REF_AUX25
//!     - \ref AUXCOMPA_GPIO_REF_AUX26
//!     - \ref AUXCOMPA_GPIO_REF_NC
//
//*****************************************************************************
__STATIC_INLINE void AUXCOMPASelectGpioRef(uint8_t auxio)
{
    // Ensure that the DAC output is not connected to COMPA reference
    HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_DACCTL) &= ~AUX_ANAIF_DACCTL_DAC_VOUT_SEL_M;

    // Ensure that no internal voltages are connected to the reference voltage of COMPA
    ADI8BitsClear(AUX_ADI4_BASE, ADI_4_AUX_O_MUX0, ADI_4_AUX_MUX0_COMPA_REF_M);

    // Select external reference voltage for COMPA
    ADI8RegWrite(AUX_ADI4_BASE, ADI_4_AUX_O_MUX4, auxio);
}

//*****************************************************************************
//
//! \brief Selects the DAC output as analog reference of COMPA.
//!
//! This function selects the reference DAC output as analog reference for
//! COMPA. The function makes sure that any previously connected reference is
//! disconnected from the COMPA_REF line, before connecting the DAC reference.
//! This function must be called before enabling COMPA using \ref AUXCOMPAEnable().
//
//*****************************************************************************
void AUXCOMPASelectDacRef(void);

//*****************************************************************************
//
//! \brief Selects an internal voltage as analog reference of COMPA.
//!
//! This function selects one of the available internal voltages as analog
//! reference for COMPA. The function makes sure that any previously connected
//! reference is disconnected from the COMPA_REF line, before connecting the
//! internal voltage reference. This function must be called before enabling
//! COMPA using \ref AUXCOMPAEnable().
//!
//! \param internalRef
//!     COMPA internal analog reference:
//!     - \ref AUXCOMPA_INT_REF_ADCVREFP
//!     - \ref AUXCOMPA_INT_REF_VDDS
//!     - \ref AUXCOMPA_INT_REF_VSS
//!     - \ref AUXCOMPA_INT_REF_DCOUPL
//!     - \ref AUXCOMPA_INT_REF_NC
//
//*****************************************************************************
void AUXCOMPASelectInternalRef(uint8_t internalRef);

//*****************************************************************************
//
//! \brief Selects the edge polarity of COMPA output on which an interrupt
//! is triggered.
//!
//! This function selects whether a rising or falling edge of COMPA output
//! will trigger an interrupt on the COMPA hw interrupt line. This function
//! is called inside \ref AUXCOMPAIntRegister() before registering the handler
//! and enabling the interrupt line.
//!
//! \param polarity
//!     Edge polarity of COMPA output on which the interrupt is triggered:
//!     - \ref AUXCOMPA_POL_RISE
//!     - \ref AUXCOMPA_POL_FALL
//
//*****************************************************************************
__STATIC_INLINE void AUXCOMPAIntSetPolarity(uint8_t polarity)
{
    if (polarity == AUXCOMPA_POL_RISE)
    {
        // Clear bit in case RISE edge is selected
        HWREGB(AUX_EVCTL_BASE + AUX_EVCTL_O_EVTOMCUPOL) &= ~(1 << AUX_EVCTL_EVTOMCUPOL_AUX_COMPA_S);
    }
    else
    {
        // Set bit in case FALL edge is selected
        HWREGB(AUX_EVCTL_BASE + AUX_EVCTL_O_EVTOMCUPOL) |= (1 << AUX_EVCTL_EVTOMCUPOL_AUX_COMPA_S);
    }
}

//*****************************************************************************
//
//! \brief Clears COMPA's interrupt flag.
//!
//! This function clears the interrupt flag related to the COMPA interrupt
//! line. This function should be called inside the ISR registered with
//! \ref AUXCOMPAIntRegister().
//
//*****************************************************************************
__STATIC_INLINE void AUXCOMPAIntClear(void)
{
    // Clear interrupt flag for AUX_COMPA
    HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_EVTOMCUFLAGSCLR) = AUX_EVCTL_EVTOMCUFLAGSCLR_AUX_COMPA;
}

//*****************************************************************************
//
//! \brief Registers an interrupt handler on COMPA output trigger.
//!
//! This function registers an interrupt handler and enables the AUX_COMPA
//! interrupt line. Before enabling the interrupt line the function:
//! - sets whether the interrupt should trigger on a rising or falling edge of
//!   COMPA output
//! - clears the flag of the AUX_COMPA interrupt line
//! After enabling COMPA using \ref AUXCOMPAEnable() and before registering an
//! interrupt using \ref AUXCOMPAIntRegister(), a delay of at least 10us is
//! required to let COMPA output stabilize. This allows to avoid incorrect
//! interrupt triggers in the brief interval following the enablement of COMPA.
//!
//! \param polarity
//!     Edge polarity of COMPA output on which the interrupt is triggered:
//!     - \ref AUXCOMPA_POL_RISE
//!     - \ref AUXCOMPA_POL_FALL
//! \param fnHandler
//!     Interrupt handler invoked when COMPA output triggers with the
//!     specified edge polarity
//
//*****************************************************************************
__STATIC_INLINE void AUXCOMPAIntRegister(uint8_t polarity, void (*fnHandler)(void))
{
    AUXCOMPAIntSetPolarity(polarity);

    // Register the interrupt handler for AUX_COMPA
    IntRegister(INT_AUX_COMPA, fnHandler);

    // Clear any pending interrupt
    AUXCOMPAIntClear();

    // Enable the AUX_COMPA interrupt.
    IntEnable(INT_AUX_COMPA);
}

//*****************************************************************************
//
//! \brief Disables and unregisters the interrupt handler on COMPA output
//! trigger.
//!
//! This function disables interrupts on the AUX_COMPA line, and unregister a
//! previously set interrupt handler. Note that this function does not clear
//! COMPA's interrupt flag. The interrupt flag is either cleared implicitly
//! when calling \ref AUXCOMPAIntRegister() or explicitly with \ref AUXCOMPAIntClear().
//
//*****************************************************************************
__STATIC_INLINE void AUXCOMPAIntUnregister(void)
{
    // Disable the AUX_COMPA interrupt
    IntDisable(INT_AUX_COMPA);

    // Unregister the interrupt handler for AUX_COMPA
    IntUnregister(INT_AUX_COMPA);
}

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __AUX_COMPA_H__

//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//! @}
//
//*****************************************************************************
