/**************************************************************************************************
  Filename:       MT_AF.c
  Revised:        $Date: 2015-01-26 08:25:50 -0800 (Mon, 26 Jan 2015) $
  Revision:       $Revision: 42025 $

  Description:    MonitorTest functions for the AF layer.

  Copyright 2007-2015 Texas Instruments Incorporated.

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

**************************************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "zcomdef.h"
#include "rom_jt_154.h"
#include "ti_zstack_config.h"
#include "mt.h"
#include "mt_af.h"
#include "mt_zdo.h"
#include "nwk.h"

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
#include "stub_aps.h"
#endif


/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

#if !defined MT_AF_EXEC_CNT
#define MT_AF_EXEC_CNT  15
#endif

#if !defined MT_AF_EXEC_DLY
#define MT_AF_EXEC_DLY  1000
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

typedef struct
{
  uint8_t *data;
  afAddrType_t dstAddr;
  endPointDesc_t *epDesc;
  uint16_t cId;
  uint16_t dataLen;
  uint8_t transId;
  uint8_t txOpts;
  uint8_t radius;
  uint8_t tick;
} mtAfDataReq_t;

typedef struct _mtAfInMsgList_t
{
  struct _mtAfInMsgList_t *next;
  uint8_t *data;
  uint32_t timestamp;         // Receipt timestamp from MAC.
  uint8_t tick;
} mtAfInMsgList_t;

/* ------------------------------------------------------------------------------------------------
 *                                        Local Variables
 * ------------------------------------------------------------------------------------------------
 */

mtAfInMsgList_t *pMtAfInMsgList = NULL;
mtAfDataReq_t *pMtAfDataReq = NULL;

/* ------------------------------------------------------------------------------------------------
 *                                        Global Variables
 * ------------------------------------------------------------------------------------------------
 */

#if defined ( MT_AF_CB_FUNC )
uint16_t _afCallbackSub;
#endif

/* ------------------------------------------------------------------------------------------------
 *                                        Local Functions
 * ------------------------------------------------------------------------------------------------
 */

static void MT_AfRegister(uint8_t *pBuf);
static void MT_AfDelete(uint8_t *pBuf);
static void MT_AfDataRequest(uint8_t *pBuf);
static void MT_AfDataRequestSrcRtg(uint8_t *pBuf);

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
static void MT_AfInterPanCtl(uint8_t *pBuf);
#endif

static void MT_AfDataRetrieve(uint8_t *pBuf);
static void MT_AfDataStore(uint8_t *pBuf);
static void MT_AfAPSF_ConfigSet(uint8_t *pBuf);
static void MT_AfAPSF_ConfigGet(uint8_t *pBuf);


/**************************************************************************************************
 * @fn          MT_AfExec
 *
 * @brief       This function is invoked by an MT timer event.
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
void MT_AfExec(void)
{
  mtAfInMsgList_t *pPrev, *pItem = pMtAfInMsgList;

  while (pItem != NULL)
  {
    if (--(pItem->tick) == 0)
    {
      if (pMtAfInMsgList == pItem)
      {
        pMtAfInMsgList = pItem->next;
        (void)OsalPort_free(pItem);
        pItem = pMtAfInMsgList;
      }
      else
      {
        pPrev->next = pItem->next;
        (void)OsalPort_free(pItem);
        pItem = pPrev->next;
      }
    }
    else
    {
      pPrev = pItem;
      pItem = pItem->next;
    }
  }

  if (pMtAfDataReq != NULL)
  {
    if (--(pMtAfDataReq->tick) == 0)
    {
      (void)OsalPort_free(pMtAfDataReq);
      pMtAfDataReq = NULL;
    }
  }

  if ((pMtAfInMsgList != NULL) || (pMtAfDataReq != NULL))
  {
    if (ZSuccess != OsalPortTimers_startTimer(MT_TaskID, MT_AF_EXEC_EVT, MT_AF_EXEC_DLY))
    {
      OsalPort_setEvent(MT_TaskID, MT_AF_EXEC_EVT);
    }
  }
}

/***************************************************************************************************
 * @fn      MT_AfCommandProcessing
 *
 * @brief   Process all the AF commands that are issued by test tool
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  status
 ***************************************************************************************************/
