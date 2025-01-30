/******************************************************************************

 @file  mac_rx.h

 @brief Describe the purpose and contents of the file.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2006-2025, Texas Instruments Incorporated

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

#ifndef MAC_RX_H
#define MAC_RX_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_types.h"
#include "rf_data_entry.h"
#include "mac_high_level.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Defines
 * ------------------------------------------------------------------------------------------------
 */
#define RX_FILTER_OFF                   0
#define RX_FILTER_ALL                   1
#define RX_FILTER_NON_BEACON_FRAMES     2
#define RX_FILTER_NON_COMMAND_FRAMES    3

/* bit value used to form values of macRxActive */
#define MAC_RX_ACTIVE_PHYSICAL_BV       0x80

#define MAC_RX_ACTIVE_NO_ACTIVITY       0x00  /* zero reserved for boolean use, e.g. !macRxActive */
#define MAC_RX_ACTIVE_STARTED          (0x01 | MAC_RX_ACTIVE_PHYSICAL_BV)
#define MAC_RX_ACTIVE_DONE              0x02

/* RX PACKET status */
#define MAC_RX_PKT_STATUS_OK            0x00
#define MAC_RX_PKT_STATUS_DISCARD       0x01
#define MAC_RX_PKT_STATUS_CLEANUP       0x02
#define MAC_RX_PKT_STATUS_CRC_BAD       0x03
#define MAC_RX_PKT_STATUS_OTHER         0x04


/* ------------------------------------------------------------------------------------------------
 *                                          Macros
 * ------------------------------------------------------------------------------------------------
 */
#define MAC_RX_IS_PHYSICALLY_ACTIVE()   ((macRxActive & MAC_RX_ACTIVE_PHYSICAL_BV) || macRxOutgoingAckFlag)
#define MAC_RX_IEEE_IS_PHYSICALLY_ACTIVE()   (macRxActive & MAC_RX_ACTIVE_PHYSICAL_BV)

/* ------------------------------------------------------------------------------------------------
 *                                   Global Variable Externs
 * ------------------------------------------------------------------------------------------------
 */
extern volatile uint8 macRxActive;
extern uint8 macRxFilter;
extern volatile uint8 macRxOutgoingAckFlag;
extern dataQueue_t macRxDataEntryQueue;


/* ------------------------------------------------------------------------------------------------
 *                                         Prototypes
 * ------------------------------------------------------------------------------------------------
 */
MAC_INTERNAL_API void macRxInit(void);
MAC_INTERNAL_API void macRxRadioPowerUpInit(void);
MAC_INTERNAL_API void macRxHaltCleanup(void);
MAC_INTERNAL_API void macRxFifoOverflowIsr(void);
MAC_INTERNAL_API void macRxAckTxDoneCallback(void);
#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
MAC_INTERNAL_API void macRxEDFETxDoneCallback(void);
#endif

#ifdef TI_WISUN_FAN_DEBUG
bool macRxIsMatchFound(sAddrExt_t euiAddress);
bool macRx_insertAddrIntoList(uint8_t* euiAddress);
bool macRx_removeAddrFromList(uint8_t* euiAddress);
#endif

extern void macRxFrameIsr(void);
extern void macRxNokIsr(void);

/**************************************************************************************************
 */
#endif
