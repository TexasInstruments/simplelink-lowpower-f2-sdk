/**************************************************************************************************
  Filename:       zcl_ll.c
  Revised:        $Date: 2013-09-10 17:57:03 -0700 (Tue, 10 Sep 2013) $
  Revision:       $Revision: 35271 $

  Description:    Zigbee Cluster Library - Light Link commissioning cluster.


  Copyright (c) 2019, Texas Instruments Incorporated
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  *  Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

  *  Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

  *  Neither the name of Texas Instruments Incorporated nor the names of
      its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************/


/*********************************************************************
 * INCLUDES
 */
#include "ti_zstack_config.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ll.h"

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  #include "stub_aps.h"
#endif

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

typedef struct zclLLCBRec
{
  struct zclLLCBRec     *next;
  uint8_t endpoint;               // Used to link it into the endpoint descriptor
  zclLL_AppCallbacks_t  *CBs;   // Pointer to Callback function
} zclLLCBRec_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static zclLLCBRec_t *zclLLCBs = (zclLLCBRec_t *)NULL;
static uint8_t zclLLPluginRegisted = FALSE;


#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
static zclLL_InterPANCallbacks_t *pInterPANCBs = (zclLL_InterPANCallbacks_t *)NULL;
#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static zclLL_AppCallbacks_t *zclLL_FindCallbacks( uint8_t endpoint );
static ZStatus_t zclLL_HdlIncoming( zclIncoming_t *pInMsg );
static ZStatus_t zclLL_HdlInSpecificCommands( zclIncoming_t *pInMsg );
static ZStatus_t zclLL_ProcessInLLCmds( zclIncoming_t *pInMsg, zclLL_AppCallbacks_t *pCBs );

static ZStatus_t zclLL_ProcessInCmd_GetGrpIDsReq( zclIncoming_t *pInMsg, zclLL_AppCallbacks_t *pCBs );
static ZStatus_t zclLL_ProcessInCmd_GetEPListReq( zclIncoming_t *pInMsg, zclLL_AppCallbacks_t *pCBs );

static ZStatus_t zclLL_ProcessInCmd_EndpointInfo( zclIncoming_t *pInMsg, zclLL_AppCallbacks_t *pCBs );
static ZStatus_t zclLL_ProcessInCmd_GetGrpIDsRsp( zclIncoming_t *pInMsg, zclLL_AppCallbacks_t *pCBs );
static ZStatus_t zclLL_ProcessInCmd_GetEPListRsp( zclIncoming_t *pInMsg, zclLL_AppCallbacks_t *pCBs );

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
static ZStatus_t zclLL_HdlInInterPANCommands( zclIncoming_t *pInMsg );

static ZStatus_t zclLL_ProcessInLLInterPANCmds( zclIncoming_t *pInMsg );

static void zclLL_ParseInCmd_NwkJoinReq( uint8_t *pBuf, zclLLNwkJoinReq_t *pReq );

static ZStatus_t zclLL_ProcessInCmd_ScanReq( zclIncoming_t *pInMsg );
static ZStatus_t zclLL_ProcessInCmd_DeviceInfoReq( zclIncoming_t *pInMsg );
static ZStatus_t zclLL_ProcessInCmd_IdentifyReq( zclIncoming_t *pInMsg );
static ZStatus_t zclLL_ProcessInCmd_ResetToFNReq( zclIncoming_t *pInMsg );
static ZStatus_t zclLL_ProcessInCmd_NwkStartReq( zclIncoming_t *pInMsg );
static ZStatus_t zclLL_ProcessInCmd_NwkJoinRtrReq( zclIncoming_t *pInMsg );
static ZStatus_t zclLL_ProcessInCmd_NwkJoinEDReq( zclIncoming_t *pInMsg );
static ZStatus_t zclLL_ProcessInCmd_NwkUpdateReq( zclIncoming_t *pInMsg );

static ZStatus_t zclLL_ProcessInCmd_ScanRsp( zclIncoming_t *pInMsg );
static ZStatus_t zclLL_ProcessInCmd_DeviceInfoRsp( zclIncoming_t *pInMsg );
static ZStatus_t zclLL_ProcessInCmd_NwkStartRsp( zclIncoming_t *pInMsg );
static ZStatus_t zclLL_ProcessInCmd_NwkJoinRtrRsp( zclIncoming_t *pInMsg );
static ZStatus_t zclLL_ProcessInCmd_NwkJoinEDRsp( zclIncoming_t *pInMsg );
#endif

/*********************************************************************
 * @fn      zclLL_RegisterCmdCallbacks
 *
 * @brief   Register an applications command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
ZStatus_t zclLL_RegisterCmdCallbacks( uint8_t endpoint, zclLL_AppCallbacks_t *callbacks )
{
  zclLLCBRec_t *pNewItem;
  zclLLCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( !zclLLPluginRegisted )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_TOUCHLINK,
                        ZCL_CLUSTER_ID_TOUCHLINK,
                        zclLL_HdlIncoming );
    zclLLPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclLLCBRec_t ) );
  if ( pNewItem == NULL )
    return (ZMemError);

  pNewItem->next = (zclLLCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclLLCBs == NULL )
  {
    zclLLCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclLLCBs;
    while ( pLoop->next != NULL )
      pLoop = pLoop->next;

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclLL_RegisterInterPANCmdCallbacks
 *
 * @brief   Register an applications Inter-PAN command callbacks
 *
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZSuccess
 */
ZStatus_t zclLL_RegisterInterPANCmdCallbacks( zclLL_InterPANCallbacks_t *callbacks )
{
  // Register as a ZCL Plugin
  if ( !zclLLPluginRegisted )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_TOUCHLINK,
                        ZCL_CLUSTER_ID_TOUCHLINK,
                        zclLL_HdlIncoming );
    zclLLPluginRegisted = TRUE;
  }
#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  pInterPANCBs = callbacks;
#endif

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclLL_Send_ScanReq
 *
 * @brief   Call to send out an Scan Request command
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pReq - request parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLL_Send_ScanReq( uint8_t srcEP, afAddrType_t *dstAddr,
                              zclLLScanReq_t *pReq, uint8_t seqNum )
{
  uint8_t buf[ZLL_CMDLEN_SCAN_REQ];

  VOID zcl_buffer_uint32( buf, pReq->transID );

  buf[4] = pReq->zInfo.zInfoByte;
  buf[5] = pReq->zllInfo.zllInfoByte;

  return zclLL_SendInterPANCommand( srcEP, dstAddr, COMMAND_ZLL_SCAN_REQ,
                                    ZCL_FRAME_CLIENT_SERVER_DIR, seqNum, ZLL_CMDLEN_SCAN_REQ, buf );
}

