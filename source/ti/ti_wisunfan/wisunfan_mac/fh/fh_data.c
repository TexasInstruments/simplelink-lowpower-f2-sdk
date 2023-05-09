/******************************************************************************

 @file fh_data.c

 @brief TIMAC 2.0 FH data API

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

#include "fh_api.h"
#include "hal_mcu.h"
#include "timac_api.h"
#include "mac_mcu.h"
#include "mac_main.h"
#include "mac_pib.h"
#include "mac_timer.h"
#include "mac_spec.h"
#include "mac_data.h"
#include "mac_radio.h"
#include "mac_tx.h"
#include "mac.h"
#include "mac_low_level.h"
#include "mac_ie_parse.h"
#include "mac_security.h"
#include "fh_data.h"
#include "fh_util.h"
#include "fh_pib.h"
#include "fh_sm.h"
#include "fh_mgr.h"
#include "rom_jt_154.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE fh_data_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

#ifdef FEATURE_FREQ_HOP_MODE

#define ONE_MS      1000
#define HALF_MS     500
/******************************************************************************
  ROM jump table will support
  1. TIMAC ROM image build
  2. TIMAC stack build without ROM image
  3. TIMAC stack with calling ROM image
 *****************************************************************************/
#include "rom_jt_154.h"

/******************************************************************************
 Local Variables
 *****************************************************************************/

/******************************************************************************
 Global Variables
 *****************************************************************************/

#ifdef FH_HOP_DEBUG
extern FH_DEBUG_s FH_Dbg;
#endif

/**************************************************************************************************
 * @fn          FHDATA_checkPktFromQueue
 *
 * @brief       This function checks if a packet whose frame type
 *                matches to the input param frameType is present in the txQueue
 *
 * input parameters
 *
 * MAP_osal_msg_q_t txQueue
 *
 * frameType
 *
 * output parameters
 *
 * pointer to the packet to be transmitted.
 *
 * @return      None.
 **************************************************************************************************
 */
MAP_osal_msg_q_t FHDATA_checkPktFromQueue(MAP_osal_msg_q_t *txQueue, uint8_t frameType)
{
    macTx_t *pCurr = NULL;

    /* find pkt of given frame type to transmit */
    pCurr = (macTx_t *)MAP_OSAL_MSG_Q_HEAD(txQueue);

    while (pCurr != NULL)
    {
        if ((pCurr->internal.frameType) == frameType)
        {
            break;
        }

        pCurr = MAP_OSAL_MSG_NEXT(pCurr);
    }

    return(pCurr);
}

/**************************************************************************************************
 * @fn          FHDATA_getPktFromQueue
 *
 * @brief       This function extracts the packet from tx queue whose frame type
 *                matches to the input param frameType
 * @param
 * input parameters
 *
 * MAP_osal_msg_q_t txQueue
 *
 * frameType
 *
 * output parameters
 *  None
 *
 *  @return
 *   pointer to the packet to be transmitted.
 **************************************************************************************************
 */
MAP_osal_msg_q_t FHDATA_getPktFromQueue(MAP_osal_msg_q_t *txQueue, uint8_t frameType)
{
    macTx_t *pCurr=NULL;
    macTx_t *pPrev=NULL;
    uint8_t unicastCmd;

    /* find next frame to transmit */
    pCurr = (macTx_t *)MAP_OSAL_MSG_Q_HEAD(txQueue);
    while (pCurr != NULL)
    {
        unicastCmd = 0;
        if ((((pCurr->internal.frameType) == frameType) || unicastCmd) && (pCurr->internal.txSched & MAC_TX_SCHED_READY))
        {
        /* take frame out of queue */
            MAP_osal_msg_extract(txQueue, pCurr, pPrev);

            pMacDataTx = pCurr;

            /* Proprietary - switching power and channel per data request */
            if (pMacDataTx->internal.txOptions & MAC_TXOPTION_PWR_CHAN)
            {
              MAP_macRadioSetTxPower(pMacDataTx->internal.power);
            }
            /* decrement frame count */
            /*Check if still needed */
            if (pMacDataTx->internal.txSched & MAC_TX_SCHED_INDIRECT)
            {
                macData.indirectCount--;
            }
            else
            {
                macData.directCount--;
            }
            break;
        }
        pPrev = pCurr;
        pCurr = MAP_OSAL_MSG_NEXT(pCurr);
    }

    return(pCurr);
}

