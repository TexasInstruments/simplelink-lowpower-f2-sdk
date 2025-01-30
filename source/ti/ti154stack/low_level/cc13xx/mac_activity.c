
/******************************************************************************

 @file  mac_activity.c

 @brief This file contains the data structures and API definitions for the MAC
        Activity tracking module.
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

/*******************************************************************************
 * INCLUDES
 */

/* High Level MAC */
#include "mac_timer.h"
#include "mac_hl_patch.h"

/* Low Level MAC */
#include "mac_activity.h"

/* debug */
#include "mac_assert.h"
#include "dbg.h"

/* CM0 related */
#include "mac_settings.h"
#if defined(USE_DMM) || defined(IEEE_COEX_3_WIRE)
/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/* Activity tracking variables, updated within ISR and MAC Task context. */
static volatile macTxIntActivityData_t txActivityData = {0};
static volatile macRxIntActivityData_t rxActivityData = {0};


/*******************************************************************************
 * GLOBAL VARIABLES
 */

/* ------------------------------------------------------------------------------------------------
 *                                        Local Functions
 * ------------------------------------------------------------------------------------------------
 */
static void setActivityTrackingTx(macTxIntData_t *txData, uint16_t cmdStatus, RF_EventMask rfEvent);
static void setActivityTrackingRx(macRx_t *pRxBuf, bool resetCount);
static uint32_t getActivityPriorityTx(void);
static uint32_t getActivityPriorityRx(void);
static void setActivityTx(macTxIntData_t *txData);
static void setActivityRx(uint8_t rxEnableFlags);
static macActivity_t getActivityTx(void);
static macActivity_t getActivityRx(void);
#ifdef MAC_ACTIVITY_PROFILING
static void activityPreemptCb(void *arg);
static void startActivityProfilingTimer(RF_Handle handle);
static void printActivityInfo(uint32_t activity, uint32_t priority, uint8_t frameType, sAddr_t destAddr);
#endif /* MAC_ACTIVITY_PROFILING */
static bool getCoexActivityTx(void);
static bool getCoexActivityRx(void);


const activityObject_t activityObject = {
    .pSetActivityTrackingTxFn = setActivityTrackingTx,
    .pSetActivityTrackingRxFn = setActivityTrackingRx,
    .pGetActivityPriorityTxFn = getActivityPriorityTx,
    .pGetActivityPriorityRxFn = getActivityPriorityRx,
    .pSetActivityTxFn = setActivityTx,
    .pSetActivityRxFn = setActivityRx,
    .pGetActivityTxFn = getActivityTx,
    .pGetActivityRxFn = getActivityRx,
#ifdef MAC_ACTIVITY_PROFILING
    .pStartActivityProfilingTimerFn = startActivityProfilingTimer,
    .pPrintActivityInfoFn = printActivityInfo,
#endif
    .pGetCoexActivityTxFn = getCoexActivityTx,
    .pGetCoexActivityRxFn = getCoexActivityRx,
};

