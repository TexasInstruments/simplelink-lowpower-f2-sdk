/******************************************************************************

 @file fh_api.c

 @brief TIMAC 2.0 FH API

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2023, Texas Instruments Incorporated

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

/******************************************************************************
 Includes
 *****************************************************************************/

#ifndef LIBRARY
#include "ti_wisunfan_features.h"
#endif

#include "hal_types.h"
#include "mac_main.h"
#include "mac_data.h"
#include "fh_api.h"
#include "fh_pib.h"
#include "fh_nt.h"
#include "fh_data.h"
#include "fh_util.h"
#include "fh_mgr.h"
#include "fh_sm.h"
#include "fh_ie.h"
#include "mac_high_level.h"
#include "mac_pib.h"
#include "mac_hl_patch.h"
/* LMAC function prototypes */
#include "mac.h"
#include "mac_rx.h"
#include "mac_radio.h"
#include "mac_tx.h"
#include "mac_mcu.h"
#include "mac_mgmt.h"
#include "mac_spec.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE fh_api_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE


#ifdef FEATURE_FREQ_HOP_MODE

/******************************************************************************
  ROM jump table will support
  1. TIMAC ROM image build
  2. TIMAC stack build without ROM image
  3. TIMAC stack with calling ROM image
 *****************************************************************************/
#include "rom_jt_154.h"

/*!
 This module is the FH module interface for the HMAC and LMAC. All function
 APIS in this file can be used by HMAC and LMAC.
 */

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/******************************************************************************
 Local variables
 *****************************************************************************/

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/

/******************************************************************************
 Global variables
 *****************************************************************************/
#ifdef FH_HOP_DEBUG
extern FH_DEBUG_s FH_Dbg;
#endif

/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 reset FH module.

 Public function defined in fh_api.h
 */
MAC_INTERNAL_API void FHAPI_reset(void)
{
    uint8_t maxChannels;
    uint8_t macBcDwellTime, macUcDwellTime;

    /*Stop timers if running */
    MAP_FHMGR_macCancelFHTimer(&FH_hnd.bcTimer);

    MAP_FHMGR_macCancelFHTimer(&FH_hnd.ucTimer);

    MAP_FHMGR_macCancelFHTimer(&FH_hnd.minTxOffTimer);

    /*reset internal data structure */
    MAP_osal_memset(&FH_hnd, 0, sizeof(FH_HND_s));

    /*reset all the FH PIBs */
    MAP_FHPIB_reset();

    /* update the default ucNumChannels and bcNumChannels */
    maxChannels = MAP_FHUTIL_getMaxChannels();
    FH_hnd.ucNumChannels = maxChannels;
    FH_hnd.bcNumChannels = maxChannels;

    /* register the SFD callback function */
    MAP_macRegisterSfdDetect(MAP_FHAPI_SFDRxCb);
    FH_hnd.regSFDFlag = 1;

    MAP_FHPIB_get(FHPIB_UC_DWELL_INTERVAL, &macUcDwellTime);

    MAP_FHPIB_get(FHPIB_BC_DWELL_INTERVAL, &macBcDwellTime);

    /* initialize the unicast and broadcast timers */
    FH_hnd.ucTimer.pFunc = MAP_FHMGR_ucTimerIsrCb;

    FH_hnd.ucTimer.duration = macUcDwellTime * (TICKPERIOD_MS_US);

    FH_hnd.bcTimer.pFunc = MAP_FHMGR_bcTimerIsrCb;

    FH_hnd.bcTimer.duration = macBcDwellTime * (TICKPERIOD_MS_US);

    /* set up callback function */
    FH_hnd.minTxOffTimer.pFunc = MAP_FHDATA_asyncMinTxOffTimerCb;
}

/*!
 start FH module.

 Public function defined in fh_api.h
 */
