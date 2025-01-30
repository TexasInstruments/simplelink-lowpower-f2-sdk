/******************************************************************************

 @file  itm.c

 @brief Provides ITM module functions

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2011-2025, Texas Instruments Incorporated

 All rights reserved not granted herein.
 Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free,
 non-exclusive license under copyrights and patents it now or hereafter
 owns or controls to make, have made, use, import, offer to sell and sell
 ("Utilize") this software subject to the terms herein. With respect to the
 foregoing patent license, such license is granted solely to the extent that
 any such patent is necessary to Utilize the software alone. The patent
 license shall not apply to any combinations which include this software,
 other than combinations with devices manufactured by or for TI ("TI
 Devices"). No hardware patent is licensed hereunder.

 Redistributions must preserve existing copyright notices and reproduce
 this license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution.

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

   * No reverse engineering, decompilation, or disassembly of this software
     is permitted with respect to any software provided in binary form.
   * Any redistribution and use are licensed by TI for use only with TI Devices.
   * Nothing shall obligate TI to provide you with source code for the software
     licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution
 of the source code are permitted provided that the following conditions are
 met:

   * Any redistribution and use of the source code, including any resulting
     derivative works, are licensed by TI for use only with TI Devices.
   * Any redistribution and use of any object code compiled from the source
     code and any resulting derivative works, are licensed by TI for use
     only with TI Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this
 software without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

/* Standard Includes */
#include "itm.h"
#include "itm_private.h"
#include <stdint.h>

#include <stdbool.h>
#include <string.h>

#include <ioc.h>

#ifdef DEBUG_SW_TRACE

#define ITM_STIM_PORT_8(x)    (*(volatile uint8_t *) ITM_STIM_PORT((x)))
#define ITM_STIM_PORT_32(x)   (*(volatile uint32_t *) ITM_STIM_PORT((x)))

void ITM_initModule(const ITM_config itm_config)
{
    // Disable module
    SCS_DEMCR &= (~SCS_DEMCR_TRCEN);
    ITM_TCR   = 0x00000000;

    // Enable trace
    SCS_DEMCR |= SCS_DEMCR_TRCEN;

    // Unlock and Setup TPIU for SWO UART mode
    TPIU_LAR   = CS_LAR_UNLOCK;
    TPIU_SPPR  = TPIU_SPPR_SWO_UART;
    TPIU_CSPSR = TPIU_CSPSR_PIN_1;

    // Unlock and enable all ITM stimulus ports with default settings
    ITM_LAR    = CS_LAR_UNLOCK;
    ITM_TER    = ITM_TER_ENABLE_ALL;
    ITM_TPR    = ITM_TPR_ENABLE_USER_ALL;

    // Setup Baud rate
    if (itm_config.systemClock)
    {
        uint32_t prescalar = itm_config.systemClock / itm_config.baudRate;

        // Offset with current prescalar value
        uint32_t diff1 = itm_config.systemClock - (prescalar * itm_config.baudRate);
        // Offset with prescalar+1 value
        uint32_t diff2 = ((prescalar+1) * itm_config.baudRate) - itm_config.systemClock;

        if (diff2 < diff1)  prescalar++;
        // Program prescalar value as (prescalar factor - 1)
        TPIU_ACPR = (prescalar - 1);
    }

    // Disable formatter
    TPIU_FFCR  = 0;

    // Unlock DWT
    DWT_LAR    = CS_LAR_UNLOCK;

    // Configure SWO Traces
    IOCPortConfigureSet(IOID_18, IOC_PORT_MCU_SWV, IOC_STD_OUTPUT);

}

void ITM_enableModule()
{
    // Enable ITM module
    ITM_TCR |= ITM_TCR_ENABLE_ITM;
}

void ITM_disableModule()
{
    // Disable ITM module
    ITM_TCR &= ~ITM_TCR_ENABLE_ITM;
}

void ITM_putStringBlocking(const char* msg, const uint8_t port)
{
    // Write one character at a time.
    while('\0' != *msg)
    {
        while(0 == ITM_STIM_PORT_8(port));
        ITM_STIM_PORT_8(port) = *msg++;
    }
}

void ITM_putValueBlocking(uint32_t value, const uint8_t port)
{
    char strValue[8];
    int8_t digitIdx;
    bool putStarted = false;

    for(digitIdx = 7; digitIdx >= 0; digitIdx--)
    {
        strValue[digitIdx] = (value & (0xF << digitIdx * 4)) >> (digitIdx*4);
        if(strValue[digitIdx] < 10)
        {
            strValue[digitIdx] += '0';
        }
        else
        {
            strValue[digitIdx] += 'a'-10;
        }

        if( (strValue[digitIdx] != '0') || putStarted )
        {
            while(0 == ITM_STIM_PORT_8(port));
            ITM_STIM_PORT_8(port) = strValue[digitIdx];
            putStarted = true;
        }
    }
}