uint8_t MT_AfCommandProcessing(uint8_t *pBuf)
{
  uint8_t status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
    case MT_AF_REGISTER:
      MT_AfRegister(pBuf);
      break;

    case MT_AF_DELETE:
      MT_AfDelete( pBuf );
      break;

    case MT_AF_DATA_REQUEST:
    case MT_AF_DATA_REQUEST_EXT:
      MT_AfDataRequest(pBuf);
      break;

    case MT_AF_DATA_REQUEST_SRCRTG:
      MT_AfDataRequestSrcRtg(pBuf);
      break;

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
    case MT_AF_INTER_PAN_CTL:
      MT_AfInterPanCtl(pBuf);
      break;
#endif

    case MT_AF_DATA_RETRIEVE:
      MT_AfDataRetrieve(pBuf);
      break;

    case MT_AF_DATA_STORE:
      MT_AfDataStore(pBuf);
      break;

    case MT_AF_APSF_CONFIG_SET:
      MT_AfAPSF_ConfigSet(pBuf);
      break;

    case MT_AF_APSF_CONFIG_GET:
      MT_AfAPSF_ConfigGet(pBuf);
      break;

    default:
      status = MT_RPC_ERR_COMMAND_ID;
      break;
  }

  return status;
}

/***************************************************************************************************
 * @fn      MT_AfRegister
 *
 * @brief   Process AF Register command
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
static void MT_AfRegister(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue = ZMemError;
  endPointDesc_t *epDesc;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  epDesc = (endPointDesc_t *)OsalPort_malloc(sizeof(endPointDesc_t));
  if ( epDesc )
  {
    epDesc->task_id = &MT_TaskID;
    retValue = MT_BuildEndpointDesc( pBuf, epDesc );
    if ( retValue == ZSuccess )
    {
      retValue = afRegister( epDesc );
    }

    if ( retValue != ZSuccess )
    {
      OsalPort_free( epDesc );
    }
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_AF), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_AfDelete
 *
 * @brief   Process AF Delete Endpoint command
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
static void MT_AfDelete(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue = ZMemError;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  retValue = afDelete( *pBuf );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_AF), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_AfDataRequest
 *
 * @brief   Process AF Register command
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
static void MT_AfDataRequest(uint8_t *pBuf)
{
  #define MT_AF_REQ_MSG_LEN  10
  #define MT_AF_REQ_MSG_EXT  10

  endPointDesc_t *epDesc;
  afAddrType_t dstAddr;
  cId_t cId;
  uint8_t transId, txOpts, radius;
  uint8_t cmd0, cmd1;
  uint8_t retValue = ZFailure;
  uint16_t dataLen, tempLen;

  /* Parse header */
  cmd0 = pBuf[MT_RPC_POS_CMD0];
  cmd1 = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if (cmd1 == MT_AF_DATA_REQUEST_EXT)
  {
    dstAddr.addrMode = (afAddrMode_t)*pBuf++;

    if (dstAddr.addrMode == afAddr64Bit)
    {
      (void)OsalPort_memcpy(dstAddr.addr.extAddr, pBuf, Z_EXTADDR_LEN);
    }
    else
    {
      dstAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
    }
    pBuf += Z_EXTADDR_LEN;

    dstAddr.endPoint = *pBuf++;
    dstAddr.panId = OsalPort_buildUint16( pBuf );
    pBuf += 2;
  }
  else
  {
    /* Destination address */
    dstAddr.addrMode = afAddr16Bit;
    dstAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
    pBuf += 2;

    /* Destination endpoint */
    dstAddr.endPoint = *pBuf++;
    dstAddr.panId = 0;
  }

  /* Source endpoint */
  epDesc = afFindEndPointDesc(*pBuf++);

  /* ClusterId */
  cId = OsalPort_buildUint16( pBuf );
  pBuf +=2;

  /* TransId */
  transId = *pBuf++;

  /* TxOption */
  txOpts = *pBuf++;

  /* Radius */
  radius = *pBuf++;

  /* Length */
  if (cmd1 == MT_AF_DATA_REQUEST_EXT)
  {
    dataLen = OsalPort_buildUint16( pBuf );
    tempLen = dataLen + MT_AF_REQ_MSG_LEN + MT_AF_REQ_MSG_EXT;
    pBuf += 2;
  }
  else
  {
    dataLen = *pBuf++;
    tempLen = dataLen + MT_AF_REQ_MSG_LEN;
  }

  if ( epDesc == NULL )
  {
    retValue = afStatus_INVALID_PARAMETER;
  }
  else if (tempLen > (uint16_t)MT_RPC_DATA_MAX)
  {
    if (pMtAfDataReq != NULL)
    {
      retValue = afStatus_INVALID_PARAMETER;
    }
    else if ((pMtAfDataReq = OsalPort_malloc(sizeof(mtAfDataReq_t) + dataLen)) == NULL)
    {
      retValue = afStatus_MEM_FAIL;
    }
    else
    {
      retValue = afStatus_SUCCESS;

      pMtAfDataReq->data = (uint8_t *)(pMtAfDataReq+1);
      (void)OsalPort_memcpy(&(pMtAfDataReq->dstAddr), &dstAddr, sizeof(afAddrType_t));
      pMtAfDataReq->epDesc = epDesc;
      pMtAfDataReq->cId = cId;
      pMtAfDataReq->dataLen = dataLen;
      pMtAfDataReq->transId = transId;
      pMtAfDataReq->txOpts = txOpts;
      pMtAfDataReq->radius = radius;

      // Setup to time-out the huge outgoing item if host does not MT_AF_DATA_STORE it.
      pMtAfDataReq->tick = MT_AF_EXEC_CNT;
      if (ZSuccess != OsalPortTimers_startTimer(MT_TaskID, MT_AF_EXEC_EVT, MT_AF_EXEC_DLY))
      {
        (void)OsalPort_setEvent(MT_TaskID, MT_AF_EXEC_EVT);
      }
    }
  }
  else
  {
    retValue = AF_DataRequest(&dstAddr, epDesc, cId, dataLen, pBuf, &transId, txOpts, radius);
  }

  if (MT_RPC_CMD_SREQ == (cmd0 & MT_RPC_CMD_TYPE_MASK))
  {
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP|(uint8_t)MT_RPC_SYS_AF), cmd1, 1, &retValue);
  }
}