MAC_INTERNAL_API void FHAPI_start(void)
{
    uint8_t macBcDwellTime, macUcDwellTime;
    uint32_t macBcInterval;
    uint8_t excludedChannelMask[FHPIB_MAX_BIT_MAP_SIZE];
    uint16_t tableSize;
    macMRFSKPHYDesc_t *pPhyDesc;

    MAP_FHPIB_get(FHPIB_UC_DWELL_INTERVAL, &macUcDwellTime);

    MAP_FHPIB_get(FHPIB_BC_DWELL_INTERVAL, &macBcDwellTime);

    MAP_FHPIB_get(FHPIB_BC_INTERVAL, &macBcInterval);

    /*initialize the unicast and broadcast timers */
    FH_hnd.ucTimer.pFunc = MAP_FHMGR_ucTimerIsrCb;

    FH_hnd.ucTimer.duration = macUcDwellTime * (TICKPERIOD_MS_US);

    FH_hnd.bcTimer.pFunc = MAP_FHMGR_bcTimerIsrCb;

    FH_hnd.bcTimer.duration = macBcDwellTime * (TICKPERIOD_MS_US);

    /* set up callback function */
    FH_hnd.minTxOffTimer.pFunc = MAP_FHDATA_asyncMinTxOffTimerCb;

    /* If coordinator, start broadcast timer */
    if(macPanCoordinator)
    {
        MAP_FHPIB_get(FHPIB_BC_EXCLUDED_CHANNELS, excludedChannelMask);
        MAP_FHUTIL_updateBcNumChannels(excludedChannelMask);

        if((macBcDwellTime > 0) && (macBcInterval > 0))
        {
            MAP_FHMGR_macStartFHTimer(&FH_hnd.bcTimer, TRUE);

            /*safer to replace with SM event later */
            MAP_FHMGR_updateRadioBCChannel(&FH_hnd);

            FH_hnd.bcDwellActive = 1;
            FH_hnd.bsStarted = 1;
        }

        MAP_FHPIB_get(FHPIB_UC_EXCLUDED_CHANNELS, excludedChannelMask);
        MAP_FHUTIL_updateUcNumChannels(excludedChannelMask);
        /*Start Unicast Timer */
        MAP_FHMGR_macStartFHTimer(&FH_hnd.ucTimer, TRUE);

    }
    else
    {
        MAP_FHPIB_get(FHPIB_UC_EXCLUDED_CHANNELS, excludedChannelMask);
        MAP_FHUTIL_updateUcNumChannels(excludedChannelMask);
        /*Start Unicast Timer */
        MAP_FHMGR_macStartFHTimer(&FH_hnd.ucTimer, TRUE);

        /*safer to replace with SM event later */
        MAP_FHMGR_updateRadioUCChannel(&FH_hnd);
    }

    /* get the LBT flag */
    /* get the band info from PHY ID */
    pPhyDesc = MAP_macMRFSKGetPhyDesc(pMacPib->curPhyID);
    if (pPhyDesc->ccaType == MAC_CCA_TYPE_LBT)
    {
        FH_hnd.LBT = FH_LBT_ENABLE_FLAG;
    }

    if (FH_hnd.fhStarted == 0 )
    {
        uint16_t memSize;
        bool macSec;

        /* init the FH NT table */
        MAP_FHNT_init();

        /* based on PIB values to set the NT table */
        MAP_FHPIB_get(FHPIB_NUM_MAX_NON_SLEEP_NODES, &tableSize);
        FHNT_hnd.maxNumNonSleepNodes = tableSize;

        MAP_FHPIB_get(FHPIB_NUM_MAX_SLEEP_NODES, &tableSize);
        FHNT_hnd.maxNumSleepNodes = tableSize;

        /* get the temp table size */
        MAP_FHPIB_get(FHPIB_NUM_MAX_TEMP_NODES, &tableSize);
        FHNT_tempTable.maxNumNode = tableSize;

        /* read the MAC secuirty setting */
        MAP_MAC_MlmeGetReq(MAC_SECURITY_ENABLED, &macSec);
        if (macSec)
        {
            FHNT_hnd.macSecurity = 1;
        }
        else
        {
            FHNT_hnd.macSecurity = 0;
        }

        /* allocate the corresponding memory */
        if (FHNT_hnd.maxNumNonSleepNodes)
        {
            memSize = (FHNT_hnd.maxNumNonSleepNodes) * sizeof(NODE_ENTRY_s) ;
            FHNT_hnd.pNonSleepNtTable = MAP_osal_mem_alloc( memSize);

            if (FHNT_hnd.pNonSleepNtTable == NULL )
            {
                MAP_FHUTIL_assert(FHNT_hnd.pNonSleepNtTable);
            }
        }
#ifdef FHNT_SPLIT_TABLE
        if (FHNT_hnd.maxNumSleepNodes)
        {
            memSize = (FHNT_hnd.maxNumSleepNodes ) * sizeof(NODE_FIXED_ENTRY_s) ;
            FHNT_hnd.pSleepNtTable = MAP_osal_mem_alloc( memSize);

            if (FHNT_hnd.pSleepNtTable == NULL )
            {
                MAP_FHUTIL_assert(FHNT_hnd.pSleepNtTable);
            }
        }
#endif

        /* dynamically allocate temp table */
        if (FHNT_tempTable.maxNumNode)
        {
            memSize = (FHNT_tempTable.maxNumNode ) * sizeof(sAddrExt_t);
            FHNT_tempTable.pAddr = MAP_osal_mem_alloc( memSize);
            if (FHNT_tempTable.pAddr == NULL)
            {
                MAP_FHUTIL_assert(FHNT_tempTable.pAddr);
            }
        }

        MAP_FHNT_initTempTable();

        FH_hnd.fhStarted = 1;
    }
}
/*!
 start Broadcast timer

 Public function defined in fh_api.h
 */
