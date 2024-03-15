/**************************************************************************************************
  Filename:       zcl_partition.c
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    Zigbee Cluster Library - Partition Cluster allows sending
  larger packets than APS Fragmentation (up to 100K).

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
#include "zcomdef.h"
#include "ti_zstack_config.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_partition.h"

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
#include "stub_aps.h"
#endif

#ifdef ZCL_PARTITION

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct zclPartitionCBRec
{
  struct zclPartitionCBRec    *next;
  uint8_t                        endpoint; // Used to link it into the endpoint descriptor
  zclPartition_AppCallbacks_t *CBs;      // Pointer to Callback function
} zclPartitionCBRec_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static zclPartitionCBRec_t *zclPartitionCBs = (zclPartitionCBRec_t *)NULL;
static uint8_t zclPartitionPluginRegisted = FALSE;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclPartition_HdlIncoming( zclIncoming_t *pInHdlrMsg );
static ZStatus_t zclPartition_HdlInSpecificCommands( zclIncoming_t *pInMsg );
static zclPartition_AppCallbacks_t *zclPartition_FindCallbacks( uint8_t endpoint );
static ZStatus_t zclPartition_ProcessInCmds( zclIncoming_t *pInMsg, zclPartition_AppCallbacks_t *pCBs );

static ZStatus_t zclPartition_ProcessInCmd_TransferPartitionedFrame( zclIncoming_t *pInMsg, zclPartition_AppCallbacks_t *pCBs );
static ZStatus_t zclPartition_ProcessInCmd_ReadHandshakeParam( zclIncoming_t *pInMsg, zclPartition_AppCallbacks_t *pCBs );
static ZStatus_t zclPartition_ProcessInCmd_WriteHandshakeParam( zclIncoming_t *pInMsg, zclPartition_AppCallbacks_t *pCBs );
static ZStatus_t zclPartition_ProcessInCmd_MultipleAck( zclIncoming_t *pInMsg, zclPartition_AppCallbacks_t *pCBs );
static ZStatus_t zclPartition_ProcessInCmd_ReadHandshakeParamRsp( zclIncoming_t *pInMsg, zclPartition_AppCallbacks_t *pCBs );


/*********************************************************************
 * @fn      zclPartition_RegisterCmdCallbacks
 *
 * @brief   Register applications command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
ZStatus_t zclPartition_RegisterCmdCallbacks( uint8_t endpoint, zclPartition_AppCallbacks_t *callbacks )
{
  zclPartitionCBRec_t *pNewItem;
  zclPartitionCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( zclPartitionPluginRegisted == FALSE )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_GENERAL_PARTITION,
                        ZCL_CLUSTER_ID_GENERAL_PARTITION,
                        zclPartition_HdlIncoming );
    zclPartitionPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclPartitionCBRec_t ) );
  if ( pNewItem == NULL )
  {
    return ( ZMemError ); // memory error
  }

  pNewItem->next = (zclPartitionCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclPartitionCBs == NULL )
  {
    zclPartitionCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclPartitionCBs;
    while ( pLoop->next != NULL )
      pLoop = pLoop->next;

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclPartition_Send_TransferPartitionedFrame
 *
 * @brief   send a single block (partitioned frame) to a remote receiver
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - the partitioned frame
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t or ZCL_STATUS_INSUFFICIENT_SPACE
 */
ZStatus_t zclPartition_Send_TransferPartitionedFrame( uint8_t srcEP, afAddrType_t *dstAddr,
                                                      zclCmdTransferPartitionedFrame_t *pCmd,
                                                      uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buflen;
  uint8_t offset;
  uint8_t *buf;
  uint16_t indicator;
  ZStatus_t status;

  // allocate the memory large enough to hold the OTA frame with a 2-byte indicator
  buflen = PAYLOAD_LEN_TRANSFER_PARTITIONED_FRAME + pCmd->frameLen;
  buf = zcl_mem_alloc( buflen );
  if ( !buf )
  {
    return ( ZMemError ); // memory error
  }

  // fill in the OTA frame buffer
  buf[0] = pCmd->fragmentationOptions;
  indicator = pCmd->partitionIndicator;
  buf[1] = LO_UINT16(indicator);
  offset = 2;
  if ( pCmd->fragmentationOptions & ZCL_PARTITION_OPTIONS_INDICATOR_16BIT )
  {
    buf[offset++] = HI_UINT16(indicator);
  }
  buf[offset++] = pCmd->frameLen;   // octet string begins with length
  zcl_memcpy( &buf[offset], pCmd->pFrame, pCmd->frameLen );
  offset += pCmd->frameLen;

  // send, with payload
  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_PARTITION,
                            COMMAND_PARTITION_TRANSFER_PARTITIONED_FRAME, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                            disableDefaultRsp, 0, seqNum, offset, buf );

  // done, free the memory
  zcl_mem_free( buf );
  return ( status );
}