/***************************************************************************************************
 * @fn      MT_AfDataRequestSrcRtg
 *
 * @brief   Process AF Register command
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
static void MT_AfDataRequestSrcRtg(uint8_t *pBuf)
{
  uint8_t cmdId, dataLen = 0;
  uint8_t retValue = ZFailure;
  endPointDesc_t *epDesc;
  byte transId;
  afAddrType_t dstAddr;
  cId_t cId;
  byte txOpts, radius, srcEP, relayCnt;
  uint16_t *pRelayList;
  uint8_t i;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Destination address */
  /* Initialize the panID field to zero to avoid inter-pan */
  memset( &dstAddr, 0, sizeof(afAddrType_t) );
  dstAddr.addrMode = afAddr16Bit;
  dstAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Destination endpoint */
  dstAddr.endPoint = *pBuf++;

  /* Source endpoint */
  srcEP = *pBuf++;
  epDesc = afFindEndPointDesc( srcEP );

  /* ClusterId */
  cId = OsalPort_buildUint16( pBuf );
  pBuf +=2;

  /* TransId */
  transId = *pBuf++;

  /* TxOption */
  txOpts = *pBuf++;

  /* Radius */
  radius = *pBuf++;

  /* Source route relay count */
  relayCnt = *pBuf++;

  /* Convert the source route relay list */
  if( (pRelayList = OsalPort_malloc( relayCnt * sizeof( uint16_t ))) != NULL )
  {
    for( i = 0; i < relayCnt; i++ )
    {
      pRelayList[i] = OsalPort_buildUint16( pBuf );
      pBuf += 2;
    }

    /* Data payload Length */
    dataLen = *pBuf++;

    if ( epDesc == NULL )
    {
      retValue = afStatus_INVALID_PARAMETER;
    }
    else
    {
      retValue = AF_DataRequestSrcRtg( &dstAddr, epDesc, cId, dataLen, pBuf,
                                     &transId, txOpts, radius, relayCnt, pRelayList );
    }

    /* Free the memory allocated */
    OsalPort_free( pRelayList );
  }
  else
  {
    retValue = afStatus_MEM_FAIL;
  }


  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_AF), cmdId, 1, &retValue);
}

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
/***************************************************************************************************
 * @fn      MT_AfInterPanCtl
 *
 * @brief   Process the AF Inter Pan control command.
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
static void MT_AfInterPanCtl(uint8_t *pBuf)
{
  uint8_t cmd, rtrn;
  uint16_t panId;
  endPointDesc_t *pEP;

  cmd = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  switch (*pBuf++)  // Inter-pan request parameter.
  {
  case InterPanClr:
    rtrn = StubAPS_SetIntraPanChannel();           // Switch channel back to the NIB channel.
    break;

  case InterPanSet:
    rtrn = StubAPS_SetInterPanChannel(*pBuf);      // Set channel for inter-pan communication.
    break;

  case InterPanReg:
    if ((pEP = afFindEndPointDesc(*pBuf)))
    {
      StubAPS_RegisterApp(pEP);
      rtrn = SUCCESS;
    }
    else
    {
      rtrn = FAILURE;
    }
    break;

  case InterPanChk:
    panId = OsalPort_buildUint16( pBuf );
    rtrn = (StubAPS_InterPan(panId, pBuf[2])) ? ZSuccess : ZFailure;
    break;

  default:
    rtrn = afStatus_INVALID_PARAMETER;
    break;
  }

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_AF), cmd, 1, &rtrn);
}
#endif

/***************************************************************************************************
 * @fn      MT_AfDataConfirm
 *
 * @brief   Process
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
void MT_AfDataConfirm(afDataConfirm_t *pMsg)
{
  uint8_t retArray[3];

  retArray[0] = pMsg->hdr.status;
  retArray[1] = pMsg->endpoint;
  retArray[2] = pMsg->transID;

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_AF), MT_AF_DATA_CONFIRM, 3, retArray);
}

/***************************************************************************************************
 * @fn      MT_AfReflectError
 *
 * @brief   Process
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
void MT_AfReflectError(afReflectError_t *pMsg)
{
  uint8_t retArray[6];

  retArray[0] = pMsg->hdr.status;
  retArray[1] = pMsg->endpoint;
  retArray[2] = pMsg->transID;
  retArray[3] = pMsg->dstAddrMode;
  retArray[4] = LO_UINT16( pMsg->dstAddr );
  retArray[5] = HI_UINT16( pMsg->dstAddr );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_AF), MT_AF_REFLECT_ERROR, 6, retArray);
}

/***************************************************************************************************
 * @fn          MT_AfIncomingMsg
 *
 * @brief       Process the callback subscription for AF Incoming data.
 *
 * @param       pkt - Incoming AF data.
 *
 * @return      none
 ***************************************************************************************************/