void ITM_putNewLineBlocking(const uint8_t port)
{
    while(0 == ITM_STIM_PORT_8(port));
    ITM_STIM_PORT_8(port) = '\n';
}

void ITM_putWordBlocking(const uint32_t data, const uint8_t port)
{
    // Write the entire32-bit value
    while(0 == ITM_STIM_PORT_32(port));
    ITM_STIM_PORT_32(port) = data;
}

void ITM_enableExceptionTrace()
{
    DWT_CTRL |= DWT_CTRL_ENABLE_EXC_TRC;
    ITM_TCR  |= ITM_TCR_ENABLE_DWT_TX;
}

void ITM_enablePCSampling()
{
    // Clear the PC Sampling and Cycle Event bits
    DWT_CTRL &= ~(DWT_CTRL_ENABLE_PC_SAMP | DWT_CTRL_ENABLE_CYC_EVT);
    // Setup Sampling interval
    DWT_CTRL |= DWT_CTRL_CYC_CNT_1024;
    ITM_TCR  |= ITM_TCR_ENABLE_DWT_TX;
    DWT_CTRL |= DWT_CTRL_ENABLE_CYC_CNT;
    // Enable PC sampling event
    DWT_CTRL |= DWT_CTRL_ENABLE_PC_SAMP;
}

bool ITM_enableDataTrace(const uint32_t *variable)
{
    uint_least8_t numDwtComp = (DWT_CTRL & DWT_CTRL_MASK_NUM_COMP) >> DWT_CTRL_SHIFT_NUM_COMP;
    uint_least8_t dwtIndex = 0;
    bool dwtAvailable = false;
    for (dwtIndex = 0; dwtIndex < numDwtComp ; dwtIndex++)
    {
        if (0 == DWT_FUNC(dwtIndex))
        {
            DWT_COMP(dwtIndex) = (uint32_t)variable;
            DWT_MASK(dwtIndex) = 0x0;
            DWT_FUNC(dwtIndex) = (DWT_FUNC_DATA_SIZE_32 | DWT_FUNC_ENABLE_ADDR_OFFSET | DWT_FUNC_ENABLE_COMP_RW);
            dwtAvailable = true;
        }
    }
    return dwtAvailable;
}

void ITM_enableCycleCounter()
{
    // Clear the PC Sampling and Cycle Event bits
    DWT_CTRL &= ~(DWT_CTRL_ENABLE_PC_SAMP | DWT_CTRL_ENABLE_CYC_EVT);
    // Setup Sampling interval
    DWT_CTRL |= DWT_CTRL_CYC_CNT_1024;
    ITM_TCR  |= ITM_TCR_ENABLE_DWT_TX;
    DWT_CTRL |= DWT_CTRL_ENABLE_CYC_CNT;
    // Enable Cycle Count event
    DWT_CTRL |= DWT_CTRL_ENABLE_CYC_EVT;
}

void ITM_enableTiming(ITM_tsPrescale tsPrescale)
{
    // Set timestamp prescalar enable timestamp generation
    ITM_TCR   |= ((tsPrescale << ITM_TCR_TS_PRESCALE_SHIFT) & ITM_TCR_TS_PRESCALE_MASK);
    ITM_TCR   |= (ITM_TCR_ENABLE_TS);
}

void ITM_enableSyncPackets(ITM_syncPacketRate syncPacketRate)
{
    // Clear sync packet rate
    DWT_CTRL &= ~(DWT_CTRL_MASK_SYNCTAP);
    // Set sync packet rate
    DWT_CTRL |= ((syncPacketRate << DWT_CTRL_SHIFT_SYNCTAP) & DWT_CTRL_MASK_SYNCTAP);
    // Enable sync packet generation
    DWT_CTRL |= DWT_CTRL_ENABLE_CYC_CNT;
    ITM_TCR  |= (ITM_TCR_ENABLE_SYNC);
}

void ITM_getLibraryVersion(uint32_t *majorVersion, uint32_t *minorVersion)
{
    if (majorVersion && minorVersion)
    {
        *majorVersion = ITM_LIB_MAJOR_VER;
        *minorVersion = ITM_LIB_MINOR_VER;
    }
}

void ITM_flushModule()
{
    // Wait till the ITM Busy status has been cleared
    while (ITM_TCR & ITM_TCR_BUSY)
    {
        asm (" NOP");
    }
}

#endif