/**************************************************************************************************
  Filename:       touchlink_initiator_app.c
  Revised:        $Date: 2013-11-26 15:12:49 -0800 (Tue, 26 Nov 2013) $
  Revision:       $Revision: 36298 $

  Description:    Zigbee Cluster Library - Light Link Initiator.


  Copyright 2011-2013 Texas Instruments Incorporated.

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
#include "ti_zstack_config.h"
#if defined ( BDB_TL_INITIATOR)
/*********************************************************************
 * INCLUDES
 */
#include "zcomdef.h"
#include "rom_jt_154.h"
#include "osal_nv.h"
#include "af.h"
#include "zd_app.h"
#include "nwk_util.h"
#include "addr_mgr.h"
#include "zd_sec_mgr.h"
#include "zstacktask.h"
#include "zstackapi.h"

#include "stub_aps.h"

#include "zcl.h"
#include "zcl_port.h"
#include "zcl_general.h"
#include "bdb.h"
#include "bdb_tl_commissioning.h"
#include "bdb_touchlink.h"
#include "touchlink_initiator_app.h"
#include "bdb_touchlink_initiator.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8_t tlAppEntity;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static ZStatus_t initiatorScanReqCB( afAddrType_t *srcAddr, bdbTLScanReq_t *pReq, uint8_t seqNum );
static ZStatus_t initiatorDeviceInfoReqCB( afAddrType_t *srcAddr, bdbTLDeviceInfoReq_t *pReq, uint8_t seqNum );
static ZStatus_t initiatorIdentifyReqCB( afAddrType_t *srcAddr, bdbTLIdentifyReq_t *pReq );
static ZStatus_t initiatorNwkJoinReqCB( afAddrType_t *srcAddr, bdbTLNwkJoinReq_t *pReq, uint8_t seqNum );
static ZStatus_t initiatorNwkUpdateReqCB( afAddrType_t *srcAddr, bdbTLNwkUpdateReq_t *pReq );
static ZStatus_t initiatorScanRspCB( afAddrType_t *srcAddr, bdbTLScanRsp_t *pRsp );
static ZStatus_t initiatorDeviceInfoRspCB( afAddrType_t *srcAddr, bdbTLDeviceInfoRsp_t *pRsp );
static ZStatus_t initiatorNwkStartRspCB( afAddrType_t *srcAddr, bdbTLNwkStartRsp_t *pRsp );
static ZStatus_t initiatorNwkJoinRspCB( afAddrType_t *srcAddr, bdbTLNwkJoinRsp_t *pRsp );
static ZStatus_t initiatorGetGrpIDsReq(afAddrType_t *srcAddr, bdbTLGetGrpIDsReq_t *pReq, uint8_t seqNum);
static ZStatus_t initiatorGetEPListReq(afAddrType_t *srcAddr, bdbTLGetEPListReq_t *pReq, uint8_t SeqNum);

/*********************************************************************
 * LOCAL VARIABLES
 */

static tl_BDBFindingBindingCb_t commissioningCb;

/*********************************************************************
 * TOUCHLINK Initiator Callback Table
 */
// Initiator Command Callbacks table
static bdbTL_InterPANCallbacks_t touchLinkInitiator_CmdCBs =
{
  // Received Server Commands
  initiatorScanReqCB,       // Scan Request command
  initiatorDeviceInfoReqCB, // Device Information Request command
  initiatorIdentifyReqCB,   // Identify Request command
  NULL,                     // Reset to Factory New Request command
  NULL,                     // Network Start Request command
#if ( ZSTACK_ROUTER_BUILD )
  initiatorNwkJoinReqCB,    // Network Join Router Request command
  NULL,                     // Network Join End Device Request command
#else
  NULL,                     // Network Join Router Request command
  initiatorNwkJoinReqCB,    // Network Join End Device Request command
#endif
  initiatorNwkUpdateReqCB,  // Network Update Request command

  // Received Client Commands
  initiatorScanRspCB,       // Scan Response command
  initiatorDeviceInfoRspCB, // Device Information Response command
  initiatorNwkStartRspCB,   // Network Start Response command
  initiatorNwkJoinRspCB,    // Network Join Router Response command
  initiatorNwkJoinRspCB     // Network Join End Device Response command
};