/**************************************************************************************************
* @fn          FHDATA_purgePktFromQueue
*
* @brief       This function removes all the packets from tx queue whose frame type
*                matches to the input param frameType
* @param
* input parameters
*
* MAP_osal_msg_q_t txQueue
*
* frameType
*
* output parameters
*  None
*
*  @return
*   None.
**************************************************************************************************
*/

FHAPI_status FHDATA_purgePktFromQueue(MAP_osal_msg_q_t *txQueue, uint8_t frameType)
{
   macTx_t *pCurr=NULL;
   macTx_t *pPrev=NULL;
   macTx_t *pTemp=NULL;
   FHAPI_status status = FHAPI_STATUS_ERR_NOT_IN_ASYNC;

   /* find next frame to transmit */
   pCurr = (macTx_t *)MAP_OSAL_MSG_Q_HEAD(txQueue);
   while (pCurr != NULL)
   {
       if ((pCurr->internal.frameType) == frameType)
       {
           /* take frame out of queue */
           MAP_osal_msg_extract(txQueue, pCurr, pPrev);

           status = FHAPI_STATUS_SUCCESS;

           /* decrement frame count */
           /*Check if still needed */
           if (pCurr->internal.txSched & MAC_TX_SCHED_INDIRECT)
           {
               macData.indirectCount--;
           }
           else
           {
               macData.directCount--;
           }
           /* purge this packet, set status as ABORTED */
           pCurr->hdr.status = MAC_TX_ABORTED;
#ifdef FH_HOP_DEBUG
           FH_Dbg.numPurge++;
#endif
           pTemp = MAP_OSAL_MSG_NEXT(pCurr);
           /* pCuur will be freed in macDataTxComplete
            * need to save pTemp for later use
            */
           MAP_macDataTxComplete(pCurr);
           pCurr = pTemp;
       }
       else
       {
         pPrev = pCurr;
         pCurr = MAP_OSAL_MSG_NEXT(pCurr);
       }
   }

   return (status);
}

/**************************************************************************************************
* @fn          FHDATA_purgeUnSupportedFrameTypes
*
* @brief       This function removes packets of unsupported types from queue
*
* @param
* input parameters
*
* MAP_osal_msg_q_t txQueue
*
* output parameters
*  None
*
*  @return
*   None.
**************************************************************************************************
*/