/*********************************************************************
 * @fn      zclPartition_Send_ReadHandshakeParam
 *
 * @brief   Call to send out Poll Control CheckIn command from ZED to ZR/ZC. The Rsp
 *          will indicate whether to stay awake or go back to sleep.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - what parameters to read
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclPartition_Send_ReadHandshakeParam( uint8_t srcEP, afAddrType_t *dstAddr,
                                                zclCmdReadHandshakeParam_t *pCmd,
                                                uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buflen;
  uint8_t offset = 0;
  uint8_t *buf;
  uint8_t i;
  uint16_t clusterID;
  uint16_t attrID;
  ZStatus_t status;

  // allocate the memory
  buflen = sizeof( uint16_t ) + pCmd->numAttrs * sizeof( uint16_t );
  buf = zcl_mem_alloc( buflen );
  if ( !buf )
  {
    return ( ZMemError ); // memory error
  }

  // fill in the buffer
  clusterID = pCmd->clusterID;
  buf[0] = LO_UINT16( clusterID );
  buf[1] = HI_UINT16( clusterID );
  for ( i = 0; i < pCmd->numAttrs; ++i )
  {
    attrID = pCmd->pAttrID[i];
    buf[offset++] = LO_UINT16( attrID );
    buf[offset++] = HI_UINT16( attrID );
  }

  // send, with payload
  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_PARTITION,
                            COMMAND_PARTITION_READ_HANDSHAKE_PARAM, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                            disableDefaultRsp, 0, seqNum, buflen, buf);

  // done, free the memory
  zcl_mem_free( buf );
  return ( status );
}

/*********************************************************************
 * @fn      zclPartition_Send_WriteHandshakeParam
 *
 * @brief   Call to send out Poll Control CheckIn command from ZED to ZR/ZC. The Rsp
 *          will indicate whether to stay awake or go back to sleep.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - what parameters to write
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclPartition_Send_WriteHandshakeParam( uint8_t srcEP, afAddrType_t *dstAddr,
                                                 zclCmdWriteHandshakeParam_t *pCmd,
                                                 uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buflen;
  uint8_t offset;
  uint8_t i;
  uint8_t *buf;
  uint16_t clusterID;
  ZStatus_t status;

  // allocate the memory. write records are assumed to be either uint8_t or uint16_t
  // WriteRecord: [AttrID][AttrType][AttrData] = max 5 bytes per record, if attrType is uint16_t or uint8_t
  buflen = PAYLOAD_LEN_WRITE_HANDSHAKE_PARAM + ( pCmd->numRecords * PAYLOAD_LEN_WRITE_REC );
  buf = zcl_mem_alloc( buflen );
  if ( !buf )
  {
    return ( ZMemError ); // memory error
  }

  //   fill in the buffer
  clusterID = pCmd->clusterID;
  buf[0] = LO_UINT16( clusterID );
  buf[1] = HI_UINT16( clusterID );
  offset = 2;
  for ( i = 0; i < pCmd->numRecords; ++i )
  {
    // copy write record to output buffer
    buf[offset++] = LO_UINT16( pCmd->pWriteRecord[i].attrID );
    buf[offset++] = HI_UINT16( pCmd->pWriteRecord[i].attrID );
    buf[offset++] = pCmd->pWriteRecord[i].dataType;

    // all writeable Partition attributes are either uint8_t or uint16_t
    buf[offset++] = LO_UINT16( pCmd->pWriteRecord[i].attr );
    if ( pCmd->pWriteRecord[i].dataType == ZCL_DATATYPE_UINT16 )
    {
      buf[offset++] = HI_UINT16( pCmd->pWriteRecord[i].attr );
    }
  }

  // send, with payload
  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_PARTITION,
                            COMMAND_PARTITION_WRITE_HANDSHAKE_PARAM, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                            disableDefaultRsp, 0, seqNum, offset, buf);

  // done, free the memory
  zcl_mem_free( buf );
  return ( status );
}

/*********************************************************************
 * @fn      zclPartition_Send_MultipleAck
 *
 * @brief   Call to send out Poll Control CheckIn command from ZED to ZR/ZC. The Rsp
 *          will indicate whether to stay awake or go back to sleep.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - multi ack response
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclPartition_Send_MultipleAck( uint8_t srcEP, afAddrType_t *dstAddr,
                                         zclCmdMultipleAck_t *pCmd,
                                         uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buflen;
  uint8_t NAckSize;
  uint8_t offset;
  uint8_t i;
  uint8_t *buf;
  ZStatus_t status;

  // determine if NACKs are 1 or 2 bytes
  if ( pCmd->options & ZCL_PARTITION_OPTIONS_NACK_16BIT )
  {
    NAckSize = 2;
  }
  else
  {
    NAckSize = 1;
  }

  // create buffer large enough for multiple ACKs command
  // ACKOptions is 1 byte, FirstFrameID and NACKIds are 1 or 2 bytes depending on options
  // [ACKOptions][FirstFrameID][NACKId]...[NACKId]
  buflen = 1 + NAckSize * ( 1 + pCmd->numNAcks );
  buf = zcl_mem_alloc( buflen );
  if ( !buf )
  {
    return ( ZMemError ); // memory error
  }

  //   fill in the buffer
  buf[0] = pCmd->options;
  offset = 1;
  buf[offset++] = LO_UINT16( pCmd->firstFrameID );
  if ( NAckSize == 2 )
  {
    buf[offset++] = HI_UINT16( pCmd->firstFrameID );
  }
  for ( i = 0; i < pCmd->numNAcks ; ++i )
  {
    buf[offset++] = LO_UINT16( pCmd->pNAckID[i] );
    if ( NAckSize == 2 )
    {
      buf[offset++] = HI_UINT16( pCmd->pNAckID[i] );
    }
  }

  // send, with payload
  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_PARTITION,
                            COMMAND_PARTITION_MULTIPLE_ACK, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                            disableDefaultRsp, 0, seqNum, offset, buf);

  // done, free the memory
  zcl_mem_free( buf );
  return ( status );
}

/*********************************************************************
 * @fn      zclPartition_Send_ReadHandshakeParamRsp
 *
 * @brief   Call to send out Poll Control CheckIn command from ZED to ZR/ZC. The Rsp
 *          will indicate whether to stay awake or go back to sleep.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - read multi-attribues response
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclPartition_Send_ReadHandshakeParamRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                                   zclCmdReadHandshakeParamRsp_t *pCmd,
                                                   uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buflen;
  uint8_t offset;
  uint8_t i;
  uint8_t *buf;
  ZStatus_t status;

  // allocate enough memory for all records. some may end up being short, so we may not use all this buffer.
  buflen = sizeof( uint16_t ) + ( pCmd->numRecords * sizeof ( zclPartitionReadRec_t ) );
  buf = zcl_mem_alloc( buflen );
  if ( !buf )
  {
    return ( ZMemError ); // memory error
  }

  //   fill in the buffer
  buf[0] = LO_UINT16( pCmd->clusterID );
  buf[1] = HI_UINT16( pCmd->clusterID );
  offset = 2;
  for ( i = 0; i < pCmd->numRecords; ++i )
  {
    buf[offset++] = LO_UINT16( pCmd->pReadRecord[i].attrID );
    buf[offset++] = HI_UINT16( pCmd->pReadRecord[i].attrID );
    buf[offset++] = pCmd->pReadRecord[i].status;
    if ( pCmd->pReadRecord[i].status == ZCL_STATUS_SUCCESS )
    {
      buf[offset++] = pCmd->pReadRecord[i].dataType;

      // this only works for Partition Cluster because it's attributes are either uint8_t or uint16_t
      buf[offset++] = LO_UINT16( pCmd->pReadRecord[i].attr );
      if ( pCmd->pReadRecord[i].dataType == ZCL_DATATYPE_UINT16 )
      {
        buf[offset++] = LO_UINT16( pCmd->pReadRecord[i].attr );
      }
    }
  }
  buflen = offset;

  // send, with payload
  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_PARTITION,
                            COMMAND_PARTITION_READ_HANDSHAKE_PARAM_RSP, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                            disableDefaultRsp, 0, seqNum, buflen, buf);

  // done, free the memory
  zcl_mem_free( buf );
  return ( status );
}

/*********************************************************************
 * @fn      zclPartition_FindCallbacks
 *
 * @brief   Find the callbacks for an endpoint
 *
 * @param   endpoint - endpoint to find the application callbacks for
 *
 * @return  pointer to the callbacks
 */