/*********************************************************************
 * @fn      zclLL_Send_DeviceInfoReq
 *
 * @brief   Call to send out a Device Information Request command
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pReq - request parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLL_Send_DeviceInfoReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zclLLDeviceInfoReq_t *pReq, uint8_t seqNum )
{
  uint8_t buf[ZLL_CMDLEN_DEVICE_INFO_REQ];

  VOID zcl_buffer_uint32( buf, pReq->transID );

  buf[4] = pReq->startIndex;

  return zclLL_SendInterPANCommand( srcEP, dstAddr, COMMAND_ZLL_DEVICE_INFO_REQ,
                                    ZCL_FRAME_CLIENT_SERVER_DIR, seqNum, ZLL_CMDLEN_DEVICE_INFO_REQ, buf );
}

/*********************************************************************
 * @fn      zclLL_Send_IndentifyReq
 *
 * @brief   Call to send out a Identify Request command
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pReq - request parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLL_Send_IndentifyReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                   zclLLIdentifyReq_t *pReq, uint8_t seqNum )
{
  uint8_t buf[ZLL_CMDLEN_IDENTIFY_REQ];

  VOID zcl_buffer_uint32( buf, pReq->transID );

  buf[4] = LO_UINT16( pReq->IdDuration );
  buf[5] = HI_UINT16( pReq->IdDuration );

  return zclLL_SendInterPANCommand( srcEP, dstAddr, COMMAND_ZLL_IDENTIFY_REQ,
                                    ZCL_FRAME_CLIENT_SERVER_DIR, seqNum, ZLL_CMDLEN_IDENTIFY_REQ, buf );
}

/*********************************************************************
 * @fn      zclLL_Send_ResetToFNReq
 *
 * @brief   Call to send out a Reset to Factory New Request command
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pReq - request parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLL_Send_ResetToFNReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                   zclLLResetToFNReq_t *pReq, uint8_t seqNum )
{
  uint8_t buf[ZLL_CMDLEN_RESET_TO_FN_REQ];

  VOID zcl_buffer_uint32( buf, pReq->transID );

  return zclLL_SendInterPANCommand( srcEP, dstAddr, COMMAND_ZLL_RESET_TO_FN_REQ,
                                    ZCL_FRAME_CLIENT_SERVER_DIR, seqNum, ZLL_CMDLEN_RESET_TO_FN_REQ, buf );
}

/*********************************************************************
 * @fn      zclLL_Send_NwkStartReq
 *
 * @brief   Call to send out a Network Start Request command
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pRsp - response parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLL_Send_NwkStartReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                  zclLLNwkStartReq_t *pRsp, uint8_t seqNum )
{
  uint8_t *buf;
  uint8_t *pBuf;
  ZStatus_t status;

  buf = zcl_mem_alloc( ZLL_CMDLEN_NWK_START_REQ );
  if ( buf != NULL )
  {
    pBuf = buf;

    pBuf = zcl_buffer_uint32( pBuf, pRsp->transID );
    pBuf = zcl_cpyExtAddr( pBuf, pRsp->nwkParams.extendedPANID );

    *pBuf++ = pRsp->nwkParams.keyIndex;

    pBuf = zcl_memcpy( pBuf, pRsp->nwkParams.nwkKey, SEC_KEY_LEN );

    *pBuf++ = pRsp->nwkParams.logicalChannel;

    *pBuf++ = LO_UINT16( pRsp->nwkParams.panId );
    *pBuf++ = HI_UINT16( pRsp->nwkParams.panId );

    *pBuf++ = LO_UINT16( pRsp->nwkParams.nwkAddr );
    *pBuf++ = HI_UINT16( pRsp->nwkParams.nwkAddr );

    *pBuf++ = LO_UINT16( pRsp->nwkParams.grpIDsBegin );
    *pBuf++ = HI_UINT16( pRsp->nwkParams.grpIDsBegin );

    *pBuf++ = LO_UINT16( pRsp->nwkParams.grpIDsEnd );
    *pBuf++ = HI_UINT16( pRsp->nwkParams.grpIDsEnd );

    *pBuf++ = LO_UINT16( pRsp->nwkParams.freeNwkAddrBegin );
    *pBuf++ = HI_UINT16( pRsp->nwkParams.freeNwkAddrBegin );

    *pBuf++ = LO_UINT16( pRsp->nwkParams.freeNwkAddrEnd );
    *pBuf++ = HI_UINT16( pRsp->nwkParams.freeNwkAddrEnd );

    *pBuf++ = LO_UINT16( pRsp->nwkParams.freeGrpIDBegin );
    *pBuf++ = HI_UINT16( pRsp->nwkParams.freeGrpIDBegin );

    *pBuf++ = LO_UINT16( pRsp->nwkParams.freeGrpIDEnd );
    *pBuf++ = HI_UINT16( pRsp->nwkParams.freeGrpIDEnd );

    pBuf = zcl_cpyExtAddr( pBuf, pRsp->initiatorIeeeAddr );

    *pBuf++ = LO_UINT16( pRsp->initiatorNwkAddr );
    *pBuf++ = HI_UINT16( pRsp->initiatorNwkAddr );

    status = zclLL_SendInterPANCommand( srcEP, dstAddr, COMMAND_ZLL_NWK_START_REQ,
                                        ZCL_FRAME_CLIENT_SERVER_DIR, seqNum, ZLL_CMDLEN_NWK_START_REQ, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_Send_NwkJoinReq
 *
 * @brief   Call to send out a Network Join Router/End Device Request command
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pRsp - response parameters
 * @param   cmd - COMMAND_ZLL_NWK_JOIN_RTR_REQ or COMMAND_ZLL_NWK_JOIN_ED_REQ
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLL_Send_NwkJoinReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                 zclLLNwkJoinReq_t *pRsp, uint8_t cmd, uint8_t seqNum )
{
  uint8_t *buf;
  uint8_t *pBuf;
  ZStatus_t status;

  buf = zcl_mem_alloc( ZLL_CMDLEN_NWK_JOIN_REQ );
  if ( buf != NULL )
  {
    pBuf = buf;

    pBuf = zcl_buffer_uint32( pBuf, pRsp->transID );
    pBuf = zcl_cpyExtAddr( pBuf, pRsp->nwkParams.extendedPANID );

    *pBuf++ = pRsp->nwkParams.keyIndex;

    pBuf = zcl_memcpy( pBuf, pRsp->nwkParams.nwkKey, SEC_KEY_LEN );

    *pBuf++ = pRsp->nwkUpdateId;
    *pBuf++ = pRsp->nwkParams.logicalChannel;

    *pBuf++ = LO_UINT16( pRsp->nwkParams.panId );
    *pBuf++ = HI_UINT16( pRsp->nwkParams.panId );

    *pBuf++ = LO_UINT16( pRsp->nwkParams.nwkAddr );
    *pBuf++ = HI_UINT16( pRsp->nwkParams.nwkAddr );

    *pBuf++ = LO_UINT16( pRsp->nwkParams.grpIDsBegin );
    *pBuf++ = HI_UINT16( pRsp->nwkParams.grpIDsBegin );

    *pBuf++ = LO_UINT16( pRsp->nwkParams.grpIDsEnd );
    *pBuf++ = HI_UINT16( pRsp->nwkParams.grpIDsEnd );

    *pBuf++ = LO_UINT16( pRsp->nwkParams.freeNwkAddrBegin );
    *pBuf++ = HI_UINT16( pRsp->nwkParams.freeNwkAddrBegin );

    *pBuf++ = LO_UINT16( pRsp->nwkParams.freeNwkAddrEnd );
    *pBuf++ = HI_UINT16( pRsp->nwkParams.freeNwkAddrEnd );

    *pBuf++ = LO_UINT16( pRsp->nwkParams.freeGrpIDBegin );
    *pBuf++ = HI_UINT16( pRsp->nwkParams.freeGrpIDBegin );

    *pBuf++ = LO_UINT16( pRsp->nwkParams.freeGrpIDEnd );
    *pBuf++ = HI_UINT16( pRsp->nwkParams.freeGrpIDEnd );

    status = zclLL_SendInterPANCommand( srcEP, dstAddr, cmd, ZCL_FRAME_CLIENT_SERVER_DIR, seqNum,
                                        ZLL_CMDLEN_NWK_JOIN_REQ, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_Send_NwkUpdateReq
 *
 * @brief   Call to send out a Network Update Request command
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pReq - response parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLL_Send_NwkUpdateReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                   zclLLNwkUpdateReq_t *pReq, uint8_t seqNum )
{
  uint8_t *buf;
  uint8_t *pBuf;
  ZStatus_t status;

  buf = zcl_mem_alloc( ZLL_CMDLEN_NWK_UPDATE_REQ );
  if ( buf != NULL )
  {
    pBuf = buf;

    pBuf = zcl_buffer_uint32( pBuf, pReq->transID );
    pBuf = zcl_cpyExtAddr( pBuf, pReq->extendedPANID );

    *pBuf++ = pReq->nwkUpdateId;
    *pBuf++ = pReq->logicalChannel;

    *pBuf++ = LO_UINT16( pReq->PANID );
    *pBuf++ = HI_UINT16( pReq->PANID );

    *pBuf++ = LO_UINT16( pReq->nwkAddr );
    *pBuf++ = HI_UINT16( pReq->nwkAddr );

    status = zclLL_SendInterPANCommand( srcEP, dstAddr, COMMAND_ZLL_NWK_UPDATE_REQ,
                                        ZCL_FRAME_CLIENT_SERVER_DIR, seqNum, ZLL_CMDLEN_NWK_UPDATE_REQ, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_Send_ScanRsp
 *
 * @brief   Call to send out an Scan Response command
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pRsp - response parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLL_Send_ScanRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                              zclLLScanRsp_t *pRsp, uint8_t seqNum )
{
  uint8_t *buf;
  uint8_t bufLen = ZLL_CMDLEN_SCAN_RSP;
  ZStatus_t status;

  // Calculate the total length needed
  if ( pRsp->numSubDevices == 1 )
  {
    bufLen += ZLL_CMDLENOPTIONAL_SCAN_RSP;
  }

  buf = zcl_mem_alloc( bufLen );
  if ( buf != NULL )
  {
    uint8_t *pBuf = buf;

    pBuf = zcl_buffer_uint32( pBuf, pRsp->transID );

    *pBuf++ = pRsp->rssiCorrection;
    *pBuf++ = pRsp->zInfo.zInfoByte;
    *pBuf++ = pRsp->zllInfo.zllInfoByte;

    *pBuf++ = LO_UINT16( pRsp->keyBitmask );
    *pBuf++ = HI_UINT16( pRsp->keyBitmask );

    pBuf = zcl_buffer_uint32( pBuf, pRsp->responseID );
    pBuf = zcl_cpyExtAddr( pBuf, pRsp->extendedPANID );

    *pBuf++ = pRsp->nwkUpdateId;
    *pBuf++ = pRsp->logicalChannel;

    *pBuf++ = LO_UINT16( pRsp->PANID );
    *pBuf++ = HI_UINT16( pRsp->PANID );

    *pBuf++ = LO_UINT16( pRsp->nwkAddr );
    *pBuf++ = HI_UINT16( pRsp->nwkAddr );

    *pBuf++ = pRsp->numSubDevices;
    *pBuf++ = pRsp->totalGrpIDs;

    if ( pRsp->numSubDevices == 1 )
    {
      *pBuf++ = pRsp->deviceInfo.endpoint;

      *pBuf++ = LO_UINT16( pRsp->deviceInfo.profileID );
      *pBuf++ = HI_UINT16( pRsp->deviceInfo.profileID );

      *pBuf++ = LO_UINT16( pRsp->deviceInfo.deviceID );
      *pBuf++ = HI_UINT16( pRsp->deviceInfo.deviceID );

      *pBuf++ = pRsp->deviceInfo.version;
      *pBuf++ = pRsp->deviceInfo.grpIdCnt;
    }

    status = zclLL_SendInterPANCommand( srcEP, dstAddr, COMMAND_ZLL_SCAN_RSP,
                                        ZCL_FRAME_SERVER_CLIENT_DIR, seqNum, bufLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_Send_DeviceInfoRsp
 *
 * @brief   Call to send out a Device Information Response command
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pRsp - response parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLL_Send_DeviceInfoRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zclLLDeviceInfoRsp_t *pRsp, uint8_t seqNum )
{
  uint8_t *buf;
  uint8_t bufLen = ZLL_CMDLEN_DEVICE_INFO_RSP;
  ZStatus_t status;

  // Calculate the total length needed
  bufLen += ( pRsp->cnt * ZLL_CMDLENOPTIONAL_DEVICE_INFO_RSP );

  buf = zcl_mem_alloc( bufLen );
  if ( buf != NULL )
  {
    uint8_t *pBuf = buf;
    uint8_t i;

    pBuf = zcl_buffer_uint32( pBuf, pRsp->transID );

    *pBuf++ = pRsp->numSubDevices;
    *pBuf++ = pRsp->startIndex;
    *pBuf++ = pRsp->cnt;

    // Device information record
    for ( i = 0; i < pRsp->cnt; i++ )
    {
      devInfoRec_t *pRec = &(pRsp->devInfoRec[i]);

      pBuf = zcl_cpyExtAddr( pBuf, pRec->ieeeAddr );

      *pBuf++ = pRec->deviceInfo.endpoint;

      *pBuf++ = LO_UINT16( pRec->deviceInfo.profileID );
      *pBuf++ = HI_UINT16( pRec->deviceInfo.profileID );

      *pBuf++ = LO_UINT16( pRec->deviceInfo.deviceID );
      *pBuf++ = HI_UINT16( pRec->deviceInfo.deviceID );

      *pBuf++ = pRec->deviceInfo.version;
      *pBuf++ = pRec->deviceInfo.grpIdCnt;

      *pBuf++ = pRec->sort;
    }

    status = zclLL_SendInterPANCommand( srcEP, dstAddr, COMMAND_ZLL_DEVICE_INFO_RSP,
                                        ZCL_FRAME_SERVER_CLIENT_DIR, seqNum, bufLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_Send_NwkStartRsp
 *
 * @brief   Call to send out a Network Start Response command
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pRsp - response parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLL_Send_NwkStartRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                  zclLLNwkStartRsp_t *pRsp, uint8_t seqNum )
{
  uint8_t *buf;
  ZStatus_t status;

  buf = zcl_mem_alloc( ZLL_CMDLEN_NWK_START_RSP );
  if ( buf != NULL )
  {
    uint8_t *pBuf = buf;

    pBuf = zcl_buffer_uint32( pBuf, pRsp->transID );

    *pBuf++ = pRsp->status;

    pBuf = zcl_cpyExtAddr( pBuf, pRsp->extendedPANID );

    *pBuf++ = pRsp->nwkUpdateId;
    *pBuf++ = pRsp->logicalChannel;

    *pBuf++ = LO_UINT16( pRsp->panId );
    *pBuf++ = HI_UINT16( pRsp->panId );

    status = zclLL_SendInterPANCommand( srcEP, dstAddr, COMMAND_ZLL_NWK_START_RSP,
                                        ZCL_FRAME_SERVER_CLIENT_DIR, seqNum, ZLL_CMDLEN_NWK_START_RSP, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_Send_NwkJoinRsp
 *
 * @brief   Call to send out a Network Join Response command
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pRsp - response parameters
 * @param   cmd - COMMAND_ZLL_NWK_JOIN_RTR_RSP or COMMAND_ZLL_NWK_JOIN_ED_RSP
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLL_Send_NwkJoinRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                 zclLLNwkJoinRsp_t *pRsp, uint8_t cmd, uint8_t seqNum )
{
  uint8_t buf[ZLL_CMDLEN_NWK_JOIN_RSP];

  VOID zcl_buffer_uint32( buf, pRsp->transID );

  buf[4] = pRsp->status;

  return zclLL_SendInterPANCommand( srcEP, dstAddr, cmd, ZCL_FRAME_SERVER_CLIENT_DIR, seqNum,
                                    ZLL_CMDLEN_NWK_JOIN_RSP, buf );
}

/*********************************************************************
 * @fn      zclLL_Send_EndpointInfo
 *
 * @brief   Call to send out an Endpoint Information command
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - cmd parameters
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLL_Send_EndpointInfo( uint8_t srcEP, afAddrType_t *dstAddr,
                                   zclLLEndpointInfo_t *pCmd,
                                   uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t *buf;
  ZStatus_t status;

  buf = zcl_mem_alloc( ZLL_CMDLEN_EP_INFO );
  if ( buf != NULL )
  {
    uint8_t *pBuf = buf;

    pBuf = zcl_cpyExtAddr( pBuf, pCmd->ieeeAddr );

    *pBuf++ = LO_UINT16( pCmd->nwkAddr );
    *pBuf++ = HI_UINT16( pCmd->nwkAddr );

    *pBuf++ = pCmd->endpoint;

    *pBuf++ = LO_UINT16( pCmd->profileID );
    *pBuf++ = HI_UINT16( pCmd->profileID );

    *pBuf++ = LO_UINT16( pCmd->deviceID );
    *pBuf++ = HI_UINT16( pCmd->deviceID );

    *pBuf++ = pCmd->version;

    status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TOUCHLINK,
                              COMMAND_ZLL_EP_INFO, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                              disableDefaultRsp, 0, seqNum, ZLL_CMDLEN_EP_INFO, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_Send_GetGrpIDsRsp
 *
 * @brief   Call to send out a Get Group Identifiers Response command
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pRsp - response parameters
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLL_Send_GetGrpIDsRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zclLLGetGrpIDsRsp_t *pRsp,
                                    uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t *buf;
  uint8_t bufLen = ZLL_CMDLEN_GET_GRP_IDS_RSP;
  ZStatus_t status;

  // Calculate the total length needed
  bufLen += ( pRsp->cnt * ZLL_CMDLENOPTIONAL_GET_GRP_IDS_RSP );

  buf = zcl_mem_alloc( bufLen );
  if ( buf != NULL )
  {
    uint8_t *pBuf = buf;
    uint8_t i;

    *pBuf++ = pRsp->total;
    *pBuf++ = pRsp->startIndex;
    *pBuf++ = pRsp->cnt;

    // Group information record
    for ( i = 0; i < pRsp->cnt; i++ )
    {
      grpInfoRec_t *pRec = &(pRsp->grpInfoRec[i]);

      *pBuf++ = LO_UINT16( pRec->grpID );
      *pBuf++ = HI_UINT16( pRec->grpID );

      *pBuf++ = pRec->grpType;
    }

    status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TOUCHLINK,
                              COMMAND_ZLL_GET_GRP_IDS_RSP, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                              disableDefaultRsp, 0, seqNum, bufLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_Send_GetEPListRsp
 *
 * @brief   Call to send out a Get Endpoint List Response command
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pRsp - response parameters
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLL_Send_GetEPListRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                   zclLLGetEPListRsp_t *pRsp,
                                   uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t *buf;
  uint8_t bufLen = ZLL_CMDLEN_GET_EP_LIST_RSP;
  ZStatus_t status;

  // Calculate the total length needed
  bufLen += ( pRsp->cnt * ZLL_CMDLENOPTIONAL_GET_EP_LIST_RSP );

  buf = zcl_mem_alloc( bufLen );
  if ( buf != NULL )
  {
    uint8_t *pBuf = buf;
    uint8_t i;

    *pBuf++ = pRsp->total;
    *pBuf++ = pRsp->startIndex;
    *pBuf++ = pRsp->cnt;

    // Endpoint information record
    for ( i = 0; i < pRsp->cnt; i++ )
    {
      epInfoRec_t *pRec = &(pRsp->epInfoRec[i]);

      *pBuf++ = LO_UINT16( pRec->nwkAddr );
      *pBuf++ = HI_UINT16( pRec->nwkAddr );

      *pBuf++ = pRec->endpoint;

      *pBuf++ = LO_UINT16( pRec->profileID );
      *pBuf++ = HI_UINT16( pRec->profileID );

      *pBuf++ = LO_UINT16( pRec->deviceID );
      *pBuf++ = HI_UINT16( pRec->deviceID );

      *pBuf++ = pRec->version;
    }

    status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TOUCHLINK,
                              COMMAND_ZLL_GET_EP_LIST_RSP, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                              disableDefaultRsp, 0, seqNum, bufLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_SendInterPANCommand
 *
 * @brief   Used to send ZLL Profile and Cluster Specific Inter-PAN Command
 *          messages.
 *
 *          NOTE: The calling application is responsible for incrementing
 *                the Sequence Number.
 *
 * @param   srcEp - source endpoint
 * @param   destAddr - destination address
 * @param   cmd - command ID
 * @param   direction - direction of the command
 * @param   seqNumber - identification number for the transaction
 * @param   cmdFormatLen - length of the command to be sent
 * @param   cmdFormat - command to be sent
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zclLL_SendInterPANCommand( uint8_t srcEP, afAddrType_t *destAddr, uint8_t cmd,
                                     uint8_t direction, uint8_t seqNum, uint16_t cmdFormatLen, uint8_t *cmdFormat )
{
  //
  // Note: ZLL Frame Control has a defferent format than ZCL Frame Control
  //

  // ZLL Header Format:
  // - Frame control (1 octect):
  //   * b3-b0: ZLL version (0b0000)
  //   * b7-b4: Reserved (0b0000)
  // - Transaction sequence number (1 octet)
  // - Command identifier (1 octect)
  return zcl_SendCommand( srcEP, destAddr, ZCL_CLUSTER_ID_TOUCHLINK,
                          cmd, TRUE, direction, TRUE, 0,
                          seqNum, cmdFormatLen, cmdFormat );
}

/*********************************************************************
 * @fn      zclLL_FindCallbacks
 *
 * @brief   Find the callbacks for an endpoint
 *
 * @param   endpoint
 *
 * @return  pointer to the callbacks
 */
