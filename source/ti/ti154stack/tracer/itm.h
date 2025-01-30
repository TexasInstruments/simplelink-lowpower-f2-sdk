/******************************************************************************

 @file  itm.h

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

#ifndef ITM_H_
#define ITM_H_


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

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef enum
{
    ITM_9600 = 9600,
    ITM_57600 = 57600,
    ITM_115200 = 115200,
    ITM_230400 = 230400,
    ITM_460800 = 460800,
    ITM_500000 = 500000,
    ITM_576000 = 576000,
    ITM_921600 = 921600,
    ITM_1000000 = 1000000,
    ITM_1152000 = 1152000,
    ITM_1500000 = 1500000,
    ITM_2000000 = 2000000,
    ITM_2500000 = 2500000,
    ITM_3000000 = 3000000,
    ITM_3500000 = 3500000,
    ITM_4000000 = 4000000,
    ITM_6000000 = 6000000
} ITM_baudrate;


typedef struct _ITM_config_
{
    uint_fast32_t systemClock;
    ITM_baudrate  baudRate;
} ITM_config;


typedef enum
{
    ITM_TS_DIV_NONE = 0,
    ITM_TS_DIV_4 = 1,
    ITM_TS_DIV_16 = 2,
    ITM_TS_DIV_64 = 3
} ITM_tsPrescale;

typedef enum
{
    ITM_SYNC_NONE = 0,
    ITM_SYNC_16M_CYCLES = 1,
    ITM_SYNC_64M_CYCLES = 2,
    ITM_SYNC_256M_CYCLES = 3
} ITM_syncPacketRate;

//*****************************************************************************
//
//! Initialize the ITM module.
//!
//! \param itm_config provides configuration parameters.
//!
//! This function will initialize the ITM/TPIU module for UART SWO mode.
//!
//! \return None.
//
//*****************************************************************************
extern void ITM_initModule(const ITM_config itm_config);

//*****************************************************************************
//
//! Enable ITM module.
//!
//! \param None.
//!
//! This function will enable ITM module for trace generation.
//!
//! \return None.
//
//*****************************************************************************
extern void ITM_enableModule();

//*****************************************************************************
//
//! Disable ITM module.
//!
//! \param None.
//!
//! This function will disable trace generation by ITM module.
//!
//! \return None.
//
//*****************************************************************************
extern void ITM_disableModule();

//*****************************************************************************
//
//! Generate string message.
//!
//! \param msg Null terminated string to transmit.
//! \param port Stimulus port number to use for transmission.
//!
//! This function will write out the characters to the stimulus port specified.
//! This function is blocking which means the function will not return until
//! all characters are transferred.
//!
//! \return None.
//
//*****************************************************************************
extern void ITM_putStringBlocking(const char* msg, const uint8_t port);

//*****************************************************************************
//
//! Generate value string message.
//!
//! \param value integer to be preonted in hex
//! \param port Stimulus port number to use for transmission.
//!
//! This function will write out the characters to the stimulus port specified.
//! This function is blocking which means the function will not return until
//! all characters are transferred.
//!
//! \return None.
//
//*****************************************************************************
void ITM_putValueBlocking(uint32_t value, const uint8_t port);

//*****************************************************************************
//
//! Generate new line.
//!
//! \param port Stimulus port number to use for transmission.
//!
//! This function will write out the characters to the stimulus port specified.
//! This function is blocking which means the function will not return until
//! all characters are transferred.
//!
//! \return None.
//
//*****************************************************************************
void ITM_putNewLineBlocking(const uint8_t port);

//*****************************************************************************
//
//! Generate 32-bit word message.
//!
//! \param msg 32-bit word.
//! \param port Stimulus port number to use for transmission.
//!
//! This function will write out the 32-bit word to the stimulus port
//! specified. This function is blocking which means the function will not
//! return until all data is transferred.
//!
//! \return None.
//
//*****************************************************************************
extern void ITM_putWordBlocking(const uint32_t msg, const uint8_t port);

//*****************************************************************************
//
//! Enable exception trace.
//!
//! \param None.
//!
//! This function will generate hardware events for exception entry, exit and
//! return.
//!
//! \return None.
//
//*****************************************************************************
extern void ITM_enableExceptionTrace();

//*****************************************************************************
//
//! Enable Program Counter sampling.
//!
//! \param None.
//!
//! This function will generate hardware events for periodically sampled PC.
//! The sampling period has been set to 1024 system clock cycles.
//! return.
//!
//! \return None.
//
//*****************************************************************************
extern void ITM_enablePCSampling();

//*****************************************************************************
//
//! Enable data access event generation.
//!
//! \param variable Address of the variable to profile.
//!
//! This function will generate a hardware event for data address reads and
//! writes. This function can be used for profiling variables.
//!
//! \return true if watchpoint resource is available to program, else is false.
//
//*****************************************************************************
extern bool ITM_enableDataTrace(const uint32_t *variable);

//*****************************************************************************
//
//! Enable cycle count event generation.
//!
//! \param None
//!
//! This function will generate a hardware event every time 1024 system clock
//! cycles have elapsed.
//!
//! \return None.
//
//*****************************************************************************
extern void ITM_enableCycleCounter();

//*****************************************************************************
//
//! Enable timestamp generation.
//!
//! \param tsPrescale Timestamp scale factor.
//!
//! This function will generate local timestamp packets for time correlation.
//! The timestamp interval is set to the input value which means the local
//! timestamp counter is running at system clock/tsPrescale frequency. Lowering
//! the timestamp counter frequency (i.e using higher prescale factor) results
//! in lower timing resolution, however it helps reduce the volume of trace data
//! generated making it bandwidth efficient.
//!
//! \return None.
//
//*****************************************************************************
extern void ITM_enableTiming(ITM_tsPrescale tsPrescale);

//*****************************************************************************
//
//! Enable sync packet generation.
//!
//! \param syncPacketRate Rate at which sync packets are generated.
//!
//! This function will generate synchronization packets at specified intervals.
//! This function is necessary if the host side decoder is expected to connect
//! and decode from the middle of a live stream. The synchronization packet
//! helps host decoder identify the starting point to begin decoding given that
//! SWO trace is a byte based protocol.
//!
//! \return None.
//
//*****************************************************************************
extern void ITM_enableSyncPackets(ITM_syncPacketRate syncPacketRate);


//*****************************************************************************
//
//! Enable sync packet generation.
//!
//! \param majorVersion Major version.
//! \param majorVersion Minor version.
//!
//! This function returns the library version.
//!
//! \return None.
//
//*****************************************************************************
extern void ITM_getLibraryVersion(uint32_t *majorVersion, uint32_t *minorVersion);


//*****************************************************************************
//
//! Flush ITM module.
//!
//! \param None.
//!
//! This function will wait till data is flushed from the ITM. This API must be
//! called before core enters sleep.
//!
//! \return None.
//
//*****************************************************************************
extern void ITM_flushModule();
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* ITM_H_ */
