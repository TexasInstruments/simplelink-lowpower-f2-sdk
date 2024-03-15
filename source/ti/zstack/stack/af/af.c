/**************************************************************************************************
  Filename:       af.c
  Revised:        $Date: 2014-11-04 10:53:36 -0800 (Tue, 04 Nov 2014) $
  Revision:       $Revision: 40974 $

  Description:    Application Framework - Device Description helper functions


  Copyright 2004-2014 Texas Instruments Incorporated.

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

/*********************************************************************
 * INCLUDES
 */

#include "ti_zstack_config.h"
#include "rom_jt_154.h"
#include "af.h"
#include "nwk_globals.h"
#include "nwk_util.h"
#include "aps_groups.h"
#include "zd_profile.h"
#include "aps_frag.h"
#include "rtg.h"

#if defined ( MT_AF_CB_FUNC )
  #include "mt_af.h"
#endif

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  #include "stub_aps.h"
#endif

#include "bdb.h"
#if ( BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE)
  #include "bdb_tl_commissioning.h"
#endif

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * @fn      afSend
 *
 * @brief   Helper macro for V1 API to invoke V2 API.
 *
 * input parameters
 *
 * @param  *dstAddr - Full ZB destination address: Nwk Addr + End Point.
 * @param   srcEP - Origination (i.e. respond to or ack to) End Point.
 * @param   cID - A valid cluster ID as specified by the Profile.
 * @param   len - Number of bytes of data pointed to by next param.
 * @param  *buf - A pointer to the data bytes to send.
 * @param   options - Valid bit mask of AF Tx Options as defined in af.h.
 * @param  *transID - A pointer to a byte which can be modified and which will
 *                    be used as the transaction sequence number of the msg.
 *
 * output parameters
 *
 * @param  *transID - Incremented by one if the return value is success.
 *
 * @return  afStatus_t - See previous definition of afStatus_... types.
 */
#define afSend( dstAddr, srcEP, cID, len, buf, transID, options, radius ) \
        AF_DataRequest( (dstAddr), afFindEndPointDesc( (srcEP) ), \
                          (cID), (len), (buf), (transID), (options), (radius) )

/*********************************************************************
 * GLOBAL VARIABLES
 */

epList_t *epList;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void afBuildMSGIncoming( aps_FrameFormat_t *aff, endPointDesc_t *epDesc,
                zAddrType_t *SrcAddress, uint16_t SrcPanId, NLDE_Signal_t *sig,
                uint8_t nwkSeqNum, uint8_t SecurityUse, uint32_t timestamp, uint8_t radius );

static epList_t *afFindEndPointDescList( uint8_t EndPoint );

static pDescCB afGetDescCB( endPointDesc_t *epDesc );

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      afInit
 *
 * @brief   Initialization function for the AF.
 *
 * @param   none
 *
 * @return  none
void afInit( void )
{
}
 */

/*********************************************************************
 * @fn      afRegisterExtended
 *
 * @brief   Register an Application's EndPoint description.
 *
 * @param   epDesc - pointer to the Application's endpoint descriptor.
 * @param   descFn - pointer to descriptor callback function
 * @param   applFn - pointer to the Application callback function
 *
 * NOTE:  The memory that epDesc is pointing to must exist after this call.
 *
 * @return  Pointer to epList_t on success, NULL otherwise.
 */
epList_t *afRegisterExtended( endPointDesc_t *epDesc, pDescCB descFn, pApplCB applFn )
{
  epList_t *ep = OsalPort_malloc(sizeof(epList_t));

  if (ep != NULL)
  {
    ep->nextDesc = epList;
    epList = ep;
    ep->epDesc = epDesc;
    ep->pfnDescCB = descFn;
    ep->apsfCfg.frameDelay = APSF_DEFAULT_INTERFRAME_DELAY;
    ep->apsfCfg.windowSize = APSF_DEFAULT_WINDOW_SIZE;
    ep->flags = eEP_AllowMatch;  // Default to allow Match Descriptor.
    ep->pfnApplCB = applFn;

  #if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
    //Make sure we add at least one application endpoint
    if ((epDesc->endPoint != 0)  && (epDesc->endPoint < BDB_ZIGBEE_RESERVED_ENDPOINTS_START))
    {
      bdb_HeadEpDescriptorList = epList;
      ep->epDesc->epType = bdb_zclFindingBindingEpType(ep->epDesc);
    }

  #endif
#if defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
    // find the first empty entry in the device info table
    for ( uint8_t i = 0; i < 5; i++ )
    {
      if ( touchLinkSubDevicesTbl[i] == NULL )
      {
        touchLinkSubDevicesTbl[i] = OsalPort_malloc(sizeof(bdbTLDeviceInfo_t));
        if ( touchLinkSubDevicesTbl[i] != NULL )
        {
          touchLinkSubDevicesTbl[i]->deviceID = epDesc->simpleDesc->AppDeviceId;
          touchLinkSubDevicesTbl[i]->endpoint = epDesc->simpleDesc->EndPoint;
          touchLinkSubDevicesTbl[i]->profileID = epDesc->simpleDesc->AppProfId;
          touchLinkSubDevicesTbl[i]->version = epDesc->simpleDesc->AppDevVer;
          break;
        }
      }
    }
#endif  // BDB_TL_INITIATOR || BDB_TL_TARGET
  }

  return ep;
}

