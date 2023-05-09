/******************************************************************************

 @file fh_ie_patch.c

 @brief TIMAC 2.0 FH IE PATCH API

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
#include "fh_ie.h"
#include "fh_nt.h"
#include "fh_ie_patch.h"
#include "mac_hl_patch.h"
#include "mac_radio_defs.h"
#include "mac_tx.h"
#include "mac_assert.h"
#include "dbg.h"
#include "rom_jt_154.h"

/******************************************************************************
 Constants and definitions
 *****************************************************************************/
#define FH_UT_IE_FT_EAPOL               (6)
#define HIE_LEN_SUB_ID                  (1)
#define HIE_LEN_UT_IE                   (4)

#define ONE_MS      1000
#define HALF_MS     500
/******************************************************************************
 Local variables
 *****************************************************************************/

/******************************************************************************
 Glocal variables
 *****************************************************************************/

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/
#ifdef FHIE_DBG
extern FHIE_dbg_t  FHIE_dbg;
#endif

FHAPI_status FHIEPATCH_extractHie(uint8_t subIeId, uint16_t ieLength, uint8_t *pBuf, uint8_t *pOut)
{
    (void) pBuf;
    (void) pOut;

    switch (subIeId)
    {
/*  For example, try similiar like this when to patch UT_IE
    case FHIE_HIE_SUB_ID_UT_IE:
        if (ieLength != HIE_LEN_SUB_ID + HIE_LEN_UT_IE)
        {
            pOut = NULL;
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }

        pUtIe = (FHIE_utIE_t *)pOut;
        pUtIe->frameTypeId = *pBuf++;
        if (pUtIe->frameTypeId > FH_UT_IE_FT_EAPOL)
        {
            pOut = NULL;
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        pUtIe->ufsi = 0;
        osal_memcpy(&pUtIe->ufsi, pBuf, HIE_LEN_UT_IE - 1);
        return (FHAPI_STATUS_SUCCESS);
*/
    default:
        return(FHAPI_STATUS_ERR_NO_PATCH);
    }
}

FHAPI_status FHIEPATCH_extractPie(uint8_t subIeId, uint16_t ieLength, uint8_t *pBuf, uint8_t *pOut)
{
    (void) pBuf;
    (void) pOut;

    switch (subIeId)
    {
/*  For example, try similiar like this when to patch UT_IE
    case FHIE_HIE_SUB_ID_UT_IE:
        if (ieLength != HIE_LEN_SUB_ID + HIE_LEN_UT_IE)
        {
            pOut = NULL;
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }

        pUtIe = (FHIE_utIE_t *)pOut;
        pUtIe->frameTypeId = *pBuf++;
        if (pUtIe->frameTypeId > FH_UT_IE_FT_EAPOL)
        {
            pOut = NULL;
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        pUtIe->ufsi = 0;
        osal_memcpy(&pUtIe->ufsi, pBuf, HIE_LEN_UT_IE - 1);
        return (FHAPI_STATUS_SUCCESS);
*/
    default:
        return(FHAPI_STATUS_ERR_NO_PATCH);
    }
}

uint16_t FHIEPATCH_getHieLen(uint32_t ieBitmap)
{
    (void) ieBitmap;

    return(0);
}

uint8_t FHIEPATCH_getPieId(uint32_t ieBitmap, uint8_t *pType)
{
    (void) ieBitmap;

    if(pType)
    {
        *pType = 0;
    }
    return(0);
}

uint16_t FHIEPATCH_getPieContentLen(uint8_t id)
{
    (void) id;

    return(0);
}

uint16_t FHIEPATCH_genPieContent(uint8_t *pData, uint8_t id)
{
    (void) pData;
    (void) id;

    return(0);
}

#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
uint16_t FHIEPATCH_genHie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx,
                            FHIE_ieInfo_new_t *pIeInfo)
#else
uint16_t FHIEPATCH_genHie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx,
                            FHIE_ieInfo_t *pIeInfo)
#endif
{
    (void) pData;
    (void) bitmap;
    (void) pMacTx;
    (void) pIeInfo;

    return(0);
}

FHAPI_status FHIEPATCH_parsePie(uint8_t subIeId, uint16_t subIeLength, uint8_t *pBuf, sAddrExt_t *pSrcAddr)
{
    (void) subIeId;
    (void) subIeLength;
    (void) pBuf;
    (void) pSrcAddr;

    return(FHAPI_STATUS_ERR_NO_PATCH);
}

FHAPI_status FHIEPATCH_parseHie(uint8_t subIeId, uint16_t ieLength, uint8_t *pBuf, uint32_t ts, sAddrExt_t *pSrcAddr)
{
    (void) subIeId;
    (void) ieLength;
    (void) pBuf;
    (void) ts;
    (void) pSrcAddr;

    return(FHAPI_STATUS_ERR_NO_PATCH);
}
/******************************************************************************
 Public Functions
 *****************************************************************************/
#if defined(TIMAC_ROM_PATCH)
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

    /* Handle ASYNC frames */
    pCurr = MAP_FHDATA_getPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_ASYNC);
    if(pCurr != NULL)
    {
        HAL_EXIT_CRITICAL_SECTION(intState);

        MAP_FHDATA_chkASYNCStartReq();

        return;
    }

    /* Handle BROADCAST frames */
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

#ifdef FEATURE_WISUN_EDFE_SUPPORT
        if(FH_FSM.currentState == FH_ST_EDFE)
        {
            FH_hnd.bcPktPending = 1;
#ifdef FH_HOP_DEBUG
            FH_Dbg.numTxBCinEDFE++;
#endif
        }
        else if((bfio < (FHPIB_db.macBcDwellInterval - slotEdgeTime)))
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
#else
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
#endif

    /*If not in BC slot */

#ifdef FEATURE_WISUN_EDFE_SUPPORT
    /* Handle EDFE frames */
    pCurr = MAP_FHDATA_checkPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_EDFE);
    if(pCurr != NULL)
    {
        NODE_ENTRY_s Entry;
        /*need to init nb, be */
    #ifdef FH_HOP_DEBUG
        FH_Dbg.numEdfeStart++;
    #endif
        /*
         * check to see if the destination address is in the NT
         */
        status = MAP_FHNT_getEntry(&(pCurr->internal.dest.dstAddr), &Entry);
        if(status != FHAPI_STATUS_SUCCESS)
        {
            /*
             * send TX Cnf with error status
             */
    #ifdef FH_HOP_DEBUG
            FH_Dbg.numUnicast_entryNotInNT_1++;
    #endif
            MAP_FHDATA_getPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_EDFE);
            HAL_EXIT_CRITICAL_SECTION(intState);
            MAP_macTxCompleteCallback(status);
            return ;
        }
        HAL_EXIT_CRITICAL_SECTION(intState);
        if(bfio >= FHPIB_db.macBcDwellInterval)
        {
            FH_hnd.edfePending = 0;
            MAP_FHSM_event(&FH_FSM, FH_EVT_EDFE_REQ, pCurr);
        }
        else
        {
            FH_hnd.edfePending = 1;
        }
        return;
    }
#endif

    /* Handle UNICAST frames */
    pCurr = MAP_FHDATA_getPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_UNICAST);
    if(pCurr != NULL)
    {
        NODE_ENTRY_s Entry;
        /*need to init nb, be */
#ifdef FH_HOP_DEBUG
        FH_Dbg.numUnicastStart++;
#endif

#ifdef FEATURE_WISUN_EDFE_SUPPORT
        /* DFE packet comes in while EDFE in progress */
        if(FH_FSM.currentState == FH_ST_EDFE)
        {
#ifdef FH_HOP_DEBUG
            FH_Dbg.numTxUCinEDFE++;
#endif
            FH_hnd.pktPending = 1;
            MAP_FHDATA_requeuePkt(pMacDataTx);
            HAL_EXIT_CRITICAL_SECTION(intState);
            return;
        }
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
            HAL_EXIT_CRITICAL_SECTION(intState);
            MAP_macTxCompleteCallback(status);
            return ;
        }
        HAL_EXIT_CRITICAL_SECTION(intState);
        //in broadcast dwell time
        if(bfio >= FHPIB_db.macBcDwellInterval || !FH_hnd.bsStarted)
        {
            FH_hnd.pktPending = 0;
            MAP_macTxFrame(MAC_TX_TYPE_FH_CSMA);
        }
        else
        {
            txComplete(MAC_NO_TIME);
        }
        return;
    }
    HAL_EXIT_CRITICAL_SECTION(intState);
}