static zclPartition_AppCallbacks_t *zclPartition_FindCallbacks( uint8_t endpoint )
{
  zclPartitionCBRec_t *pCBs;

  pCBs = zclPartitionCBs;
  while ( pCBs != NULL )
  {
    if ( pCBs->endpoint == endpoint )
    {
      return ( pCBs->CBs );
    }
    pCBs = pCBs->next;
  }
  return ( (zclPartition_AppCallbacks_t *)NULL );
}

/*********************************************************************
 * @fn      zclPartition_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPartition_HdlIncoming( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  if ( StubAPS_InterPan( pInMsg->msg->srcAddr.panId, pInMsg->msg->srcAddr.endPoint ) )
  {
    return ( stat ); // Cluster not supported thru Inter-PAN
  }
#endif
  if ( zcl_ClusterCmd( pInMsg->hdr.fc.type ) )
  {
    // Is this a manufacturer specific command?
    if ( pInMsg->hdr.fc.manuSpecific == 0 )
    {
      stat = zclPartition_HdlInSpecificCommands( pInMsg );
    }
    else
    {
      // We don't support any manufacturer specific command.
      stat = ZFailure;
    }
  }
  else
  {
    // Handle all the normal (Read, Write...) commands -- should never get here
    stat = ZFailure;
  }
  return ( stat );
}

/*********************************************************************
 * @fn      zclPartition_HdlInSpecificCommands
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library

 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPartition_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  zclPartition_AppCallbacks_t *pCBs;

  // make sure endpoint exists
  pCBs = zclPartition_FindCallbacks( pInMsg->msg->endPoint );
  if ( pCBs == NULL )
  {
    return ( ZFailure );
  }

  stat = zclPartition_ProcessInCmds( pInMsg, pCBs );

  return ( stat );
}

/*********************************************************************
 * @fn      zclPartition_ProcessInCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis

 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPartition_ProcessInCmds( zclIncoming_t *pInMsg,
                                             zclPartition_AppCallbacks_t *pCBs )
{
  ZStatus_t stat;

  // Client-to-Server
  if( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    switch ( pInMsg->hdr.commandID )
    {
      case COMMAND_PARTITION_TRANSFER_PARTITIONED_FRAME:
        stat = zclPartition_ProcessInCmd_TransferPartitionedFrame( pInMsg, pCBs );
      break;

      case COMMAND_PARTITION_READ_HANDSHAKE_PARAM:
        stat = zclPartition_ProcessInCmd_ReadHandshakeParam( pInMsg, pCBs );
      break;

      case COMMAND_PARTITION_WRITE_HANDSHAKE_PARAM:
        stat = zclPartition_ProcessInCmd_WriteHandshakeParam( pInMsg, pCBs );
      break;

      default:
        // Unknown command
        stat = ZFailure;
      break;
    }
  }
  // Sent Server-to-Client
  else
  {
    switch ( pInMsg->hdr.commandID )
    {
      case COMMAND_PARTITION_MULTIPLE_ACK:
        stat = zclPartition_ProcessInCmd_MultipleAck( pInMsg, pCBs );
      break;

      case COMMAND_PARTITION_READ_HANDSHAKE_PARAM_RSP:
        stat = zclPartition_ProcessInCmd_ReadHandshakeParamRsp( pInMsg, pCBs );
      break;

      default:
        // Unknown command
        stat = ZFailure;
      break;
    }
  }

  return ( stat );
}


/*********************************************************************
 * @fn      zclPartition_ConvertOtaToNative_TransferPartitionedFrame
 *
 * @brief   Helper function used to process an incoming TransferPartionFrame
 *          command.
 *
 * @param   pCmd   - (output) the converted command
 * @param   buf    - pointer to incoming frame (just after ZCL header)
 * @param   buflen - length of buffer (ZCL payload)
 *
 * @return  ZStatus_t
 */