/*********************************************************************
 * @fn      afRegister
 *
 * @brief   Register an Application's EndPoint description.
 *
 * @param   epDesc - pointer to the Application's endpoint descriptor.
 *
 * NOTE:  The memory that epDesc is pointing to must exist after this call.
 *
 * @return  afStatus_SUCCESS - Registered
 *          afStatus_MEM_FAIL - not enough memory to add descriptor
 *          afStatus_INVALID_PARAMETER - duplicate endpoint
 */
afStatus_t afRegister( endPointDesc_t *epDesc )
{
  if (afFindEndPointDescList(epDesc->endPoint))  // Look for duplicate endpoint.
  {
    return afStatus_INVALID_PARAMETER;
  }

  return ((NULL == afRegisterExtended(epDesc, NULL, NULL)) ? afStatus_MEM_FAIL : afStatus_SUCCESS);
}

/*********************************************************************
 * @fn      afDelete
 *
 * @brief   Delete an Application's EndPoint descriptor and frees the memory
 *
 * @param   EndPoint - Application Endpoint to delete
 *
 * @return  afStatus_SUCCESS - endpoint deleted
 *          afStatus_INVALID_PARAMETER - endpoint not found
 *          afStatus_FAILED - endpoint list empty
 */
afStatus_t afDelete( uint8_t EndPoint )
{
  epList_t *epCurrent;
  epList_t *epPrevious;

  if ( epList != NULL )
  {
    epPrevious = epCurrent = epList;

    // first element of the list matches
    if ( epCurrent->epDesc->endPoint == EndPoint )
    {
      epList = epCurrent->nextDesc;
      OsalPort_free( epCurrent );

      return ( afStatus_SUCCESS );
    }
    else
    {
      // search the list
      for ( epCurrent = epPrevious->nextDesc;
            epCurrent != NULL;
            epCurrent = epCurrent->nextDesc )
      {
        if ( epCurrent->epDesc->endPoint == EndPoint )
        {
          epPrevious->nextDesc = epCurrent->nextDesc;
          OsalPort_free( epCurrent );

          // delete the entry and free the memory
          return ( afStatus_SUCCESS );
        }
        epPrevious = epCurrent;
      }
    }

    // no endpoint found
    return ( afStatus_INVALID_PARAMETER );
  }
  else
  {
    // epList is empty
    return ( afStatus_FAILED );
  }
}

/*********************************************************************
 * @fn          afDataConfirm
 *
 * @brief       This function will generate the Data Confirm back to
 *              the application.
 *
 * @param       endPoint - confirm end point
 * @param       transID - transaction ID from APSDE_DATA_REQUEST
 * @param       status - status of APSDE_DATA_REQUEST
 *
 * @return      none
 */
void afDataConfirm( uint8_t endPoint, uint8_t transID, uint16_t clusterID, ZStatus_t status )
{
  endPointDesc_t *epDesc;
  afDataConfirm_t *msgPtr;

  // Find the endpoint description
  epDesc = afFindEndPointDesc( endPoint );
  if ( epDesc == NULL )
    return;

  // Determine the incoming command type
  msgPtr = (afDataConfirm_t *)OsalPort_msgAllocate( sizeof(afDataConfirm_t) );
  if ( msgPtr )
  {
    // Build the Data Confirm message
    msgPtr->hdr.event = AF_DATA_CONFIRM_CMD;
    msgPtr->hdr.status = status;
    msgPtr->endpoint = endPoint;
    msgPtr->clusterID = clusterID;
    msgPtr->transID = transID;

#if defined ( MT_AF_CB_FUNC )
    /* If MT has subscribed for this callback, don't send as a message. */
    if ( AFCB_CHECK(CB_ID_AF_DATA_CNF,*(epDesc->task_id)) )
    {
      /* Send callback if it's subscribed */
      MT_AfDataConfirm ((void *)msgPtr);
      /* Release the memory. */
      OsalPort_msgDeallocate( (void *)msgPtr );
    }
    else
#endif
    {
      /* send message through task message */
      OsalPort_msgSend( *(epDesc->task_id), (uint8_t *)msgPtr );
    }
  }
}

/*********************************************************************
 * @fn          afReflectError
 *
 * @brief       This function will generate the Reflect Error message to
 *              the application.
 *
 * @param       srcEP - Source Endpoint
 * @param       dstAddrMode - mode of dstAdd - 0 - normal short addr, 1 - group Address
 * @param       dstAddr - intended destination
 * @param       dstEP - Destination Endpoint
 * @param       transID - transaction ID from APSDE_DATA_REQUEST
 * @param       status - status of APSDE_DATA_REQUEST
 *
 * @return      none
 */