/*******************************************************************************
 * @fn          setActivityTrackingTx
 *
 * @brief       This call is used by the LMAC to update the Transmit activity
 *              tracking metrics.
 *
 * input parameters
 *
 * @param       txData - pointer to transmit packet.
 *
 * @param       resetCount - Whether to reset activity tracking count.
 *                           True to reset, False to increment count.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
static void setActivityTrackingTx(macTxIntData_t *txData, uint16_t cmdStatus, RF_EventMask rfEvent)
{
    bool resetCount = false;
    uint8 rfFreq = pMacPib->rfFreq;

    /* Replicate logic within Subg, IEEE & TX Ack callback functions */
    if (rfFreq == MAC_RF_FREQ_SUBG)
    {
        if (rfEvent & RF_EventLastCmdDone)
        {
          /* Normal cases */
          if (cmdStatus == PROP_DONE_OK)
          {
              /* Successful transmission, reset tracking data */
              resetCount = true;
          }
          else if (cmdStatus & 0x0800)
          {
              /* Increment failure status for current activity */
              resetCount = false;
          }
          else {
              /* Successful transmission of Tx Ack, reset tracking data */
              resetCount = true;
          }
        }
        else if (rfEvent & (RF_EventCmdStopped | RF_EventCmdAborted | RF_EventCmdCancelled | RF_EventCmdPreempted))
        {
          /* Increment failure status for current activity as TX was aborted */
          resetCount = false;
        }
        /* Ignore Rx ack chain events */
        else if (!(rfEvent & (RF_EventRxOk | RF_EventRxBufFull | RF_EventRxNOk | RF_EventMdmSoft)))
        {
            /* Increment failure status for current activity as TX was aborted */
            resetCount = false;
        }
    }
    else
    {
        if (rfEvent & (RF_EventCmdStopped | RF_EventCmdAborted | RF_EventCmdCancelled | RF_EventCmdPreempted))
        {
            /* TX is aborted */
            resetCount = false;
        }
        else
        {
            if ((rfEvent & RF_EventLastFGCmdDone) == RF_EventLastFGCmdDone)
            {
                /* Normal cases */
                if (cmdStatus == IEEE_DONE_OK)
                {
                    /* Successful transmission, reset tracking data */
                    resetCount = true;
                }
                else
                {
                    /* Command error */
                    resetCount = false;
                }
            }
            else
            {
                /* Command error */
                resetCount = false;
            }
        }
    }

    /* Update previous frame type for successfully transmitted packet */
    if ((txData) && (resetCount)) {
        txActivityData.prevTxFrameType = txData->frameType;
    }

    /* Update tracking information for activity */
    switch(txActivityData.txActivity)
    {
        case MAC_ACTIVITY_LINK_EST:
            txActivityData.numMissedAssocTxFrames = resetCount ? 0 : (txActivityData.numMissedAssocTxFrames + MAC_ACTIVITY_DEFAULT_INC);
            break;
        case MAC_ACTIVITY_TX_BEACON:
            txActivityData.numMissedBeaconTxFrames = resetCount ? 0 : (txActivityData.numMissedBeaconTxFrames + MAC_ACTIVITY_DEFAULT_INC);
            break;
        case MAC_ACTIVITY_FH:
            txActivityData.numMissedFHTxFrames = resetCount ? 0 : (txActivityData.numMissedFHTxFrames + MAC_ACTIVITY_DEFAULT_INC);
            break;
        case MAC_ACTIVITY_DATA:
            txActivityData.numMissedDataTxFrames = resetCount ? 0 : (txActivityData.numMissedDataTxFrames + MAC_ACTIVITY_DEFAULT_INC);
            break;
        default:
            break;
    }
}
/*******************************************************************************
 * @fn          setActivityTrackingRx
 *
 * @brief       This call is used by the LMAC to update the Receive activity
 *              tracking metrics.
 *
 * input parameters
 *
 * @param       pRxBuf - Pointer to MAC Receive buffer.
 *
 * @param       resetCount - Whether to reset activity tracking count.
 *                           True to reset, False to increment count.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
static void setActivityTrackingRx(macRx_t *pRxBuf, bool resetCount)
{
    uint8_t cmdActivityFrameId = 0;

    if (pRxBuf) {
        /* Pre-calculate command ID for Link Establishment activity */
        if (((pRxBuf->internal.fcf & MAC_FCF_FRAME_TYPE_MASK) == MAC_FRAME_TYPE_COMMAND) && (pRxBuf->msdu.len > 0))
        {
          cmdActivityFrameId = *pRxBuf->msdu.p;
        }
    }

    /* Update previous frame type for successfully transmitted packet */
    if (cmdActivityFrameId && resetCount) {
        rxActivityData.prevRxFrameType = cmdActivityFrameId;
    }

    switch(rxActivityData.rxActivity)
    {
        case MAC_ACTIVITY_LINK_EST:
            rxActivityData.numRxAssocAbort = resetCount ? 0 : (rxActivityData.numRxAssocAbort + MAC_ACTIVITY_DEFAULT_INC);
            break;
        case MAC_ACTIVITY_FH:
            rxActivityData.numRxBCAbort = resetCount ? 0 : (rxActivityData.numRxBCAbort + MAC_ACTIVITY_DEFAULT_INC);
            break;
        case MAC_ACTIVITY_SCAN:
            rxActivityData.numRxScanAbort = resetCount ? 0 : (rxActivityData.numRxScanAbort + MAC_ACTIVITY_DEFAULT_INC);
            break;
        case MAC_ACTIVITY_DATA:
            rxActivityData.numRxDataAbort = resetCount ? 0 : (rxActivityData.numRxDataAbort + MAC_ACTIVITY_DEFAULT_INC);
            break;
        default:
            break;
    }
}