#ifndef FEATURE_WISUN_EDFE_SUPPORT
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
#else
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
    macTx_t *pTxData = (macTx_t *)pData;
    FHAPI_status status;
    NODE_ENTRY_s entry;
    uint32_t txUfsi;

    if(txRetransmitFlag == 0)
    {
        MAP_osal_memset(&macEdfeInfo, 0, sizeof(macEdfeInfo_t));
    }
    macEdfeInfo.txFrameType = 0;
    if(pMacDataTx && (pTxData == pMacDataTx))
    {
        MAP_osal_msg_push(&macData.txQueue, pTxData);
        pTxData = NULL;
        pMacDataTx = NULL;
    }

    pTxData = MAP_FHDATA_checkPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_EDFE);

    if(pTxData)
    {
        MAP_osal_memcpy(&macEdfeInfo.dstAddr, &pTxData->internal.dest.dstAddr, sizeof(sAddr_t));
        macEdfeInfo.packets++;
        macEdfeInfo.txfc = ((pTxData->msdu.len + MAC_FCS4_FIELD_LEN) * 8 * macSpecUsecsPerSymbol + HALF_MS) / ONE_MS;
        macEdfeInfo.rxfc = MAC_EDFE_MAX_RXFC;
        MAC_ASSERT(macEdfeInfo.dstAddr.addrMode == SADDR_MODE_EXT);

        macEdfeInfo.initiator = TRUE;
        /* channel selection */
        if(txRetransmitFlag == 0)
        {
            macEdfeInfo.txStartTime = 0; // usec
        }
        status = MAP_FHNT_getEntry(&pTxData->internal.dest.dstAddr, &entry);
        status |= MAP_FHUTIL_adjBackOffDur(&entry, &macEdfeInfo.txStartTime, &txUfsi);
        if(status == FHAPI_STATUS_SUCCESS)
        {
            txChannel = MAP_FHUTIL_getTxChannel(&entry, txUfsi, macEdfeInfo.txStartTime);
            FH_hnd.lastTxChannel = txChannel;
        }
        else
        {
            macEdfeInfo.initiator = FALSE;
            macEdfeInfo.dstAddr.addrMode = SADDR_MODE_NONE;
            MAP_FHSM_event(&FH_FSM, FH_EVT_EDFE_FIN, &FH_hnd);
            FH_hnd.edfePending = 1;
            return;
        }
        MAP_macRadioSendEDFEEfrm(&pTxData->internal.dest.dstAddr, &pTxData->sec, MAC_FRAME_TYPE_EDFE_IFRM);
    }
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
extern MAP_ICall_TimerID macCsmaRtcTimerId;
void FHDATA_procEDFEReq(void *pData)
{
    macRx_t *pRxData = (macRx_t *)pData;
    macTx_t *pTxPkt;
    macTx_t *pAsyncPkt;
    uint8 *pHdrIes = pRxData->pHdrIes;
    uint8 txfc = *(pHdrIes + MAC_FC_IE_TXFC_OFFSET);
    uint8 rxfc = *(pHdrIes + MAC_FC_IE_RXFC_OFFSET);
    txRetransmitFlag = 0;

    if(pRxData->mac.srcAddr.addrMode == SADDR_MODE_EXT)
    {
        MAP_osal_memcpy(&macEdfeInfo.dstAddr, &pRxData->mac.srcAddr, sizeof(sAddr_t));
    }

    MAC_ASSERT(macEdfeInfo.dstAddr.addrMode == SADDR_MODE_EXT);

    if(pMacDataTx)
    {
      /* EDFE received when about to transmit DFE packet */
      if(pMacDataTx->internal.frameType == MAC_FRAME_TYPE_UNICAST)
      {
#ifdef FH_HOP_DEBUG
        FH_Dbg.numRxEDFEinDFE++;
#endif
        MAP_FHDATA_requeuePkt(pMacDataTx);
        FH_hnd.pktPending = 1;
        MAP_ICall_stopTimer(&macCsmaRtcTimerId);
      }
    }

    /* Indicates to application that EDFE has been received */
    pRxData->mac.fhProtoDispatch = 1;

#if 0 // handling case when async comes in during EDFE transaction
    pAsyncPkt = MAP_FHDATA_checkPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_ASYNC);
#else
    pAsyncPkt = NULL;
#endif
    /* The peer wants to finish transaction */
    if(rxfc == 0)
    {
        /* Finish the transaction */
        macEdfeInfo.initiator = FALSE;
        macEdfeInfo.txFrameType = 0;
        macEdfeDbg.rxFFRM++;
        macEdfeInfo.dstAddr.addrMode = SADDR_MODE_NONE;
        MAP_FHSM_event(&FH_FSM, FH_EVT_EDFE_FIN, &FH_hnd);
        if(!MAP_OSAL_MSG_Q_EMPTY(&macData.txQueue))
        {
            /* If new data request is in queue, handle it immediately */
            MAP_FHAPI_sendData();
        }
    }
    /* The peer says that will not send anymore data, but can receive my data */
    else if(txfc == 0)
    {
        if (pRxData->msdu.len)
        {
            macEdfeDbg.rxDFRM++;
        }
        else
        {
            macEdfeDbg.rxRFRM++;
        }
        /* If new EDFE request is in queue, send it */
        pTxPkt = MAP_FHDATA_checkPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_EDFE);
        if(pTxPkt)
        {
          macEdfeInfo.packets++;
            FH_hnd.edfePending = 0;
            /* get the data from the queue */
            MAP_FHDATA_getPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_EDFE);
            /* look if more data in the queue */
            pTxPkt = MAP_FHDATA_checkPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_EDFE);
            if(pTxPkt && !pAsyncPkt)
            {
                macEdfeInfo.ieInfo.txfc = ((pTxPkt->msdu.len + MAC_FCS4_FIELD_LEN) * 8 * macSpecUsecsPerSymbol + 500) / 1000;
                macEdfeInfo.ieInfo.rxfc = MAC_EDFE_MAX_RXFC;
            }
            else
            {
                macEdfeInfo.ieInfo.txfc = 0;
                macEdfeInfo.ieInfo.rxfc = MAC_EDFE_MAX_RXFC;
            }
            MAP_macTxFrame(MAC_TX_TYPE_NO_CSMA);;
        }
        /* Otherwise, send the FRFRM */
        else
        {
            MAP_macRadioSendEDFEEfrm(&macEdfeInfo.dstAddr, &pRxData->sec, MAC_FRAME_TYPE_EDFE_FFRM);
            macEdfeInfo.initiator = FALSE;
            macEdfeInfo.txFrameType = 0;
            macEdfeInfo.dstAddr.addrMode = SADDR_MODE_NONE;
        }
    }
    /* The peer says that will keep sending data and can receive my data */
    else
    {
        if (pRxData->msdu.len)
        {
            macEdfeDbg.rxDFRM++;
        }
        else
        {
            macEdfeDbg.rxIFRM++;
        }
#if 0   // This case will be considered later */
        /* If new EDFE request is in queue, send it */
        pTxPkt = MAP_FHDATA_checkPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_EDFE);
        if(pTxPkt)
        {
            MAP_macTxFrame(MAC_TX_TYPE_NO_CSMA);;
        }
        /* Otherwise, send the FRFRM */
        else
#endif
        {
            if(macEdfeInfo.asyncPending)
            {
                macEdfeInfo.asyncPending = FALSE;
                MAP_macRadioSendEDFEEfrm(&macEdfeInfo.dstAddr, &pRxData->sec, MAC_FRAME_TYPE_EDFE_FFRM);
                macEdfeInfo.initiator = FALSE;
                macEdfeInfo.txFrameType = 0;
                macEdfeInfo.dstAddr.addrMode = SADDR_MODE_NONE;
            }
            else
            {
                MAP_macRadioSendEDFEEfrm(&macEdfeInfo.dstAddr, &pRxData->sec, MAC_FRAME_TYPE_EDFE_RFRM);
                if(pAsyncPkt)
                {
                  macEdfeInfo.asyncPending = TRUE;
                }
            }
        }
    }

    return;
}

void FHDATA_compEDFEReq(uint8_t status)
{
    uint8 retries = 0;
    uint8 be;
    macTx_t *pCurr = NULL;

    switch(status)
    {
    case FHAPI_STATUS_SUCCESS:
      MAP_FHSM_event(&FH_FSM, FH_EVT_EDFE_FIN, &FH_hnd);
      if(!MAP_OSAL_MSG_Q_EMPTY(&macData.txQueue))
      {
          /* If new data request is in queue, handle it immediately */
          MAP_FHAPI_sendData();
      }
      break;
    case FHAPI_STATUS_ERR_EDFE_DFE_RCV:
        macEdfeDbg.txDFERcv++;
    case FHAPI_STATUS_ERR_EDFE_NO_RFRM:
        if(pMacDataTx)
        {
            MAP_FHDATA_requeuePkt(pMacDataTx);
        }
    case FHAPI_STATUS_ERR_EDFE_CCA_FAIL:
        macEdfeInfo.txFrameType = 0;
        macEdfeInfo.dstAddr.addrMode = SADDR_MODE_NONE;
        MAP_FHSM_event(&FH_FSM, FH_EVT_EDFE_FIN, &FH_hnd);
        if(macEdfeInfo.initiator)
        {
            pCurr = MAP_FHDATA_getPktFromQueue(&macData.txQueue, MAC_FRAME_TYPE_EDFE);
            if(pCurr)
            {
                retries = pMacDataTx->internal.retries--;
                if(retries > 0)
                {
                    be = pMacDataTx->internal.be;
                    pMacDataTx->internal.be = pMacDataTx->internal.be + 1 < pMacPib->maxBe ? pMacDataTx->internal.be + 1 : pMacPib->maxBe;
                    macEdfeInfo.txStartTime = macRadioRandomByte() & ((1 << be) - 1);
                    macEdfeInfo.txStartTime *= (MAC_A_UNIT_BACKOFF_CCA_PERIOD * macSpecUsecsPerSymbol);
                    MAP_FHDATA_requeuePkt(pMacDataTx);
                    txRetransmitFlag = 1;
                    MAP_FHSM_event(&FH_FSM, FH_EVT_EDFE_REQ, NULL);
                }
                else
                {
                    txRetransmitFlag = 0;
                    pMacDataTx->hdr.status = status;
                    MAP_macTxCompleteCallback(status);
                    if(!MAP_OSAL_MSG_Q_EMPTY(&macData.txQueue))
                    {
                        /* If new data request is in queue, handle it immediately */
                        MAP_FHAPI_sendData();
                    }
                }
            }
        }
        break;
    default:
        break;
    }

    return;
}
#endif