void afReflectError( uint8_t srcEP, uint8_t dstAddrMode, uint16_t dstAddr, uint8_t dstEP,
                     uint8_t transID, ZStatus_t status )
{
  endPointDesc_t *epDesc;
  afReflectError_t *msgPtr;

  // Find the endpoint description
  epDesc = afFindEndPointDesc( srcEP );
  if ( epDesc == NULL )
    return;

  // Determine the incoming command type
  msgPtr = (afReflectError_t *)OsalPort_msgAllocate( sizeof(afReflectError_t) );
  if ( msgPtr )
  {
    // Build the Data Confirm message
    msgPtr->hdr.event = AF_REFLECT_ERROR_CMD;
    msgPtr->hdr.status = status;
    msgPtr->endpoint = srcEP;       //As the error is internal the dst endpoint is the endpoint generating the frame (srcEp)
    msgPtr->transID = transID;
    msgPtr->dstAddrMode = dstAddrMode;
    msgPtr->dstAddr = dstAddr;

#if defined ( MT_AF_CB_FUNC )
    /* If MT has subscribed for this callback, don't send as a message. */
    if ( AFCB_CHECK( CB_ID_AF_REFLECT_ERROR, *(epDesc->task_id) ) )
    {
      /* Send callback if it's subscribed */
      MT_AfReflectError( (void *)msgPtr );
      /* Release the memory. */
      OsalPort_msgDeallocate( (void *)msgPtr );
    }
    else
#endif
    {
      /* send message through task message */
      OsalPort_msgSend( *(epDesc->task_id), (uint8_t *)msgPtr );
    }
  }
}

/*********************************************************************
 * @fn          afIncomingData
 *
 * @brief       Transfer a data PDU (ASDU) from the APS sub-layer to the AF.
 *
 * @param       aff  - pointer to APS frame format
 * @param       SrcAddress  - Source address
 * @param       SrcPanId  - Source PAN ID
 * @param       sig - incoming message's link quality
 * @param       nwkSeqNum - incoming network sequence number (from nwk header frame)
 * @param       SecurityUse - Security enable/disable
 * @param       timestamp - the MAC Timer2 timestamp at Rx.
 * @param       radius - incoming messages received radius
 *
 * @return      none
 */
void afIncomingData( aps_FrameFormat_t *aff, zAddrType_t *SrcAddress, uint16_t SrcPanId,
                     NLDE_Signal_t *sig, uint8_t nwkSeqNum, uint8_t SecurityUse,
                     uint32_t timestamp, uint8_t radius )
{
  endPointDesc_t *epDesc = NULL;
  epList_t *pList = epList;
#if !defined ( APS_NO_GROUPS )
  uint8_t grpEp = APS_GROUPS_EP_NOT_FOUND;
#endif

  if ( ((aff->FrmCtrl & APS_DELIVERYMODE_MASK) == APS_FC_DM_GROUP) )
  {
#if !defined ( APS_NO_GROUPS )
    // Find the first endpoint for this group
    grpEp = aps_FindGroupForEndpoint( aff->GroupID, APS_GROUPS_FIND_FIRST );
    if ( grpEp == APS_GROUPS_EP_NOT_FOUND )
      return;   // No endpoint found

    epDesc = afFindEndPointDesc( grpEp );
    if ( epDesc == NULL )
      return;   // Endpoint descriptor not found

    pList = afFindEndPointDescList( epDesc->endPoint );
#else
    return; // Not supported
#endif
  }
  else if ( aff->DstEndPoint == AF_BROADCAST_ENDPOINT )
  {
    // Set the list
    if ( pList != NULL )
    {
      epDesc = pList->epDesc;
    }
  }
  else if ( (epDesc = afFindEndPointDesc( aff->DstEndPoint )) )
  {
    pList = afFindEndPointDescList( epDesc->endPoint );
  }

  while ( epDesc )
  {
    uint16_t epProfileID = 0xFFFE;  // Invalid Profile ID

    if ( pList->pfnDescCB )
    {
      uint16_t *pID = (uint16_t *)(pList->pfnDescCB(
                                 AF_DESCRIPTOR_PROFILE_ID, epDesc->endPoint ));
      if ( pID )
      {
        epProfileID = *pID;
        OsalPort_free( pID );
      }
    }
    else if ( epDesc->simpleDesc )
    {
      epProfileID = epDesc->simpleDesc->AppProfId;
    }

    // First part of verification is to make sure that:
    // the local Endpoint ProfileID matches the received ProfileID OR
    // the message is specifically send to ZDO (this excludes the broadcast endpoint) OR
    // if the Wildcard ProfileID is received the message should not be sent to ZDO endpoint
    if ( (aff->ProfileID == epProfileID) ||
         ((epDesc->endPoint == ZDO_EP) && (aff->ProfileID == ZDO_PROFILE_ID)) ||
         ((epDesc->endPoint != ZDO_EP) && ( aff->ProfileID == ZDO_WILDCARD_PROFILE_ID )) )
    {
      // Save original endpoint
      uint8_t endpoint = aff->DstEndPoint;

      // overwrite with descriptor's endpoint
      aff->DstEndPoint = epDesc->endPoint;

      afBuildMSGIncoming( aff, epDesc, SrcAddress, SrcPanId, sig,
                         nwkSeqNum, SecurityUse, timestamp, radius );

      // Restore with original endpoint
      aff->DstEndPoint = endpoint;
    }

    if ( ((aff->FrmCtrl & APS_DELIVERYMODE_MASK) == APS_FC_DM_GROUP) )
    {
#if !defined ( APS_NO_GROUPS )
      // Find the next endpoint for this group
      grpEp = aps_FindGroupForEndpoint( aff->GroupID, grpEp );
      if ( grpEp == APS_GROUPS_EP_NOT_FOUND )
        return;   // No endpoint found

      epDesc = afFindEndPointDesc( grpEp );
      if ( epDesc == NULL )
        return;   // Endpoint descriptor not found

      pList = afFindEndPointDescList( epDesc->endPoint );
#else
      return;
#endif
    }
    else if ( aff->DstEndPoint == AF_BROADCAST_ENDPOINT )
    {
      pList = pList->nextDesc;
      if ( pList )
        epDesc = pList->epDesc;
      else
        epDesc = NULL;
    }
    else
      epDesc = NULL;
  }
}