void MT_AfIncomingMsg(afIncomingMSGPacket_t *pMsg)
{
  #define MT_AF_INC_MSG_LEN  20
  #define MT_AF_INC_MSG_EXT  10

  uint16_t dataLen = pMsg->cmd.DataLength;  // Length of the data section in the response packet.
  uint16_t respLen = MT_AF_INC_MSG_LEN + dataLen;
  uint8_t cmd = MT_AF_INCOMING_MSG;
  uint8_t *pRsp, *pTmp;
  mtAfInMsgList_t *pItem = NULL;

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  if (StubAPS_InterPan(pMsg->srcAddr.panId, pMsg->srcAddr.endPoint))
  {
    cmd = MT_AF_INCOMING_MSG_EXT;
  }
  else
#endif
  if ((pMsg->srcAddr.addrMode == afAddr64Bit) ||
      (respLen > (uint16_t)(MT_RPC_DATA_MAX - MT_AF_INC_MSG_EXT)))
  {
    cmd = MT_AF_INCOMING_MSG_EXT;
  }

  if (cmd == MT_AF_INCOMING_MSG_EXT)
  {
    respLen += MT_AF_INC_MSG_EXT;
  }

  if (respLen > (uint16_t)MT_RPC_DATA_MAX)
  {
    if ((pItem = (mtAfInMsgList_t *)OsalPort_malloc(sizeof(mtAfInMsgList_t) + dataLen)) == NULL)
    {
      return;  // If cannot hold a huge message, cannot give indication at all.
    }

    pItem->data = (uint8_t *)(pItem+1);
    respLen -= dataLen;  // Zero data bytes are sent with an over-sized incoming indication.
  }

  // Attempt to allocate memory for the response packet.
  if ((pRsp = OsalPort_malloc(respLen)) == NULL)
  {
    if (pItem != NULL)
    {
      (void)OsalPort_free(pItem);
    }
    return;
  }
  pTmp = pRsp;

  /* Group ID */
  *pTmp++ = LO_UINT16(pMsg->groupId);
  *pTmp++ = HI_UINT16(pMsg->groupId);

  /* Cluster ID */
  *pTmp++ = LO_UINT16(pMsg->clusterId);
  *pTmp++ = HI_UINT16(pMsg->clusterId);

  if (cmd == MT_AF_INCOMING_MSG_EXT)
  {
    *pTmp++ = pMsg->srcAddr.addrMode;

    if (pMsg->srcAddr.addrMode == afAddr64Bit)
    {
      (void)OsalPort_memcpy(pTmp, pMsg->srcAddr.addr.extAddr, Z_EXTADDR_LEN);
    }
    else
    {
      pTmp[0] = LO_UINT16(pMsg->srcAddr.addr.shortAddr);
      pTmp[1] = HI_UINT16(pMsg->srcAddr.addr.shortAddr);
    }
    pTmp += Z_EXTADDR_LEN;

    *pTmp++ = pMsg->srcAddr.endPoint;
#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
    *pTmp++ = LO_UINT16(pMsg->srcAddr.panId);
    *pTmp++ = HI_UINT16(pMsg->srcAddr.panId);
#else
    *pTmp++ = 0;
    *pTmp++ = 0;
#endif
  }
  else
  {
    /* Source Address */
    *pTmp++ = LO_UINT16(pMsg->srcAddr.addr.shortAddr);
    *pTmp++ = HI_UINT16(pMsg->srcAddr.addr.shortAddr);

    /* Source EP */
    *pTmp++ = pMsg->srcAddr.endPoint;
  }

  /* Destination EP */
  *pTmp++ = pMsg->endPoint;

  /* WasBroadCast */
  *pTmp++ = pMsg->wasBroadcast;

  /* LinkQuality */
  *pTmp++ = pMsg->LinkQuality;

  /* SecurityUse */
  *pTmp++ = pMsg->SecurityUse;

  /* Timestamp */
  OsalPort_bufferUint32( pTmp, pMsg->timestamp );
  pTmp += 4;

  /* Data Length */
  if (cmd == MT_AF_INCOMING_MSG_EXT)
  {
    /* Z-Tool apparently takes the last Byte before the data buffer as the dynamic length and
     * ignores the bigger uint16_t length of an EXT incoming message. But no data bytes will be sent
     * with a huge message, so it's necessary to work-around and fake-out Z-Tool with a zero here.
     */
    *pTmp++ = 0;  // TODO - remove this field, it used to be unused (pMsg->cmd.TransSeqNumber)
    *pTmp++ = LO_UINT16(dataLen);
    *pTmp++ = HI_UINT16(dataLen);
  }
  else
  {
    *pTmp++ = 0; // TODO - remove this field, it used to be unused (pMsg->cmd.TransSeqNumber)
    *pTmp++ = dataLen;
  }

  /* Data */
  if (pItem != NULL)
  {
    // Enqueue the new huge incoming item.
    pItem->next = pMtAfInMsgList;
    pMtAfInMsgList = pItem;

    // Setup to time-out the huge incoming item if host does not MT_AF_DATA_RETRIEVE it.
    pItem->tick = MT_AF_EXEC_CNT;
    if (ZSuccess != OsalPortTimers_startTimer(MT_TaskID, MT_AF_EXEC_EVT, MT_AF_EXEC_DLY))
    {
      (void)OsalPort_setEvent(MT_TaskID, MT_AF_EXEC_EVT);
    }

    pItem->timestamp = pMsg->timestamp;
    (void)OsalPort_memcpy(pItem->data, pMsg->cmd.Data, dataLen);
  }
  else
  {
    (void)OsalPort_memcpy(pTmp, pMsg->cmd.Data, dataLen);
    pTmp += dataLen;
  }

  // MAC Source address
  *pTmp++ = LO_UINT16(pMsg->macSrcAddr);
  *pTmp++ = HI_UINT16(pMsg->macSrcAddr);

  // messages result radius
  *pTmp = pMsg->radius;

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ|(uint8_t)MT_RPC_SYS_AF), cmd, respLen, pRsp);

  (void)OsalPort_free(pRsp);
}