#ifndef FEATURE_WISUN_EDFE_SUPPORT
void FHIE_genUTIE(uint8_t *pBuf, uint8_t fhFrameType)
#else
/* UFSI = floor((t1-t0)/(2^16 * UDI) * 2^24)*/
void FHIE_genUTIE(uint8_t *pBuf, uint8_t fhFrameType, uint32_t offset)
#endif
{
    uint8_t  ucDwellTime;
    uint32_t temp32;

    MAP_FHPIB_get(FHPIB_UC_DWELL_INTERVAL, &ucDwellTime);

    MAP_FHUTIL_getCurUfsi(&temp32 );

    /* compensate for SFD time and CCA
     * round up to ms
     */
    temp32 += MAP_FHIE_getCcaSfdTime(fhFrameType);
#ifdef FEATURE_WISUN_EDFE_SUPPORT
    /* add tx offset */
    temp32 += offset;
    temp32 %= (ucDwellTime << 16);
#endif
    /*multiply by 256, divide by 250 */
    temp32 <<= 8;
    //24 bit value
    //utie = ufsi/dwell_time
    temp32 = (temp32 + (ucDwellTime >> 1))/ucDwellTime;
    *pBuf++ = fhFrameType;
    *pBuf++ = ((temp32 >> 0));
    *pBuf++ = ((temp32 >> 8));
    *pBuf++ = ((temp32 >>16));
}

#ifndef FEATURE_WISUN_EDFE_SUPPORT
void FHIE_genBTIE(uint8_t *pBuf, uint8_t frameType)
#else
/* BFIO = floor((t1-t0)/BI*2^32)*/
void FHIE_genBTIE(uint8_t *pBuf, uint8_t frameType, uint32_t offset)
#endif
{
    uint32_t temp32, bcInterval;
    uint16_t slotIdx;
#ifndef FEATURE_WISUN_EDFE_SUPPORT
    uint64_t temp64;
#endif

    MAP_FHPIB_get(FHPIB_BC_INTERVAL, &bcInterval);

    MAP_FHUTIL_getCurBfio(&temp32, &slotIdx);

    /* compensate for SFD time and CCA
     * round up to ms
     */
    temp32 += MAP_FHIE_getCcaSfdTime(frameType);

    if(temp32 >= bcInterval)
    {
        temp32  = temp32 - bcInterval;
        slotIdx += 1;
    }

#ifdef FEATURE_WISUN_EDFE_SUPPORT
    temp32 += offset;
    if(temp32 >= bcInterval)
    {
        temp32  = temp32 - bcInterval;
        slotIdx += 1;
    }
#endif

    DBG_PRINTL2(DBGSYS, "FHTraceLog: Generated BT IE values: bfio=(0x%X) slotNo=(0x%X)", temp32,slotIdx);

#ifdef FEATURE_WISUN_SUPPORT
    /* Spec changed BT-IE format to 2 byte slotIdx and 3 byte bfio.
       no more fractional bfio offset */
    *pBuf++ = ((slotIdx >> 0));
    *pBuf++ = ((slotIdx >> 8));

    *pBuf++ = ((temp32 >> 0));
    *pBuf++ = ((temp32 >> 8));
    *pBuf++ = ((temp32 >> 16));
#else
    /*32-bit value */
    /* BTIE = bfio/broadcast interval */
    //long division, lot of cycles.
    temp64 = temp32 << 16;
    temp64 = temp64 << 16;
    //temp32 = (temp64 + (bcInterval >> 1))/bcInterval;
    temp32 = MAP_macMcuLongDiv(temp64, bcInterval);
    *pBuf++ = ((slotIdx >> 0));
    *pBuf++ = ((slotIdx >> 8));

    *pBuf++ = ((temp32 >> 0));
    *pBuf++ = ((temp32 >> 8));
    *pBuf++ = ((temp32 >> 16));
    *pBuf++ = ((temp32 >> 24));
#endif
}

uint16_t FHIE_genPieContent(uint8_t *pData, uint8_t id)
{
    uint16_t ieContentLength;
    uint16_t ieCommonLength;
    uint32_t broadcastInterval;
    uint16_t broadcastSchedId;
    uint8_t dwellInterval;
    uint8_t clockDrift;
    uint8_t timingAccuracy;
    uint8_t channelPlan;
    uint8_t channelFunction;
    uint8_t excludedChannelControl;
    FHIE_channelPlan_t chPlan;
#ifndef FEATURE_WISUN_SUPPORT
    FHIE_panIE_t panIe;
#else
    FHIE_panIE_new_t panIe;
#endif
    FHIE_netNameIE_t netNameIe;
    FHIE_panVerIE_t panVerIe;
    FHIE_gtkHashIE_t gtkHashIe;

    ieContentLength = 0;
#ifdef FHIEPATCH
    ieContentLength = MAP_FHIEPATCH_genPieContent(pData, id);
    if(ieContentLength)
    {
        return(ieContentLength);
    }
#endif
    switch(id)
    {
    case FHIE_PIE_SUB_IE_SUB_ID_US_IE:
        MAP_FHPIB_get(FHPIB_CLOCK_DRIFT, &clockDrift);
        MAP_FHPIB_get(FHPIB_TIMING_ACCURACY, &timingAccuracy);
        MAP_FHPIB_get(FHPIB_UC_DWELL_INTERVAL, &dwellInterval);
        channelPlan = MAP_FHIE_getChannelPlan(&chPlan);
        MAP_FHPIB_get(FHPIB_UC_CHANNEL_FUNCTION, &channelFunction);
        if(channelFunction)
        {
            excludedChannelControl = MAP_FHIE_getExcludedChannelControl(id);
        }
        else
        {
            /* ECC field MUST be set to 0 when CF field is set to zero*/
            excludedChannelControl = FHIE_ECC_NO_EC;
        }
        *pData++ = dwellInterval;
        *pData++ = clockDrift;
        *pData++ = timingAccuracy;
        *pData++ = MAKE_CP(channelPlan, channelFunction,
                           excludedChannelControl);
        ieContentLength += FH_US_IE_FIXED_PART_LEN;
        ieCommonLength = MAP_FHIE_genCommonInformation(pData, channelPlan,
                                                   channelFunction,
                                                   excludedChannelControl,
                                                   &chPlan, FH_UC);
        ieContentLength += ieCommonLength;
        pData += ieCommonLength;
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_BS_IE:
        MAP_FHPIB_get(FHPIB_BC_INTERVAL, &broadcastInterval);
        MAP_FHPIB_get(FHPIB_BROCAST_SCHED_ID, &broadcastSchedId);
        MAP_FHPIB_get(FHPIB_CLOCK_DRIFT, &clockDrift);
        MAP_FHPIB_get(FHPIB_TIMING_ACCURACY, &timingAccuracy);
        MAP_FHPIB_get(FHPIB_BC_DWELL_INTERVAL, &dwellInterval);
        channelPlan = MAP_FHIE_getChannelPlan(&chPlan);
        MAP_FHPIB_get(FHPIB_BC_CHANNEL_FUNCTION, &channelFunction);
        if(channelFunction)
        {
            excludedChannelControl = MAP_FHIE_getExcludedChannelControl(id);
        }
        else
        {
            /* ECC field MUST be set to 0 when CF field is set to zero*/
            excludedChannelControl = FHIE_ECC_NO_EC;
        }
        MAP_osal_memcpy(pData, &broadcastInterval, sizeof(broadcastInterval));
        pData += sizeof(broadcastInterval);
        MAP_osal_memcpy(pData, &broadcastSchedId, sizeof(broadcastSchedId));
        pData += sizeof(broadcastSchedId);
        *pData++ = dwellInterval;
        *pData++ = clockDrift;
        *pData++ = timingAccuracy;
        *pData++ = MAKE_CP(channelPlan, channelFunction,
                           excludedChannelControl);
        ieContentLength += FH_BS_IE_FIXED_PART_LEN;
        ieCommonLength = MAP_FHIE_genCommonInformation(pData, channelPlan,
                                                   channelFunction,
                                                   excludedChannelControl,
                                                   &chPlan, FH_BC);
        ieContentLength += ieCommonLength;
        pData += ieCommonLength;
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_PAN_IE:
        MAP_FHPIB_get(FHPIB_PAN_SIZE, &panIe.panSize);
#ifndef FEATURE_WISUN_SUPPORT
        MAP_FHPIB_get(FHPIB_ROUTING_COST, &panIe.routingCost);
#else
        panIe.routingCost = FHPIB_db_new.macRoutingCost;
#endif
        MAP_FHPIB_get(FHPIB_USE_PARENT_BS_IE, &panIe.useParentBSIE);
        MAP_FHPIB_get(FHPIB_ROUTING_METHOD, &panIe.routingMethod);
#ifndef FEATURE_WISUN_SUPPORT
        MAP_FHPIB_get(FHPIB_EAPOL_READY, &panIe.eapolReady);
#endif
        MAP_FHPIB_get(FHPIB_FAN_TPS_VERSION, &panIe.fanTpsVersion);
        *pData++ = GET_BYTE(panIe.panSize, 0);
        *pData++ = GET_BYTE(panIe.panSize, 1);
#ifndef FEATURE_WISUN_SUPPORT
        *pData++ = panIe.routingCost;
        *pData++ = MAKE_PANCTL(panIe.useParentBSIE, panIe.routingMethod,
                               panIe.eapolReady, panIe.fanTpsVersion);
#else
        *pData++ = GET_BYTE(panIe.routingCost, 0);
        *pData++ = GET_BYTE(panIe.routingCost, 1);
        *pData++ = MAKE_PANCTL(panIe.useParentBSIE, panIe.routingMethod,
                               panIe.fanTpsVersion);
#endif
        ieContentLength += FH_PAN_IE_LEN;
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_NETNAME_IE:
        MAP_osal_memset(netNameIe.netName, 0, FH_NETNAME_IE_LEN_MAX);
        MAP_FHPIB_get(FHPIB_NET_NAME, &netNameIe.netName);
        MAP_osal_memcpy(pData, &netNameIe.netName, MAP_osal_strlen((char *)netNameIe.netName));
        ieContentLength += MAP_osal_strlen((char *)netNameIe.netName);
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_PANVER_IE:
        MAP_FHPIB_get(FHPIB_PAN_VERSION, &panVerIe.panVersion);
        *pData++ = GET_BYTE(panVerIe.panVersion, 0);
        *pData++ = GET_BYTE(panVerIe.panVersion, 1);
        ieContentLength += FH_PANVER_IE_LEN;
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_GTKHASH_IE:
        MAP_FHPIB_get(FHPIB_GTK_0_HASH, &gtkHashIe.gtk0Hash);
        MAP_FHPIB_get(FHPIB_GTK_1_HASH, &gtkHashIe.gtk1Hash);
        MAP_FHPIB_get(FHPIB_GTK_2_HASH, &gtkHashIe.gtk2Hash);
        MAP_FHPIB_get(FHPIB_GTK_3_HASH, &gtkHashIe.gtk3Hash);
        MAP_osal_memcpy(pData, gtkHashIe.gtk0Hash, FH_GTK0HASH_LEN);
        pData += FH_GTK0HASH_LEN;
        MAP_osal_memcpy(pData, gtkHashIe.gtk1Hash, FH_GTK0HASH_LEN);
        pData += FH_GTK0HASH_LEN;
        MAP_osal_memcpy(pData, gtkHashIe.gtk2Hash, FH_GTK0HASH_LEN);
        pData += FH_GTK0HASH_LEN;
        MAP_osal_memcpy(pData, gtkHashIe.gtk3Hash, FH_GTK0HASH_LEN);
        pData += FH_GTK0HASH_LEN;
        ieContentLength += FH_GTKHASH_IE_LEN;
        break;

    default:
        return(0);
    }

    return(ieContentLength);
}