/*********************************************************************
 * @fn          afBuildMSGIncoming
 *
 * @brief       Build the message for the app
 *
 * @param
 *
 * @return      pointer to next in data buffer
 */
static void afBuildMSGIncoming( aps_FrameFormat_t *aff, endPointDesc_t *epDesc,
                 zAddrType_t *SrcAddress, uint16_t SrcPanId, NLDE_Signal_t *sig,
                 uint8_t nwkSeqNum, uint8_t SecurityUse, uint32_t timestamp, uint8_t radius )
{
  afIncomingMSGPacket_t *MSGpkt;
  const uint8_t len = sizeof( afIncomingMSGPacket_t ) + aff->asduLength;
  uint8_t *asdu = aff->asdu;
  MSGpkt = (afIncomingMSGPacket_t *)OsalPort_msgAllocate( len );

  if ( MSGpkt == NULL )
  {
    return;
  }

  MSGpkt->hdr.event = AF_INCOMING_MSG_CMD;
  MSGpkt->groupId = aff->GroupID;
  MSGpkt->clusterId = aff->ClusterID;
  afCopyAddress( &MSGpkt->srcAddr, SrcAddress );
  MSGpkt->srcAddr.endPoint = aff->SrcEndPoint;
  MSGpkt->endPoint = epDesc->endPoint;
  MSGpkt->wasBroadcast = aff->wasBroadcast;
  MSGpkt->LinkQuality = sig->LinkQuality;
  MSGpkt->correlation = sig->correlation;
  MSGpkt->rssi = sig->rssi;
  MSGpkt->SecurityUse = SecurityUse;
  MSGpkt->timestamp = timestamp;
  MSGpkt->nwkSeqNum = nwkSeqNum;
  MSGpkt->macSrcAddr = aff->macSrcAddr;
  MSGpkt->macDestAddr = aff->macDestAddr;
  MSGpkt->srcAddr.panId = SrcPanId;
  MSGpkt->cmd.DataLength = aff->asduLength;
  MSGpkt->radius = radius;

  if ( MSGpkt->cmd.DataLength )
  {
    MSGpkt->cmd.Data = (uint8_t *)(MSGpkt + 1);
    OsalPort_memcpy( MSGpkt->cmd.Data, asdu, MSGpkt->cmd.DataLength );
  }
  else
  {
    MSGpkt->cmd.Data = NULL;
  }

#if defined ( MT_AF_CB_FUNC )
  // If ZDO or SAPI have registered for this endpoint, dont intercept it here
  if (AFCB_CHECK(CB_ID_AF_DATA_IND, *(epDesc->task_id)))
  {
    MT_AfIncomingMsg( (void *)MSGpkt );
    // Release the memory.
    OsalPort_msgDeallocate( (void *)MSGpkt );
  }
  else
#endif
  {
    // Send message through task message.
    OsalPort_msgSend( *(epDesc->task_id), (uint8_t *)MSGpkt );
  }
}