MAC_INTERNAL_API void FHAPI_startBS(void)
{
    uint8_t macBcDwellTime;
    uint32_t macBcInterval;

    MAP_FHPIB_get(FHPIB_BC_DWELL_INTERVAL, &macBcDwellTime);
    MAP_FHPIB_get(FHPIB_BC_INTERVAL, &macBcInterval);

    /* Start BS only if broadcast dwell time and interval > 0 */
    if((macBcDwellTime > 0) && (macBcInterval > 0))
    {
        /* check if in dwell time or interval time */
        /*only start timer and broadcast state, set channel only
         * on timer expiration as function can be called at any state */

        /* bfio may need to be compensated for time expired between reception and update */
        if(FH_hnd.bfio < macBcDwellTime)
        {
            /* units in us */
            FH_hnd.bcTimer.duration = (macBcDwellTime - FH_hnd.bfio) * TICKPERIOD_MS_US;

            FH_hnd.bcDwellActive = 1;

        }
        else if(FH_hnd.bfio < macBcInterval)
        {
            FH_hnd.bcTimer.duration = (macBcInterval - FH_hnd.bfio) * TICKPERIOD_MS_US;

            FH_hnd.bcDwellActive = 0;
        }
        else
        {
            MAP_FHUTIL_assert(&FH_hnd);
        }

        MAP_FHMGR_macCancelFHTimer(&FH_hnd.bcTimer);
        /*Start Broadcast Timer */
        MAP_FHMGR_macStartFHTimer(&FH_hnd.bcTimer, TRUE);

        FH_hnd.bsStarted = 1;
        DBG_PRINT0(DBGSYS, "FHTraceLog: Start BS");
    }
}

#if !defined(TIMAC_ROM_PATCH)
/*!
 LMAC TX complete callback function

 Public function defined in fh_api.h
 */
MAC_INTERNAL_API void FHAPI_completeTxCb(uint8_t status)
{
    halIntState_t intState;

    DBG_PRINTL1(DBGSYS, "FHTraceLog: LMACTXComplete RTCTime(0x%x)",MAP_ICall_getTicks());
    if (pMacDataTx != NULL)
    {
        /* save the TX transmission time
         * 1. MAC_SUCCESS: packet is transmitted
         * 2. MAC_NO_ACK: packet is sent, but no ACK
         */
        if ( (status == MAC_SUCCESS) || (status == MAC_NO_ACK ) )
        {
            FH_txTiming.pTxPkt[FH_txTiming.wrIndex].txTimeStamp = MAP_ICall_getTicks();
            FH_txTiming.pTxPkt[FH_txTiming.wrIndex].txChannel = FH_hnd.lastTxChannel;

            FH_txTiming.wrIndex++;

            /* handle wrap around */
            if (FH_txTiming.wrIndex >= FH_txTiming.maxNumEntry)
                FH_txTiming.wrIndex = 0;
        }

        if(status == MAC_NO_TIME) //uc to bc or bc to uc or due to RX
        {
            /* ASSERT on ASYNC frame : requeue to head of queue
               also, update nb and be to internal data structure
               push frame on to the tx queue
            */
#ifdef FH_HOP_DEBUG
            FH_Dbg.numTxMacNoTime++;
#endif
            /* CHECK FOR SIZE OF QUEUE */
            HAL_ENTER_CRITICAL_SECTION(intState);
            //if(macData.directCount < macCfg.txDataMax)
            if ((!(pMacDataTx->internal.txOptions & MAC_TXOPTION_INDIRECT) &&
               (macCfg.txDataMax > macData.directCount)) ||
               ((pMacDataTx->internal.txOptions & MAC_TXOPTION_INDIRECT) &&
               (macCfg.txDataMax > macData.indirectCount)))
            {
                MAP_osal_msg_push(&macData.txQueue, pMacDataTx);

                if (!(pMacDataTx->internal.txOptions & MAC_TXOPTION_INDIRECT))
                {
                    macData.directCount++;
                }
                else
                {
                    macData.indirectCount++;
                }
                HAL_EXIT_CRITICAL_SECTION(intState);

                if(pMacDataTx->internal.frameType == MAC_FRAME_TYPE_BROADCAST)
                {
                    //need to set bc or uc
                    FH_hnd.bcPktPending = 1;
                }
                else
                {
                    FH_hnd.pktPending = 1;
                }
                pMacDataTx = NULL;
            }
            else
            {
                HAL_EXIT_CRITICAL_SECTION(intState);
#ifdef FH_HOP_DEBUG
                FH_Dbg.numTxMacNoTimeQueFull++;
#endif
                MAP_macTxCompleteCallback(MAC_TRANSACTION_OVERFLOW);
            }
            return;
        }
#ifdef FH_HOP_DEBUG
        if ( status == MAC_SUCCESS)
        {
            FH_Dbg.numTxMacTxOk++;
        }
        else if ( status == MAC_NO_ACK)
        {
            FH_Dbg.numTxMacNoACK++;
        }
        else if ( status == MAC_CHANNEL_ACCESS_FAILURE )
        {
            FH_Dbg.numTxMacCCAFail++;
        }
        else if ( status == MAC_TX_ABORTED )
        {
            FH_Dbg.numTxMacAbort++;
        }
        else if ( status == MAC_NO_RESOURCES )
        {
            FH_Dbg.numTxMacNoResource++;
        }
        else
        {
            FH_Dbg.numTxMacOther++;
        }
#endif
        /* TBD - If ASYNC or BC is a priority over re-transmission, we can exit
         * from here
         */
#ifdef FH_PRINT_DEBUG
        FHUTIL_printTxDebugInfo();
#endif
        /*In case of any error happens, stop ASYNC operation.*/
        if(  (status != MAC_SUCCESS ) &&
             (FH_FSM.currentState == FH_ST_ASYNC))
        {
            if (status == MAC_CHANNEL_ACCESS_FAILURE )
            {   /* CCA failure in ASYNC, we will go to next channel any way */
#ifdef FH_PRINT_DEBUG
                FH_Dbg.numTxAsyncCCAError++;
#endif
                MAP_FHSM_event(&FH_FSM, FH_EVT_TX_DONE, &FH_hnd);
            }
            else
            {
                FH_hnd.asyncChIdx = 0;
                MAP_FHSM_event(&FH_FSM, FH_EVT_ASYNC_DONE, &FH_hnd);
            }
        }
        else
        {
            /* drive the FH state machine */
            MAP_FHSM_event(&FH_FSM, FH_EVT_TX_DONE, &FH_hnd);
        }

        if((FH_FSM.currentState != FH_ST_ASYNC) && (status != MAC_NO_TIME)
            && (pMacDataTx != NULL))
        {
#ifdef FH_HOP_DEBUG
            FH_Dbg.numTxCnf++;
#endif
            /* reset nb and be */
            pMacDataTx->internal.nb = 0;
            pMacDataTx->internal.be =  (pMacDataTx->internal.txOptions & MAC_TXOPTION_ALT_BE) ? pMacPib->altBe : pMacPib->minBe;
            /* callback to HMAC */
            MAP_macTxCompleteCallback(status);
        }

        /* check if ASYNC purge is request */
        if (FH_hnd.asyncStop == 1)
        {
            HAL_ENTER_CRITICAL_SECTION(intState);

            status = MAP_FHDATA_purgePktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_ASYNC);

            HAL_EXIT_CRITICAL_SECTION(intState);

            /* reset async purge flag */
            FH_hnd.asyncStop = 0 ;
        }
    }
}
#endif