uint16_t FHIE_getPieContentLen(uint8_t id)
{
    uint16_t ieContentLength;
    uint16_t ieCommonLength;
    uint8_t channelPlan;
    uint8_t channelFunction;
    uint8_t excludedChannelControl;
    FHIE_channelPlan_t chPlan;
    FHIE_netNameIE_t netName;

    ieContentLength = 0;
#ifdef FHIEPATCH
    ieContentLength = MAP_FHIEPATCH_getPieContentLen(id);
    if(ieContentLength)
    {
        return(ieContentLength);
    }
#endif
    switch(id)
    {
    case FHIE_PIE_SUB_IE_SUB_ID_US_IE:
        channelPlan = MAP_FHIE_getChannelPlan(&chPlan);
        MAP_FHPIB_get(FHPIB_UC_CHANNEL_FUNCTION, &channelFunction);
        if(channelFunction)
        {
            excludedChannelControl = MAP_FHIE_getExcludedChannelControl(id);
        }
        else
        {
            /* ECC field MUST be set to 0 when CF field is set to zero*/
            excludedChannelControl = FHIE_ECC_NO_EC;
        }
        ieContentLength += FH_US_IE_FIXED_PART_LEN;
        ieCommonLength = MAP_FHIE_getCommonInformationLen(channelPlan,
                                                      channelFunction,
                                                      excludedChannelControl,
                                                      &chPlan, FH_UC);
        ieContentLength += ieCommonLength;
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_BS_IE:
        channelPlan = MAP_FHIE_getChannelPlan(&chPlan);
        MAP_FHPIB_get(FHPIB_BC_CHANNEL_FUNCTION, &channelFunction);
        if(channelFunction)
        {
            excludedChannelControl = MAP_FHIE_getExcludedChannelControl(id);
        }
        else
        {
            /* ECC field MUST be set to 0 when CF field is set to zero*/
            excludedChannelControl = FHIE_ECC_NO_EC;
        }
        ieContentLength += FH_BS_IE_FIXED_PART_LEN;
        ieCommonLength = MAP_FHIE_getCommonInformationLen(channelPlan,
                                                      channelFunction,
                                                      excludedChannelControl,
                                                      &chPlan, FH_BC);
        ieContentLength += ieCommonLength;
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_PAN_IE:
        ieContentLength += FH_PAN_IE_LEN;
        break;
    case FHIE_PIE_SUB_IE_SUB_ID_NETNAME_IE:
        MAP_FHPIB_get(FHPIB_NET_NAME, netName.netName);
        ieContentLength += MAP_osal_strlen((char *)netName.netName);
        break;
    case FHIE_PIE_SUB_IE_SUB_ID_PANVER_IE:
        ieContentLength += FH_PANVER_IE_LEN;
        break;
    case FHIE_PIE_SUB_IE_SUB_ID_GTKHASH_IE:
        ieContentLength += FH_GTKHASH_IE_LEN;
        break;

    default:
        return(0);
    }

    return(ieContentLength);
}

uint16_t FHIE_getHieLen(uint32_t bitmap)
{
    uint16_t ieLength;
    uint16_t subIeLength;
    uint32_t ieBitmap;
    uint32_t bitmapMask;
    uint8_t  macBcDwellTime;

    ieLength = 0;

    bitmapMask = FH_WISUN_HIE_BITMAP_START;
    do
    {
#ifdef FHIE_DBG
        FHIE_dbg.hie_getlen_try++;
#endif
        ieBitmap = bitmap & bitmapMask;
        bitmap &= ~bitmapMask;
        bitmapMask <<= 1;

        subIeLength = 0;
#ifdef FHIEPATCH
        subIeLength = MAP_FHIEPATCH_getHieLen(ieBitmap);
        if(subIeLength)
        {
            ieLength += subIeLength;
            continue;
        }
#endif
        switch(ieBitmap)
        {
        case FH_WISUN_HIE_FC_IE_BITMAP:
            subIeLength = HIE_LEN_FC_IE;
            ieLength += (IE_DESCRIPTOR_LEN + HIE_LEN_SUB_ID + subIeLength);
            break;

        case FH_WISUN_HIE_UT_IE_BITMAP:
            subIeLength = HIE_LEN_UT_IE;
            ieLength += (IE_DESCRIPTOR_LEN + HIE_LEN_SUB_ID + subIeLength);
            break;

        case FH_WISUN_HIE_RSL_IE_BITMAP:
            subIeLength = HIE_LEN_RSL_IE;
            ieLength += (IE_DESCRIPTOR_LEN + HIE_LEN_SUB_ID + subIeLength);
            break;

        case FH_WISUN_HIE_BT_IE_BITMAP:
            MAP_FHPIB_get(FHPIB_BC_DWELL_INTERVAL, &macBcDwellTime);
            if(macBcDwellTime)
            {
              subIeLength = HIE_LEN_BT_IE;
              ieLength += (IE_DESCRIPTOR_LEN + HIE_LEN_SUB_ID + subIeLength);
            }
            break;
#ifdef FEATURE_WISUN_SUPPORT
        case FH_WISUN_HIE_EA_IE_BITMAP:
            subIeLength = HIE_LEN_EA_IE;
            ieLength += (IE_DESCRIPTOR_LEN + HIE_LEN_SUB_ID + subIeLength);
            break;
#endif
        default:
        if(ieBitmap)
        {
#ifdef FHIE_DBG
            FHIE_dbg.hie_getlen_fail++;
#endif
        }
            break;
        }
    } while(bitmap);

    return(ieLength);
}

#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
uint16_t FHIE_genHie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx,
                            FHIE_ieInfo_new_t *pIeInfo)
#else
uint16_t FHIE_genHie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx,
                            FHIE_ieInfo_t *pIeInfo)
