/******************************************************************************
*  Filename:       aux_compa.c
*
*  Description:     Driverlib module for the AUX high-performance continuous-time
*                   Comparator.
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

#include "aux_compa.h"


//*****************************************************************************
//
// Selects an internal voltage as analog reference of AUX_COMPA
//
//*****************************************************************************
void AUXCOMPASelectInternalRef(uint8_t internalRef)
{
    // Ensure that the DAC output is not connected to COMPA reference
    HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_DACCTL) &= ~AUX_ANAIF_DACCTL_DAC_VOUT_SEL_M;

    // Ensure that no AUXIO pins are connected to COMPA's reference voltage
    ADI8RegWrite(AUX_ADI4_BASE, ADI_4_AUX_O_MUX4, ADI_4_AUX_MUX4_COMPA_REF_NC);

    // Select Internal reference voltage for COMPA
    ADI8SetValBit(AUX_ADI4_BASE, ADI_4_AUX_O_MUX0, ADI_4_AUX_MUX0_COMPA_REF_M, internalRef);
}

//*****************************************************************************
//
// Selects the DAC's output as analog reference of AUX_COMPA
//
//*****************************************************************************
void AUXCOMPASelectDacRef(void){
    uint32_t dacCtl;

    // Use the "Not Connected" output as an intermediate step when changing DAC_VOUT_SEL
    HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_DACCTL) &= ~AUX_ANAIF_DACCTL_DAC_VOUT_SEL_M;

    // Ensure that no internal voltages are connected to the reference voltage
    // of COMPA and that the DAC is not driving an external pin.
    ADI8BitsClear(AUX_ADI4_BASE, ADI_4_AUX_O_MUX0, ADI_4_AUX_MUX0_COMPA_REF_M);
    ADI8RegWrite(AUX_ADI4_BASE, ADI_4_AUX_O_MUX4, ADI_4_AUX_MUX4_COMPA_REF_NC);

    // Disable DAC buffer when driving an internal load
    HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_DACCTL) &= ~AUX_ANAIF_DACCTL_DAC_BUFFER_EN_M;

    // Connect the DAC output to the reference voltage of COMPA
    dacCtl = HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_DACCTL);
    HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_DACCTL) = (dacCtl & ~AUX_ANAIF_DACCTL_DAC_VOUT_SEL_M) | \
                                                 AUX_ANAIF_DACCTL_DAC_VOUT_SEL_COMPA_REF;
}