static zclLL_AppCallbacks_t *zclLL_FindCallbacks( uint8_t endpoint )
{
  zclLLCBRec_t *pCBs;

  pCBs = zclLLCBs;
  while ( pCBs )
  {
    if ( pCBs->endpoint == endpoint )
      return ( pCBs->CBs );
    pCBs = pCBs->next;
  }

  return ( (zclLL_AppCallbacks_t *)NULL );
}

/*********************************************************************
 * @fn      zclLL_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_HdlIncoming(  zclIncoming_t *pInMsg )
{
  ZStatus_t status = ZSuccess;

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  if ( StubAPS_InterPan( pInMsg->msg->srcAddr.panId, pInMsg->msg->srcAddr.endPoint ) )
  {
    return ( zclLL_HdlInInterPANCommands( pInMsg ) );
  }
#endif

  if ( zcl_ClusterCmd( pInMsg->hdr.fc.type ) )
  {
    // Is this a manufacturer specific command?
    if ( pInMsg->hdr.fc.manuSpecific == 0 )
    {
      status = zclLL_HdlInSpecificCommands( pInMsg );
    }
    else
    {
      // We don't support any manufacturer specific command.
      status = ZFailure;
    }
  }
  else
  {
    // Handle all the normal (Read, Write...) commands -- should never get here
    status = ZFailure;
  }
  return ( status );
}

/*********************************************************************
 * @fn      zclLL_HdlInSpecificCommands
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t status;
  zclLL_AppCallbacks_t *pCBs;

  // make sure endpoint exists
  pCBs = zclLL_FindCallbacks( pInMsg->msg->endPoint );
  if ( pCBs == NULL )
    return ( ZFailure );

  switch ( pInMsg->msg->clusterId )
  {
    case ZCL_CLUSTER_ID_TOUCHLINK:
      status = zclLL_ProcessInLLCmds( pInMsg, pCBs );
      break;

    default:
      status = ZFailure;
      break;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_ProcessInLLCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInLLCmds( zclIncoming_t *pInMsg, zclLL_AppCallbacks_t *pCBs )
{
  ZStatus_t status = SUCCESS;

  if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    switch ( pInMsg->hdr.commandID )
    {
      case COMMAND_ZLL_GET_GRP_IDS_REQ:
        status = zclLL_ProcessInCmd_GetGrpIDsReq( pInMsg, pCBs );
        break;

      case COMMAND_ZLL_GET_EP_LIST_REQ:
        status = zclLL_ProcessInCmd_GetEPListReq( pInMsg, pCBs );
        break;

      default:
        status = ZFailure;   // Error ignore the command
        break;
     }
  }
  else // Client Commands
  {
    switch ( pInMsg->hdr.commandID )
    {
      case COMMAND_ZLL_EP_INFO:
        status = zclLL_ProcessInCmd_EndpointInfo( pInMsg, pCBs );
        break;

      case COMMAND_ZLL_GET_GRP_IDS_RSP:
        status = zclLL_ProcessInCmd_GetGrpIDsRsp( pInMsg, pCBs );
        break;

      case COMMAND_ZLL_GET_EP_LIST_RSP:
        status = zclLL_ProcessInCmd_GetEPListRsp( pInMsg, pCBs );
        break;

      default:
        status = ZFailure;   // Error ignore the command
        break;
    }
  }

  return ( status );
}


/*********************************************************************
 * @fn      zclLL_ProcessInCmd_GetGrpIDsReq
 *
 * @brief   Process in the received Get Group Identifiers Request command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_GetGrpIDsReq( zclIncoming_t *pInMsg,
                                                  zclLL_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnGetGrpIDsReq )
  {
    zclLLGetGrpIDsReq_t req;

    req.startIndex = pInMsg->pData[0];

    if ( pCBs->pfnGetGrpIDsReq( &(pInMsg->msg->srcAddr), &req, pInMsg->hdr.transSeqNum ) == ZSuccess )
    {
      return ( ZCL_STATUS_CMD_HAS_RSP );
    }
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclLL_ProcessInCmd_GetEPListReq
 *
 * @brief   Process in the received Get Endpoint List Request command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_GetEPListReq( zclIncoming_t *pInMsg,
                                                  zclLL_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnGetEPListReq )
  {
    zclLLGetEPListReq_t req;

    req.startIndex = pInMsg->pData[0];

    if ( pCBs->pfnGetEPListReq( &(pInMsg->msg->srcAddr), &req, pInMsg->hdr.transSeqNum ) == ZSuccess )
    {
      return ( ZCL_STATUS_CMD_HAS_RSP );
    }
  }

  return ( ZFailure );
}


/*********************************************************************
 * @fn      zclLL_ProcessInCmd_EndpointInfo
 *
 * @brief   Process in the received Endpoint Information command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_EndpointInfo( zclIncoming_t *pInMsg,
                                                  zclLL_AppCallbacks_t *pCBs )
{
  ZStatus_t status = ZFailure;

  if ( pCBs->pfnEndpointInfo )
  {
    zclLLEndpointInfo_t *pCmd;

    pCmd = (zclLLEndpointInfo_t *)zcl_mem_alloc( sizeof( zclLLEndpointInfo_t ) );
    if ( pCmd )
    {
      uint8_t *pBuf = pInMsg->pData;

      zcl_cpyExtAddr( pCmd->ieeeAddr, pBuf );
      pBuf += Z_EXTADDR_LEN;

      pCmd->nwkAddr = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      pCmd->endpoint = *pBuf++;

      pCmd->profileID = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      pCmd->deviceID = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      pCmd->version = *pBuf++;

      status = pCBs->pfnEndpointInfo( &(pInMsg->msg->srcAddr), pCmd );

      zcl_mem_free( pCmd );
    }
  }
  else
  {
    status = ZSuccess;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_ProcessInCmd_GetGrpIDsRsp
 *
 * @brief   Process in the received Get Group Identifiers Response command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_GetGrpIDsRsp( zclIncoming_t *pInMsg,
                                                  zclLL_AppCallbacks_t *pCBs )
{
  ZStatus_t status = ZFailure;

  if ( pCBs->pfnGetGrpIDsRsp )
  {
    zclLLGetGrpIDsRsp_t *pRsp;
    uint8_t cnt = pInMsg->pData[ZLL_CMDLEN_GET_GRP_IDS_RSP-1];
    uint16_t rspLen = sizeof( zclLLGetGrpIDsRsp_t ) + ( cnt * sizeof( grpInfoRec_t ) );
    uint16_t actualDataLen = pInMsg->pDataLen;

    if ( rspLen > actualDataLen )
    {
      return status;
    }

    pRsp = (zclLLGetGrpIDsRsp_t *)zcl_mem_alloc( rspLen );
    if ( pRsp )
    {
      uint8_t *pBuf = pInMsg->pData;
      uint8_t i;

      pRsp->total = *pBuf++;
      pRsp->startIndex = *pBuf++;
      pRsp->cnt = *pBuf++;

      for ( i = 0; i < cnt; i++ )
      {
        grpInfoRec_t *pRec = &(pRsp->grpInfoRec[i]);

        pRec->grpID = BUILD_UINT16( pBuf[0], pBuf[1] );
        pBuf += 2;

        pRec->grpType = *pBuf++;
      }

      status = pCBs->pfnGetGrpIDsRsp( &(pInMsg->msg->srcAddr), pRsp );

      zcl_mem_free( pRsp );
    }
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_ProcessInCmd_GetEPListRsp
 *
 * @brief   Process in the received Get Endpoint List Response command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_GetEPListRsp( zclIncoming_t *pInMsg,
                                                  zclLL_AppCallbacks_t *pCBs )
{
  ZStatus_t status = ZFailure;

  if ( pCBs->pfnGetEPListRsp )
  {
    zclLLGetEPListRsp_t *pRsp;
    uint8_t cnt = pInMsg->pData[ZLL_CMDLEN_GET_EP_LIST_RSP-1];
    uint16_t rspLen = sizeof( zclLLGetEPListRsp_t ) + ( cnt * sizeof( epInfoRec_t ) );
    uint16_t actualDataLen = pInMsg->pDataLen;

    if ( rspLen > actualDataLen )
    {
      return status;
    }

    pRsp = (zclLLGetEPListRsp_t *)zcl_mem_alloc( rspLen );
    if ( pRsp )
    {
      uint8_t *pBuf = pInMsg->pData;
      uint8_t i;

      pRsp->total = *pBuf++;
      pRsp->startIndex = *pBuf++;
      pRsp->cnt = *pBuf++;

      for ( i = 0; i < cnt; i++ )
      {
        epInfoRec_t *pRec = &(pRsp->epInfoRec[i]);

        pRec->nwkAddr = BUILD_UINT16( pBuf[0], pBuf[1] );
        pBuf += 2;

        pRec->endpoint = *pBuf++;

        pRec->profileID = BUILD_UINT16( pBuf[0], pBuf[1] );
        pBuf += 2;

        pRec->deviceID = BUILD_UINT16( pBuf[0], pBuf[1] );
        pBuf += 2;

        pRec->version = *pBuf++;
      }

      status = pCBs->pfnGetEPListRsp( &(pInMsg->msg->srcAddr), pRsp );

      zcl_mem_free( pRsp );
    }
  }

  return ( status );
}

/*********************************************************************
* Inter-PAN functions
**********************************************************************/

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
/*********************************************************************
 * @fn      zclLL_HdlInInterPANCommands
 *
 * @brief   Callback from ZCL to process incoming Inter-PAN Commands
 *          specific to this cluster library

 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_HdlInInterPANCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t status;

  // make sure Inter-PAN callbacks exist
  if ( pInterPANCBs == NULL )
    return ( ZFailure );

  switch ( pInMsg->msg->clusterId )
  {
    case ZCL_CLUSTER_ID_TOUCHLINK:
      status = zclLL_ProcessInLLInterPANCmds( pInMsg );
      break;

    default:
      status = ZFailure;
      break;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_ProcessInLLInterPANCmds
 *
 * @brief   Callback from ZCL to process incoming Inter-PAN Commands
 *          specific to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInLLInterPANCmds( zclIncoming_t *pInMsg )
{
  ZStatus_t status = SUCCESS;

  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_ZLL_SCAN_REQ:
      status = zclLL_ProcessInCmd_ScanReq( pInMsg );
      break;

    case COMMAND_ZLL_DEVICE_INFO_REQ:
      status = zclLL_ProcessInCmd_DeviceInfoReq( pInMsg );
      break;

    case COMMAND_ZLL_IDENTIFY_REQ:
      status = zclLL_ProcessInCmd_IdentifyReq( pInMsg );
      break;

    case COMMAND_ZLL_RESET_TO_FN_REQ:
      status = zclLL_ProcessInCmd_ResetToFNReq( pInMsg );
      break;

    case COMMAND_ZLL_NWK_START_REQ:
      status = zclLL_ProcessInCmd_NwkStartReq( pInMsg );
      break;

    case COMMAND_ZLL_NWK_JOIN_RTR_REQ:
      status = zclLL_ProcessInCmd_NwkJoinRtrReq( pInMsg );
      break;

    case COMMAND_ZLL_NWK_JOIN_ED_REQ:
      status = zclLL_ProcessInCmd_NwkJoinEDReq( pInMsg );
      break;

    case COMMAND_ZLL_NWK_UPDATE_REQ:
      status = zclLL_ProcessInCmd_NwkUpdateReq( pInMsg );
      break;

    case COMMAND_ZLL_SCAN_RSP:
      status = zclLL_ProcessInCmd_ScanRsp( pInMsg );
      break;

    case COMMAND_ZLL_DEVICE_INFO_RSP:
      status = zclLL_ProcessInCmd_DeviceInfoRsp( pInMsg );
      break;

    case COMMAND_ZLL_NWK_START_RSP:
      status = zclLL_ProcessInCmd_NwkStartRsp( pInMsg );
      break;

    case COMMAND_ZLL_NWK_JOIN_RTR_RSP:
      status = zclLL_ProcessInCmd_NwkJoinRtrRsp( pInMsg );
      break;

    case COMMAND_ZLL_NWK_JOIN_ED_RSP:
      status = zclLL_ProcessInCmd_NwkJoinEDRsp( pInMsg );
      break;

    default:
      status = ZFailure;   // Error ignore the command
      break;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_ProcessInCmd_ScanReq
 *
 * @brief   Process in the received Scan Request command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_ScanReq( zclIncoming_t *pInMsg )
{
  if ( pInterPANCBs->pfnScanReq )
  {
    zclLLScanReq_t req;

    req.transID = zcl_build_uint32( pInMsg->pData, 4 );

    req.zInfo.zInfoByte = pInMsg->pData[4];
    req.zllInfo.zllInfoByte = pInMsg->pData[5];

    return ( pInterPANCBs->pfnScanReq( &(pInMsg->msg->srcAddr), &req,
                                       pInMsg->hdr.transSeqNum ) );
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclLL_ProcessInCmd_DeviceInfoReq
 *
 * @brief   Process in the received Device Information Request command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_DeviceInfoReq( zclIncoming_t *pInMsg )
{
  if ( pInterPANCBs->pfnDeviceInfoReq )
  {
    zclLLDeviceInfoReq_t req;

    req.transID = zcl_build_uint32( pInMsg->pData, 4 );

    req.startIndex = pInMsg->pData[4];

    return ( pInterPANCBs->pfnDeviceInfoReq( &(pInMsg->msg->srcAddr), &req,
                                             pInMsg->hdr.transSeqNum ) );
  }

  return ( ZSuccess );
}


/*********************************************************************
 * @fn      zclLL_ProcessInCmd_IdentifyReq
 *
 * @brief   Process in the received Identify Request command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_IdentifyReq( zclIncoming_t *pInMsg )
{
  if ( pInterPANCBs->pfnIdentifyReq )
  {
    zclLLIdentifyReq_t req;

    req.transID = zcl_build_uint32( pInMsg->pData, 4 );

    req.IdDuration = BUILD_UINT16( pInMsg->pData[4], pInMsg->pData[5] );

    return ( pInterPANCBs->pfnIdentifyReq( &(pInMsg->msg->srcAddr), &req ) );
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclLL_ProcessInCmd_ResetToFNReq
 *
 * @brief   Process in the received Reset to Factory New Request command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_ResetToFNReq( zclIncoming_t *pInMsg )
{
  if ( pInterPANCBs->pfnResetToFNReq )
  {
    zclLLResetToFNReq_t req;

    req.transID = zcl_build_uint32( pInMsg->pData, 4 );

    return ( pInterPANCBs->pfnResetToFNReq( &(pInMsg->msg->srcAddr), &req ) );
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclLL_ProcessInCmd_NwkStartReq
 *
 * @brief   Process in the received Network Start Request command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_NwkStartReq( zclIncoming_t *pInMsg )
{
  uint8_t status = ZSuccess;

  if ( pInterPANCBs->pfnNwkStartReq )
  {
    zclLLNwkStartReq_t *pReq;

    pReq = (zclLLNwkStartReq_t *)zcl_mem_alloc( sizeof( zclLLNwkStartReq_t ) );
    if ( pReq )
    {
      uint8_t *pBuf = pInMsg->pData;

      pReq->transID = zcl_build_uint32( pBuf, 4 );
      pBuf += 4;

      zcl_cpyExtAddr( pReq->nwkParams.extendedPANID, pBuf );
      pBuf += Z_EXTADDR_LEN;

      pReq->nwkParams.keyIndex = *pBuf++;

      zcl_memcpy( pReq->nwkParams.nwkKey, pBuf, SEC_KEY_LEN );
      pBuf += SEC_KEY_LEN;

      pReq->nwkParams.logicalChannel = *pBuf++;

      pReq->nwkParams.panId = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      pReq->nwkParams.nwkAddr = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      pReq->nwkParams.grpIDsBegin = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      pReq->nwkParams.grpIDsEnd = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      pReq->nwkParams.freeNwkAddrBegin = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      pReq->nwkParams.freeNwkAddrEnd = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      pReq->nwkParams.freeGrpIDBegin = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      pReq->nwkParams.freeGrpIDEnd = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      zcl_cpyExtAddr( pReq->initiatorIeeeAddr, pBuf );
      pBuf += Z_EXTADDR_LEN;

      pReq->initiatorNwkAddr = BUILD_UINT16( pBuf[0], pBuf[1] );

      status = pInterPANCBs->pfnNwkStartReq( &(pInMsg->msg->srcAddr), pReq,
                                             pInMsg->hdr.transSeqNum );

      zcl_mem_free( pReq );
    }
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_ParseInCmd_NwkJoinReq
 *
 * @brief   Parse in the received Network Router/End Device Join Request command.
 *
 * @param   pBuf - pointer to the incoming message
 *
 * @return  void
 */