#endif
{
    uint8_t *pBuf;
    uint16_t ieDescriptor;
    uint16_t ieLength;
    uint16_t subIeLength;
    uint32_t ieBitmap;
    uint32_t bitmapMask;
    uint8_t  macBcDwellTime, frameType = 0;
    sAddrExt_t    extAddr;
#ifdef FEATURE_WISUN_EDFE_SUPPORT
    uint32_t txOffset = 0;
#endif

    pBuf = pData;
    ieLength = 0;

    bitmapMask = FH_WISUN_HIE_BITMAP_START;
    do
    {
#ifdef FHIE_DBG
        FHIE_dbg.hie_gen_try++;
#endif
        ieBitmap = bitmap & bitmapMask;
        bitmap &= ~bitmapMask;
        bitmapMask <<= 1;

        subIeLength = 0;
#ifdef FHIEPATCH
        subIeLength = MAP_FHIEPATCH_genHie(pBuf, ieBitmap, pMacTx, pIeInfo);
        if(subIeLength)
        {
            ieLength += subIeLength; /* Descriptor + content */
            pBuf += subIeLength;
            continue;
        }
#endif
        switch(ieBitmap)
        {
        case FH_WISUN_HIE_FC_IE_BITMAP:
            subIeLength = HIE_LEN_SUB_ID + HIE_LEN_FC_IE;
            ieDescriptor = MAKE_HIE(HIE_ELEM_ID_WISUN, subIeLength);
            *pBuf++ = GET_BYTE(ieDescriptor, 0);
            *pBuf++ = GET_BYTE(ieDescriptor, 1);
            *pBuf++ = FHIE_HIE_SUB_ID_FC_IE;

            if(pIeInfo)
            {
#ifdef FEATURE_WISUN_EDFE_SUPPORT
                pIeInfo->ptrFCIE = pBuf;
#endif
                *pBuf++ = pIeInfo->fcIe.txFlowControl;
                *pBuf++ = pIeInfo->fcIe.rxFlowControl;
            }
            else    /*if data not given, set to 0 */
            {
                *pBuf++ = 0;
                *pBuf++ = 0;
            }
            ieLength += (IE_DESCRIPTOR_LEN + subIeLength);
            break;

        case FH_WISUN_HIE_UT_IE_BITMAP:
            /* pass UTIE content pointer */
            if(pMacTx)
            {
                pMacTx->internal.ptrUTIE = pBuf;
                frameType = pMacTx->internal.fhFrameType;
            }

#ifdef FEATURE_WISUN_EDFE_SUPPORT
            txOffset = 0;
            if(pIeInfo)
            {
                txOffset = pIeInfo->txOffset;
            }
#endif

            subIeLength = HIE_LEN_SUB_ID + HIE_LEN_UT_IE;
            ieDescriptor = MAKE_HIE(HIE_ELEM_ID_WISUN, subIeLength);
            *pBuf++ = GET_BYTE(ieDescriptor, 0);
            *pBuf++ = GET_BYTE(ieDescriptor, 1);
            *pBuf++ = FHIE_HIE_SUB_ID_UT_IE;
            /* pass UTIE content pointer */
#ifndef FEATURE_WISUN_EDFE_SUPPORT
            MAP_FHIE_genUTIE(pBuf, frameType);
#else
            MAP_FHIE_genUTIE(pBuf, frameType, txOffset);
#endif
            pBuf += HIE_LEN_UT_IE;
            ieLength += (IE_DESCRIPTOR_LEN + subIeLength);
            break;

        case FH_WISUN_HIE_RSL_IE_BITMAP:
            subIeLength = HIE_LEN_SUB_ID + HIE_LEN_RSL_IE;
            ieDescriptor = MAKE_HIE(HIE_ELEM_ID_WISUN, subIeLength);
            *pBuf++ = GET_BYTE(ieDescriptor, 0);
            *pBuf++ = GET_BYTE(ieDescriptor, 1);
            *pBuf++ = FHIE_HIE_SUB_ID_RSL_IE;

            if(pIeInfo)
            {
#ifdef FEATURE_WISUN_EDFE_SUPPORT
                pIeInfo->ptrRSLIE = pBuf;
#endif
                *pBuf++ = pIeInfo->rslIe.rsl;
            }
            else    /*if data not given, set to 0 */
            {
                *pBuf++ = 0;
            }
            ieLength += (IE_DESCRIPTOR_LEN + subIeLength);
            break;

        case FH_WISUN_HIE_BT_IE_BITMAP:
            MAP_FHPIB_get(FHPIB_BC_DWELL_INTERVAL, &macBcDwellTime);
            if(macBcDwellTime)
            {
            /* pass BTIE content pointer */
            if(pMacTx)
            {
                pMacTx->internal.ptrBTIE = pBuf;
                frameType = pMacTx->internal.fhFrameType;
            }

#ifdef FEATURE_WISUN_EDFE_SUPPORT
            txOffset = 0;
            if(pIeInfo)
            {
              txOffset = pIeInfo->txOffset;
            }
#endif
            subIeLength = HIE_LEN_SUB_ID + HIE_LEN_BT_IE;
            ieDescriptor = MAKE_HIE(HIE_ELEM_ID_WISUN, subIeLength);
            *pBuf++ = GET_BYTE(ieDescriptor, 0);
            *pBuf++ = GET_BYTE(ieDescriptor, 1);
            *pBuf++ = FHIE_HIE_SUB_ID_BT_IE;
#ifndef FEATURE_WISUN_EDFE_SUPPORT
            MAP_FHIE_genBTIE(pBuf, frameType);
#else
            MAP_FHIE_genBTIE(pBuf, frameType, txOffset);
#endif
            pBuf += HIE_LEN_BT_IE;
            ieLength += (IE_DESCRIPTOR_LEN + subIeLength);
          }
            break;
#ifdef FEATURE_WISUN_SUPPORT
#ifdef FEATURE_MAC_SECURITY
        case FH_WISUN_HIE_EA_IE_BITMAP:
            subIeLength = HIE_LEN_SUB_ID + HIE_LEN_EA_IE;
            ieDescriptor = MAKE_HIE(HIE_ELEM_ID_WISUN, subIeLength);
            *pBuf++ = GET_BYTE(ieDescriptor, 0);
            *pBuf++ = GET_BYTE(ieDescriptor, 1);
            *pBuf++ = FHIE_HIE_SUB_ID_EA_IE;
            MAC_MlmeGetSecurityReq(MAC_PAN_COORD_EXTENDED_ADDRESS, &extAddr);
            MAP_osal_memcpy(pBuf, &extAddr, HIE_LEN_EA_IE);
            pBuf += HIE_LEN_EA_IE;
            ieLength += (IE_DESCRIPTOR_LEN + subIeLength);
            break;
#endif
#endif
        default:
        if(ieBitmap)
        {
#ifdef FHIE_DBG
            FHIE_dbg.hie_gen_fail++;
#endif
        }
            break;
        }
    } while(bitmap);

    return(ieLength);
}

#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
uint16_t FHIE_genPie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx,
                            FHIE_ieInfo_new_t *pIeInfo)
#else
uint16_t FHIE_genPie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx,
                            FHIE_ieInfo_t *pIeInfo)
#endif
{
    uint8_t *pBuf;
    uint16_t ieDescriptor;
    uint16_t subIeDescriptor;
    uint16_t ieLength;
    uint16_t subIeLength;
    uint32_t ieBitmap;
    uint32_t bitmapMask;
    uint8_t ieId;
    uint8_t typeLong;

#ifdef FHIE_DBG
    FHIE_dbg.pie_gen_try++;
#endif

    pBuf = pData;
    ieLength = 0;
    typeLong = 1;

    pBuf += IE_DESCRIPTOR_LEN;
    ieLength += IE_DESCRIPTOR_LEN;

    bitmapMask = FH_WISUN_PIE_BITMAP_START;
    do
    {
        ieBitmap = bitmap & bitmapMask;
        bitmap &= ~bitmapMask;
        bitmapMask <<= 1;

        subIeLength = 0;
        ieId = 0;
        switch(ieBitmap)
        {
        case FH_WISUN_PIE_US_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_US_IE;
            typeLong = 1;
            break;

        case FH_WISUN_PIE_BS_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_BS_IE;
            typeLong = 1;
            break;

        case FH_WISUN_PIE_PAN_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_PAN_IE;
            typeLong = 0;
            break;

        case FH_WISUN_PIE_NETNAME_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_NETNAME_IE;
            typeLong = 0;
            break;

        case FH_WISUN_PIE_PANVER_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_PANVER_IE;
            typeLong = 0;
            break;

        case FH_WISUN_PIE_GTKHASH_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_GTKHASH_IE;
            typeLong = 0;
            break;

        default:
        if(ieBitmap)
        {
#ifdef FHIEPATCH
            ieId = MAP_FHIEPATCH_getPieId(ieBitmap, &typeLong);
#endif
#ifdef FHIE_DBG
            FHIE_dbg.pie_gen_not_supported++;
#endif
        }
            break;
        }
        if(ieId)
        {
            subIeLength = MAP_FHIE_genPieContent(pBuf+SUB_IE_DESCRIPTOR_LEN, ieId);
            if(typeLong)
            {
                subIeDescriptor = MAKE_PIE_SUB_IE_LONG(ieId, subIeLength);
            }
            else
            {
                subIeDescriptor = MAKE_PIE_SUB_IE_SHORT(ieId, subIeLength);
            }
            *pBuf++ = GET_BYTE(subIeDescriptor, 0);
            *pBuf++ = GET_BYTE(subIeDescriptor, 1);
            pBuf += subIeLength;
            ieLength += (SUB_IE_DESCRIPTOR_LEN + subIeLength);
        }
    } while(bitmap);

    if(ieLength > IE_DESCRIPTOR_LEN)
    {
        ieDescriptor = MAKE_PIE(PIE_GROUP_ID_WISUN,
                                ieLength - IE_DESCRIPTOR_LEN);
        *pData++ = GET_BYTE(ieDescriptor, 0);
        *pData++ = GET_BYTE(ieDescriptor, 1);
    }
    else
    {
        ieLength = 0;
#ifdef FHIE_DBG
        FHIE_dbg.pie_gen_fail++;
#endif
    }

    return(ieLength);
}