/*!
 LMAC RX complete callback function

 Public function defined in fh_api.h
 */
MAC_INTERNAL_API void FHAPI_completeRxCb(macRx_t *pMsg)
{
    uint16_t Len1, Len2;
    FHAPI_status status;
    FHIE_utIE_t utie;
    sAddrExt_t parentEUI;
    sAddrExt_t *pSrcAddr = (sAddrExt_t *)&pMsg->mac.srcAddr;
    sAddrExt_t defaultEUI;
    sAddr_t localSrcAddr;
    sAddr_t *pLocalSrcAddr = &localSrcAddr;

    MAP_osal_memcpy(&defaultEUI,&(FHPIB_defaults.macTrackParentEUI),sizeof(sAddrExt_t));

    if (FH_hnd.regSFDFlag == 0 )
    {	/* drop this RX done callback because the SFD is not register yet */
        return;
    }
#ifdef FH_HOP_DEBUG
    FH_Dbg.numRxDone++;
#endif
    if (pMsg->hdr.status != MAC_SUCCESS)
    {
#ifdef FH_HOP_DEBUG
        FH_Dbg.numRxLMACErr++;
#endif
        /* FH should discard this frame */
        return;
    }

    /* check for PHY_ID  and add to NT, return */
    /*check for poll packets only */
    /*update frameType */
    FHIE_usIe_t usie;
    uint8_t channel;

    usie.schedInfo.channelFunction = 255;
    channel = macPhyChannel;

    status = MAP_FHIE_extractPie(pMsg->mac.pPayloadIE, FH_IE_TYPE_US_IE, (uint8_t *)&usie);

    if((status == FHAPI_STATUS_SUCCESS) && (!usie.schedInfo.channelFunction))
    {
        channel = usie.chanInfo.fixedChannel;
    }

    if((status != FHAPI_STATUS_SUCCESS)
       || ((status == FHAPI_STATUS_SUCCESS) && (!usie.schedInfo.channelFunction)))
    {
        if((pMsg->internal.fcf & MAC_FCF_FRAME_TYPE_MASK) == MAC_FRAME_TYPE_ACK)
        {
            pMsg->internal.fhFrameType = MAC_FH_FRAME_ACK;
        }
        else
        {
            pMsg->internal.fhFrameType = MAC_FH_FRAME_DATA;
        }

      if(((pMsg->internal.fcf & MAC_FRAME_TYPE_EXTENDED) == MAC_FRAME_TYPE_COMMAND)
          && (*pMsg->msdu.p == MAC_DATA_REQ_FRAME))
      {
        MAP_FHNT_addOptEntry(&pMsg->mac.srcAddr, channel);

        pMsg->internal.fhFrameType = MAC_FH_FRAME_DATA;

        return;
      }

      /* check for coord address and map to ext.. addr */
      if((pMsg->mac.srcAddr.addrMode == SADDR_MODE_SHORT ) && !macPanCoordinator)
      {
        if(!MAP_memcmp(&pMsg->mac.srcAddr.addr.shortAddr, &macPib.coordShortAddress, 2))
        {
          MAP_sAddrExtCpy(pLocalSrcAddr->addr.extAddr, macPib.coordExtendedAddress.addr.extAddr);
          pSrcAddr = &pLocalSrcAddr->addr.extAddr;
        }
      }
    }

    if ( pMsg->pHdrIes == NULL)
    {
#ifdef FH_HOP_DEBUG
        /* drop this packet IE */
        FH_Dbg.err_HIEPtr++;
#endif
        return;
    }

    /*update frameType */
    status = MAP_FHIE_extractHie(pMsg->pHdrIes, FH_IE_TYPE_UT_IE, (uint8_t *)&utie);

    if(status == FHAPI_STATUS_SUCCESS)
    {   /* set up the RX fhFrame type for all RX indication */
        pMsg->internal.fhFrameType = utie.frameTypeId;
#ifdef FH_HOP_DEBUG
        FH_Dbg.numRxFhFrameType[utie.frameTypeId & 0x07]++;
#endif

        if ( ( (utie.frameTypeId & 0x07) == 0x4) &&
               (pMsg->mac.dstAddr.addrMode == SADDR_MODE_NONE) )
        {   /* broadcast data packet */
#ifdef FH_HOP_DEBUG
            FH_Dbg.numRxDoneBroadcast++;
#endif
        }
    }
    else
    {   /* UT_IE is wrong */
#ifdef FH_HOP_DEBUG
        FH_Dbg.errRxUTIE++;
#endif
        return;
    }

    /*
     * LMAC has updated the payload IE pointer
     */
    if (pMsg->mac.pPayloadIE != NULL)
    {
        status = MAP_FHIE_parsePie(pMsg->mac.pPayloadIE, pSrcAddr,
                               &Len1, &Len2);
        if (status != FHAPI_STATUS_SUCCESS)
        {
            return;
        }
    }
    /* one more pareseHie to update UT IE in the node table */
    status = MAP_FHIE_parseHie(pMsg->pHdrIes, pSrcAddr, 0, &Len1, &Len2);
    if (status != FHAPI_STATUS_SUCCESS)
    {
        return;
    }
    /* update Broadcast timing, if BT-IE present and packet from PARENT_EUI */
    if(!macPanCoordinator)
    {
        MAP_FHPIB_get(FHPIB_TRACK_PARENT_EUI, &parentEUI);
        if((!MAP_memcmp(&parentEUI, &defaultEUI, sizeof(sAddrExt_t)) ||
            !MAP_memcmp(&parentEUI, pSrcAddr, sizeof(sAddrExt_t))) &&
           ((FH_hnd.btiePresent) && (FH_hnd.fhBSRcvd)) )
        {
            if(MAP_FHUTIl_updateBTIE(FH_hnd.btie, FH_hnd.rxSlotIdx))
            {
                /* enables to cancel and restart timer to match with preferred parent */
                /* Also, enables to set new BC timer to be with reference to updated FH_hnd.bfio */
                MAP_FHAPI_startBS();
            }
            FH_hnd.bsStarted = 1;
        }
    }

    /*BT_IE present = 0, for next frame */
	FH_hnd.btiePresent = 0;

    DBG_PRINTL2(DBGSYS, "FHTraceLog: LMACRxDone, BTIE updated RTCTime(0x%x) bfio(0x%x)", MAP_ICall_getTicks(), FH_hnd.bfio);

}