static bdbTL_AppCallbacks_t touchlinkInitiator_AppCBs =
{
  // Received Server Commands
  initiatorGetGrpIDsReq,  // Get Group Identifiers Request command
  initiatorGetEPListReq,  // Get Endpoint List Request command

  // Received Client Commands
  NULL,  // Endpoint Information command
  NULL,  // Get Group Identifiers Response command
  NULL  // Get Endpoint List Response command
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn          touchLinkInitiatorApp_Init
 *
 * @brief       Initialize the touchlink initiator module.
 *              This function must be called by the application during its initialization.
 *
 * @param       zclSampleAppEvents - the events process by the sample application
 *
 * @return      none
 */
void touchLinkInitiatorApp_Init(uint8_t zclSampleApp_Entity)
{
  tlAppEntity = zclSampleApp_Entity;

  StubAPS_RegisterApp( &touchLink_EP );

  zclport_registerEndpoint(tlAppEntity, &touchLink_EP);

  bdbTL_RegisterInterPANCmdCallbacks(&touchLinkInitiator_CmdCBs);

  bdbTL_RegisterCmdCallbacks(TOUCHLINK_INTERNAL_ENDPOINT, &touchlinkInitiator_AppCBs);
}

/*********************************************************************
 * @fn          touchLinkApp_registerFindingBindingCb
 *
 * @brief       Register application finding and binding callback
 *
 * @param       fbCb - bdb finding and binding callback function
 *
 * @return      none
 */
void touchLinkApp_registerFindingBindingCb(tl_BDBFindingBindingCb_t fbCb)
{
  commissioningCb = fbCb;
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      initiatorScanReqCB
 *
 * @brief   This callback is called to process a Scan Request command.
 *
 * @param   srcAddr - sender's address
 * @param   pReq - parsed command
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorScanReqCB( afAddrType_t *srcAddr, bdbTLScanReq_t *pReq, uint8_t seqNum )
{
  int8_t rssi;
  rssi = touchLink_GetMsgRssi();
  if( ( rssi > TOUCHLINK_WORST_RSSI ) && ( pReq->touchLinkInitiator == TRUE ) )
  {
    // response to the originator, but switch to dst PAN 0xFFFF
    afAddrType_t dstAddr;
    OsalPort_memcpy(&dstAddr, srcAddr, sizeof(afAddrType_t));
    dstAddr.panId = 0xFFFF;

    if ( pReq->touchLinkFactoryNew == TRUE )
    {
      // If we are factory new and revice a Scan Request from other factory new
      // device then drop the request.
      if ( bdbAttributes.bdbNodeIsOnANetwork == FALSE )
       {
         return ( ZSuccess );
       }
       // If, during its scan, a non factory new initiator receives another scan
       // request inter-PAN command frame from a factory new target, it shall be ignored.
      else
      {
        uint16_t time;
        zstack_touchlinkGetScanBaseTime_t getScanBaseTime;

        getScanBaseTime.stopTimer = FALSE;
        Zstackapi_touchlinkGetScanBaseTime(tlAppEntity, &getScanBaseTime);
        time = getScanBaseTime.time;

        if (time > 0)
        {
          return ( ZSuccess );
        }
      }
    }

    zstack_tlScanReq_t *pScanReq;
    pScanReq = (zstack_tlScanReq_t*)zcl_mem_alloc( sizeof(zstack_tlScanReq_t) );

    zcl_memcpy(&(pScanReq->addr), &dstAddr, sizeof(afAddrType_t));
    pScanReq->transID = pReq->transID;
    pScanReq->seqNum = seqNum;

    Zstackapi_tlScanReqInd(tlAppEntity, pScanReq);
    zcl_mem_free(pScanReq);
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      initiatorDeviceInfoReqCB
 *
 * @brief   This callback is called to process a Device Information
 *          Request command.
 *
 * @param   srcAddr - sender's address
 * @param   pReq - parsed command
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorDeviceInfoReqCB( afAddrType_t *srcAddr, bdbTLDeviceInfoReq_t *pReq, uint8_t seqNum )
{
  if ( touchLink_IsValidTransID( pReq->transID ) == FALSE )
  {
    return ( ZFailure );
  }
  return ( touchLink_SendDeviceInfoRsp( TOUCHLINK_INTERNAL_ENDPOINT, srcAddr,
                                  pReq->startIndex, pReq->transID, seqNum ) );
}

/*********************************************************************
 * @fn      initiatorIdentifyReqCB
 *
 * @brief   This callback is called to process an Identify Request command.
 *
 * @param   srcAddr - sender's address
 * @param   pReq - parsed command
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorIdentifyReqCB( afAddrType_t *srcAddr, bdbTLIdentifyReq_t *pReq )
{
  zstack_bdbZCLIdentifyCmdIndReq_t Req;
  zstack_bdbSetEpDescListToActiveEndpointRsp_t Rsp;

  if ( touchLink_IsValidTransID( pReq->transID ) == FALSE )
  {
    return ( ZFailure );
  }

  uint16_t identifyTime;

  // Values of the Identify Duration field:
  // - Exit identify mode: 0x0000
  // - Length of time to remain in identify mode: 0x0001-0xfffe
  // - Remain in identify mode for a default time known by the receiver: 0xffff
  if ( pReq->IdDuration == 0xffff )
  {
    identifyTime = TOUCHLINK_DEFAULT_IDENTIFY_TIME;
  }
  else
  {
    identifyTime = pReq->IdDuration;
  }

  Zstackapi_bdbSetEpDescListToActiveEndpoint(tlAppEntity,&Rsp);

  Req.identifyTime = identifyTime;
  Req.endpoint = Rsp.EndPoint;

  Zstackapi_bdbZclIdentifyCmdIndReq(tlAppEntity, &Req);


  return ( ZSuccess );
}

/*********************************************************************
 * @fn      initiatorNwkJoinReqCB
 *
 * @brief   This callback is called to process Network Join
 *          Request and Network Join End Device Request commands.
 *
 * @param   srcAddr - sender's address
 * @param   pReq - parsed command
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorNwkJoinReqCB( afAddrType_t *srcAddr, bdbTLNwkJoinReq_t *pReq, uint8_t seqNum )
{
  if ( touchLink_IsValidTransID( pReq->transID ) == FALSE )
  {
    return ( ZFailure );
  }

  uint8_t newUpdateId = touchLink_NewNwkUpdateId( pReq->nwkUpdateId, _NIB.nwkUpdateId);
  if ( _NIB.nwkUpdateId != newUpdateId )
  {
    NLME_SetUpdateID(newUpdateId);
  }
  zstack_touchlinkNwkJointReq_t *pInitiatorNwkJoinReq;
  pInitiatorNwkJoinReq = (zstack_touchlinkNwkJointReq_t*)zcl_mem_alloc( sizeof(zstack_touchlinkNwkJointReq_t) );

  zcl_memcpy(&(pInitiatorNwkJoinReq->addr), srcAddr, sizeof(afAddrType_t));
  zcl_memcpy(&(pInitiatorNwkJoinReq->nwkJoinReq), pReq, sizeof(bdbTLNwkJoinReq_t));
  pInitiatorNwkJoinReq->seqNum = seqNum;

  Zstackapi_touchlinkNwkJoinReqInd(tlAppEntity, pInitiatorNwkJoinReq);
  zcl_mem_free(pInitiatorNwkJoinReq);

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      initiatorNwkUpdateReqCB
 *
 * @brief   This callback is called to process a Network Update Request
 *          command.
 *
 * @param   srcAddr - sender's address
 * @param   pReq - parsed command
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorNwkUpdateReqCB( afAddrType_t *srcAddr, bdbTLNwkUpdateReq_t *pReq )
{
  if ( touchLink_IsValidTransID( pReq->transID ) == FALSE )
  {
    return ( ZFailure );
  }

  zstack_touchlinkNwkUpdateReq_t *pInitiatorNwkUpdateReq;
  pInitiatorNwkUpdateReq = (zstack_touchlinkNwkUpdateReq_t*)zcl_mem_alloc( sizeof(zstack_touchlinkNwkUpdateReq_t) );

  pInitiatorNwkUpdateReq->transID = pReq->transID;
  zcl_cpyExtAddr(pInitiatorNwkUpdateReq->extendedPANID, pReq->extendedPANID);
  pInitiatorNwkUpdateReq->nwkUpdateId = pReq->nwkUpdateId;
  pInitiatorNwkUpdateReq->logicalChannel = pReq->logicalChannel;
  pInitiatorNwkUpdateReq->PANID = pReq->PANID;
  pInitiatorNwkUpdateReq->nwkAddr = pReq->nwkAddr;

  Zstackapi_touchlinkNwkUpdateReqInd(tlAppEntity, pInitiatorNwkUpdateReq);
  zcl_mem_free(pInitiatorNwkUpdateReq);

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      initiatorScanRspCB
 *
 * @brief   This callback is called to process a Scan Response command.
 *
 * @param   srcAddr - sender's address
 * @param   pRsp - parsed command
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorScanRspCB( afAddrType_t *srcAddr, bdbTLScanRsp_t *pRsp )
{
  if  ( touchLink_IsValidTransID( pRsp->transID ) &&
      ( pRsp->keyBitmask & touchLink_GetNwkKeyBitmask() ) )
  {
    zstack_touchlinkScanRsp_t *pInitiatorScanRsp;
    pInitiatorScanRsp = (zstack_touchlinkScanRsp_t*)zcl_mem_alloc( sizeof(zstack_touchlinkScanRsp_t) );

    zcl_memcpy(&pInitiatorScanRsp->addr, srcAddr, sizeof(afAddrType_t));
    pInitiatorScanRsp->scanRsp.transID = pRsp->transID;
    pInitiatorScanRsp->scanRsp.rssiCorrection = pRsp->rssiCorrection;
    zcl_memcpy(&pInitiatorScanRsp->scanRsp.zInfo, &pRsp->zInfo, sizeof(zInfo_t));
    zcl_memcpy(&pInitiatorScanRsp->scanRsp.touchLinkInfo, &pRsp->touchLinkInfo, sizeof(touchLinkInfo_t));
    pInitiatorScanRsp->scanRsp.keyBitmask = pRsp->keyBitmask;
    pInitiatorScanRsp->scanRsp.responseID = pRsp->responseID;
    zcl_cpyExtAddr(pInitiatorScanRsp->scanRsp.extendedPANID, pRsp->extendedPANID);
    pInitiatorScanRsp->scanRsp.nwkUpdateId = pRsp->nwkUpdateId;
    pInitiatorScanRsp->scanRsp.logicalChannel = pRsp->logicalChannel;
    pInitiatorScanRsp->scanRsp.PANID = pRsp->PANID;
    pInitiatorScanRsp->scanRsp.nwkAddr = pRsp->nwkAddr;
    pInitiatorScanRsp->scanRsp.numSubDevices = pRsp->numSubDevices;
    pInitiatorScanRsp->scanRsp.totalGrpIDs = pRsp->totalGrpIDs;
    zcl_memcpy(&pInitiatorScanRsp->scanRsp.deviceInfo, &pRsp->deviceInfo, sizeof(bdbTLDeviceInfo_t));

    Zstackapi_tlInitiatorScanRspInd(tlAppEntity, pInitiatorScanRsp);
    zcl_mem_free(pInitiatorScanRsp);
    return ( ZSuccess );
  }
  return ( ZFailure );
}

/*********************************************************************
 * @fn      initiatorDeviceInfoRspCB
 *
 * @brief   This callback is called to process a Device Information
 *          Response command.
 *          If sub-device is selected, selectedTarget data is updated.
 *
 * @param   srcAddr - sender's address
 * @param   pRsp - parsed command
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorDeviceInfoRspCB( afAddrType_t *srcAddr, bdbTLDeviceInfoRsp_t *pRsp )
{
  if ( touchLink_IsValidTransID( pRsp->transID ) )
  {
    zstack_touchlinkDevInfoRsp_t *pDevInfoRsp;
    pDevInfoRsp = (zstack_touchlinkDevInfoRsp_t*)zcl_mem_alloc( sizeof(zstack_touchlinkDevInfoRsp_t) + ( pRsp->cnt * sizeof(devInfoRec_t) ) );

    zcl_memcpy(&pDevInfoRsp->addr, srcAddr, sizeof(afAddrType_t));
    pDevInfoRsp->devInfoRsp.transID = pRsp->transID;
    pDevInfoRsp->devInfoRsp.numSubDevices = pRsp->numSubDevices;
    pDevInfoRsp->devInfoRsp.startIndex = pRsp->startIndex;
    pDevInfoRsp->devInfoRsp.cnt = pRsp->cnt;
    zcl_memcpy(&pDevInfoRsp->devInfoRsp.devInfoRec, &pRsp->devInfoRec, (pRsp->cnt * sizeof(devInfoRec_t)));

    Zstackapi_tlInitiatorDevInfoRspInd(tlAppEntity, pDevInfoRsp);
    zcl_mem_free(pDevInfoRsp);
    return ( ZSuccess );
  }
  return ( ZFailure );
}

/*********************************************************************
 * @fn      initiatorNwkStartRspCB
 *
 * @brief   This callback is called to process a Network Start Response command.
 *
 * @param   srcAddr - sender's address
 * @param   pRsp - parsed command
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorNwkStartRspCB( afAddrType_t *srcAddr, bdbTLNwkStartRsp_t *pRsp )
{
  if ( touchLink_IsValidTransID( pRsp->transID ) == FALSE )
  {
    return ( ZFailure );
  }

  zstack_touchlinkNwkStartRsp_t *pNwkStartRsp;
  pNwkStartRsp = (zstack_touchlinkNwkStartRsp_t*)zcl_mem_alloc( sizeof(zstack_touchlinkNwkStartRsp_t) );

  pNwkStartRsp->nwkStartRsp.transID = pRsp->transID;
  pNwkStartRsp->nwkStartRsp.status = pRsp->status;
  zcl_cpyExtAddr(pNwkStartRsp->nwkStartRsp.extendedPANID, pRsp->extendedPANID);
  pNwkStartRsp->nwkStartRsp.nwkUpdateId = pRsp->nwkUpdateId;
  pNwkStartRsp->nwkStartRsp.logicalChannel = pRsp->logicalChannel;
  pNwkStartRsp->nwkStartRsp.panId = pRsp->panId;

  Zstackapi_tlInitiatorNwkStartRspInd(tlAppEntity, pNwkStartRsp);
  zcl_mem_free(pNwkStartRsp);
  return ( ZSuccess );
}

/*********************************************************************
 * @fn      initiatorNwkJoinRspCB
 *
 * @brief   This callback is called to process a Network Join Router
 *          Response command.
 *
 * @param   srcAddr - sender's address
 * @param   pRsp - parsed command
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorNwkJoinRspCB( afAddrType_t *srcAddr, bdbTLNwkJoinRsp_t *pRsp )
{
  if ( ( touchLink_IsValidTransID( pRsp->transID ) == FALSE ) || ( ( srcAddr->addrMode != afAddr64Bit ) ) )
  {
    return ( ZFailure );
  }

  zstack_touchlinkNwkJoinRsp_t *pNwkJoinRsp;
  pNwkJoinRsp = (zstack_touchlinkNwkJoinRsp_t*)zcl_mem_alloc( sizeof(zstack_touchlinkNwkJoinRsp_t) );

  zcl_memcpy(&pNwkJoinRsp->addr, srcAddr, sizeof(afAddrType_t));
  pNwkJoinRsp->nwkJoinRsp.transID = pRsp->transID;
  pNwkJoinRsp->nwkJoinRsp.status = pRsp->status;

  Zstackapi_tlInitiatorNwkJoinRspInd(tlAppEntity, pNwkJoinRsp);
  zcl_mem_free(pNwkJoinRsp);

  if(commissioningCb)
  {
    commissioningCb();
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      initiatorGetGrpIDsReq
 *
 * @brief   This callback is called to process a Get Group Identifiers
 *          Request
 *
 * @param   *srcAddr -- source address of the request
 * @param   *pReq - parsed request payload
 * @param   seqNum - incoming sequence number
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorGetGrpIDsReq(afAddrType_t *srcAddr, bdbTLGetGrpIDsReq_t *pReq, uint8_t seqNum)
{
  // TODO: Implement application behavior
  return ZSuccess;
}

/*********************************************************************
 * @fn      initiatorGetEPListReq
 *
 * @brief   This callback is called to process a Get Endpoint List
 *          Request
 *
 * @param   *srcAddr -- source address of the request
 * @param   *pReq - parsed request payload
 * @param   seqNum - incoming sequence number
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorGetEPListReq(afAddrType_t *srcAddr, bdbTLGetEPListReq_t *pReq, uint8_t SeqNum)
{
  // TODO: Implement application behavior
  return ZSuccess;
}

#endif
/*********************************************************************
*********************************************************************/