/**************************************************************************************************
 * @fn          MT_AfDataRetrieve
 *
 * @brief   Process AF Data Retrieve command to incrementally read out a very large
 *          incoming AF message.
 *
 * input parameters
 *
 * @param pBuf - pointer to the received buffer
 *
 * output parameters
 *
 * @param rtrn - AF-Status of the operation.
 *
 * @return      None.
 **************************************************************************************************
 */
static void MT_AfDataRetrieve(uint8_t *pBuf)
{
  #define MT_AF_RTV_HDR_SZ  2

  uint32_t timestamp;
  mtAfInMsgList_t *pPrev, *pItem = pMtAfInMsgList;
  uint8_t rtrn = afStatus_FAILED;
  uint8_t len = 0;

  pBuf += MT_RPC_FRAME_HDR_SZ;
  timestamp = OsalPort_buildUint32( pBuf, 4 );

  while (pItem != NULL)
  {
    pPrev = pItem;
    if (pItem->timestamp == timestamp)
    {
      break;
    }
    pItem = pItem->next;
  }

  if (pItem != NULL)
  {
    uint16_t idx;
    uint8_t *pRsp;

    pBuf += 4;
    idx = OsalPort_buildUint16( pBuf );
    len = pBuf[2];

    if (len == 0)  // Indication to delete the afIncomingMSGPacket.
    {
      if (pMtAfInMsgList == pItem)
      {
        pMtAfInMsgList = pItem->next;
      }
      else
      {
        pPrev->next = pItem->next;
      }
      (void)OsalPort_free(pItem);
      rtrn = afStatus_SUCCESS;
    }
    else if ((pRsp = OsalPort_malloc(len + MT_AF_RTV_HDR_SZ)) == NULL)
    {
      rtrn = afStatus_MEM_FAIL;
      len = 0;
    }
    else
    {
      pRsp[0] = ZSuccess;
      pRsp[1] = len;
      (void)OsalPort_memcpy(pRsp + MT_AF_RTV_HDR_SZ, pItem->data+idx, len);
      MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_AF),
                                           MT_AF_DATA_RETRIEVE, len + MT_AF_RTV_HDR_SZ, pRsp);
      (void)OsalPort_free(pRsp);
      return;
    }
  }

  pBuf[0] = rtrn;
  pBuf[1] = len;
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_AF),
                                       MT_AF_DATA_RETRIEVE, MT_AF_RTV_HDR_SZ, pBuf);
}