#if !defined(TIMAC_ROM_PATCH)
/*!
 FH transmit data packet

 Public function defined in fh_api.h
 */
MAC_INTERNAL_API void  FHAPI_sendData(void)
{
    macTx_t *pCurr = NULL;
    uint32_t bfio;
    uint16_t slotIdx;
    halIntState_t intState;
    FHAPI_status status;

    /* if transmission in progress return */
    if (pMacDataTx != NULL)
    {
#ifdef FH_HOP_DEBUG
        FH_Dbg.numTxInProgess++;
#endif
        return;
    }

    if (MAP_OSAL_MSG_Q_EMPTY(&macData.txQueue))
    { /* TX queue is empty */
#ifdef FH_HOP_DEBUG
        FH_Dbg.numTxQueueEmpty++;
#endif
        return;
    }

    /* Will get here only on TX free  */
    /* TBD - Add getCurrState */
    if(FH_FSM.currentState == FH_ST_RX)
    {
#ifdef FH_HOP_DEBUG
        FH_Dbg.numTxinRxMode++;
#endif
        return;
    }

    DBG_PRINTL1(DBGSYS, "FHTraceLog: FH Send Data(0x%X)", 0);
#ifdef FH_HOP_DEBUG
    FH_Dbg.numTxDataSend++;
#endif
    HAL_ENTER_CRITICAL_SECTION(intState);

    /* Sanity check to remove all unsupported frame types from queue */
    MAP_FHDATA_purgeUnSupportedFrameTypes(&macData.txQueue);

    pCurr = MAP_FHDATA_getPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_ASYNC);
    if(pCurr != NULL)
    {
        HAL_EXIT_CRITICAL_SECTION(intState);

        MAP_FHDATA_chkASYNCStartReq();

        return;
    }

    pCurr = MAP_FHDATA_getPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_EDFE);
    if(pCurr != NULL)
    {
        HAL_EXIT_CRITICAL_SECTION(intState);
        MAP_FHSM_event(&FH_FSM, FH_EVT_EDFE_START, &FH_hnd);
        return;
    }

    MAP_FHUTIL_getCurBfio(&bfio, &slotIdx);

    if(MAP_FHDATA_checkPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_BROADCAST))
    {
        if (FH_hnd.bsStarted == 0)
        {   /* the broadcast schedule is not started, need to drop this packet
             * send the TX conf with error
            */
#ifdef FH_HOP_DEBUG
            FH_Dbg.numBroadcastDrop++;
#endif
            MAP_FHDATA_getPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_BROADCAST);
            HAL_EXIT_CRITICAL_SECTION(intState);
            MAP_macTxCompleteCallback(MAC_BAD_STATE);
            return ;
        }

        uint8_t slotEdgeTime;
        slotEdgeTime = FH_SLOT_EDGE_PROTECTION_TIME_MS;

        if(MAP_macCheckPhyRate(pMacPib->curPhyID) == PHY_MODE_SLR_5K)
        {
            slotEdgeTime = FH_LRM_SLOT_EDGE_PROTECTION_TIME_MS;
        }

        if((bfio < (FHPIB_db.macBcDwellInterval - slotEdgeTime)))
        {
            pCurr = MAP_FHDATA_getPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_BROADCAST);

            HAL_EXIT_CRITICAL_SECTION(intState);

            FH_hnd.bcPktPending = 0;