/*********************************************************************
 * @fn      AF_DataRequest
 *
 * @brief   Common functionality for invoking APSDE_DataReq() for both
 *          SendMulti and MSG-Send.
 *
 * input parameters
 *
 * @param  *dstAddr - Full ZB destination address: Nwk Addr + End Point.
 * @param  *srcEP - Origination (i.e. respond to or ack to) End Point Descr.
 * @param   cID - A valid cluster ID as specified by the Profile.
 * @param   len - Number of bytes of data pointed to by next param.
 * @param  *buf - A pointer to the data bytes to send.
 * @param  *transID - A pointer to a byte which can be modified and which will
 *                    be used as the transaction sequence number of the msg.
 * @param   options - Valid bit mask of Tx options.
 * @param   radius - Normally set to AF_DEFAULT_RADIUS.
 *
 * output parameters
 *
 * @param  *transID - Incremented by one if the return value is success.
 *
 * @return  afStatus_t - See previous definition of afStatus_... types.
 */
uint8_t AF_DataRequestDiscoverRoute = DISC_ROUTE_NETWORK;
afStatus_t AF_DataRequest( afAddrType_t *dstAddr, endPointDesc_t *srcEP,
                           uint16_t cID, uint16_t len, uint8_t *buf, uint8_t *transID,
                           uint8_t options, uint8_t radius )
{
  pDescCB pfnDescCB;
  ZStatus_t stat;
  APSDE_DataReq_t req;
  afDataReqMTU_t mtu;
  epList_t *pList;

  // Verify source end point
  if ( srcEP == NULL )
  {
    return afStatus_INVALID_PARAMETER;
  }

  // copy the addressing mode, to get the length of the packet
  mtu.aps.addressingMode = dstAddr->addrMode;

  // Check if route is available before sending data
  if ( options & AF_LIMIT_CONCENTRATOR  )
  {
    if ( dstAddr->addrMode != afAddr16Bit )
    {
      return ( afStatus_INVALID_PARAMETER );
    }

    // First, make sure the destination is not its self, then check for an existing route.
    if ( (dstAddr->addr.shortAddr != NLME_GetShortAddr())
        && (RTG_CheckRtStatus( dstAddr->addr.shortAddr, RT_ACTIVE, (MTO_ROUTE | NO_ROUTE_CACHE) ) != RTG_SUCCESS) )
    {
      // A valid route to a concentrator wasn't found
      return ( afStatus_NO_ROUTE );
    }
  }

  // Validate broadcasting
  if ( ( dstAddr->addrMode == afAddr16Bit     ) ||
       ( dstAddr->addrMode == afAddrBroadcast )    )
  {
    // Check for valid broadcast values
    if( ADDR_NOT_BCAST != NLME_IsAddressBroadcast( dstAddr->addr.shortAddr )  )
    {
      // Force mode to broadcast
      dstAddr->addrMode = afAddrBroadcast;
    }
    else
    {
      // Address is not a valid broadcast type
      if ( dstAddr->addrMode == afAddrBroadcast )
      {
        return afStatus_INVALID_PARAMETER;
      }
    }
  }
  else if ( dstAddr->addrMode != afAddr64Bit &&
            dstAddr->addrMode != afAddrGroup &&
            dstAddr->addrMode != afAddrNotPresent )
  {
    return afStatus_INVALID_PARAMETER;
  }

  // Set destination address
  req.dstAddr.addrMode = dstAddr->addrMode;
  if ( dstAddr->addrMode == afAddr64Bit )
  {
    osal_cpyExtAddr( req.dstAddr.addr.extAddr, dstAddr->addr.extAddr );
  }
  else
  {
    req.dstAddr.addr.shortAddr = dstAddr->addr.shortAddr;
  }

  // This option is to use Wildcard ProfileID in outgoing packets
  if ( options & AF_WILDCARD_PROFILEID )
  {
    req.profileID = ZDO_WILDCARD_PROFILE_ID;
  }
  else
  {
    req.profileID = ZDO_PROFILE_ID;

    if ( (pfnDescCB = afGetDescCB( srcEP )) )
    {
      uint16_t *pID = (uint16_t *)(pfnDescCB(
                                   AF_DESCRIPTOR_PROFILE_ID, srcEP->endPoint ));
      if ( pID )
      {
        req.profileID = *pID;
        OsalPort_free( pID );
      }
    }
    else if ( srcEP->simpleDesc )
    {
      req.profileID = srcEP->simpleDesc->AppProfId;
    }
  }

  req.txOptions = 0;

  if ( ( options & AF_ACK_REQUEST              ) &&
       ( req.dstAddr.addrMode != AddrBroadcast ) &&
       ( req.dstAddr.addrMode != AddrGroup     )    )
  {
    req.txOptions |=  APS_TX_OPTIONS_ACK;
  }

  if ( options & AF_SKIP_ROUTING )
  {
    req.txOptions |=  APS_TX_OPTIONS_SKIP_ROUTING;
  }

  if ( options & AF_EN_SECURITY )
  {
    req.txOptions |= APS_TX_OPTIONS_SECURITY_ENABLE;
    mtu.aps.secure = TRUE;
  }
  else
  {
    mtu.aps.secure = FALSE;
  }

  if ( options & AF_PREPROCESS )
  {
    req.txOptions |=  APS_TX_OPTIONS_PREPROCESS;
  }

  mtu.kvp = FALSE;

  if ( options & AF_SUPRESS_ROUTE_DISC_NETWORK )
  {
    req.discoverRoute = DISC_ROUTE_INITIATE;
  }
  else
  {
    req.discoverRoute = AF_DataRequestDiscoverRoute;
  }

  req.transID       = *transID;
  req.srcEP         = srcEP->endPoint;
  req.dstEP         = dstAddr->endPoint;
  req.clusterID     = cID;
  req.asduLen       = len;
  req.asdu          = buf;
  req.radiusCounter = radius;
#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  req.dstPanId      = dstAddr->panId;
#endif // INTER_PAN || BDB_TL_INITIATOR || BDB_TL_TARGET

  // Look if there is a Callback function registered for this endpoint
  // The callback is used to control the AF Transaction ID used when sending messages
  pList = afFindEndPointDescList( srcEP->endPoint );

  if ( ( pList != NULL ) && ( pList->pfnApplCB != NULL ) )
  {
    pList->pfnApplCB( &req );
  }

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  if ( StubAPS_InterPan( dstAddr->panId, dstAddr->endPoint ) )
  {
    if ( len > INTERP_DataReqMTU() )
    {
      stat = afStatus_INVALID_PARAMETER;
    }
    else
    {
      stat = INTERP_DataReq( &req );
    }
  }
  else
#endif // INTER_PAN || BDB_TL_INITIATOR || BDB_TL_TARGET
  {
    if (len > afDataReqMTU( &mtu ) )
    {
      if (apsfSendFragmented)
      {
        stat = (*apsfSendFragmented)( &req );
      }
      else
      {
        stat = afStatus_INVALID_PARAMETER;
      }
    }
    else
    {
      stat = APSDE_DataReq( &req );
    }
  }

  /*
   * If this is an EndPoint-to-EndPoint message on the same device, it will not
   * get added to the NWK databufs. So it will not go OTA and it will not get
   * a MACCB_DATA_CONFIRM_CMD callback. Thus it is necessary to generate the
   * AF_DATA_CONFIRM_CMD here. Note that APSDE_DataConfirm() only generates one
   * message with the first in line TransSeqNumber, even on a multi message.
   * Also note that a reflected msg will not have its confirmation generated
   * here.
   */
  if ( (req.dstAddr.addrMode == Addr16Bit) &&
       (req.dstAddr.addr.shortAddr == NLME_GetShortAddr()) )
  {
    afDataConfirm( srcEP->endPoint, *transID, cID, stat );
  }

  if ( stat == afStatus_SUCCESS )
  {
    (*transID)++;
  }

  return (afStatus_t)stat;
}