void FHDATA_purgeUnSupportedFrameTypes(MAP_osal_msg_q_t *txQueue)
{
   macTx_t *pCurr=NULL;
   macTx_t *pPrev=NULL;
   macTx_t *pTemp=NULL;
   uint8_t supportedFrameType = 0;

   pCurr = (macTx_t *)MAP_OSAL_MSG_Q_HEAD(txQueue);
   while (pCurr != NULL)
   {
       supportedFrameType = 0;
       for( int i = 0; i < FH_SUPPORTED_FRAME_TYPES; i++)
       {
           if(pCurr->internal.frameType == fhSupFrameTypes[i])
           {
               supportedFrameType = 1;
               break;
           }

       }

       if(!supportedFrameType)
       {
           /* Un Supported Frame Type Found */
           /* take frame out of queue */
           MAP_osal_msg_extract(txQueue, pCurr, pPrev);

           /* decrement frame count */
           if (pCurr->internal.txSched & MAC_TX_SCHED_INDIRECT)
           {
               macData.indirectCount--;
           }
           else
           {
               macData.directCount--;
           }

           /* purge this packet, set status as ABORTED */
           pCurr->hdr.status = MAC_TX_ABORTED;

#ifdef FH_HOP_DEBUG
           FH_Dbg.numUnSupportedTypes++;
#endif
           pTemp = MAP_OSAL_MSG_NEXT(pCurr);
           /* pCuur will be freed in macDataTxComplete
            * need to save pTemp for later use
            */

           MAP_macDataTxComplete(pCurr);
           pCurr = pTemp;
       }
       else
       {
         pPrev = pCurr;
         pCurr = MAP_OSAL_MSG_NEXT(pCurr);
       }
   }
}
void FHDATA_chkASYNCStartReq(void)
{
#ifdef FH_HOP_DEBUG
    FH_Dbg.numAsyncStartReq++;
#endif
    DBG_PRINTL1(DBGSYS, "FHTraceLog: AsyncStart RTCTime(0x%X)", (uint32_t) MAP_ICall_getTicks());
    uint8 remaining_time;

    remaining_time = MAP_FHDATA_getASYNCDelay();

    if (remaining_time)
    {   /* need to delay  in ticks*/
       FH_hnd.minTxOffTimer.duration = remaining_time;

#ifdef FH_HOP_DEBUG
       FH_Dbg.numTxAsyncDelay++;
#endif
       MAP_FHMGR_macStartOneShotTimer(&FH_hnd.minTxOffTimer);
    }
    else
    {
        MAP_FHSM_event(&FH_FSM, FH_EVT_ASYNC_START, &FH_hnd);
    }
}
/**************************************************************************************************
 * @fn          FHDATA_procASYNCReq
 *
 * @brief       This function starts the transmission of ASYNC frames on every channel in the list
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void FHDATA_procASYNCReq(void *pData)
{
    FH_HND_s *pFH_hnd = (FH_HND_s *)pData;
    uint8_t curAsyncChBitMap = 0;
    uint8_t curChListPos = 0, bitIdx;
    int8_t i,chanBitMapSize;
#ifdef FH_HOP_DEBUG_FIXED
    if(pFH_hnd->asyncChIdx)
        pFH_hnd->asyncChIdx = MAP_FHUTIL_getMaxChannels();
#endif
    /*
     * Fix me later
     * ASYNC channel bit map size is dependent on regulation
     * For FCC, the maximum channel is 129, bitmap size is 16
     *
     * This variable should be in the FH_Hnd
     */
    chanBitMapSize = (MAP_FHUTIL_getMaxChannels() + 7) >> 3;
    if((pFH_hnd->asyncStop) || (pFH_hnd->asyncChIdx >= MAP_FHUTIL_getMaxChannels()))
    {
        pFH_hnd->asyncChIdx = 0;

        DBG_PRINTL1(DBGSYS, "FHTraceLog: AsyncEnd RTCTime(0x%X)", (uint32_t) MAP_ICall_getTicks());
        MAP_FHSM_event(&FH_FSM, FH_EVT_ASYNC_DONE, &FH_hnd);
        return;
    }

    curChListPos = pFH_hnd->asyncChIdx >> 3;
    bitIdx = pFH_hnd->asyncChIdx & 7;

    curAsyncChBitMap = pMacDataTx->internal.dest.chList[curChListPos] >> bitIdx;
    while((!curAsyncChBitMap) && (curChListPos < (chanBitMapSize -1 )))
    {
        curChListPos++;
        curAsyncChBitMap = pMacDataTx->internal.dest.chList[curChListPos];
        pFH_hnd->asyncChIdx = curChListPos * 8;
    }

    if(curAsyncChBitMap != 0)
    {
        i = pFH_hnd->asyncChIdx;
        while(i < (8*(curChListPos + 1)))
        {
            if(curAsyncChBitMap & 1)
            {
              DBG_PRINTL2(DBGSYS, "FHTraceLog: AsyncSetCh(%d) RTCTime(0x%X) ",pFH_hnd->asyncChIdx, (uint32_t) MAP_ICall_getTicks());
              /* Accessing LMAc variable, to be cleaned up in next release -  */
              /* channel will be set in RadioTxGo */
              /* This is to prevent RF queue overflow */
              macPhyChannel = pFH_hnd->asyncChIdx;
              /* save the last TX channel */
              pFH_hnd->lastTxChannel = macPhyChannel;

              pFH_hnd->asyncChIdx += 1;
              /* can this be done, I think still in ISR context, is a set_evt needed here? */
              MAP_macTxFrame(MAC_TX_TYPE_NO_CSMA);
              break;
            }
            else
            {
            	pFH_hnd->asyncChIdx += 1;
              curAsyncChBitMap >>= 1;
            }
            i++;
        }
    }
    /*check for ASYNC DONE */
    else
    {
        pFH_hnd->asyncChIdx = 0;
        /* ASYNC CNF status */
        pMacDataTx->hdr.status = MAC_SUCCESS;
        DBG_PRINTL1(DBGSYS, "FHTraceLog: AsyncEnd RTCTime(0x%X)", (uint32_t) MAP_ICall_getTicks());
        MAP_FHSM_event(&FH_FSM, FH_EVT_ASYNC_DONE, &FH_hnd);
    }
}