#ifdef FH_HOP_DEBUG
            FH_Dbg.numBroadcastStart++;
#endif
            MAP_macTxFrame(MAC_TX_TYPE_FH_CSMA);

            return;

        }
        else
        {
            FH_hnd.bcPktPending = 1;
        }
    }

    /*If not in BC slot */

    pCurr = MAP_FHDATA_getPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_UNICAST);
    if(pCurr != NULL)
    {
        NODE_ENTRY_s Entry;

        HAL_EXIT_CRITICAL_SECTION(intState);

        /*need to init nb, be */
#ifdef FH_HOP_DEBUG
        FH_Dbg.numUnicastStart++;
#endif
        /*
         * check to see if the destination address is in the NT
         */
        status = MAP_FHNT_getEntry(&(pMacDataTx->internal.dest.dstAddr), &Entry);
        if(status != FHAPI_STATUS_SUCCESS)
        {
            /*
             * send TX Cnf with error status
             */
#ifdef FH_HOP_DEBUG
            FH_Dbg.numUnicast_entryNotInNT_1++;
#endif
            MAP_macTxCompleteCallback(status);
            return ;
        }

        if(bfio >= FHPIB_db.macBcDwellInterval)
        {
            FH_hnd.pktPending = 0;
            MAP_macTxFrame(MAC_TX_TYPE_FH_CSMA);
        }
        else
        {
            MAP_FHAPI_completeTxCb(MAC_NO_TIME);
        }
        return;
    }

    HAL_EXIT_CRITICAL_SECTION(intState);
}
#endif

/*!
 LMAC RX complete callback function

 Public function defined in fh_api.h
 */