/*********************************************************************
 * @fn      AF_DataRequestSrcRtg
 *
 * @brief   Common functionality for invoking APSDE_DataReq() for both
 *          SendMulti and MSG-Send.
 *
 * input parameters
 *
 * @param  *dstAddr - Full ZB destination address: Nwk Addr + End Point.
 * @param  *srcEP - Origination (i.e. respond to or ack to) End Point Descr.
 * @param   cID - A valid cluster ID as specified by the Profile.
 * @param   len - Number of bytes of data pointed to by next param.
 * @param  *buf - A pointer to the data bytes to send.
 * @param  *transID - A pointer to a byte which can be modified and which will
 *                    be used as the transaction sequence number of the msg.
 * @param   options - Valid bit mask of Tx options.
 * @param   radius - Normally set to AF_DEFAULT_RADIUS.
 * @param   relayCnt - Number of devices in the relay list
 * @param   pRelayList - Pointer to the relay list
 *
 * output parameters
 *
 * @param  *transID - Incremented by one if the return value is success.
 *
 * @return  afStatus_t - See previous definition of afStatus_... types.
 */

afStatus_t AF_DataRequestSrcRtg( afAddrType_t *dstAddr, endPointDesc_t *srcEP,
                           uint16_t cID, uint16_t len, uint8_t *buf, uint8_t *transID,
                           uint8_t options, uint8_t radius, uint8_t relayCnt, uint16_t* pRelayList )
{
  uint8_t status;

  /* Add the source route to the source routing table */
  status = RTG_AddSrcRtgEntry_Guaranteed( dstAddr->addr.shortAddr, relayCnt,
                                         pRelayList );

  if( status == RTG_SUCCESS)
  {
    /* Call AF_DataRequest to send the data */
    status = AF_DataRequest( dstAddr, srcEP, cID, len, buf, transID, options, radius );
  }
  else if( status == RTG_INVALID_PATH )
  {
    /* The source route relay count is exceeding the network limit */
    status = afStatus_INVALID_PARAMETER;
  }
  else
  {
    /* The guaranteed adding entry fails due to memory failure */
    status = afStatus_MEM_FAIL;
  }
  return status;
}