/*******************************************************************************
 * @fn          getActivityPriorityTx
 *
 * @brief       This call is used by the LMAC to calculate the activity priority
 *              index for the current activity.
 *
 * input parameters
 *
 * @param       None
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
static uint32_t getActivityPriorityTx(void)
{
    uint32_t currPri;

    switch(txActivityData.txActivity)
    {
        case MAC_ACTIVITY_LINK_EST:
        {
            /* Default Priority for association is HIGH */
            if(txActivityData.numMissedAssocTxFrames < MAC_ACTIVITY_THRESHOLD_PRI_URGENT)
            {
                currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_LINK_EST, MAC_ACTIVITY_PRI_HIGH_INDEX);
            }
            else
            {
                currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_LINK_EST, MAC_ACTIVITY_PRI_URGENT_INDEX);
            }
        }
        break;

        case MAC_ACTIVITY_TX_BEACON:
        {
            /* Default Priority for Beacon Tx is HIGH, increase after single missed/rejected beacon */
            if(txActivityData.numMissedBeaconTxFrames == 0)
            {
                currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_TX_BEACON, MAC_ACTIVITY_PRI_HIGH_INDEX);
            }
            else
            {
                currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_TX_BEACON, MAC_ACTIVITY_PRI_URGENT_INDEX);
            }
        }
        break;

        case MAC_ACTIVITY_FH:
        {
           /* Default Priority is Normal */
           if(txActivityData.numMissedFHTxFrames < MAC_ACTIVITY_THRESHOLD_PRI_NORMAL)
           {
               currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_FH, MAC_ACTIVITY_PRI_NORMAL_INDEX);
           }
           else if(txActivityData.numMissedFHTxFrames < MAC_ACTIVITY_THRESHOLD_PRI_HIGH)
           {
               currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_FH, MAC_ACTIVITY_PRI_HIGH_INDEX);
           }
           else
           {
               currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_FH, MAC_ACTIVITY_PRI_URGENT_INDEX);
           }
        }
        break;

        case MAC_ACTIVITY_DATA:
        {
            /* Default Priority is Normal */
            if (txActivityData.numMissedDataTxFrames < MAC_ACTIVITY_THRESHOLD_PRI_NORMAL)
            {
                currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_DATA, MAC_ACTIVITY_PRI_NORMAL_INDEX);
            }
            else if (txActivityData.numMissedDataTxFrames < MAC_ACTIVITY_THRESHOLD_PRI_HIGH)
            {
                currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_DATA, MAC_ACTIVITY_PRI_HIGH_INDEX);
            }
            else
            {
                currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_DATA, MAC_ACTIVITY_PRI_URGENT_INDEX);
            }
        }
        break;
        default:
            currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_DATA, MAC_ACTIVITY_PRI_NORMAL_INDEX);
            break;
    }
    return (currPri);
}

