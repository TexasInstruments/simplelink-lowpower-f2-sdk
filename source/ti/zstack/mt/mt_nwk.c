/**************************************************************************************************
  Filename:       MT_NWK.c
  Revised:        $Date: 2015-01-26 08:25:50 -0800 (Mon, 26 Jan 2015) $
  Revision:       $Revision: 42025 $

  Description:    MonitorTest functions for the NWK layer.

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

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
#include "zcomdef.h"
#include "mt.h"
#include "mt_nwk.h"
#include "nl_mede.h"
#include "nwk.h"
#include "nwk_globals.h"
#include "nwk_util.h"
#include "rom_jt_154.h"
#include "zd_app.h"
#include "npi_config.h"

/***************************************************************************************************
 * GLOBAL VARIABLES
 ***************************************************************************************************/
uint16_t _nwkCallbackSub;

/*********************************************************************
 * TYPEDEFS
 */

/***************************************************************************************************
 * EXTERN VARIABLES
 ***************************************************************************************************/

#ifdef NWK_LAYER_WHITE_LIST
extern NwkLayerBlackWhiteList_t NwkLayerBlackWhiteList;
#endif


/***************************************************************************************************
 * EXTERN FUNCTIONS
 ***************************************************************************************************/
#ifdef NWK_LAYER_WHITE_LIST
extern uint8_t NwkLayerBlackWhiteListSetBehavior(uint8_t isAllowed);
extern uint8_t NwkLayerBlackWhiteListAddNode(uint16_t shortAddr);
extern uint8_t NwkLayerBlackWhiteListRemoveNode(uint16_t shortAddr);
extern uint8_t NwkLayerCommWhiteListAddNode(uint16_t shortAddr,uint16_t dstAddr);
extern uint8_t NwkLayerCommWhiteListRemoveNode(uint16_t shortAddr,uint16_t dstAddr);
#endif


/***************************************************************************************************
 * LOCAL FUNCTIONS
 ***************************************************************************************************/
#if defined (MT_NWK_FUNC)
static void MT_NldeDataRequest(uint8_t *pBuf);
static void MT_NlmeNetworkFormationRequest(uint8_t *pBuf);
static void MT_NlmePermitJoiningRequest(uint8_t *pBuf);
static void MT_NlmeStartRouterRequest(uint8_t *pBuf);
static void MT_NlmeJoinRequest(uint8_t *pBuf);
static void MT_NlmeLeaveRequest(uint8_t *pBuf);
static void MT_NlmeResetRequest(uint8_t *pBuf);
static void MT_NlmeGetRequest(uint8_t *pBuf);
static void MT_NlmeSetRequest(uint8_t *pBuf);
static void MT_NlmeNetworkDiscoveryRequest(uint8_t *pBuf);
static void MT_NlmeRouteDiscoveryRequest(uint8_t *pBuf);
static void MT_NlmeDirectJoinRequest(uint8_t *pBuf);
static void MT_NlmeOrphanJoinRequest(uint8_t *pBuf);

static uint8_t MT_Nwk_DataRequest( uint16_t dstAddr, uint8_t nsduLen, uint8_t* nsdu,
                                 uint8_t nsduHandle, uint16_t nsduHandleOptions,
                                 uint8_t secure, uint8_t discoverRoute,
                                 uint8_t radius);

#ifdef NWK_LAYER_WHITE_LIST
static void MT_NwkLayerBlackWhiteListSetBehaviorReq(uint8_t *pBuf);
static void MT_NwkLayerBlackWhiteListAddNodeReq(uint8_t *pBuf);
static void MT_NwkLayerBlackWhiteListRemoveNodeReq(uint8_t *pBuf);

static void MT_NwkLayerCommWhiteListAddNodeReq(uint8_t *pBuf);
static void MT_NwkLayerCommWhiteListRemoveNodeReq(uint8_t *pBuf);
#endif

#endif /* MT_NWK_FUNC */

#if defined (MT_NWK_FUNC)