/**************************************************************************************************
 * @fn          MT_AfDataStore
 *
 * @brief   Process AF Data Store command to incrementally store the data buffer for very large
 *          outgoing AF message.
 *
 * input parameters
 *
 * @param pBuf - pointer to the received buffer
 *
 * output parameters
 *
 * @param rtrn - AF-Status of the operation.
 *
 * @return      None.
 **************************************************************************************************
 */
static void MT_AfDataStore(uint8_t *pBuf)
{
  uint16_t idx;
  uint8_t len, rtrn = afStatus_FAILED;

  pBuf += MT_RPC_FRAME_HDR_SZ;
  idx = OsalPort_buildUint16( pBuf );
  len = pBuf[2];
  pBuf += 3;

  if (pMtAfDataReq == NULL)
  {
    rtrn = afStatus_MEM_FAIL;
  }
  else if (len == 0)  // Indication to send the message.
  {
    rtrn = AF_DataRequest(&(pMtAfDataReq->dstAddr), pMtAfDataReq->epDesc, pMtAfDataReq->cId,
                            pMtAfDataReq->dataLen,  pMtAfDataReq->data,
                          &(pMtAfDataReq->transId), pMtAfDataReq->txOpts, pMtAfDataReq->radius);
    (void)OsalPort_free(pMtAfDataReq);
    pMtAfDataReq = NULL;
  }
  else
  {
    (void)OsalPort_memcpy(pMtAfDataReq->data+idx, pBuf, len);
    rtrn = afStatus_SUCCESS;
  }

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_AF),
                                                                MT_AF_DATA_STORE, 1, &rtrn);
}