/*******************************************************************************
 * @fn          getActivityPriorityRx
 *
 * @brief       This call is used by the LMAC to calculate the activity priority
 *              index for the current activity.
 *              Note: The priority calculation will is based on a number of missed
 *              events. For most activities the priority will increase incrementally
 *              until the max priority is reached. This is synonymous to the amount
 *              of missed events.
 *              Ex) Every missed abort shall increase the priority, up to the maximum.
 *
 * input parameters
 *
 * @param       None
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
static uint32_t getActivityPriorityRx(void)
{
    uint8_t beaconLost;
    uint32_t currPri;

    switch(rxActivityData.rxActivity)
    {
        case MAC_ACTIVITY_LINK_EST:
        {
            /* Default Priority for association is HIGH */
            if(rxActivityData.numRxAssocAbort < MAC_ACTIVITY_THRESHOLD_PRI_URGENT)
            {
                currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_LINK_EST, MAC_ACTIVITY_PRI_HIGH_INDEX);
            }
            else
            {
                currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_LINK_EST, MAC_ACTIVITY_PRI_URGENT_INDEX);
            }
        }
        break;

        case MAC_ACTIVITY_RX_BEACON:
        {
            /* Obtain number of missed beacons */
            beaconLost = getLostBeaconCount();

            /* Default Priority is High */
            if(beaconLost == 0)
            {
                currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_RX_BEACON, MAC_ACTIVITY_PRI_HIGH_INDEX);
            }
            else
            {
                currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_RX_BEACON, MAC_ACTIVITY_PRI_URGENT_INDEX);
            }
        }
        break;

        case MAC_ACTIVITY_FH:
        {
           /* Default Priority is High */
           if(rxActivityData.numRxBCAbort < MAC_ACTIVITY_THRESHOLD_PRI_URGENT)
           {
               currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_FH, MAC_ACTIVITY_PRI_HIGH_INDEX);
           }
           else
           {
               currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_FH, MAC_ACTIVITY_PRI_URGENT_INDEX);
           }
        }
        break;
        case MAC_ACTIVITY_SCAN:
        {
            /* All scans high priority by default */
            if(rxActivityData.numRxScanAbort < MAC_ACTIVITY_THRESHOLD_PRI_URGENT)
            {
                currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_SCAN, MAC_ACTIVITY_PRI_HIGH_INDEX);
            }
            else
            {
                currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_SCAN, MAC_ACTIVITY_PRI_URGENT_INDEX);
            }
        }
        break;
        case MAC_ACTIVITY_DATA:
        {
            /* Default Priority is Normal */
            if(rxActivityData.numRxDataAbort < MAC_ACTIVITY_THRESHOLD_PRI_NORMAL)
            {
                currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_DATA, MAC_ACTIVITY_PRI_NORMAL_INDEX);
            }
            else if(rxActivityData.numRxDataAbort < MAC_ACTIVITY_THRESHOLD_PRI_HIGH)
            {
                currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_DATA, MAC_ACTIVITY_PRI_HIGH_INDEX);
            }
            else
            {
                currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_DATA, MAC_ACTIVITY_PRI_URGENT_INDEX);
            }
        }
        break;
        case MAC_ACTIVITY_RX_ALWAYS_ON:
        {
            /* Default Priority is Normal */
            currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_RX_ALWAYS_ON, MAC_ACTIVITY_PRI_NORMAL_INDEX);
        }
        break;
        default:
            currPri = CALC_ACTIVITY_PRIORITY(MAC_ACTIVITY_DATA, MAC_ACTIVITY_PRI_NORMAL_INDEX);
        break;
    }
    return (currPri);
}

/*******************************************************************************
 * @fn          setActivityTx
 *
 * @brief       This call is used by the LMAC to set the transmit activity.
 *
 * input parameters
 *
 * @param       txData - pointer to transmit packet.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      macActivity_t Current activity.
 */
static void setActivityTx(macTxIntData_t *txData)
{
    macActivity_t currActivity = MAC_ACTIVITY_DATA;

    /* Treat as ACK frame */
    if (!txData)
    {
        currActivity = MAC_ACTIVITY_DATA;

        if ((rxActivityData.prevRxFrameType == MAC_ASSOC_REQ_FRAME) ||
            (rxActivityData.prevRxFrameType == MAC_ASSOC_RSP_FRAME) ||
            (rxActivityData.prevRxFrameType == MAC_COORD_REALIGN_FRAME))
        {
            currActivity = MAC_ACTIVITY_LINK_EST;
        }
    }
    else
    {
        switch(txData->frameType)
        {
            /* MAC_ACTIVITY_LINK_EST */
            case MAC_INTERNAL_ASSOC_REQ:
            case MAC_INTERNAL_ASSOC_RSP:
            case MAC_INTERNAL_DISASSOC_NOTIF:
            {
                currActivity = MAC_ACTIVITY_LINK_EST;
            }
            break;

            /* MAC_ACTIVITY_TX_BEACON */
            case MAC_INTERNAL_BEACON:
            case MAC_INTERNAL_BEACON_REQ:
            case MAC_INTERNAL_COEXIST_EB_REQ:
            case MAC_INTERNAL_ENHANCED_BEACON_REQ:
            {
                currActivity = MAC_ACTIVITY_TX_BEACON;
            }
            break;

            /* MAC_ACTIVITY_FH */
            case MAC_INTERNAL_DATA_UNICAST:
            case MAC_INTERNAL_DATA_BROADCAST:
            {
                currActivity = MAC_ACTIVITY_FH;
            }
            break;

            /* MAC_ACTIVITY_DATA */
            case MAC_INTERNAL_ASYNC:
            case MAC_INTERNAL_DATA:
            case MAC_INTERNAL_DATA_REQ:
            {
                /* For Association poll requests, stay in link establishment activity */
                if (txActivityData.prevTxFrameType == MAC_INTERNAL_ASSOC_REQ)
                {
                    currActivity = MAC_ACTIVITY_LINK_EST;
                }
                else
                {
                    currActivity = MAC_ACTIVITY_DATA;
                }
            }
            break;
        }
    }
    txActivityData.txActivity = currActivity;
}