/***************************************************************************************************
 * @fn      MT_NwkCommandProcessing
 *
 * @brief
 *
 *   Process all the NWK commands that are issued by test tool
 *
 * @param   cmd_id - Command ID
 * @param   len    - Length of received SPI data message
 * @param   pData  - pointer to received SPI data message
 *
 * @return  status
 ***************************************************************************************************/
uint8_t MT_NwkCommandProcessing(uint8_t *pBuf)
{
  uint8_t status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
    case MT_NWK_INIT:
      nwk_init(NWK_TaskID);
      break;

    case MT_NLDE_DATA_REQ:
      MT_NldeDataRequest(pBuf);
      break;

    case MT_NLME_NETWORK_FORMATION_REQ:
      MT_NlmeNetworkFormationRequest(pBuf);
      break;

    case MT_NLME_PERMIT_JOINING_REQ:
       MT_NlmePermitJoiningRequest(pBuf);
      break;

     case MT_NLME_JOIN_REQ:
       MT_NlmeJoinRequest(pBuf);
       break;

     case MT_NLME_LEAVE_REQ:
       MT_NlmeLeaveRequest(pBuf);
       break;

     case MT_NLME_RESET_REQ:
       MT_NlmeResetRequest(pBuf);
       break;

     case MT_NLME_GET_REQ:
       MT_NlmeGetRequest(pBuf);
       break;

     case MT_NLME_SET_REQ:
       MT_NlmeSetRequest(pBuf);
       break;

     case MT_NLME_NETWORK_DISCOVERY_REQ:
       MT_NlmeNetworkDiscoveryRequest(pBuf);
       break;

     case MT_NLME_ROUTE_DISCOVERY_REQ:
       MT_NlmeRouteDiscoveryRequest(pBuf);
       break;

     case MT_NLME_DIRECT_JOIN_REQ:
       MT_NlmeDirectJoinRequest(pBuf);
       break;

     case MT_NLME_ORPHAN_JOIN_REQ:
       MT_NlmeOrphanJoinRequest(pBuf);
       break;

    case MT_NLME_START_ROUTER_REQ:
      MT_NlmeStartRouterRequest(pBuf);
      break;

#ifdef NWK_LAYER_WHITE_LIST
    case MT_NWK_LAYER_WHITE_LIST_SET_BEHAVIOR:
        MT_NwkLayerBlackWhiteListSetBehaviorReq(pBuf);
    break;

    case MT_NWK_LAYER_WHITE_LIST_ADD_NODE:
        MT_NwkLayerBlackWhiteListAddNodeReq(pBuf);
    break;

    case MT_NWK_LAYER_WHITE_LIST_REMOVE_NODE:
        MT_NwkLayerBlackWhiteListRemoveNodeReq(pBuf);
    break;
    case MT_NWK_LAYER_COMM_WHITE_LIST_ADD_NODE:
        MT_NwkLayerCommWhiteListAddNodeReq(pBuf);
    break;
    case MT_NWK_LAYER_COMM_WHITE_LIST_REMOVE_NODE:
        MT_NwkLayerCommWhiteListRemoveNodeReq(pBuf);
    break;
#endif

    default:
      status = MT_RPC_ERR_COMMAND_ID;
      break;
  }

  return status;
}




#ifdef NWK_LAYER_WHITE_LIST