static void zclLL_ParseInCmd_NwkJoinReq( uint8_t *pBuf, zclLLNwkJoinReq_t *pReq )
{
  pReq->transID = zcl_build_uint32( pBuf, 4 );
  pBuf += 4;

  zcl_cpyExtAddr( pReq->nwkParams.extendedPANID, pBuf );
  pBuf += Z_EXTADDR_LEN;

  pReq->nwkParams.keyIndex = *pBuf++;

  zcl_memcpy( pReq->nwkParams.nwkKey, pBuf, SEC_KEY_LEN );
  pBuf += SEC_KEY_LEN;

  pReq->nwkUpdateId = *pBuf++;
  pReq->nwkParams.logicalChannel = *pBuf++;

  pReq->nwkParams.panId = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  pReq->nwkParams.nwkAddr = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  pReq->nwkParams.grpIDsBegin = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  pReq->nwkParams.grpIDsEnd = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  pReq->nwkParams.freeNwkAddrBegin = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  pReq->nwkParams.freeNwkAddrEnd = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  pReq->nwkParams.freeGrpIDBegin = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  pReq->nwkParams.freeGrpIDEnd = BUILD_UINT16( pBuf[0], pBuf[1] );
}

/*********************************************************************
 * @fn      zclLL_ProcessInCmd_NwkJoinRtrReq
 *
 * @brief   Process in the received Network Join Router Request command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_NwkJoinRtrReq( zclIncoming_t *pInMsg )
{
  ZStatus_t status = ZSuccess;

  if ( pInterPANCBs->pfnNwkJoinRtrReq )
  {
    zclLLNwkJoinReq_t *pReq;

    pReq = (zclLLNwkJoinReq_t *)zcl_mem_alloc( sizeof( zclLLNwkJoinReq_t ) );
    if ( pReq )
    {
      zclLL_ParseInCmd_NwkJoinReq( pInMsg->pData, pReq );

      status = pInterPANCBs->pfnNwkJoinRtrReq( &(pInMsg->msg->srcAddr), pReq,
                                               pInMsg->hdr.transSeqNum  );
      zcl_mem_free( pReq );
    }
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_ProcessInCmd_NwkJoinEDReq
 *
 * @brief   Process in the received Network Join End Device  Request command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_NwkJoinEDReq( zclIncoming_t *pInMsg )
{
  ZStatus_t status = ZSuccess;

  if ( pInterPANCBs->pfnNwkJoinEDReq )
  {
    zclLLNwkJoinReq_t *pReq;

    pReq = (zclLLNwkJoinReq_t *)zcl_mem_alloc( sizeof( zclLLNwkJoinReq_t ) );
    if ( pReq )
    {
      zclLL_ParseInCmd_NwkJoinReq( pInMsg->pData, pReq );

      status = pInterPANCBs->pfnNwkJoinEDReq( &(pInMsg->msg->srcAddr), pReq,
                                              pInMsg->hdr.transSeqNum );
      zcl_mem_free( pReq );
    }
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_ProcessInCmd_NwkUpdateReq
 *
 * @brief   Process in the received Network Update Request command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_NwkUpdateReq( zclIncoming_t *pInMsg )
{
  ZStatus_t status = ZSuccess;

  if ( pInterPANCBs->pfnNwkUpdateReq )
  {
    zclLLNwkUpdateReq_t *pReq;

    pReq = (zclLLNwkUpdateReq_t *)zcl_mem_alloc( sizeof( zclLLNwkUpdateReq_t ) );
    if ( pReq )
    {
      uint8_t *pBuf = pInMsg->pData;

      pReq->transID = zcl_build_uint32( pBuf, 4 );
      pBuf += 4;

      zcl_cpyExtAddr( pReq->extendedPANID, pBuf );
      pBuf += Z_EXTADDR_LEN;

      pReq->nwkUpdateId = *pBuf++;
      pReq->logicalChannel = *pBuf++;

      pReq->PANID = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      pReq->nwkAddr = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      status = pInterPANCBs->pfnNwkUpdateReq( &(pInMsg->msg->srcAddr), pReq );

      zcl_mem_free( pReq );
    }
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_ProcessInCmd_ScanRsp
 *
 * @brief   Process in the received Scan Response command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_ScanRsp( zclIncoming_t *pInMsg )
{
  ZStatus_t status = ZSuccess;

  if ( pInterPANCBs->pfnScanRsp )
  {
    zclLLScanRsp_t *pRsp;

    pRsp = (zclLLScanRsp_t *)zcl_mem_alloc( sizeof( zclLLScanRsp_t ) );
    if ( pRsp )
    {
      uint8_t *pBuf = pInMsg->pData;

      zcl_memset( pRsp, 0, sizeof( zclLLScanRsp_t ) );

      pRsp->transID = zcl_build_uint32( pBuf, 4 );
      pBuf += 4;

      pRsp->rssiCorrection = *pBuf++;
      pRsp->zInfo.zInfoByte = *pBuf++;
      pRsp->zllInfo.zllInfoByte = *pBuf++;

      pRsp->keyBitmask = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      pRsp->responseID = zcl_build_uint32( pBuf, 4 );
      pBuf += 4;

      zcl_cpyExtAddr( pRsp->extendedPANID, pBuf );
      pBuf += Z_EXTADDR_LEN;

      pRsp->nwkUpdateId = *pBuf++;
      pRsp->logicalChannel = *pBuf++;

      pRsp->PANID = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      pRsp->nwkAddr = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      pRsp->numSubDevices = *pBuf++;
      pRsp->totalGrpIDs = *pBuf++;

      if ( pRsp->numSubDevices == 1 )
      {
        pRsp->deviceInfo.endpoint = *pBuf++;

        pRsp->deviceInfo.profileID = BUILD_UINT16( pBuf[0], pBuf[1] );
        pBuf += 2;

        pRsp->deviceInfo.deviceID = BUILD_UINT16( pBuf[0], pBuf[1] );
        pBuf += 2;

        pRsp->deviceInfo.version = *pBuf++;
        pRsp->deviceInfo.grpIdCnt = *pBuf;
      }

      status = pInterPANCBs->pfnScanRsp( &(pInMsg->msg->srcAddr), pRsp );
      zcl_mem_free( pRsp );
    }
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_ProcessInCmd_DeviceInfoRsp
 *
 * @brief   Process in the received Device Information Response command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_DeviceInfoRsp( zclIncoming_t *pInMsg )
{
  ZStatus_t status = ZSuccess;

  if ( pInterPANCBs->pfnDeviceInfoRsp )
  {
    zclLLDeviceInfoRsp_t *pRsp;
    uint8_t cnt = pInMsg->pData[ZLL_CMDLEN_DEVICE_INFO_RSP-1];
    uint16_t rspLen = sizeof( zclLLDeviceInfoRsp_t ) + ( cnt * sizeof( devInfoRec_t ) );
    uint16_t actualDataLen = pInMsg->pDataLen;

    if ( rspLen > actualDataLen )
    {
      return status;
    }

    pRsp = (zclLLDeviceInfoRsp_t *)zcl_mem_alloc( rspLen );
    if ( pRsp )
    {
      uint8_t *pBuf = pInMsg->pData;
      uint8_t i;

      pRsp->transID = zcl_build_uint32( pBuf, 4 );
      pBuf += 4;

      pRsp->numSubDevices = *pBuf++;
      pRsp->startIndex = *pBuf++;
      pRsp->cnt = *pBuf++;

      for ( i = 0; i < cnt; i++ )
      {
        devInfoRec_t *pRec = &(pRsp->devInfoRec[i]);

        zcl_cpyExtAddr( pRec->ieeeAddr, pBuf );
        pBuf += Z_EXTADDR_LEN;

        pRec->deviceInfo.endpoint = *pBuf++;

        pRec->deviceInfo.profileID = BUILD_UINT16( pBuf[0], pBuf[1] );
        pBuf += 2;

        pRec->deviceInfo.deviceID = BUILD_UINT16( pBuf[0], pBuf[1] );
        pBuf += 2;

        pRec->deviceInfo.version = *pBuf++;
        pRec->deviceInfo.grpIdCnt = *pBuf++;

        pRec->sort = *pBuf++;
      }

      status = pInterPANCBs->pfnDeviceInfoRsp( &(pInMsg->msg->srcAddr), pRsp );

      zcl_mem_free( pRsp );
    }
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_ProcessInCmd_NwkStartRsp
 *
 * @brief   Process in the received Network Start Response command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_NwkStartRsp( zclIncoming_t *pInMsg )
{
  ZStatus_t status = ZSuccess;

  if ( pInterPANCBs->pfnNwkStartRsp )
  {
    zclLLNwkStartRsp_t *pRsp;

    pRsp = (zclLLNwkStartRsp_t *)zcl_mem_alloc( sizeof( zclLLNwkStartRsp_t ) );
    if ( pRsp )
    {
      uint8_t *pBuf = pInMsg->pData;

      pRsp->transID = zcl_build_uint32( pBuf, 4 );
      pBuf += 4;

      pRsp->status = *pBuf++;

      zcl_cpyExtAddr( pRsp->extendedPANID, pBuf );
      pBuf += Z_EXTADDR_LEN;

      pRsp->nwkUpdateId = *pBuf++;
      pRsp->logicalChannel = *pBuf++;

      pRsp->panId = BUILD_UINT16( pBuf[0], pBuf[1] );

      status = pInterPANCBs->pfnNwkStartRsp( &(pInMsg->msg->srcAddr), pRsp );

      zcl_mem_free( pRsp );
    }
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclLL_ProcessInCmd_NwkJoinRtrRsp
 *
 * @brief   Process in the received Network Join Router Response command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_NwkJoinRtrRsp( zclIncoming_t *pInMsg )
{
  if ( pInterPANCBs->pfnNwkJoinRtrRsp )
  {
    zclLLNwkJoinRsp_t rsp;

    rsp.transID = zcl_build_uint32( pInMsg->pData, 4 );

    rsp.status = pInMsg->pData[4];

    return ( pInterPANCBs->pfnNwkJoinRtrRsp( &(pInMsg->msg->srcAddr), &rsp ) );
  }

  return ( ZSuccess );
}


/*********************************************************************
 * @fn      zclLL_ProcessInCmd_NwkJoinEDRsp
 *
 * @brief   Process in the received Network Join End Device Response command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLL_ProcessInCmd_NwkJoinEDRsp( zclIncoming_t *pInMsg )
{
  if ( pInterPANCBs->pfnNwkJoinEDRsp )
  {
    zclLLNwkJoinRsp_t rsp;

    rsp.transID = zcl_build_uint32( pInMsg->pData, 4 );

    rsp.status = pInMsg->pData[4];

    return ( pInterPANCBs->pfnNwkJoinEDRsp( &(pInMsg->msg->srcAddr), &rsp ) );
  }

  return ( ZSuccess );
}
#endif

/********************************************************************************************
*********************************************************************************************/
