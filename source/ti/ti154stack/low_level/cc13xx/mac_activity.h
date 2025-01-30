/******************************************************************************

 @file  mac_activity.h

 @brief This file contains the data structures and APIs the MAC Activity
        tracking module.
        Note: Only utilized in DMM applications.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2019-2025, Texas Instruments Incorporated

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

#ifndef MAC_ACTIVITY_H
#define MAC_ACTIVITY_H

/*******************************************************************************
 * INCLUDES
 */
#include "mac_user_config.h"

#ifdef ZSTACK_SECURITY
/* Using this define since it is common across all Zstack projects */
#include "ti_zstack_config.h"
#endif

/*******************************************************************************
 * CONSTANTS
 */
/* MAC Activity priorities */
#define MAC_ACTIVITY_PRI_NORMAL_INDEX  (0)
#define MAC_ACTIVITY_PRI_HIGH_INDEX    (1)
#define MAC_ACTIVITY_PRI_URGENT_INDEX  (2)

#define MAC_ACTIVITY_PRI_NORMAL (0x0000)
#define MAC_ACTIVITY_PRI_HIGH   (0x0001)
#define MAC_ACTIVITY_PRI_URGENT (0x0002)
#define MAC_ACTIVITY_NUM_PRI    (3)
#define MAC_ACTIVITY_COUNT      (6)

#define MAC_ACTIVITY_THRESHOLD_PRI_NORMAL (1)
#define MAC_ACTIVITY_THRESHOLD_PRI_HIGH   (2)
#define MAC_ACTIVITY_THRESHOLD_PRI_URGENT (3)

/* Priority threshold for RX BEACON Activity */
#define MAC_ACTIVITY_THRESHOLD_REM_BEACON_NORMAL_PRI (3)
#define MAC_ACTIVITY_THRESHOLD_REM_BEACON_HIGH_PRI (2)

/* Default increment for preemption tracking */
#define MAC_ACTIVITY_DEFAULT_INC (1)

/* MAC Activity types */
typedef enum
{
    MAC_ACTIVITY_LINK_EST     = 0x0001,
    MAC_ACTIVITY_TX_BEACON    = 0x0002,
    MAC_ACTIVITY_RX_BEACON    = 0x0003,
    MAC_ACTIVITY_FH           = 0x0004,
    MAC_ACTIVITY_SCAN         = 0x0005,
    MAC_ACTIVITY_DATA         = 0x0006,
    MAC_ACTIVITY_RX_ALWAYS_ON = 0x0007,
} macActivity_t;


typedef struct {
    uint8_t numMissedAssocTxFrames; /* Number of missed association packets. */
    uint8_t numMissedBeaconTxFrames; /* Number of missed broadcasts */
    uint8_t numMissedFHTxFrames; /* Number of missed FH frames */
    uint8_t numMissedDataTxFrames; /* Number of missed data frames */
    uint8_t prevTxFrameType; /* Last successfully transmitted frame */
    macActivity_t txActivity; /* Current activity associated with the Transmitter */
} macTxIntActivityData_t;

typedef struct {
    uint8_t numRxAssocAbort; /* Number of missed association packets. */
    uint8_t numRxBCAbort; /* Number of missed broadcasts */
    uint8_t numRxScanAbort; /* Number of aborted scans */
    uint8_t numRxDataAbort; /* Number of MAC data Rx aborts */
    uint8_t prevRxFrameType; /* Last successfully received command frame type */
    macActivity_t rxActivity; /* Current activity associated with the Receiver */
} macRxIntActivityData_t;

/* Activity tracking module function pointer typedef's. */
typedef void (*setActivityTrackingTx_t)(macTxIntData_t *txData, uint16_t cmdStatus, RF_EventMask rfEvent);
typedef void (*setActivityTrackingRx_t)(macRx_t *pRxBuf, bool resetCount);
typedef uint32_t (*getActivityPriorityTx_t)(void);
typedef uint32_t (*getActivityPriorityRx_t)(void);
typedef void (*setActivityTx_t)(macTxIntData_t *txData);
typedef void (*setActivityRx_t)(uint8_t rxEna_tbleFlags);
typedef macActivity_t (*getActivityTx_t)(void);
typedef macActivity_t (*getActivityRx_t)(void);
typedef bool (*getCoexActivityTx_t)(void);
typedef bool (*getCoexActivityRx_t)(void);

#ifdef MAC_ACTIVITY_PROFILING
typedef void (*startActivityProfilingTimer_t)(RF_Handle handle);
typedef void (*printActivityInfo_t)(uint32_t activity, uint32_t priority, uint8_t frameType, sAddr_t destAddr);
#endif

/* Activity tracking object type definition */
typedef const struct _activityObject
{
    setActivityTrackingTx_t pSetActivityTrackingTxFn;
    setActivityTrackingRx_t pSetActivityTrackingRxFn;
    getActivityPriorityTx_t pGetActivityPriorityTxFn;
    getActivityPriorityRx_t pGetActivityPriorityRxFn;
    setActivityTx_t pSetActivityTxFn;
    setActivityRx_t pSetActivityRxFn;
    getActivityTx_t pGetActivityTxFn;
    getActivityRx_t pGetActivityRxFn;

#ifdef MAC_ACTIVITY_PROFILING
    startActivityProfilingTimer_t pStartActivityProfilingTimerFn;
    printActivityInfo_t pPrintActivityInfoFn;
#endif
    getCoexActivityTx_t pGetCoexActivityTxFn;
    getCoexActivityRx_t pGetCoexActivityRxFn;
} activityObject_t;


/* ------------------------------------------------------------------------------------------------
 *                                          Macros
 * ------------------------------------------------------------------------------------------------
 */
/* MAC Activity table priority MACRO */
#define CALC_ACTIVITY_PRIORITY(ACTIVITY, PRIORITY) ((ACTIVITY << 16) | PRIORITY)

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */
extern activityObject_t activityObject;

/*******************************************************************************
 * APIs
 */

/*******************************************************************************
 */

#endif /* MAC_ACTIVITY_H */