/***************************************************************************************************
 * @fn      MT_NwkLayerBlackWhiteListSetBehaviorReq
 *
 * @brief   Set the behavior of the NwkLayerBlackWhiteList
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NwkLayerBlackWhiteListSetBehaviorReq(uint8_t *pBuf)
{
    uint8_t cmdId;
    uint8_t isAllowed;
    uint8_t retValue = ZFailure;

    /* parse header */
    cmdId = pBuf[MT_RPC_POS_CMD1];
    pBuf += MT_RPC_FRAME_HDR_SZ;

    isAllowed = *pBuf;

    retValue = NwkLayerBlackWhiteListSetBehavior(isAllowed);

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NwkLayerBlackWhiteListAddNodeReq
 *
 * @brief   Add a node to the NwkLayerBlackWhiteList
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NwkLayerBlackWhiteListAddNodeReq(uint8_t *pBuf)
{
    uint8_t cmdId;
    uint8_t retValue = ZFailure;
    uint16_t ShortAddr;

    /* parse header */
    cmdId = pBuf[MT_RPC_POS_CMD1];
    pBuf += MT_RPC_FRAME_HDR_SZ;

    ShortAddr = OsalPort_buildUint16( pBuf );

    retValue = NwkLayerBlackWhiteListAddNode(ShortAddr);

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NwkLayerBlackWhiteListRemoveNodeReq
 *
 * @brief   Remove a node from the NwkLayerBlackWhiteList, if 0xFFFF is set, then all nodes are removed
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NwkLayerBlackWhiteListRemoveNodeReq(uint8_t *pBuf)
{
    uint8_t cmdId;
    uint8_t retValue = ZFailure;
    uint16_t ShortAddr;

    /* parse header */
    cmdId = pBuf[MT_RPC_POS_CMD1];
    pBuf += MT_RPC_FRAME_HDR_SZ;

    ShortAddr = OsalPort_buildUint16( pBuf );

    retValue = NwkLayerBlackWhiteListRemoveNode(ShortAddr);

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}


/***************************************************************************************************
 * @fn      MT_NwkLayerCommWhiteListAddNodeReq
 *
 * @brief   Remove a node from the NwkLayerBlackWhiteList of the remote device (dstAddr), if 0xFFFF is set, then all nodes are removed
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NwkLayerCommWhiteListAddNodeReq(uint8_t *pBuf)
{
    uint8_t cmdId;
    uint8_t retValue = ZFailure;
    uint16_t ShortAddr;
    uint16_t dstAddr;

    /* parse header */
    cmdId = pBuf[MT_RPC_POS_CMD1];
    pBuf += MT_RPC_FRAME_HDR_SZ;

    ShortAddr = OsalPort_buildUint16( pBuf );

    pBuf += sizeof(uint16_t);

    dstAddr = OsalPort_buildUint16( pBuf );

    pBuf += sizeof(uint16_t);

    retValue = NwkLayerCommWhiteListAddNode(ShortAddr,dstAddr);

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NwkLayerCommWhiteListRemoveNodeReq
 *
 * @brief   Remove a node from the NwkLayerBlackWhiteList of the remote device (dstAddr), if 0xFFFF is set, then all nodes are removed
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NwkLayerCommWhiteListRemoveNodeReq(uint8_t *pBuf)
{
    uint8_t cmdId;
    uint8_t retValue = ZFailure;
    uint16_t ShortAddr;
    uint16_t dstAddr;

    /* parse header */
    cmdId = pBuf[MT_RPC_POS_CMD1];
    pBuf += MT_RPC_FRAME_HDR_SZ;

    ShortAddr = OsalPort_buildUint16( pBuf );

    pBuf += sizeof(uint16_t);

    dstAddr = OsalPort_buildUint16( pBuf );

    pBuf += sizeof(uint16_t);

    retValue = NwkLayerCommWhiteListRemoveNode(ShortAddr,dstAddr);

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

#endif //NWK_LAYER_WHITE_LIST







/***************************************************************************************************
 * @fn      MT_NldeDataReq
 *
 * @brief   Process NLDE Data Request commands
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NldeDataRequest(uint8_t *pBuf)
{
  uint8_t retValue = ZFailure;
  uint16_t dstAddr;
  uint8_t dataLen = 0;
  uint8_t *dataPtr;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* First read the DstAddr */
  dstAddr = OsalPort_buildUint16( pBuf );
  pBuf += sizeof( dstAddr );

  /* Get the NSDU details */
  dataLen = *pBuf++;
  dataPtr = pBuf;

  /* Skip a length of ZTEST_DEFAULT_DATA_LEN */
  pBuf += dataLen;

  /* Send out Data Request */
  retValue = MT_Nwk_DataRequest(dstAddr, dataLen, dataPtr, pBuf[0], OsalPort_buildUint16( &pBuf[1] ),
                                pBuf[3], pBuf[4], pBuf[5]);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}


/***************************************************************************************************
 * @fn      MT_NlmeNetworkFormationRequest
 *
 * @brief   Network Formation Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeNetworkFormationRequest(uint8_t *pBuf)
{
  uint8_t retValue = ZFailure;
  uint16_t panId;
  uint32_t channelList;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Build panId */
  panId = OsalPort_buildUint16( pBuf );
  pBuf += sizeof(uint16_t);

  /* Build the channel list */
  channelList = OsalPort_buildUint32(pBuf, 4);
  pBuf += sizeof(uint32_t);

  if ( ZG_BUILD_RTR_TYPE )
  {
    retValue = NLME_NetworkFormationRequest( panId, NULL, channelList, pBuf[0], pBuf[1], pBuf[2], pBuf[3],pBuf[4],pBuf[5] );
  }
  else
  {
    retValue = ZUnsupportedMode;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NlmePermitJoiningRequest
 *
 * @brief   Permit Joining Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmePermitJoiningRequest(uint8_t *pBuf)
{
  uint8_t retValue = ZFailure;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if (ZSTACK_ROUTER_BUILD)
  {
    retValue = NLME_PermitJoiningRequest(*pBuf);
  }
  else
  {
    retValue = ZUnsupportedMode;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NlmeStartRouterRequest
 *
 * @brief   Start Router Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeStartRouterRequest(uint8_t *pBuf)
{
  uint8_t retValue = ZFailure;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if ( ZSTACK_ROUTER_BUILD )
  {
    retValue = (uint8_t)NLME_StartRouterRequest(pBuf[0], pBuf[1], pBuf[2]);
  }
  else
  {
    retValue = ZUnsupportedMode;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NlmeJoinRequest
 *
 * @brief   Join Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeJoinRequest(uint8_t *pBuf)
{
  uint8_t retValue = ZFailure;
  uint8_t dummyExPANID[Z_EXTADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  uint16_t panID;
  uint8_t cmdId;
  networkDesc_t *pNwkDesc;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;
  panID = OsalPort_buildUint16( pBuf );

  if((pNwkDesc = nwk_getNetworkDesc(dummyExPANID,panID, pBuf[2])) != NULL )
  {
    if (pNwkDesc->chosenRouter == INVALID_NODE_ADDR )
    {
      retValue = ZNwkNotPermitted;
    }
    else
    {
      retValue = NLME_JoinRequest( dummyExPANID, panID, pBuf[2], pBuf[3],
                                   pNwkDesc->chosenRouter, pNwkDesc->chosenRouterDepth );
    }
  }
  else
  {
    retValue = ZNwkNotPermitted;
  }

  if ( pBuf[3] & CAPINFO_RCVR_ON_IDLE )
  {
    /* The receiver is on, turn network layer polling off. */
    nwk_SetCurrentPollRateType(POLL_RATE_RX_ON_TRUE,TRUE);
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NlmeLeaveRequest
 *
 * @brief   Leave Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeLeaveRequest(uint8_t *pBuf)
{
  NLME_LeaveReq_t req;
  uint8_t retValue = ZFailure;
  uint8_t index, cmdId, len;

  /* parse header */
  len =  pBuf[MT_RPC_POS_LEN];
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* If extAddr is all zeros, it means null pointer */
  for(index=0;((index < Z_EXTADDR_LEN) && (pBuf[index] == 0));index++);

  if (index == Z_EXTADDR_LEN)
  {
    req.extAddr = NULL;
  }
  else
  {
    req.extAddr = pBuf;
  }

  /* Increment the pointer */
  pBuf += Z_EXTADDR_LEN;
  if ( len > Z_EXTADDR_LEN )
  {
    req.removeChildren = *pBuf++;
    req.rejoin         = *pBuf++;
  }
  else
  {
    req.removeChildren = FALSE;
    req.rejoin         = FALSE;
  }
  req.silent         = FALSE;

  retValue = NLME_LeaveReq(&req);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}


/***************************************************************************************************
 * @fn      MT_NlmeResetRequest
 *
 * @brief   Leave Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeResetRequest(uint8_t *pBuf)
{
  uint8_t retValue = NLME_ResetRequest();

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), MT_NLME_RESET_REQ, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NlmeGetRequest
 *
 * @brief   Get Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeGetRequest(uint8_t *pBuf)
{
  uint8_t dataBuf[11];
  uint8_t attr, index, cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  attr = *pBuf++;
  index = *pBuf;

  dataBuf[0] = NLME_GetRequest((ZNwkAttributes_t )attr, index, &dataBuf[1]);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId,
                               11, dataBuf );
}

/***************************************************************************************************
 * @fn      MT_NlmeSetRequest
 *
 * @brief   Set Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeSetRequest(uint8_t *pBuf)
{
  uint8_t retValue = ZFailure;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  retValue = NLME_SetRequest((ZNwkAttributes_t)pBuf[0], pBuf[1], &pBuf[2]);

  /* Update NV */
  ZDApp_NVUpdate();

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_NlmeNetworkDiscoveryRequest
 *
 * @brief   Network Discovery Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeNetworkDiscoveryRequest(uint8_t *pBuf)
{
  uint8_t retValue = ZFailure;
  uint8_t cmdId;
  uint32_t scanChannels;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Scan channels */
  scanChannels = OsalPort_buildUint32(pBuf, 4);
  pBuf += sizeof(uint32_t);

  retValue = NLME_NetworkDiscoveryRequest(scanChannels, *pBuf);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_NlmeRouteDiscoveryRequest
 *
 * @brief   Route Discovery Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeRouteDiscoveryRequest(uint8_t *pBuf)
{
  uint8_t retValue = ZFailure;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if ( ZSTACK_ROUTER_BUILD )
  {
    retValue = NLME_RouteDiscoveryRequest(OsalPort_buildUint16( pBuf ), pBuf[2], pBuf[3]);
  }
  else
  {
    retValue = ZUnsupportedMode;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NlmeDirectJoinRequest
 *
 * @brief   Direct Join Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeDirectJoinRequest(uint8_t *pBuf)
{
  uint8_t retValue = ZFailure;
  uint8_t cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if ( ZSTACK_ROUTER_BUILD )
  {
    retValue = NLME_DirectJoinRequest( pBuf, pBuf[8] );
  }
  else
  {
    retValue = ZUnsupportedMode;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NlmeOrphanJoinRequest
 *
 * @brief   Orphan Join Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeOrphanJoinRequest(uint8_t *pBuf)
{
  uint8_t i, j, attr;
  uint8_t retValue = ZFailure;
  uint32_t channelList;
  uint8_t cmdId = pBuf[MT_RPC_POS_CMD1];

  if ( ZSTACK_END_DEVICE_BUILD )
  {
    /* parse header */
    pBuf += MT_RPC_FRAME_HDR_SZ;

    /* Channel list bit mask */
    channelList = OsalPort_buildUint32(pBuf, 4);
    pBuf += sizeof(uint32_t);

    /* Count number of channels */
    j = attr = 0;

    for (i = 0; i < ED_SCAN_MAXCHANNELS; i++)
    {
      if (channelList & (1 << i))
      {
        j++;
        attr = i;
      }
    }

    /* If only one channel specified */
    if (j == 1)
    {
      _NIB.scanDuration = *pBuf;
      _NIB.nwkLogicalChannel = attr;
      _NIB.channelList = channelList;
      if ( !_NIB.CapabilityFlags )
      {
        _NIB.CapabilityFlags = ZDO_Config_Node_Descriptor.CapabilityFlags;
      }

      devState = DEV_NWK_ORPHAN;
      retValue = (uint8_t)NLME_OrphanJoinRequest(channelList, *pBuf);
    }
    else
    {
      retValue = ZNwkInvalidParam;
    }
  }
  else
  {
    retValue = ZUnsupportedMode;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_Nwk_DataRequest
 *
 * @brief   Nwk Data Request
 *
 * @param   dstAddr, nsduLen, nsdu, nsduHandle, nsduHandleOptions, secure, discoverRoute, radius
 *
 * @return  void
 ***************************************************************************************************/
static uint8_t MT_Nwk_DataRequest(uint16_t dstAddr, uint8_t nsduLen, uint8_t* nsdu,
                                uint8_t nsduHandle, uint16_t nsduHandleOptions,
                                uint8_t secure, uint8_t discoverRoute,
                                uint8_t radius)
{
    uint8_t               status;
    NLDE_DataReqAlloc_t dra;
    NLDE_DataReq_t*     req;


    dra.overhead = sizeof(NLDE_DataReq_t);
    dra.nsduLen  = nsduLen;
    dra.secure   = secure;

    req = NLDE_DataReqAlloc(&dra);

    if ( req != NULL )
    {
      OsalPort_memcpy(req->nfd.nsdu, nsdu, nsduLen);

      req->nfd.dstAddr           = dstAddr;
      req->nfd.nsduHandleOptions = nsduHandleOptions;
      req->nfd.discoverRoute     = discoverRoute;
      req->nfd.radius            = radius;

      status = NLDE_DataReq( req );
    }
    else
    {
      status = ZMemError;
    }

    return status;
}
#endif /* MT_NWK_FUNC */

#if defined ( MT_NWK_CB_FUNC )             //NWK callback commands
/***************************************************************************************************
 * @fn          nwk_MTCallbackSubDataConfirm
 *
 * @brief       Process the callback subscription for NLDE-DATA.confirm
 *
 * @param       nsduHandle  - APS handle
 * @param       Status      - result of data request
 *
 * @return      none
 ***************************************************************************************************/
void nwk_MTCallbackSubDataConfirm(uint8_t nsduHandle, ZStatus_t status)
{
  uint8_t buf[2];

  buf[0] = nsduHandle;
  buf[1] = (uint8_t)status;

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_NWK), MT_NLDE_DATA_CONF, 2, buf );
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubDataIndication
 *
 * @brief       Process the callback subscription for NLDE-DATA.indication
 *
 * @param       SrcAddress      - 16 bit address
 * @param       nsduLength      - Length of incoming data
 * @param       nsdu            - Pointer to incoming data
 * @param       LinkQuality     - Link quality measured during
 *                                reception.
 *
 * @return      none
 ***************************************************************************************************/
void nwk_MTCallbackSubDataIndication(uint16_t SrcAddress, int16_t nsduLength, uint8_t *nsdu, uint8_t LinkQuality)
{
  uint8_t *msgPtr;
  uint8_t *msg;
  uint8_t msgLen;

  msgLen = sizeof( uint16_t ) + sizeof( uint8_t ) + ZTEST_DEFAULT_DATA_LEN
            + sizeof( uint8_t);

  msgPtr = OsalPort_malloc( msgLen );
  if ( msgPtr )
  {
    //Fill up the data bytes
    msg = msgPtr;

    //First fill in details
    *msg++ = LO_UINT16( SrcAddress );
    *msg++ = HI_UINT16( SrcAddress );

    //Since the max packet size is less than 255 bytes, a byte is enough
    //to represent nsdu length
    *msg++ = ( uint8_t ) nsduLength;

    memset( msg, 0U, ZTEST_DEFAULT_DATA_LEN ); // Clear the mem
    OsalPort_memcpy( msg, nsdu, nsduLength );
    msg += ZTEST_DEFAULT_DATA_LEN;

    *msg++ = LinkQuality;

    MT_BuildAndSendZToolResponse( ((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_NWK), MT_NLDE_DATA_IND, msgLen, msgPtr );

    OsalPort_free( msgPtr );
  }
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubInitCoordConfirm
 *
 * @brief       Process the callback subscription for NLME-INIT-COORD.confirm
 *
 * @param       Status - Result of NLME_InitCoordinatorRequest()
 *
 * @return      none
 ***************************************************************************************************/
void nwk_MTCallbackSubInitCoordConfirm( ZStatus_t Status )
{
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_NWK), MT_NLME_NETWORK_FORMATION_CONF,
                          sizeof(uint8_t), (uint8_t*)&Status);
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubStartRouterConfirm
 *
 * @brief       Process the callback subscription for NLME-START-ROUTER.confirm
 *
 * @param       Status - Result of NLME_StartRouterRequest()
 *
 * @return      none
 ***************************************************************************************************/
void nwk_MTCallbackSubStartRouterConfirm( ZStatus_t Status )
{
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_NWK), MT_NLME_START_ROUTER_CONF,
                          sizeof(uint8_t), (uint8_t*)&Status);
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubJoinConfirm
 *
 * @brief       Process the callback subscription for NLME-JOIN.confirm
 *
 * @param       Status - Result of NLME_JoinRequest()
 *
 * @return      none
 ***************************************************************************************************/
void nwk_MTCallbackSubJoinConfirm(uint16_t PanId, ZStatus_t Status)
{
  uint8_t msg[Z_EXTADDR_LEN + 3];

  /* This device's 64-bit address */
  ZMacGetReq( ZMacExtAddr, msg );

  msg[Z_EXTADDR_LEN + 0] = LO_UINT16(PanId);
  msg[Z_EXTADDR_LEN + 1] = HI_UINT16(PanId);
  msg[Z_EXTADDR_LEN + 2] = (uint8_t)Status;

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_NWK), MT_NLME_JOIN_CONF, Z_EXTADDR_LEN + 3, msg );
}
/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNetworkDiscoveryConfirm
 *
 * @brief       Process the callback subscription for NLME-NWK_DISC.confirm
 *
 * @param       ResultCount			- number of networks discovered
 * @param				NetworkList			- pointer to list of network descriptors
 *
 * @return      void
 ***************************************************************************************************/
void nwk_MTCallbackSubNetworkDiscoveryConfirm( uint8_t ResultCount, networkDesc_t *NetworkList )
{
	uint8_t len;
	uint8_t *msgPtr;
	uint8_t *msg;
	uint8_t i;

  // The message cannot be bigger then SPI_TX_BUFF_MAX.  Reduce resultCount if necessary
  if (ResultCount * sizeof(networkDesc_t) > NPI_TL_BUF_SIZE - (1 + SPI_0DATA_MSG_LEN))
  {
    ResultCount = (NPI_TL_BUF_SIZE - (1 + SPI_0DATA_MSG_LEN)) / sizeof(networkDesc_t);
  }

	len = 1 + ResultCount * sizeof(networkDesc_t);
  msgPtr = OsalPort_malloc( len );
	if ( msgPtr )
	{
	  /* Fill up the data bytes */
    msg = msgPtr;

		*msg++ = ResultCount;

		for ( i = 0; i < ResultCount; i++ )
		{
		  *msg++ = LO_UINT16( NetworkList->panId );
		  *msg++ = HI_UINT16( NetworkList->panId );
		  *msg++ = NetworkList->logicalChannel;
		  *msg++ = BEACON_ORDER_NO_BEACONS;
		  *msg++ = BEACON_ORDER_NO_BEACONS;
		  *msg++ = NetworkList->routerCapacity;
		  *msg++ = NetworkList->deviceCapacity;
		  *msg++ = NetworkList->version;
		  *msg++ = NetworkList->stackProfile;
		  //*msg++ = NetworkList->securityLevel;

			NetworkList = (networkDesc_t*)NetworkList->nextDesc;
		}

    MT_BuildAndSendZToolResponse (((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_NWK), MT_NLME_NETWORK_DISCOVERY_CONF, len, msgPtr);

    OsalPort_free( msgPtr );
	}
}
/***************************************************************************************************
 * @fn          nwk_MTCallbackSubJoinIndication
 *
 * @brief       Process the callback subscription for NLME-INIT-COORD.indication
 *
 * @param       ShortAddress - 16-bit address
 * @param       ExtendedAddress - IEEE (64-bit) address
 * @param       CapabilityFlags - Association Capability Information
 *
 * @return      ZStatus_t
 ***************************************************************************************************/
void nwk_MTCallbackSubJoinIndication( uint16_t ShortAddress, uint8_t *ExtendedAddress,
                                      uint8_t CapabilityFlags )
{
  uint8_t *msgPtr;
  uint8_t *msg;
  uint8_t len;

  len = sizeof( uint16_t ) + Z_EXTADDR_LEN + sizeof( uint8_t );
  msgPtr = OsalPort_malloc( len );

  if ( msgPtr )
  {
    /* Fill up the data bytes */
    msg = msgPtr;

    /* First fill in details */
    *msg++ = LO_UINT16( ShortAddress );
    *msg++ = HI_UINT16( ShortAddress );

    osal_cpyExtAddr( msg, ExtendedAddress );
    msg += Z_EXTADDR_LEN;

    *msg = CapabilityFlags;

    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_NWK), MT_NLME_JOIN_IND, len, msgPtr );

    OsalPort_free( msgPtr );
  }
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubLeaveConfirm
 *
 * @brief       Process the callback subscription for NLME-LEAVE.confirm
 *
 * @param       DeviceAddress - IEEE (64-bit) address
 * @param       Status - Result of NLME_LeaveRequest()
 *
 * @return      none
 ***************************************************************************************************/
void nwk_MTCallbackSubLeaveConfirm( uint8_t *DeviceAddress, ZStatus_t Status )
{
  uint8_t *msgPtr;
  uint8_t *msg;

  msgPtr = OsalPort_malloc( Z_EXTADDR_LEN + sizeof( uint8_t ) );
  if ( msgPtr )
  {
    /* Fill up the data bytes */
    msg = msgPtr;

    /* First fill in details */
    osal_cpyExtAddr( msg, DeviceAddress );
    msg += Z_EXTADDR_LEN;

    *msg = (uint8_t)Status;

    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_NWK), MT_NLME_LEAVE_CONF,
                            Z_EXTADDR_LEN + sizeof( uint8_t ), msgPtr );

    OsalPort_free( msgPtr );
  }
}
/***************************************************************************************************
 * @fn          nwk_MTCallbackSubLeaveIndication
 *
 * @brief       Process the callback subscription for NLME-LEAVE.indication
 *
 * @param       DeviceAddress - IEEE (64-bit) address
 *
 * @return      NULL
 ***************************************************************************************************/
void nwk_MTCallbackSubLeaveIndication( uint8_t *DeviceAddress )
{
  uint8_t msg[Z_EXTADDR_LEN+1];

  /* First fill in details */
  if ( DeviceAddress )
  {
    osal_cpyExtAddr( msg, DeviceAddress );
  }
  else
  {
    memset( msg, 0, Z_EXTADDR_LEN );
  }

  /* Status, assume good if we get this far */
  msg[Z_EXTADDR_LEN] = 0;

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_NWK), MT_NLME_LEAVE_IND, Z_EXTADDR_LEN+1, msg );
}
/***************************************************************************************************
 * @fn          nwk_MTCallbackSubSyncIndication
 *
 * @brief       Process the callback subscription for NLME-SYNC.indication
 *
 * @param       none
 *
 * @return      none
 ***************************************************************************************************/
void nwk_MTCallbackSubSyncIndication( void )
{
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_NWK), MT_NLME_SYNC_IND, 0, NULL );
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubPollConfirm
 *
 * @brief       Process the callback subscription for NLME-POLL.confirm
 *
 * @param       status - status of the poll operation
 *
 * @return      none
 ***************************************************************************************************/
void nwk_MTCallbackSubPollConfirm( uint8_t status )
{
  uint8_t msg = status;

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_NWK), MT_NLME_POLL_CONF, 1, &msg );
}

#endif /* NWK Callback commands */

/***************************************************************************************************
 ***************************************************************************************************/

