/******************************************************************************

 @file  mac_tx.h

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

#ifndef MAC_TX_H
#define MAC_TX_H

/* ------------------------------------------------------------------------------------------------
 *                                         Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_types.h"
#include "mac_high_level.h"


/* ------------------------------------------------------------------------------------------------
 *                                          Define
 * ------------------------------------------------------------------------------------------------
 */
/* bit value used to form values of macTxActive */
#define MAC_TX_ACTIVE_PHYSICALLY_BV      0x80

/* state values for macTxActive; note zero is reserved for inactive state */
#define MAC_TX_ACTIVE_NO_ACTIVITY           0x00 /* zero reserved for boolean use, e.g. !macTxActive */
#define MAC_TX_ACTIVE_INITIALIZE            0x01
#define MAC_TX_ACTIVE_QUEUED                0x02
#define MAC_TX_ACTIVE_GO                   (0x03 | MAC_TX_ACTIVE_PHYSICALLY_BV)
#define MAC_TX_ACTIVE_CHANNEL_BUSY          0x04
#define MAC_TX_ACTIVE_DONE                 (0x05 | MAC_TX_ACTIVE_PHYSICALLY_BV)
#define MAC_TX_ACTIVE_LISTEN_FOR_ACK       (0x06 | MAC_TX_ACTIVE_PHYSICALLY_BV)
#define MAC_TX_ACTIVE_POST_ACK             (0x07 | MAC_TX_ACTIVE_PHYSICALLY_BV)

/* frequency hopping RSSI threshold */
#define FH_RSSI_THRESHOLD (-83)


/* ------------------------------------------------------------------------------------------------
 *                                          Define
 * ------------------------------------------------------------------------------------------------
 */
#define MAC_TX_IS_PHYSICALLY_ACTIVE()       ((macTxActive & MAC_TX_ACTIVE_PHYSICALLY_BV) || macAckTxActive)

/* ------------------------------------------------------------------------------------------------
 *                                   Global Variable Externs
 * ------------------------------------------------------------------------------------------------
 */
extern volatile uint8 macTxActive;
extern volatile uint8 macAckTxActive;
extern uint8 macTxBe;
extern uint8 macTxType;
extern uint16 macTxCsmaBackoffDelay;
extern uint8 macTxGpInterframeDelay;
extern uint8 macTxAckPending;
extern uint8  txAckReq;
extern uint8 txSeqn;

/* ------------------------------------------------------------------------------------------------
 *                                       Prototypes
 * ------------------------------------------------------------------------------------------------
 */
MAC_INTERNAL_API void macTxInit(void);
MAC_INTERNAL_API void macTxHaltCleanup(void);
MAC_INTERNAL_API void macTxStartQueuedFrame(void);
MAC_INTERNAL_API void macTxChannelBusyCallback(bool bForceComplete);
MAC_INTERNAL_API void macTxDoneCallback(void);
MAC_INTERNAL_API void macTxAckReceivedCallback(uint8 seqn, uint8 pendingFlag);
MAC_INTERNAL_API void macTxAckNotReceivedCallback(uint8 param);
MAC_INTERNAL_API void macTxTimestampCallback(void);
MAC_INTERNAL_API void macTxCollisionWithRxCallback(void);
MAC_INTERNAL_API void macCsmaTimerInit(void);
MAC_INTERNAL_API void txCsmaDelay(uint8 param);
MAC_INTERNAL_API void macTxRequestCsmaRtcDelay( uint32_t backOffDur);
MAC_INTERNAL_API void macTxUpdateIE(void);
MAC_INTERNAL_API void macHandleTxAck(void);

extern void txComplete(uint8 status);
extern void macTxFrameIsr(void);
extern void macTxAbortIsr(void);
extern void macTxAckIsr(void);
extern void macTxRequestCsmaDelay(void);
extern uint8 macTxDecrypt(void);
extern uint32_t macTxGetDefaultMinTxOffTime(void);

/**************************************************************************************************
 */
#endif