#if !defined(TIMAC_ROM_PATCH)
/**************************************************************************************************
 * @fn          FHDATA_startEDFEReq
 *
 * @brief       This function starts the transmission of EDFE frames and starts a timer for
 *              end of EDFE state
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void FHDATA_startEDFEReq(void *pData)
{
    /*
      startEDFETimer, based on TX FCIE, should be started based on TX_DONE
      FH_startEDFETimer(pData);
      store dest address into FH_hnd?? needed? or can be done at higher layer?
    */
    MAP_macTxFrame(MAC_TX_TYPE_FH_CSMA);

    return;
}

/**************************************************************************************************
 * @fn          FHDATA_procEDFEReq
 *
 * @brief       This function starts the transmission of EDFE frames
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void FHDATA_procEDFEReq(void *pData)
{

    MAP_macTxFrame(MAC_TX_TYPE_NO_CSMA);

    return;
}
#endif

void FHDATA_startEDFETimer(void *pData)
{
    /*
      if timer already running, don't restart
    */
    return;
}

/**************************************************************************************************
 * @fn          FHDATA_requeuePkt
 *
 * @brief       This function put the TX packet back to MAC TX queue
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void FHDATA_requeuePkt(void *pData)
{
    halIntState_t intState;

    if((pMacDataTx != NULL)/* && (pMacDataTx->internal.frameType == ASYNC)*/)
    {
        /* CHECK FOR SIZE OF QUEUE */
        HAL_ENTER_CRITICAL_SECTION(intState);
        if(macData.directCount < macCfg.txDataMax)
        {
           MAP_osal_msg_push(&macData.txQueue, pMacDataTx);
           macData.directCount++;

           pMacDataTx = NULL;
           HAL_EXIT_CRITICAL_SECTION(intState);
        }
        else
        {
            HAL_EXIT_CRITICAL_SECTION(intState);
#ifdef FH_HOP_DEBUG
            FH_Dbg.numTxMacNoTimeQueFull++;
#endif
            MAP_macTxCompleteCallback(MAC_TRANSACTION_OVERFLOW);
        }
    }
}

void FHDATA_updateCCA(uint32_t ccaVarTime)
{
    uint16_t ccaTime = 0;

    ccaTime = FH_LBT_CCA_TIME;
    if(pMacDataTx->internal.frameType != MAC_FRAME_TYPE_ASYNC)
    {
        ccaTime = ccaVarTime;
    }

    FH_hnd.ccaTime = ccaTime;
}