/*!
 FHIE_gen

 Public function defined in fh_ie.h
 */
#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
MAC_INTERNAL_API uint16_t FHIE_gen(uint8_t *pData, uint32_t bitmap,
                                   macTx_t *pMacTx, FHIE_ieInfo_new_t *pIeInfo)
#else
MAC_INTERNAL_API uint16_t FHIE_gen(uint8_t *pData, uint32_t bitmap,
                                   macTx_t *pMacTx, FHIE_ieInfo_t *pIeInfo)
#endif
{
    uint32_t genPie;
    uint32_t genHie;

    if(!pData)
    {
        return(0);
    }

    genPie = bitmap & FH_WISUN_PIE_BITMAP;
    genHie = bitmap & FH_WISUN_HIE_BITMAP;

    /* each IE generation at a time */
    if(genPie && genHie)
    {
        return(0);
    }

    if(genPie)
    {
        return(MAP_FHIE_genPie(pData, bitmap, pMacTx, pIeInfo));
    }

    if(genHie)
    {
        return(MAP_FHIE_genHie(pData, bitmap, pMacTx, pIeInfo));
    }

    return(0);
}

/*!
 FHIE_parseHie

 Public function defined in fh_ie.h
 */
MAC_INTERNAL_API FHAPI_status FHIE_parseHie(uint8_t *pData,
                                            sAddrExt_t *pSrcAddr,
                                            uint32_t ts,
                                            uint16_t *pLen1,
                                            uint16_t *pLen2)
{
    uint8_t subIeId;
    uint16_t ieLength;
    uint16_t parsedLength;
    uint16_t unsupportLength;
    /*
    uint8_t txFlowControl;
    uint8_t rxFlowControl;
    uint8_t rsl;
    */
    uint32_t ufsi;
    uint8_t frameTypeId;
    uint8_t *pBuf = pData;
    FHIE_ie_t hie;
    NODE_ENTRY_s NodeEntry;
#ifdef FHIEPATCH
    FHAPI_status status;
#endif
    parsedLength = 0;
    unsupportLength = 0;

    do
    {
        ieLength = MAP_FHIE_getHie(pBuf, &hie);
        if(!ieLength)
        {
            break;
        }
#ifdef FHIE_DBG
        FHIE_dbg.hie_parse_try++;
#endif
        parsedLength += IE_DESCRIPTOR_LEN;
        pBuf += IE_DESCRIPTOR_LEN;

        subIeId = pBuf[0];
#ifdef FHIEPATCH
        status = MAP_FHIEPATCH_parseHie(subIeId, ieLength, pBuf, ts, pSrcAddr);
        if (status == FHAPI_STATUS_SUCCESS)
        {
            pBuf += ieLength;
            parsedLength += ieLength;
            continue;
        }
#endif
        switch(subIeId)
        {
        case FHIE_HIE_SUB_ID_FC_IE:
            if(ieLength != HIE_LEN_SUB_ID + HIE_LEN_FC_IE)
            {
                unsupportLength += (IE_DESCRIPTOR_LEN + ieLength);
            }
            else
            {
                /*
                txFlowControl = pBuf[1];
                rxFlowControl = pBuf[2];
                */
            }
            break;

        case FHIE_HIE_SUB_ID_UT_IE:
            if(ieLength != HIE_LEN_SUB_ID + HIE_LEN_UT_IE)
            {
                unsupportLength += (IE_DESCRIPTOR_LEN + ieLength);
            }
            else
            {
                frameTypeId = (pBuf[1] & 0xF);
                if(frameTypeId > FH_UT_IE_FT_EAPOL)
                {
                    /* do we need to handle this case? */
                }
                ufsi = 0;
                MAP_osal_memcpy(&ufsi, &pBuf[2], HIE_LEN_UT_IE - 1);
                /* update neighbor table */
                MAP_FHNT_getEntry((sAddr_t *)pSrcAddr, &NodeEntry);

                if(NodeEntry.valid)
                {
                    NodeEntry.valid |= FHNT_NODE_W_UTIE;
                    NodeEntry.valid &= ~FHNT_NODE_EXPIRED;
                    NodeEntry.ufsi = ufsi;
                    NodeEntry.ref_timeStamp = FH_hnd.rxSfdTs;

                    /* need to save the entry */
                    MAP_FHNT_putEntry(&NodeEntry);
                }
            }
            break;

        case FHIE_HIE_SUB_ID_RSL_IE:
            if(ieLength != HIE_LEN_SUB_ID + HIE_LEN_RSL_IE)
            {
                unsupportLength += (IE_DESCRIPTOR_LEN + ieLength);
            }
            else
            {
                /*
                rsl = pBuf[1];
                */
            }
            break;

        case FHIE_HIE_SUB_ID_BT_IE:
            if(ieLength != HIE_LEN_SUB_ID + HIE_LEN_BT_IE)
            {
                unsupportLength += (IE_DESCRIPTOR_LEN + ieLength);
            }
            else
            {
                FH_hnd.rxSlotIdx = MAKE_UINT16(pBuf[1], pBuf[2]);
                MAP_osal_memcpy(&FH_hnd.btie, &pBuf[3], HIE_LEN_BT_IE - sizeof(uint16_t));
                FH_hnd.btiePresent = 1;
            }
            break;

        default:
            unsupportLength += (IE_DESCRIPTOR_LEN + ieLength);
#ifdef FHIE_DBG
            FHIE_dbg.hie_parse_not_supported++;
#endif
            break;
        }
        pBuf += ieLength;
        parsedLength += ieLength;
    } while(1);

    if(parsedLength > IE_DESCRIPTOR_LEN)
    {
        *pLen1 = parsedLength;
        *pLen2 = unsupportLength;
        return(FHAPI_STATUS_SUCCESS);
    }
    else
    {
        *pLen1 = 0;
        *pLen2 = 0;
#ifdef FHIE_DBG
        FHIE_dbg.hie_parse_invalid++;
#endif
        return(FHAPI_STATUS_ERR_INVALID_FORMAT);
    }
}

/*!
 FHIE_extractPie

 Public function defined in fh_ie.h
 */