/**************************************************************************************************
 * @fn          MT_AfAPSF_ConfigSet
 *
 * @brief       This function is the MT proxy for afAPSF_ConfigSet().
 *
 * input parameters
 *
 * @param       pBuf - Pointer to the received buffer.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
static void MT_AfAPSF_ConfigSet(uint8_t *pBuf)
{
  afAPSF_Config_t cfg = { pBuf[MT_RPC_POS_DAT0+1], pBuf[MT_RPC_POS_DAT0+2] };
  afStatus_t rtrn = afAPSF_ConfigSet(pBuf[MT_RPC_POS_DAT0], &cfg);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_AF),
                                       MT_AF_APSF_CONFIG_SET, 1, (uint8_t *)&rtrn);
}

/**************************************************************************************************
 * @fn          MT_AfAPSF_ConfigGet
 *
 * @brief       This function is the MT proxy for afAPSF_ConfigGet().
 *
 * input parameters
 *
 * @param       pBuf - Pointer to the received buffer.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
static void MT_AfAPSF_ConfigGet(uint8_t *pBuf)
{
  afAPSF_Config_t cfg = { 0, 0 };
  uint8_t buf[2];

  afAPSF_ConfigGet( pBuf[MT_RPC_POS_DAT0], &cfg );

  buf[0] = cfg.frameDelay;
  buf[1] = cfg.windowSize;

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_AF),
                                       MT_AF_APSF_CONFIG_GET, 2, buf );
}

/***************************************************************************************************
***************************************************************************************************/