ZStatus_t zclPartition_ConvertOtaToNative_TransferPartitionedFrame( zclCmdTransferPartitionedFrame_t *pCmd, uint8_t *buf, uint8_t buflen )
{
  uint8_t offset;
  pCmd->fragmentationOptions = buf[0];
  if ( pCmd->fragmentationOptions & ZCL_PARTITION_OPTIONS_INDICATOR_16BIT )
  {
    pCmd->partitionIndicator = BUILD_UINT16( buf[1], buf[2] );
    offset = 3;
  }
  else
  {
    pCmd->partitionIndicator = buf[1];
    offset = 2;
  }
  pCmd->frameLen = buf[offset++];
  pCmd->pFrame = &buf[offset];

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclPartition_ConvertOtaToNative_ReadHandshakeParam
 *
 * @brief   Helper function used to process an incoming ReadHandshakeParam
 *          command. Remember to free memory.
 *
 * @param   pCmd   - (output) the converted command
 * @param   buf    - pointer to incoming frame (just after ZCL header)
 * @param   buflen - length of buffer (ZCL payload)
 *
 * @return  ZStatus_t - ZCL_STATUS_MALFORMED_COMMAND, ZMemError or ZCL_STATUS_SUCCESS
 */
ZStatus_t zclPartition_ConvertOtaToNative_ReadHandshakeParam( zclCmdReadHandshakeParam_t *pCmd, uint8_t *buf, uint8_t buflen )
{
  uint8_t offset;
  uint8_t i;

  // must have at least 1 attribute (clusterID + attrID)
  if ( buflen < 4 )
  {
    return ( ZCL_STATUS_MALFORMED_COMMAND );
  }

  pCmd->clusterID = BUILD_UINT16( buf[0], buf[1] );

  // allocate memory for attribute list
  pCmd->numAttrs = ( buflen - 2 ) >> 1;   // each AttrID is 2 bytes, - length of ClusterID
  pCmd->pAttrID = zcl_mem_alloc( (pCmd->numAttrs) << 1 );  // allocate 2 bytes per attribute
  if ( !pCmd->pAttrID )
  {
    return ( ZMemError ); // memory error
  }

  // convert attributes to native
  offset = 2;
  for ( i = 0; i < pCmd->numAttrs; ++i )
  {
    pCmd->pAttrID[i] = BUILD_UINT16( buf[offset], buf[offset+1] );;
    offset += 2;
  }

  return ( ZCL_STATUS_SUCCESS );
}

/*********************************************************************
 * @fn      zclPartition_ConvertOtaToNative_WriteHandshakeParam
 *
 * @brief   Helper function used to process an incoming WriteHandshakeParam
 *          command. Remember to free allocated memory.
 *
 * @param   pCmd   - (output) the converted command
 * @param   buf    - pointer to incoming frame (just after ZCL header)
 * @param   buflen - length of buffer (ZCL payload)
 *
 * @return  ZStatus_t - ZCL_STATUS_MALFORMED_COMMAND, ZMemError or ZCL_STATUS_SUCCESS
 */
ZStatus_t zclPartition_ConvertOtaToNative_WriteHandshakeParam( zclCmdWriteHandshakeParam_t *pCmd, uint8_t *buf, uint8_t buflen )
{
  uint8_t offset;
  uint8_t dataType;
  uint8_t i;
  uint8_t attrSize;

  // must have at least 1 attribute (clusterID + attrID)
  if ( buflen < 4 )
  {
    return ( ZCL_STATUS_MALFORMED_COMMAND );
  }

  pCmd->clusterID = BUILD_UINT16( buf[0], buf[1] );

  // allocate memory for attribute list
  // [clusterID][WriteRecord1][WriteRecord2]...
  pCmd->numRecords = 0;
  offset = 2;
  while ( offset < buflen )
  {
    dataType = buf[offset + 2];   // [AttrID][dataType][Attr]
    if ( dataType == ZCL_DATATYPE_UINT16 )
    {
      attrSize = 2;
    }
    else if ( dataType == ZCL_DATATYPE_UINT8 )
    {
      attrSize = 1;
    }
    else
    {
     return ( ZCL_STATUS_MALFORMED_COMMAND );  // can't convert invalid attributes
    }

    offset += 3 + attrSize;

    // ignore partial records
    if ( offset > buflen )
    {
      break;
    }
    ++pCmd->numRecords;
  }

  // allocate the memory
  pCmd->pWriteRecord = zcl_mem_alloc( pCmd->numRecords * sizeof( zclPartitionWriteRec_t ) );
  if ( !pCmd->pWriteRecord )
  {
    return ( ZMemError ); // memory error
  }

  // convert records native
  offset = 2;
  for ( i = 0; i < pCmd->numRecords; ++i )
  {
    pCmd->pWriteRecord[i].attrID = BUILD_UINT16( buf[offset], buf[offset+1] );
    pCmd->pWriteRecord[i].dataType = dataType = buf[offset+2];

    if ( dataType == ZCL_DATATYPE_UINT16 )
    {
      pCmd->pWriteRecord[i].attr = BUILD_UINT16( buf[offset+3], buf[offset+4] );
      attrSize = 2;
    }
    else if ( dataType == ZCL_DATATYPE_UINT8 )
    {
      pCmd->pWriteRecord[i].attr = buf[offset+3];
      attrSize = 1;
    }

    // move offset passed write record (2 bytes for attrID, 1 byte for AttrType + size of attr)
    offset += ( 3 + attrSize );
  }

  return ( ZCL_STATUS_SUCCESS );
}

/*********************************************************************
 * @fn      zclPartition_ConvertOtaToNative_MultipleAck
 *
 * @brief   Helper function used to process an incoming MultipleACK
 *          command. Remember to free allocated memory.
 *
 * @param   pCmd   - (output) the converted command
 * @param   buf    - pointer to incoming frame (just after ZCL header)
 * @param   buflen - length of buffer (ZCL payload)
 *
 * @return  ZStatus_t - ZCL_STATUS_MALFORMED_COMMAND, ZMemError or ZCL_STATUS_SUCCESS
 */
ZStatus_t zclPartition_ConvertOtaToNative_MultipleAck( zclCmdMultipleAck_t *pCmd, uint8_t *buf, uint8_t buflen )
{
  uint8_t nackSize;
  uint8_t offset;
  uint8_t i;

  // buffer not large enough!
  if ( buflen < 2 )
  {
    return ( ZCL_STATUS_MALFORMED_COMMAND );
  }

  pCmd->options = buf[0];
  if ( pCmd->options & ZCL_PARTITION_OPTIONS_NACK_16BIT )
  {
    pCmd->firstFrameID = BUILD_UINT16( buf[1], buf[2] );
    nackSize = 2;
    offset = 3;
  }
  else
  {
    pCmd->firstFrameID = buf[1];
    nackSize = 1;
    offset = 2;
  }

  // determine # of NACKs
  pCmd->numNAcks = ( buflen - offset ) / nackSize;

  // allocate enough memory for NACK IDs
  pCmd->pNAckID = zcl_mem_alloc ( pCmd->numNAcks * sizeof(uint16_t) );
  if ( !pCmd->pNAckID )
  {
    return ( ZMemError ); // memory error
  }

  // copy in NACK IDs
  for( i = 0; i < pCmd->numNAcks; ++i)
  {
    // copy NACk in native form
    if ( nackSize == 2 )
    {
      pCmd->pNAckID[i] = BUILD_UINT16( buf[offset], buf[offset+1] );
      offset += 2;
    }
    else
    {
      pCmd->pNAckID[i] = buf[offset++];
    }
  }

  return ( ZCL_STATUS_SUCCESS );
}

/*********************************************************************
 * @fn      zclPartition_ConvertOtaToNative_ReadHandshakeParamRsp
 *
 * @brief   Helper function used to process an incoming ReadHandshakeParamRsp
 *          command. Remember to free allocated memory.
 *
 * @param   pCmd   - (output) the converted command
 * @param   buf    - pointer to incoming frame (just after ZCL header)
 * @param   buflen - length of buffer (ZCL payload)
 *
 * @return  ZStatus_t - ZCL_STATUS_MALFORMED_COMMAND, ZMemError or ZCL_STATUS_SUCCESS
 */
ZStatus_t zclPartition_ConvertOtaToNative_ReadHandshakeParamRsp( zclCmdReadHandshakeParamRsp_t *pCmd, uint8_t *buf, uint8_t buflen )
{
  uint8_t status;
  uint8_t  dataType;
  uint8_t offset;
  uint8_t i;

  // must have at least 1 attribute (clusterID + attrID)
  if ( buflen < 4 )
  {
    return ( ZCL_STATUS_MALFORMED_COMMAND );
  }

  pCmd->clusterID = BUILD_UINT16( buf[0], buf[1] );

  // determine # of attributes
  offset = 2;
  pCmd->numRecords = 0;
  while ( offset < buflen )
  {
    // each record begins with [AttrID][Status]
    status = buf[offset+2];   // status listed in spec as 2 bytes, but it's really 1 byte
    offset += 3;              // skip [AttrID][Status]

    // followed by attrtype and data
    if ( status == ZCL_STATUS_SUCCESS )
    {
      // for partition cluster, all attributes are uint8_t or uint16_t
      dataType = buf[offset];
      if ( dataType == ZCL_DATATYPE_UINT8 )
      {
        offset += 2;
      }
      else if ( dataType == ZCL_DATATYPE_UINT16 )
      {
        offset += 3;
      }
      else
      {
        break;    // invalidate data type, we don't know what to do with it.
      }
    }

    ++pCmd->numRecords;
  }

  // allocate memory for attribute list
  pCmd->pReadRecord =  zcl_mem_alloc( pCmd->numRecords * sizeof( zclPartitionReadRec_t ) );
  if ( !pCmd->pReadRecord )
  {
    return ( ZMemError ); // memory error
  }

  // convert attributes records to native
  offset = 2;
  for ( i = 0; i < pCmd->numRecords; ++i )
  {
    pCmd->pReadRecord[i].attrID = BUILD_UINT16( buf[offset], buf[offset+1] );
    pCmd->pReadRecord[i].status = buf[offset+2];
    offset += 3;

    if ( status == ZCL_STATUS_SUCCESS )
    {
      // for partition cluster, all attributes are uint8_t or uint16_t
      pCmd->pReadRecord[i].dataType = dataType = buf[offset];
      if ( dataType == ZCL_DATATYPE_UINT8 )
      {
        pCmd->pReadRecord[i].attr = buf[offset+1];
        offset += 2;
      }
      else
      {
        pCmd->pReadRecord[i].attr = BUILD_UINT16( buf[offset+1], buf[offset+2] );
        offset += 3;
      }
    }
  }

  return ( ZCL_STATUS_SUCCESS );
}

/*********************************************************************
 * @fn      zclPartition_ProcessInCmd_TransferPartitionedFrame
 *
 * @brief   Convert an incoming TransferPartitionFrame to native format and pass to the app
 *          if registered.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPartition_ProcessInCmd_TransferPartitionedFrame( zclIncoming_t *pInMsg,
                                                                     zclPartition_AppCallbacks_t *pCBs )
{
  afAddrType_t *pSrcAddr;
  zclCmdTransferPartitionedFrame_t cmd;
  uint8_t msglen;
  ZStatus_t status;

  if ( pCBs->pfnPartition_TransferPartitionedFrame )
  {
    // the app callback will send response
    pSrcAddr = &(pInMsg->msg->srcAddr);

    // convert from OTA endian to native form
    msglen = (uint8_t)(pInMsg->pDataLen);   // pDataLen is a misnomer should be iDataLen (it's a 16-bit length of the data field).
    status = zclPartition_ConvertOtaToNative_TransferPartitionedFrame( &cmd, pInMsg->pData, msglen );
    if ( status != ZSuccess )
    {
      return ( status );    // failed to convert, give up
    }

    // send to the app, which will multi-ack if needed
    return ( pCBs->pfnPartition_TransferPartitionedFrame( pSrcAddr, &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPartition_ProcessInCmd_ReadHandshakeParam
 *
 * @brief   Process in the received Poll Control CheckIn cmd (sent to ZC/ZR)
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPartition_ProcessInCmd_ReadHandshakeParam( zclIncoming_t *pInMsg, zclPartition_AppCallbacks_t *pCBs )
{
  afAddrType_t * pSrcAddr;
  zclCmdReadHandshakeParam_t cmd;
  ZStatus_t status;
  uint8_t msglen;

  if ( pCBs->pfnPartition_ReadHandshakeParam )
  {

    // the app callback will send response
    pSrcAddr = &(pInMsg->msg->srcAddr);

    // convert from OTA endian to native form
    msglen = (uint8_t)(pInMsg->pDataLen);   // pDataLen is a misnomer should be iDataLen (it's a 16-bit length of the data field).
    status = zclPartition_ConvertOtaToNative_ReadHandshakeParam( &cmd, pInMsg->pData, msglen );
    if ( status != ZSuccess )
    {
      return ( status );
    }

    // also need the seq # for reply
    cmd.seqNum = pInMsg->hdr.transSeqNum;

    // send to the app.
    status = ( pCBs->pfnPartition_ReadHandshakeParam( pSrcAddr, &cmd ) );

    // done with temporary memory, free it.
    zcl_mem_free( cmd.pAttrID );

    return ( status );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPartition_ProcessInCmd_WriteHandshakeParam
 *
 * @brief   process an incoming WriteHandshakeParam command
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPartition_ProcessInCmd_WriteHandshakeParam( zclIncoming_t *pInMsg, zclPartition_AppCallbacks_t *pCBs )
{
  zclCmdWriteHandshakeParam_t cmd;
  uint8_t msglen;
  ZStatus_t status;

  if ( pCBs->pfnPartition_WriteHandshakeParam )
  {
    // convert from OTA endian to native form
    msglen = (uint8_t)(pInMsg->pDataLen);   // pDataLen is a misnomer should be iDataLen (it's a 16-bit length of the data field).
    status = zclPartition_ConvertOtaToNative_WriteHandshakeParam( &cmd, pInMsg->pData, msglen );
    if (  status != ZSuccess )
    {
      return ( status );
    }

    // send to the app
    return ( pCBs->pfnPartition_WriteHandshakeParam( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPartition_ProcessInCmd_MultipleAck
 *
 * @brief   Process the incoming multiple ACK so the app can move the state
 *          machine on to next set of blocks
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPartition_ProcessInCmd_MultipleAck( zclIncoming_t *pInMsg, zclPartition_AppCallbacks_t *pCBs )
{
  afAddrType_t * pSrcAddr;
  zclCmdMultipleAck_t cmd;
  uint8_t msglen;

  if ( pCBs->pfnPartition_MultipleAck )
  {

    // the app callback will respond
    pSrcAddr = &(pInMsg->msg->srcAddr);

    // convert from OTA endian to native form
    msglen = (uint8_t)(pInMsg->pDataLen);   // pDataLen is a misnomer should be iDataLen (it's a 16-bit length of the data field).
    zclPartition_ConvertOtaToNative_MultipleAck( &cmd, pInMsg->pData, msglen );

    // send to the app
    return ( pCBs->pfnPartition_MultipleAck( pSrcAddr, &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPartition_ProcessInCmd_ReadHandshakeParamRsp
 *
 * @brief   Process in the received ReadHandshake cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPartition_ProcessInCmd_ReadHandshakeParamRsp( zclIncoming_t *pInMsg, zclPartition_AppCallbacks_t *pCBs )
{
  zclCmdReadHandshakeParamRsp_t cmd;
  uint8_t msglen;
  ZStatus_t status;

  if ( pCBs->pfnPartition_ReadHandshakeParamRsp )
  {
    // convert from OTA endian to native form
    msglen = (uint8_t)(pInMsg->pDataLen);   // pDataLen is a misnomer should be iDataLen (it's a 16-bit length of the data field).
    status = zclPartition_ConvertOtaToNative_ReadHandshakeParamRsp( &cmd, pInMsg->pData, msglen );
    if ( status != ZSuccess )
    {
      return ( status );
    }

    // send to the app
    return ( pCBs->pfnPartition_ReadHandshakeParamRsp( &cmd ) );
  }

  return ( ZFailure );
}


/****************************************************************************
****************************************************************************/

#endif // ZCL_PARTITION