MAC_INTERNAL_API FHAPI_status FHIE_extractPie(uint8_t *pData,
                                              IE_TYPE_t ieType,
                                              uint8_t *pOut)
{
    uint8_t subIeId;
    uint16_t ieLength;
    uint16_t subIeLength;
    uint16_t parsedLength;
    uint16_t currBroadcastSchedId;
    uint16_t ieCommonLength;
    uint8_t channelInfo;
    usieParams_t usIeParams;
    FHIE_ie_t pie;
    FHIE_ie_t subPie;
    uint8_t found;
    uint8_t *pBuf = pData;
    FHIE_usIe_t *pUsIe;
    FHIE_bsIE_t *pBsIe;
#ifndef FEATURE_WISUN_SUPPORT
    FHIE_panIE_t *pPanIe;
#else
    FHIE_panIE_new_t *pPanIe;
#endif
    FHIE_netNameIE_t *pNetNameIe;
    FHIE_panVerIE_t *pPanVerIe;
    FHIE_gtkHashIE_t *pGtkHashIe;
#ifdef FHIEPATCH
    FHAPI_status status;
#endif
#ifdef FHIE_DBG
    FHIE_dbg.pie_extract_try++;
#endif

    found = 0;
    parsedLength = 0;

    ieLength = MAP_FHIE_getPie(pBuf, &pie);
    if(!ieLength)
    {
        pOut = NULL;
#ifdef FHIE_DBG
        FHIE_dbg.pie_extract_invalid++;
#endif
        return(FHAPI_STATUS_ERR_INVALID_FORMAT);
    }

    parsedLength += IE_DESCRIPTOR_LEN;
    pBuf += IE_DESCRIPTOR_LEN;

    do
    {
        subIeLength = MAP_FHIE_getSubPie(pBuf, &subPie);
        if(!subIeLength)
        {
            break;
        }

        subIeId = subPie.id;
        if(subIeId == ieType)
        {
            found = 1;
            pBuf += SUB_IE_DESCRIPTOR_LEN;
            parsedLength += SUB_IE_DESCRIPTOR_LEN;
            break;
        }
        else
        {
            pBuf += (SUB_IE_DESCRIPTOR_LEN + subIeLength);
            parsedLength += (SUB_IE_DESCRIPTOR_LEN + subIeLength);
        }
    } while(parsedLength < ieLength + IE_DESCRIPTOR_LEN);

    if(!found)
    {
        pOut = NULL;
#ifdef FHIE_DBG
        FHIE_dbg.pie_extract_invalid++;
#endif
        return(FHAPI_STATUS_ERR_INVALID_FORMAT);
    }
#ifdef FHIEPATCH
    status = MAP_FHIEPATCH_extractPie(subIeId, subIeLength, pBuf, pOut);
    if(status != FHAPI_STATUS_ERR_NO_PATCH)
    {
        return(status);
    }
#endif
    switch(subIeId)
    {
    case FHIE_PIE_SUB_IE_SUB_ID_US_IE:
        if(subIeLength <= FH_US_IE_FIXED_PART_LEN)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return(FHAPI_STATUS_ERR_INVALID_FORMAT);
        }

        MAP_osal_memset(&usIeParams, 0, sizeof(usieParams_t));
        pUsIe = (FHIE_usIe_t *)pOut;
        pUsIe->schedInfo.dwellInterval = *pBuf++;
        pUsIe->schedInfo.clockDrift = *pBuf++;
        pUsIe->schedInfo.timingAccuracy = *pBuf++;
        channelInfo = *pBuf++;
        pUsIe->schedInfo.channelFunction = GET_CHAN_FUNC(channelInfo);

        ieCommonLength = MAP_FHIE_parseCommonInformation(pBuf, channelInfo,
                                                     &usIeParams, FH_UC);
        /* in case no common lenght or channel plan mismatches
         * or not supported channel function */
        if(!ieCommonLength)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return(FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        /* move usIeParams to pUsIe */
        pUsIe->chanInfo.noOfChannels = usIeParams.numChannels;
        pUsIe->chanInfo.fixedChannel = usIeParams.chInfo.fixedChannel;
        MAP_osal_memcpy(pUsIe->chanInfo.excludedChannelMask, usIeParams.chInfo.bitMap,
               FHPIB_MAX_BIT_MAP_SIZE);

        parsedLength += subIeLength;
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_BS_IE:
        if(subIeLength <= FH_BS_IE_FIXED_PART_LEN)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return(FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        MAP_osal_memset(&usIeParams, 0, sizeof(usieParams_t));
        pBsIe = (FHIE_bsIE_t *)pOut;
        MAP_osal_memcpy(&pBsIe->broadcastInterval, pBuf, sizeof(uint32_t));
        pBuf += sizeof(uint32_t);
        MAP_osal_memcpy(&pBsIe->broadcastSchedId, pBuf, sizeof(uint16_t));
        pBuf += sizeof(uint16_t);
        MAP_FHPIB_get(FHPIB_BROCAST_SCHED_ID, &currBroadcastSchedId);
        if(pBsIe->broadcastSchedId < currBroadcastSchedId)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return(FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        pBsIe->schedInfo.dwellInterval = *pBuf++;
        pBsIe->schedInfo.clockDrift = *pBuf++;
        pBsIe->schedInfo.timingAccuracy = *pBuf++;
        channelInfo = *pBuf++;
        pBsIe->schedInfo.channelFunction = GET_CHAN_FUNC(channelInfo);
        ieCommonLength = MAP_FHIE_parseCommonInformation(pBuf, channelInfo,
                                                     &usIeParams, FH_BC);
        /* in case no common lenght or channel plan mismatches
         * or not supported channel function */
        if(!ieCommonLength)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return(FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        /* move usIeParams to pUsIe */
        pBsIe->chanInfo.noOfChannels = usIeParams.numChannels;
        pBsIe->chanInfo.fixedChannel = usIeParams.chInfo.fixedChannel;
        MAP_osal_memcpy(pBsIe->chanInfo.excludedChannelMask, usIeParams.chInfo.bitMap,
               FHPIB_MAX_BIT_MAP_SIZE);

        parsedLength += subIeLength;
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_PAN_IE:
        if (subIeLength != FH_PAN_IE_LEN)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
#ifndef FEATURE_WISUN_SUPPORT
        pPanIe = (FHIE_panIE_t *)pOut;
        MAP_osal_memcpy(&pPanIe->panSize, pBuf, sizeof(uint16_t));
        pBuf += sizeof(uint16_t);
        pPanIe->routingCost = *pBuf++;
        pPanIe->useParentBSIE = ((*pBuf) >> 0) & 1;
        pPanIe->routingMethod = ((*pBuf) >> 1) & 1;
        pPanIe->eapolReady = ((*pBuf) >> 2) & 1;
        pPanIe->fanTpsVersion = ((*pBuf++) >> 5) & 7;
#else
        pPanIe = (FHIE_panIE_new_t *)pOut;
        MAP_osal_memcpy(&pPanIe->panSize, pBuf, sizeof(uint16_t));
        pBuf += sizeof(uint16_t);
        MAP_osal_memcpy(&pPanIe->routingCost, pBuf, sizeof(uint16_t));
        pBuf += sizeof(uint16_t);
        pPanIe->useParentBSIE = ((*pBuf) >> 0) & 1;
        pPanIe->routingMethod = ((*pBuf) >> 1) & 1;
        pPanIe->fanTpsVersion = ((*pBuf++) >> 5) & 7;
#endif
        parsedLength += subIeLength;
        break;
    case FHIE_PIE_SUB_IE_SUB_ID_NETNAME_IE:
        if (subIeLength > FH_NETNAME_IE_LEN_MAX)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        pNetNameIe = (FHIE_netNameIE_t *)pOut;
        MAP_osal_memset(&pNetNameIe->netName, 0, FH_NETNAME_IE_LEN_MAX);
        MAP_osal_memcpy(&pNetNameIe->netName, pBuf, subIeLength);

        pBuf += subIeLength;
        parsedLength += subIeLength;
        break;
    case FHIE_PIE_SUB_IE_SUB_ID_PANVER_IE:
        if (subIeLength != FH_PANVER_IE_LEN)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        pPanVerIe = (FHIE_panVerIE_t *)pOut;
        MAP_osal_memcpy(&pPanVerIe->panVersion, pBuf, sizeof(uint16_t));
        pBuf += sizeof(uint16_t);
        parsedLength += subIeLength;
        break;
    case FHIE_PIE_SUB_IE_SUB_ID_GTKHASH_IE:
        if (subIeLength != FH_GTKHASH_IE_LEN)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        pGtkHashIe = (FHIE_gtkHashIE_t *)pOut;
        MAP_osal_memcpy(&pGtkHashIe->gtk0Hash, pBuf, FH_GTK0HASH_LEN);
        pBuf += FH_GTK0HASH_LEN;
        MAP_osal_memcpy(&pGtkHashIe->gtk1Hash, pBuf, FH_GTK0HASH_LEN);
        pBuf += FH_GTK0HASH_LEN;
        MAP_osal_memcpy(&pGtkHashIe->gtk2Hash, pBuf, FH_GTK0HASH_LEN);
        pBuf += FH_GTK0HASH_LEN;
        MAP_osal_memcpy(&pGtkHashIe->gtk3Hash, pBuf, FH_GTK0HASH_LEN);
        pBuf += FH_GTK0HASH_LEN;

        parsedLength += subIeLength;
        break;

    default:
        pOut = NULL;
#ifdef FHIE_DBG
        FHIE_dbg.pie_extract_not_supported++;
#endif
        return (FHAPI_STATUS_ERR_NOT_SUPPORTED_IE);
    }

    return (FHAPI_STATUS_SUCCESS);
}

/*!
 FHIE_extractHie

 Public function defined in fh_ie.h
 */
MAC_INTERNAL_API FHAPI_status FHIE_extractHie(uint8_t *pData,
                                              IE_TYPE_t ieType,
                                              uint8_t *pOut)
{
    uint8_t subIeId;
    uint16_t ieLength;
    uint16_t parsedLength;
    FHIE_ie_t hie;
    uint8_t found;
    uint8_t *pBuf = pData;
    FHIE_utIE_t *pUtIe;
    FHIE_btIE_t *pBtIe;
    FHIE_fcIE_t *pFcIe;
    FHIE_rslIE_t *pRslIe;
#ifdef FHIEPATCH
    FHAPI_status status;
#endif
#ifdef FHIE_DBG
    FHIE_dbg.hie_extract_try++;
#endif

    found = 0;
    parsedLength = 0;

    do
    {
        ieLength = MAP_FHIE_getHie(pBuf, &hie);
        if(!ieLength)
        {
            break;
        }

        pBuf += IE_DESCRIPTOR_LEN;
        parsedLength += IE_DESCRIPTOR_LEN;

        subIeId = *pBuf;
        if(subIeId == ieType)
        {
            found = 1;
            pBuf += HIE_LEN_SUB_ID;
            parsedLength += HIE_LEN_SUB_ID;
            break;
        }
        else
        {
            pBuf += ieLength;
            parsedLength += ieLength;
        }
    } while(1);

    if(!found)
    {
        pOut = NULL;
#ifdef FHIE_DBG
        FHIE_dbg.hie_extract_invalid++;
#endif
        return(FHAPI_STATUS_ERR_INVALID_FORMAT);
    }
#ifdef FHIEPATCH
    status = MAP_FHIEPATCH_extractHie(subIeId, ieLength, pBuf, pOut);
    if(status != FHAPI_STATUS_ERR_NO_PATCH)
    {
        return(status);
    }
#endif
    switch (subIeId)
    {
    case FHIE_HIE_SUB_ID_FC_IE:
        if (ieLength != HIE_LEN_SUB_ID + HIE_LEN_FC_IE)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.hie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }

        pFcIe = (FHIE_fcIE_t *)pOut;
        pFcIe->txFlowControl = *pBuf++;
        pFcIe->rxFlowControl = *pBuf;

        parsedLength += HIE_LEN_FC_IE;
        break;

    case FHIE_HIE_SUB_ID_UT_IE:
        if (ieLength != HIE_LEN_SUB_ID + HIE_LEN_UT_IE)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.hie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }

        pUtIe = (FHIE_utIE_t *)pOut;
        pUtIe->frameTypeId = *pBuf++;
        if (pUtIe->frameTypeId > FH_UT_IE_FT_EAPOL)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.hie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        pUtIe->ufsi = 0;
        MAP_osal_memcpy(&pUtIe->ufsi, pBuf, HIE_LEN_UT_IE - 1);

        parsedLength += HIE_LEN_UT_IE;
        break;

    case FHIE_HIE_SUB_ID_RSL_IE:
        if (ieLength != HIE_LEN_SUB_ID + HIE_LEN_RSL_IE)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.hie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }

        pRslIe = (FHIE_rslIE_t *)pOut;
        pRslIe->rsl = *pBuf;

        parsedLength += HIE_LEN_RSL_IE;
        break;

    case FHIE_HIE_SUB_ID_BT_IE:
        if (ieLength != HIE_LEN_SUB_ID + HIE_LEN_BT_IE)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.hie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }

        pBtIe = (FHIE_btIE_t *)pOut;
        pBtIe->bsn = MAKE_UINT16(pBuf[0], pBuf[1]);
        MAP_osal_memcpy(&pBtIe->bfio, &pBuf[2], HIE_LEN_BT_IE - sizeof(uint16_t));

        parsedLength += HIE_LEN_BT_IE;
        break;

    default:
        pOut = NULL;
#ifdef FHIE_DBG
        FHIE_dbg.hie_extract_not_supported++;
#endif
        return (FHAPI_STATUS_ERR_NOT_SUPPORTED_IE);
    }

    return (FHAPI_STATUS_SUCCESS);
}