uint32_t FHDATA_CheckMinTxOffTime(uint8_t txChan,uint32_t csmaDelay)
{
    uint16_t rdIndex,i;
    FH_TX_TIMING_s *pEntry;
    uint32_t curTs,lastTs;
    uint32_t elapsedTime,remainTime = 0;

    if (FH_hnd.LBT == FH_LBT_DISABLE_FLAG )
    {
        return remainTime;
    }

    if ( (pMacDataTx->internal.txOptions & MAC_TXOPTION_NO_CNF) &&
         (pMacDataTx->internal.frameType == MAC_FRAME_TYPE_UNICAST))
    {   /* This is zero length packet, don't need to min TX off */
        return remainTime;
    }
    /* get current TS */
    curTs = MAP_ICall_getTicks();

    /* compare current ts with TX done history table
     * 1. backward through the table
     * 2. find the first one which the TX channel match
     *    check the time diff
     * 3. if not found, it is good
     *
     */
    if (FH_txTiming.wrIndex == 0)
    {
        rdIndex = FH_txTiming.maxNumEntry -1;
    }
    else
    {
        rdIndex = FH_txTiming.wrIndex- 1;
    }

    for (i=0; i < FH_txTiming.maxNumEntry;i++)
    {
        pEntry = &(FH_txTiming.pTxPkt[rdIndex]);

        if (pEntry->txChannel == txChan)
        {
            lastTs = pEntry->txTimeStamp;

            elapsedTime = MAP_FHUTIL_elapsedTime(curTs, lastTs) + csmaDelay * TICKPERIOD_MS_US;

            if(elapsedTime < (TICKPERIOD_MS_US * FH_LBT_MIN_TX_OFF_TIME ))
            {
                remainTime = TICKPERIOD_MS_US * FH_LBT_MIN_TX_OFF_TIME - elapsedTime ;

                /*
                 * CSMA delay is using ms as unit, need to convert tick to ms
                 * round up to 1 ms
                 */
                remainTime = (remainTime + TICKPERIOD_MS_US/2) / TICKPERIOD_MS_US;
                if (remainTime)
                {
#ifdef FH_HOP_DEBUG
                    FH_Dbg.numTxMinOffDelay++;
#endif
                }
            }
            return remainTime;
        }

        /* backward  next index */

        if (rdIndex == 0)
        {
            rdIndex = FH_txTiming.maxNumEntry -1;
        }
        else
        {
            rdIndex--;
        }
    }

    return remainTime;
}

uint32_t FHDATA_getASYNCDelay(void)
{
    uint16_t rdIndex;
    uint32_t curTs,lastTs;
    uint32_t elapsedTime, remaining=0;

    if (FH_hnd.LBT == FH_LBT_DISABLE_FLAG)
    {
        return remaining;
    }

    /* TX done time stamp to find last TX done time stamp */

    /* get current TS */
    curTs = MAP_ICall_getTicks();

    if (FH_txTiming.wrIndex == 0)
    {
        rdIndex = FH_txTiming.maxNumEntry -1;
    }
    else
    {
        rdIndex = FH_txTiming.wrIndex - 1;
    }
    lastTs = FH_txTiming.pTxPkt[rdIndex].txTimeStamp;

    elapsedTime = MAP_FHUTIL_elapsedTime(curTs, lastTs);

    if(elapsedTime < (TICKPERIOD_MS_US * FH_LBT_MIN_TX_OFF_TIME ))
    {   /* add  1 ms */
        remaining = TICKPERIOD_MS_US * FH_LBT_MIN_TX_OFF_TIME - elapsedTime + TICKPERIOD_MS_US;
    }

    return remaining;
}

void FHDATA_asyncMinTxOffTimerCb(uint8_t parameter)
{
    /* this is the ASYNC min Tx off timeout function
     * we need to start to invoke the ASYNC state machine to start to process
     *
     * This is one shot time
    */
    MAP_FHSM_event(&FH_FSM, FH_EVT_ASYNC_START, &FH_hnd);
}

#endif
