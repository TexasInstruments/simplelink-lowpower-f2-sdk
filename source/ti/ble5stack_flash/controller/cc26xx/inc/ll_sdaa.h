/******************************************************************************

 @file  ll_sdaa.h

 @brief This file contains the SDAA (Selective Detect And Avoid) moudule,
        This module is responsible for monitoring and limiting TX consumption
        per channel, the module will be activated by SDAA_ENABLE define

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2009-2025, Texas Instruments Incorporated

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

/*********************************************************************
 *
 * WARNING!!!
 *
 * THE API'S FOUND IN THIS FILE ARE FOR INTERNAL STACK USE ONLY!
 * FUNCTIONS SHOULD NOT BE CALLED DIRECTLY FROM APPLICATIONS, AND ANY
 * CALLS TO THESE FUNCTIONS FROM OUTSIDE OF THE STACK MAY RESULT IN
 * UNEXPECTED BEHAVIOR.
 *
 */

#ifndef LL_SDAA_H
#define LL_SDAA_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * CONSTANTS
 */
//*****************************************************************************
// INCLUDES
//*****************************************************************************

#include "bcomdef.h"

//*****************************************************************************
// MACROS
//*****************************************************************************

//*****************************************************************************
// CONSTANTS
//*****************************************************************************


//! @brief Default value for gSdaaInternalDB.channelInSample when
//!        there isn't channel in ample process.
//!
#define LL_SDAA_NONE_ACTIVE_CHANNEL   0xFF

//*****************************************************************************
// TYPEDEFS
//*****************************************************************************

//! @brief Channel status enum, store in DB 2 bit per channel must consist up
//!        to 4 state
//!
typedef enum
{
    //! Default value channel is clear to transmit data.
    //!
    SDAA_CH_CLEAR,

    //! Data was sent on this channel is an overload
    //!
    SDAA_CH_OVERLOAD,

    //! The channel sampled and was busy therefore blocked user selected time
    //!
    SDAA_CH_BLOCK,

    //! Error state for a channel that does not exist
    //!
    SDAA_CH_INVALID,

} sdaaChState_e;

//*****************************************************************************
// LOCAL VARIABLES
//*****************************************************************************

//*****************************************************************************
// GLOBAL VARIABLES
//*****************************************************************************

//*****************************************************************************
// API FUNCTIONS
//*****************************************************************************

//*****************************************************************************
//! @fn    LL_SDAA_Init
//
//! @brief This API Initializes the SDAA module, with input configurations
//         from sysconfig/opt file/pre defined
//
//*****************************************************************************
extern void LL_SDAA_Init();

//*****************************************************************************
//! @fn    LL_SDAA_RecordTxUsage
//
//! @brief This API is divided into 3 parts:
//!
//!            1. Call to the function that handles the observation period,
//!               if it is over, set new obs period and clear all databases.
//!            2. Calculate and record TX usage based on input variables.
//!            3. Call to the function that handles usage exception
//!               if necessary.
//
//! @param numOfBytes - The number of bytes sent over the air,
//!                     0 for empty packet.
//
//!        phyType    - phyType is affected by a convert table between
//!                     the sending rate and air time.
//
//!        txPower    - txPower affectes the usage by the convert ratio table,
//!                     the weaker the power, the smaller the ratio.
//
//!        channel    - The data channel to which the usage will be added.
//
//*****************************************************************************
extern void LL_SDAA_RecordTxUsage(uint16 numOfBytes, uint8 phyType,
                                  uint8 txPower, uint8 channel);

//*****************************************************************************
//! @fn    LL_SDAA_AddDwtRecord
//
//! @brief This API add Dwt Record to the specific task
//
//! @param dwt   - DwellTime in rat ticks
//
//!        index - Connection handle ID or Task
//
//*****************************************************************************
extern void LL_SDAA_AddDwtRecord(uint32 dwT, uint8 task, uint8 index);


//*****************************************************************************
//! @fn    LL_SDAA_SampleRXWindow
//
//! @brief This API used to sample the RSSI values when the RX window is open.
//!        RSSI values equal to RF_GET_RSSI_ERROR_VAL will be count as
//!        invalid samples.
//!        RSSI values greater than RSSI Threshold will be count as
//!        noisy samples.
//!        the function determine the channel state according to
//!        the counters (invalid samples/ noisy samples) and update
//!        the channel state in SDAA DB's.
//
//*****************************************************************************
extern void LL_SDAA_SampleRXWindow(void);

//*****************************************************************************
//! @fn    LL_SDAA_GetChannelState
//
//! @brief This API is used to get the channel state from SDAA DB
//
//! @param channel - The channel on which the data will be received
//
//! @return sdaaChState_e - SDAA_CH_CLEAR
//!                         SDAA_CH_OVERLOAD
//!                         SDAA_CH_BLOCK
//!                         SDAA_CH_INVALID
//
//*****************************************************************************
extern sdaaChState_e LL_SDAA_GetChannelState(uint8 channel);

//*****************************************************************************
//! @fn    LL_SDAA_GetRXWindowDuration
//
//! @brief This API used to get the RX window duration in rats.
//
//! @return uint16 RXWindowDuration
//
//*****************************************************************************
extern uint16 LL_SDAA_GetRXWindowDuration(void);

//*****************************************************************************
//! @fn    LL_SDAA_SetChannelInSample
//
//! @brief This API used to set the channel which in sample mode.
//
//! @param channel - channel which in sample mode
//
//*****************************************************************************
extern void LL_SDAA_SetChannelInSample(uint8 channel);

//*****************************************************************************
//! @fn    LL_SDAA_GetTimeOutOfBlockedState
//
//! @brief This API used to get the time at which the elapsed time during which
//!        the channel state is block.
//
//! @param channel - The block channel.
//
//! @return uint32 time.
//
//*****************************************************************************
extern uint32 LL_SDAA_GetTimeOutOfBlockedState(uint8 channel);

#ifdef __cplusplus
}
#endif

#endif /* LL_SDAA_H */