/*!
 * update btie based on bfio and slotIdx
 *
 * Public function defined in fh_util.h
 */

uint8_t FHUTIl_updateBTIE(uint32_t bfio, uint16_t slotIdx)
{
    uint32_t macBcInterval, temp32;
#ifndef FEATURE_WISUN_EDFE_SUPPORT
    uint64_t temp64, temp64_r;
#endif
    uint8_t  intState, status = 0;
    uint32_t comp_time;

    MAP_FHPIB_get(FHPIB_BC_INTERVAL, &macBcInterval);
#ifdef FEATURE_WISUN_SUPPORT
    /*New spec: time since last broadcast slot = bfio */
    temp32 = bfio;
#else
    /*Old spec: time since last broadcast slot = (bfsi/(2^32))*(32768*250)) */
    temp64 = ((uint64_t)bfio * macBcInterval);
    temp64_r = (uint64_t)(0x80000000);
    temp64 += temp64_r;
    temp32 = (uint32_t)(temp64 >> 32);
#endif
      /* compensate for TX time */
    comp_time = MAP_FHUTIL_compBfioTxTime();
    temp32 += comp_time;
    //temp32 += FHUTIL_compBfioTxTime();

    /* boundary check */
    if(temp32 >= macBcInterval)
    {
      temp32 = temp32 - macBcInterval;
      slotIdx += 1;
    }
    DBG_PRINTL2(DBGSYS, "FHTraceLog: Parsed BT IE values: bfio=(0x%X) slotNo=(0x%X)",\
                temp32,slotIdx);

    HAL_ENTER_CRITICAL_SECTION(intState);

    if((temp32 != FH_hnd.bfio) || (slotIdx != FH_hnd.bcSlotIdx))
    {
        FH_hnd.bfio = temp32;
        /* Note: bc timer must be restarted to ensure Fh_hnd.bfio is the new reference */
        /* Ensure StartBS is called after this point. Else, bt-ie values would become incorrect */
        FH_hnd.bcSlotIdx = slotIdx;
        FH_hnd.pendBcChHop = 0;
        /*Store btieTs for sleepy devices */
        FH_hnd.btieTs = MAP_ICall_getTicks();
        status = 1;
    }
    HAL_EXIT_CRITICAL_SECTION(intState);
    return status;
}

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
                else if(pMacDataTx->internal.frameType == MAC_FRAME_TYPE_EDFE)
                {
                    //need to set edfe
                    FH_hnd.edfePending = 1;
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

void FHMGR_bcTimerIsrCb(uint8_t parameter)
{
    uint8_t macBcDwellTime;
    uint32_t macBcInterval, temp32;
    uint16_t bc_event;

#ifdef MACRADIODBG
  macDbgRadioState[macDbgCntr++] = 9;
  macDbgRadioState[macDbgCntr++] = FH_hnd.bcDwellActive;
  macDbgRadioState[macDbgCntr++] = 0;
  macDbgRadioState[macDbgCntr++] = 0;
  macDbgCntr &= 0x1FF;
#endif
    MAP_FHPIB_get(FHPIB_BC_DWELL_INTERVAL, &macBcDwellTime);

    if(!macBcDwellTime)
    {
      /*For sleepy device, do not start timer, in case it got started during JOIN process */
      return;
    }

    MAP_FHPIB_get(FHPIB_BC_INTERVAL, &macBcInterval);
    if(!FH_hnd.bcDwellActive)
    {
        FH_hnd.bcTimer.duration = (macBcDwellTime * TICKPERIOD_MS_US);
        MAP_FHMGR_macStartFHTimer(&FH_hnd.bcTimer, TRUE);

        FH_hnd.bcSlotIdx += 1;

        FH_hnd.bfio = 0;

        FH_hnd.bcDwellActive = 1;

        MAP_macRxSoftEnable(MAC_RX_FH_BC_SLOT);
        bc_event = FH_EVT_BC_TIMER;
    }
    else
    {
        MAP_macRxSoftDisable(MAC_RX_FH_BC_SLOT);
        FH_hnd.bcDwellActive = 0;

        FH_hnd.bfio = macBcDwellTime;

        FH_hnd.bcTimer.duration = (macBcInterval - macBcDwellTime) * TICKPERIOD_MS_US;

        MAP_FHMGR_macStartFHTimer(&FH_hnd.bcTimer, TRUE);
       /*Handle corner cases where current UC slot ends and next UC slot starts
        * within 2 ms (to account for radio synthesizer and RX ON delay)
        * Work around for Back to back Flush/stop commands causing a problem,
        * in that only the first stop command works.
        */
        MAP_FHUTIL_getCurUfsi(&temp32);

        temp32 = (temp32 % FHPIB_db.macUcDwellInterval);

        if(temp32 < ( FHPIB_db.macUcDwellInterval - FH_UC_DWELLTIME_BUF))
        {
            bc_event = FH_EVT_UC_TIMER;
        }
        else
        {
            DBG_PRINT3(DBGSYS, "FHTraceLog: skipping UC, bcTimerIsr slotNo(0x%X) bfio(0x%0x) bcDwellActive(%d)", FH_hnd.bcSlotIdx,FH_hnd.bfio,FH_hnd.bcDwellActive);
            return;
        }
    }
    MAP_FHSM_event(&FH_FSM, bc_event, &FH_hnd);
}

/**************************************************************************************************
 * @fn          FHMGR_updateRadioUCChannel
 *
 * @brief       This function changes the unicast channel.
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
 *************************************************************************************************
 */
void FHMGR_updateRadioUCChannel(void *pData)
{
    uint8_t channel, chFunc;
    FH_HND_s *pFH_hnd = (FH_HND_s *)pData;

    MAP_FHPIB_get(FHPIB_UC_CHANNEL_FUNCTION, &chFunc);

    if(chFunc == FHIE_CF_SINGLE_CHANNEL)
    {
        uint16_t ucFixedChannel;
        MAP_FHPIB_get(FHPIB_UC_FIXED_CHANNEL, &ucFixedChannel);
        channel = ucFixedChannel & 0xFF;
    }
    else if(chFunc == FHIE_CF_DH1CF)
    {
        channel = MAP_FHDH1CF_getChannelNum(pFH_hnd);
    }
    else
    {
        return;
    }

    if(!pFH_hnd->bcDwellActive)
    {
        MAP_macRadioSetChannel(channel);
        pFH_hnd->lastChannel = channel;
        pFH_hnd->pendUcChHop = 0;
        DBG_PRINTL2(DBGSYS, "FHTraceLog: UnicastStart: RTCTime(0x%X) slotNo(0x%0x)", (uint32_t) MAP_ICall_getTicks(),FH_hnd.ucSlotIdx);
        /* 2. check UC pend packet*/
        if(pFH_hnd->pktPending || pFH_hnd->edfePending)
        {
            /* set up next data transmission */
            MAP_macSetEvent(MAC_TX_COMPLETE_TASK_EVT);
        }
    }
    else
    {
        pFH_hnd->pendUcChHop = 1;
        DBG_PRINTL1(DBGSYS, "FHTraceLog: UnicastPend: RTCTime(0x%X) at bcDwellActive", (uint32_t) MAP_ICall_getTicks());
    }
}

/*!
 FHPIB_reset

 Public function defined in fh_pib.h
 */
MAC_INTERNAL_API void FHPIB_reset(void)
{
    /* copy FH-related PIB defaults */
    //FHPIB_db = FHPIB_defaults;
    MAP_osal_memcpy((void *)&FHPIB_db, (const void *)&FHPIB_defaults, sizeof(FHPIB_db));
#ifdef FEATURE_WISUN_EDFE_SUPPORT
    MAP_osal_memcpy((void *)&FHPIB_db_new, (const void *)&FHPIB_defaults_new, sizeof(FHPIB_db_new));
#endif
}
#endif