/*********************************************************************
 * @fn      afFindEndPointDescList
 *
 * @brief   Find the endpoint description entry from the endpoint
 *          number.
 *
 * @param   EndPoint - Application Endpoint to look for
 *
 * @return  the address to the endpoint/interface description entry
 */
static epList_t *afFindEndPointDescList( uint8_t EndPoint )
{
  epList_t *epSearch;

  for (epSearch = epList; epSearch != NULL; epSearch = epSearch->nextDesc)
  {
    if (epSearch->epDesc->endPoint == EndPoint)
    {
      break;
    }
  }

  return epSearch;
}

/*********************************************************************
 * @fn      afFindEndPointDesc
 *
 * @brief   Find the endpoint description entry from the endpoint
 *          number.
 *
 * @param   EndPoint - Application Endpoint to look for
 *
 * @return  the address to the endpoint/interface description entry
 */
endPointDesc_t *afFindEndPointDesc( uint8_t EndPoint )
{
  epList_t *epSearch;

  // Look for the endpoint
  epSearch = afFindEndPointDescList( EndPoint );

  if ( epSearch )
    return ( epSearch->epDesc );
  else
    return ( (endPointDesc_t *)NULL );
}

/*********************************************************************
 * @fn      afFindSimpleDesc
 *
 * @brief   Find the Simple Descriptor from the endpoint number.
 *
 * @param   EP - Application Endpoint to look for.
 *
 * @return  Non-zero to indicate that the descriptor memory must be freed.
 */
uint8_t afFindSimpleDesc( SimpleDescriptionFormat_t **ppDesc, uint8_t EP )
{
  epList_t *epItem = afFindEndPointDescList( EP );
  uint8_t rtrn = FALSE;

  if ( epItem )
  {
    if ( epItem->pfnDescCB )
    {
      *ppDesc = epItem->pfnDescCB( AF_DESCRIPTOR_SIMPLE, EP );
      rtrn = TRUE;
    }
    else
    {
      *ppDesc = epItem->epDesc->simpleDesc;
    }
  }
  else
  {
    *ppDesc = NULL;
  }

  return rtrn;
}

/*********************************************************************
 * @fn      afGetDescCB
 *
 * @brief   Get the Descriptor callback function.
 *
 * @param   epDesc - pointer to the endpoint descriptor
 *
 * @return  function pointer or NULL
 */
static pDescCB afGetDescCB( endPointDesc_t *epDesc )
{
  epList_t *epSearch;

  // Start at the beginning
  epSearch = epList;

  // Look through the list until the end
  while ( epSearch )
  {
    // Is there a match?
    if ( epSearch->epDesc == epDesc )
    {
      return ( epSearch->pfnDescCB );
    }
    else
      epSearch = epSearch->nextDesc;  // Next entry
  }

  return ( (pDescCB)NULL );
}

/*********************************************************************
 * @fn      afDataReqMTU
 *
 * @brief   Get the Data Request MTU(Max Transport Unit).
 *
 * @param   fields - afDataReqMTU_t
 *
 * @return  uint8_t(MTU)
 */
uint8_t afDataReqMTU( afDataReqMTU_t* fields )
{
  uint8_t len;
  uint8_t hdr;

  if ( fields->kvp == TRUE )
  {
    hdr = AF_HDR_KVP_MAX_LEN;
  }
  else
  {
    hdr = AF_HDR_V1_1_MAX_LEN;
  }

  len = (uint8_t)(APSDE_DataReqMTU(&fields->aps) - hdr);

  return len;
}

/*********************************************************************
 * @fn      afGetMatch
 *
 * @brief   Set the allow response flag.
 *
 * @param   ep - Application Endpoint to look for
 * @param   action - true - allow response, false - no response
 *
 * @return  TRUE allow responses, FALSE no response
 */
uint8_t afGetMatch( uint8_t ep )
{
  epList_t *epSearch;

  // Look for the endpoint
  epSearch = afFindEndPointDescList( ep );

  if ( epSearch )
  {
    if ( epSearch->flags & eEP_AllowMatch )
      return ( TRUE );
    else
      return ( FALSE );
  }
  else
    return ( FALSE );
}

/*********************************************************************
 * @fn      afSetMatch
 *
 * @brief   Set the allow response flag.
 *
 * @param   ep - Application Endpoint to look for
 * @param   action - true - allow response, false - no response
 *
 * @return  TRUE if success, FALSE if endpoint not found
 */
uint8_t afSetMatch( uint8_t ep, uint8_t action )
{
  epList_t *epSearch;

  // Look for the endpoint
  epSearch = afFindEndPointDescList( ep );

  if ( epSearch )
  {
    if ( action )
    {
      epSearch->flags |= eEP_AllowMatch;
    }
    else
    {
      epSearch->flags &= (eEP_AllowMatch ^ 0xFFFF);
    }
    return ( TRUE );
  }
  else
    return ( FALSE );
}