MAC_INTERNAL_API FHAPI_status FHAPI_getTxParams_old(uint32_t *pBackOffDur, uint8_t *pChIdx)
{
    NODE_ENTRY_s Entry;
    FHAPI_status status = FHAPI_STATUS_SUCCESS;
    uint32_t txUfsi;
    uint16_t pollChannel = 0;

    /* check for broadcast slot end */
    if(pMacDataTx->internal.frameType == MAC_FRAME_TYPE_BROADCAST)
    {
        status = MAP_FHUTIL_getBcTxParams(pBackOffDur, pChIdx);
        DBG_PRINTL2(DBGSYS, "FHAPI_getTxParams(pBackOffDur=%i, pChidx=%i)", *pBackOffDur, *pChIdx);
        FH_hnd.lastTxChannel = *pChIdx;
        return(status);
    }
    else
    {   /* UNICAST, ASSOCIATION_REQ/RSP, DATA_REQ*/
        status = MAP_FHNT_getEntry(&pMacDataTx->internal.dest.dstAddr, &Entry);
        if(status != FHAPI_STATUS_SUCCESS)
        {
#ifdef FH_HOP_DEBUG
            FH_Dbg.numUnicast_entryNotInNT_2++;
#endif
            return(status);
        }

        /* check for UC to BC crossing here and return
           FHAPI_STATUS_ERR_OUT_SLOT if needed.
         */

        /* calc chIdx at curTstmp + backoffDur + CCATime */
        status = MAP_FHUTIL_adjBackOffDur(&Entry, pBackOffDur, &txUfsi);

        if(status == FHAPI_STATUS_SUCCESS)
        {
            *pChIdx = MAP_FHUTIL_getTxChannel(&Entry, txUfsi, *pBackOffDur);
            FH_hnd.lastTxChannel = *pChIdx;
            DBG_PRINTL2(DBGSYS, "FHTraceLog: FHAPI_getTxParams(pBackOffDur=%i, pChidx=%i)", *pBackOffDur, *pChIdx);
        }
        return(status);
    }
}


/*!
 LMAC get TX parameter new function with LBT

 Public function defined in fh_api.h
 */
MAC_INTERNAL_API FHAPI_status FHAPI_getTxParams(uint32_t *pBackOffDur, uint8_t *pChIdx)
{
    uint8_t txChan;
    uint16_t i;
    uint32_t txdelay, remainingTime;
    FHAPI_status status = FHAPI_STATUS_SUCCESS;

    if (FH_hnd.LBT == FH_LBT_DISABLE_FLAG)
    {   /* call old function */
        return MAP_FHAPI_getTxParams_old(pBackOffDur, pChIdx);
    }

    /* here, LBT mode is enabled */
    for (i = 0; i < FH_txTiming.maxNumEntry; i++)
    {
        status = MAP_FHAPI_getTxParams_old(pBackOffDur, pChIdx);
        if (status != FHAPI_STATUS_SUCCESS)
        {
            return status;
        }

        /* check the remaining time */
        txChan = *pChIdx;
        txdelay = *pBackOffDur;

        remainingTime = MAP_FHDATA_CheckMinTxOffTime(txChan,txdelay);

        if (remainingTime == 0)
        {
            return FHAPI_STATUS_SUCCESS;
        }

        /* add the extra time (remaining time) to csma/ca delay */
        *pBackOffDur += remainingTime;
    }

    /* should never get here */
    MAP_FHUTIL_assert(NULL);

    return FHAPI_STATUS_ERR;
}
/*!
  get channel function of target node.

 Public function defined in fh_api.h
 */
MAC_INTERNAL_API uint8_t FHAPI_getTxChHoppingFunc(void)
{
	FHAPI_status status;
    NODE_ENTRY_s Entry;

	status = MAP_FHNT_getEntry(&pMacDataTx->internal.dest.dstAddr, &Entry);
	if(status != FHAPI_STATUS_SUCCESS)
	{
		return(FHIE_CF_SINGLE_CHANNEL);
	}

	return (Entry.UsieParams_s.channelFunc);
}

/*!
  get remaining dwell time of current slot of target node.

 Public function defined in fh_api.h
 */
MAC_INTERNAL_API uint32_t FHAPI_getRemDT(void)
{
	FHAPI_status status;
	NODE_ENTRY_s Entry ;
	uint32_t txUfsi;

	status = MAP_FHNT_getEntry(&pMacDataTx->internal.dest.dstAddr, &Entry);
	if(status != FHAPI_STATUS_SUCCESS)
	{
		return (0);
	}

	txUfsi = 0;
	if(Entry.UsieParams_s.channelFunc)
	{
	   txUfsi = MAP_FHUTIL_getTxUfsi(&Entry);

       DBG_PRINTL1(DBGSYS, "FHTraceLog:  (txUfsi=%i)", txUfsi);

      /*delta = ufsi - slotIdx*usie.ucDwellTime */
      /* time into the slot */
      txUfsi = (txUfsi % (Entry.UsieParams_s.dwellInterval));
      /*remaining time in slot */
      txUfsi = (Entry.UsieParams_s.dwellInterval - txUfsi)*1000;

	}
	return (txUfsi);
}

/*!
 Stop ASYNC operation

 Public function defined in fh_api.h
 */