/*******************************************************************************
 * @fn          setActivityRx
 *
 * @brief       This call is used by the LMAC to set the receive activity.
 *
 * input parameters
 *
 * @param       rxEnableFlags - Bitmask containing the receiver enable flags.
 *
 * output parameters
 *
 * @param       None.
 *
 */
static void setActivityRx(uint8_t rxEnableFlags)
{
    macActivity_t currActivity = MAC_ACTIVITY_DATA;

    /* MAC_ACTIVITY_RX_BEACON */
    if ((!MAC_FH_ENABLED) && (rxEnableFlags & MAC_RX_BEACON_DEVICE))
    {
        currActivity = MAC_ACTIVITY_RX_BEACON;
    }
    /* MAC_ACTIVITY_FH */
    else if ((MAC_FH_ENABLED) && (rxEnableFlags & MAC_RX_FH_BC_SLOT))
    {
        currActivity = MAC_ACTIVITY_FH;
    }

    /* MAC_ACTIVITY_SCAN */
    else if (rxEnableFlags & MAC_RX_SCAN)
    {
        currActivity = MAC_ACTIVITY_SCAN;
    }

    /* MAC_ACTIVITY_DATA */
    else if (rxEnableFlags & MAC_RX_BROADCAST_PEND)
    {
        currActivity = MAC_ACTIVITY_DATA;
    }
    else if (rxEnableFlags & MAC_RX_WHEN_IDLE)
    {
        currActivity = MAC_ACTIVITY_RX_ALWAYS_ON;
    }
    /* For Rx poll (MAC_RX_POLL) requests or pre-tx explicit RxOn, accommodate Association procedure based on tx activity */
    else
    {
        /* Either switch to Link establishment or data activity */
        currActivity = (txActivityData.txActivity == MAC_ACTIVITY_LINK_EST) ? MAC_ACTIVITY_LINK_EST : MAC_ACTIVITY_DATA;
    }
    rxActivityData.rxActivity = currActivity;
}


/*******************************************************************************
 * @fn          getActivityTx
 *
 * @brief       This call is used by the LMAC to obtain the current transmit activity.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      macActivity_t Current activity.
 */
static macActivity_t getActivityTx(void)
{
    return (txActivityData.txActivity);
}

/*******************************************************************************
 * @fn          getActivityRx
 *
 * @brief       This call is used by the LMAC to obtain the current receive activity.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 */
static macActivity_t getActivityRx(void)
{
    return (rxActivityData.rxActivity);
}

#ifdef MAC_ACTIVITY_PROFILING

/* Interrupt period based on 10us clock tick */
#define ACTIVITY_PROFILING_PREEMPT_TIMEOUT (1000)

/* Number of internal command flushes occurred */
uint32_t numFlush = 0;

static void activityPreemptCb(void *arg)
{
    /* Only preempt Tx commands until Rx resume is implemented. */
    if ((arg) && (macTxCmdActive == TRUE))
    {
        /* Flush 15.4 command queue to force priority increase */
        (void)RF_flushCmd((RF_Handle) arg, RF_CMDHANDLE_FLUSH_ALL, RF_ABORT_PREEMPTION);
        numFlush++;
    }
}

static void startActivityProfilingTimer(RF_Handle handle)
{
    static ClockP_Handle macActivityClock = NULL;

    ClockP_Params params;
    ClockP_Params_init(&params);
    params.startFlag = FALSE;
    params.period = ACTIVITY_PROFILING_PREEMPT_TIMEOUT;
    params.arg = (uintptr_t)handle;
    macActivityClock = ClockP_create((ClockP_Fxn) activityPreemptCb,
                                    ACTIVITY_PROFILING_PREEMPT_TIMEOUT, &params, NULL);
    MAC_ASSERT(macActivityClock);
    //ClockP_start(macActivityClock);
}