/*********************************************************************
 * @fn      afNumEndPoints
 *
 * @brief   Returns the number of endpoints defined (including 0)
 *
 * @param   none
 *
 * @return  number of endpoints
 */
uint8_t afNumEndPoints( void )
{
  epList_t *epSearch;
  uint8_t endpoints;

  // Start at the beginning
  epSearch = epList;
  endpoints = 0;

  while ( epSearch )
  {
    endpoints++;
    epSearch = epSearch->nextDesc;
  }

  return ( endpoints );
}

/*********************************************************************
 * @fn      afEndPoints
 *
 * @brief   Fills in the passed in buffer with the endpoint (numbers).
 *          Use afNumEndPoints to find out how big a buffer to supply.
 *
 * @param   epBuf - pointer to mem used
 *
 * @return  void
 */
void afEndPoints( uint8_t *epBuf, uint8_t skipZDO )
{
  epList_t *epSearch;
  uint8_t endPoint;

  // Start at the beginning
  epSearch = epList;

  while ( epSearch )
  {
    endPoint = epSearch->epDesc->endPoint;

    if ( !skipZDO || endPoint != 0 )
      *epBuf++ = endPoint;

    epSearch = epSearch->nextDesc;
  }
}

/*********************************************************************
 * @fn      afCopyAddress
 *
 * @brief   Fills in the passed in afAddrType_t parameter with the corresponding information
 *          from the zAddrType_t parameter.
 *
 * @param   epBuf - pointer to mem used
 *
 * @return  void
 */
void afCopyAddress( afAddrType_t *afAddr, zAddrType_t *zAddr )
{
  afAddr->addrMode = (afAddrMode_t)zAddr->addrMode;
  if ( zAddr->addrMode == Addr64Bit )
  {
    (void)osal_cpyExtAddr( afAddr->addr.extAddr, zAddr->addr.extAddr );
  }
  else
  {
    afAddr->addr.shortAddr = zAddr->addr.shortAddr;
  }

  // Since zAddrType_t has no INTER-PAN information, set the panId member to zero.
  afAddr->panId = 0;
}

/**************************************************************************************************
 * @fn          afAPSF_ConfigGet
 *
 * @brief       This function ascertains the fragmentation configuration that corresponds to
 *              the specified EndPoint.
 *
 * input parameters
 *
 * @param       endPoint - The source EP of a Tx or destination EP of a Rx fragmented message.
 *
 * output parameters
 *
 * @param       pCfg - A pointer to an APSF configuration structure to fill with values.
 *
 * @return      None.
 */
void afAPSF_ConfigGet(uint8_t endPoint, afAPSF_Config_t *pCfg)
{
  epList_t *pList = afFindEndPointDescList(endPoint);

  if (pList == NULL)
  {
    pCfg->frameDelay = APSF_DEFAULT_INTERFRAME_DELAY;
    pCfg->windowSize = APSF_DEFAULT_WINDOW_SIZE;
  }
  else
  {
    (void)OsalPort_memcpy(pCfg, &pList->apsfCfg, sizeof(afAPSF_Config_t));
  }
}

/**************************************************************************************************
 * @fn          afAPSF_ConfigSet
 *
 * @brief       This function attempts to set the fragmentation configuration that corresponds to
 *              the specified EndPoint.
 *
 * input parameters
 *
 * @param       endPoint - The specific EndPoint for which to set the fragmentation configuration.
 * @param       pCfg - A pointer to an APSF configuration structure to fill with values.
 *
 * output parameters
 *
 * None.
 *
 * @return      afStatus_SUCCESS for success.
 *              afStatus_INVALID_PARAMETER if the specified EndPoint is not registered.
 */
afStatus_t afAPSF_ConfigSet(uint8_t endPoint, afAPSF_Config_t *pCfg)
{
  epList_t *pList = afFindEndPointDescList(endPoint);

  if (pList == NULL)
  {
    return afStatus_INVALID_PARAMETER;
  }

  (void)OsalPort_memcpy(&pList->apsfCfg, pCfg, sizeof(afAPSF_Config_t));
  return afStatus_SUCCESS;
}

/**************************************************************************************************
 * @fn          afSetApplCB
 *
 * @brief       Sets the pointer to the Application Callback function for a
 *              specific EndPoint.
 *
 * input parameters
 *
 * @param       endPoint - The specific EndPoint for which to set Application Callback.
 * @param       pApplFn - A pointer to the Application Callback function.
 *
 * output parameters
 *
 * None.
 *
 * @return      TRUE if success, FALSE if endpoint not found
 */
uint8_t afSetApplCB( uint8_t endPoint, pApplCB pApplFn )
{
  if ( pApplFn != NULL )
  {
    epList_t *epSearch;

    // Look for the endpoint
    epSearch = afFindEndPointDescList( endPoint );

    if ( epSearch )
    {
      epSearch->pfnApplCB = pApplFn;

      return ( TRUE );
    }
  }

  return ( FALSE );
}

/**************************************************************************************************
*/