MAC_INTERNAL_API FHAPI_status FHAPI_stopAsync(void)
{
    halIntState_t intState;
    FHAPI_status status;

#ifdef FH_HOP_DEBUG
    FH_Dbg.numAsyncStopReq++;
#endif
    /* check state in asyncStop and return if !Asyncstate */
    if(FH_FSM.currentState != FH_ST_ASYNC)
    {
        /*
         * if asyncStop, check for ASYNC frames in queue and de-queue
         * all ASYNC frames, if didn't start
         */
        HAL_ENTER_CRITICAL_SECTION(intState);

        status = MAP_FHDATA_purgePktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_ASYNC);

        HAL_EXIT_CRITICAL_SECTION(intState);

        return(status);
    }
    else
    {
        /*
         * If a request in progress, set flag and wait for TX_DONE
         * to clear that request.
         */

        FH_hnd.asyncStop = 1;

        return(FHAPI_STATUS_SUCCESS);
    }
}

/*!
 LMAC callback function to set state

 Public function defined in fh_api.h
 */
MAC_INTERNAL_API void FHAPI_setStateCb(FH_LMAC_STATUS_t state)
{
    uint8_t event;

    DBG_PRINTL1(DBGSYS, "FHTraceLog: FHAPI_setStateCb(state=%i)", state);
    switch(state)
    {
    /*if state == TX_START */
    case LMAC_TX_START:
        event = FH_EVT_TX_START;
        DBG_PRINTL1(DBGSYS, "FHTraceLog: LMACTXStart RTCTime(0x%x)", MAP_ICall_getTicks());
        break;
        /* if state == RX_SFD */
    case LMAC_RX_SFD:
        event = FH_EVT_RX_START;
        break;
        /* if state == CCA_BUSY*/
    case LMAC_CCA_BUSY:
#ifdef FH_HOP_DEBUG
        FH_Dbg.numTxCCAbusy++;
#endif
        event = FH_EVT_TX_DONE;
        break;
    case LMAC_RSSI_BUSY:
#ifdef FH_HOP_DEBUG
        FH_Dbg.numTxRssiBusy++;
#endif
        event = FH_EVT_TX_DONE;
        break;
    case LMAC_TX_PUSH_TO_QUEUE:
#ifdef FH_HOP_DEBUG
        FH_Dbg.numTxPushToQue++;
#endif
        /* no state to change */
        return;

    default:
        return;
    }

    MAP_FHSM_event(&FH_FSM, event, &FH_hnd);
}

/*!
 LMAC SFD detected callback function

 Public function defined in fh_api.h
 */
MAC_INTERNAL_API void FHAPI_SFDRxCb(uint8 status)
{
    uint8_t event;
    uint32_t temp32_1;

    if (status == MAC_SFD_DETECTED )
    {
        temp32_1 = MAP_ICall_getTicks();
        FH_hnd.rxSfdTs = temp32_1;

#ifdef FH_HOP_DEBUG
        FH_Dbg.numRxSFD_detected++;
#endif
        DBG_PRINTL1(DBGSYS, "FHTraceLog: SFDDetected RTCTime(0x%x)", temp32_1);

        if (FH_FSM.currentState == FH_ST_HOP)
        {
            event = FH_EVT_RX_START;
        }
        else
        {
#ifdef FH_HOP_DEBUG
            if (FH_FSM.currentState == FH_ST_TX)
            {
                FH_Dbg.numRxSFD_detected_in_tx++;
            }
            else
            {
                FH_Dbg.numRxSFD_detected_not_in_hop++;
            }
#endif
            return;
        }
    }
    else
    {
        event = FH_EVT_RX_DONE;

#ifdef FH_HOP_DEBUG
        FH_Dbg.numRxSFD_received++;
#endif
        if (status == MAC_SFD_FRAME_RECEIVED )
        {
            if(FH_FSM.currentState != FH_ST_RX)
                return;
        }
        else
        {	/* unsupported status */
#ifdef FH_HOP_DEBUG
            FH_Dbg.numRxSFD_unknown++;
#endif
            return;
        }
#ifdef FH_PRINT_DEBUG
        FH_printDebugInfo();
#endif
        DBG_PRINTL1(DBGSYS, "FHTraceLog: LMACRxComplete RTCTime(0x%x)", MAP_ICall_getTicks());
    }

    /* enter here only on Radio swi */
    MAP_FHSM_event(&FH_FSM, event, &FH_hnd);

    DBG_PRINTL1(DBGSYS, "FHTraceLog: SFD Cb fsm state(0x%x)", FH_FSM.currentState);
}


MAC_INTERNAL_API uint8_t FHAPI_getBitCount(void)
{
    return(MAP_FHUTIL_getBitCounts((uint8_t *)pMacDataTx->internal.dest.chList,
                                               MAP_FHUTIL_getMaxChannels()));
}
#endif