static void printActivityInfo(uint32_t activity, uint32_t priority, uint8_t frameType, sAddr_t destAddr)
{
    /* Concatenated (last 2 bytes of extended address) or short address */
    uint16_t concatAddr;

    /* Parse address type */
    if (destAddr.addrMode == ApiMac_addrType_short) {
        concatAddr = destAddr.addr.shortAddr;
    }
    else
    {
        concatAddr = (uint16_t)(((destAddr.addr.extAddr[0]) & 0x00FF) + (((destAddr.addr.extAddr[1]) & 0x00FF) << 8));
    }

    switch(activity)
    {
        case MAC_ACTIVITY_LINK_EST:
        {
            Log_printf(LogModule_154_Low_Level_MAC, Log_INFO, "ACTIVITY PROFILE: Activity | MAC_ACTIVITY_LINK_EST | Priority (%x) | TxFrameType(%x) | DST Address(%x)",  priority,  frameType,  concatAddr);
        }
        break;
        case MAC_ACTIVITY_TX_BEACON:
        {
            Log_printf(LogModule_154_Low_Level_MAC, Log_INFO, "ACTIVITY PROFILE: Activity | MAC_ACTIVITY_TX_BEACON | Priority (%x) | TxFrameType(%x) | DST Address(%x)",  priority,  frameType,  concatAddr);
        }
        break;
        case MAC_ACTIVITY_RX_BEACON:
        {
            Log_printf(LogModule_154_Low_Level_MAC, Log_INFO, "ACTIVITY PROFILE: Activity | MAC_ACTIVITY_RX_BEACON | Priority (%x) | TxFrameType(%x) | DST Address(%x)",  priority,  frameType,  concatAddr);
        }
        break;

        case MAC_ACTIVITY_FH:
        {
            Log_printf(LogModule_154_Low_Level_MAC, Log_INFO, "ACTIVITY PROFILE: Activity | MAC_ACTIVITY_FH | Priority (%x) | TxFrameType(%x) | DST Address(%x)",  priority,  frameType,  concatAddr);
        }
        break;
        case MAC_ACTIVITY_SCAN:
        {
            Log_printf(LogModule_154_Low_Level_MAC, Log_INFO, "ACTIVITY PROFILE: Activity | MAC_ACTIVITY_SCAN | Priority (%x) | TxFrameType(%x) | DST Address(%x)",  priority,  frameType,  concatAddr);
        }
        break;
        case MAC_ACTIVITY_DATA:
        {
            Log_printf(LogModule_154_Low_Level_MAC, Log_INFO, "ACTIVITY PROFILE: Activity | MAC_ACTIVITY_DATA | Priority (%x) | TxFrameType(%x) | DST Address(%x)",  priority,  frameType,  concatAddr);
        }
        break;
        case MAC_ACTIVITY_RX_ALWAYS_ON:
        {
            Log_printf(LogModule_154_Low_Level_MAC, Log_INFO, "ACTIVITY PROFILE: Activity | MAC_ACTIVITY_RX_ALWAYS_ON | Priority (%x) | TxFrameType(%x) | DST Address(%x)",  priority,  frameType,  concatAddr);
        }
        break;
    }
}
#endif /* MAC_ACTIVITY_PROFILING */

/*******************************************************************************
 * @fn          getCoexActivityTx
 *
 * @brief       This call is used by the LMAC to obtain the current tx COEX status
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       bool    true if Link Establishment in process
 *
 */
static bool getCoexActivityTx(void)
{
#ifdef IEEE_COEX_3_WIRE
    if (txActivityData.txActivity == MAC_ACTIVITY_LINK_EST)
    {
        return true;
    }
    else
    {
        return false;
    }
#else
    return false;
#endif // IEEE_COEX_3_WIRE
}

/*******************************************************************************
 * @fn          getCoexActivityRx
 *
 * @brief       This call is used by the LMAC to obtain the current rx COEX status
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       bool    true if Link Establishment in process
 *
 */
static bool getCoexActivityRx(void)
{
#ifdef IEEE_COEX_3_WIRE
    if (rxActivityData.rxActivity == MAC_ACTIVITY_LINK_EST)
    {
        return true;
    }
    else
    {
        return false;
    }
#else
    return false;
#endif // IEEE_COEX_3_WIRE
}
#endif
/********************************************************************************/
