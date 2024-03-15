/**************************************************************************************************
  Filename:       zcl_appliance_control.c
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    Zigbee Cluster Library - Appliance Control


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
#include "zcl_appliance_control.h"

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  #include "stub_aps.h"
#endif

#ifdef ZCL_APPLIANCE_CONTROL

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct zclApplianceControlCBRec
{
  struct zclApplianceControlCBRec *next;
  uint8_t endpoint;                              // Used to link it into the endpoint descriptor
  zclApplianceControl_AppCallbacks_t *CBs;     // Pointer to Callback function
} zclApplianceControlCBRec_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static zclApplianceControlCBRec_t *zclApplianceControlCBs = (zclApplianceControlCBRec_t *)NULL;
static uint8_t zclApplianceControlPluginRegisted = FALSE;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclApplianceControl_HdlIncoming( zclIncoming_t *pInHdlrMsg );
static ZStatus_t zclApplianceControl_HdlInSpecificCommands( zclIncoming_t *pInMsg );
static zclApplianceControl_AppCallbacks_t *zclApplianceControl_FindCallbacks( uint8_t endpoint );
static ZStatus_t zclApplianceControl_ProcessInCmds( zclIncoming_t *pInMsg, zclApplianceControl_AppCallbacks_t *pCBs );

static ZStatus_t zclApplianceControl_ProcessInCmd_CommandExecution( zclIncoming_t *pInMsg, zclApplianceControl_AppCallbacks_t *pCBs );
static ZStatus_t zclApplianceControl_ProcessInCmd_SignalState( zclIncoming_t *pInMsg, zclApplianceControl_AppCallbacks_t *pCBs );
static ZStatus_t zclApplianceControl_ProcessInCmd_WriteFunctions( zclIncoming_t *pInMsg, zclApplianceControl_AppCallbacks_t *pCBs );
static ZStatus_t zclApplianceControl_ProcessInCmd_OverloadPauseResume( zclIncoming_t *pInMsg, zclApplianceControl_AppCallbacks_t *pCBs );
static ZStatus_t zclApplianceControl_ProcessInCmd_OverloadPause( zclIncoming_t *pInMsg, zclApplianceControl_AppCallbacks_t *pCBs );
static ZStatus_t zclApplianceControl_ProcessInCmd_OverloadWarning( zclIncoming_t *pInMsg, zclApplianceControl_AppCallbacks_t *pCBs );
static ZStatus_t zclApplianceControl_ProcessInCmd_SignalStateRsp( zclIncoming_t *pInMsg, zclApplianceControl_AppCallbacks_t *pCBs );
static ZStatus_t zclApplianceControl_ProcessInCmd_SignalStateNotification( zclIncoming_t *pInMsg, zclApplianceControl_AppCallbacks_t *pCBs );

/*********************************************************************
 * @fn      zclApplianceControl_RegisterCmdCallbacks
 *
 * @brief   Register applications command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
ZStatus_t zclApplianceControl_RegisterCmdCallbacks( uint8_t endpoint, zclApplianceControl_AppCallbacks_t *callbacks )
{
  zclApplianceControlCBRec_t *pNewItem;
  zclApplianceControlCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( zclApplianceControlPluginRegisted == FALSE )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_GENERAL_APPLIANCE_CONTROL,
                        ZCL_CLUSTER_ID_GENERAL_APPLIANCE_CONTROL,
                        zclApplianceControl_HdlIncoming );
    zclApplianceControlPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = (zclApplianceControlCBRec_t *)zcl_mem_alloc( sizeof( zclApplianceControlCBRec_t ) );
  if ( pNewItem == NULL )
  {
    return ( ZMemError );
  }

  pNewItem->next = (zclApplianceControlCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclApplianceControlCBs == NULL )
  {
    zclApplianceControlCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclApplianceControlCBs;
    while ( pLoop->next != NULL )
      pLoop = pLoop->next;

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclApplianceControl_Send_CommandExecution
 *
 * @brief   Request sent to server for Appliance Control Execution of a Command.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   commandID - specifies the Appliance Control command to be executed
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceControl_Send_CommandExecution( uint8_t srcEP, afAddrType_t *dstAddr,
                                                            uint8_t commandID,
                                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[1];   // 1 byte payload

  buf[0] = commandID;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_APPLIANCE_CONTROL,
                          COMMAND_APPLIANCE_CONTROL_COMMAND_EXECUTION, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );
}

/*********************************************************************
 * @fn      zclApplianceControl_Send_SignalState
 *
 * @brief   Request sent to server for Appliance Control Signal State.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceControl_Send_SignalState( uint8_t srcEP, afAddrType_t *dstAddr,
                                                       uint8_t disableDefaultRsp, uint8_t seqNum )
{
  // no payload

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_APPLIANCE_CONTROL,
                          COMMAND_APPLIANCE_CONTROL_SIGNAL_STATE, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 0, 0 );
}

/*********************************************************************
 * @fn      zclApplianceControl_Send_WriteFunctions
 *
 * @brief   Request sent to server for Appliance Control Write Functions.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          functionID - Contains the ID of the function to be written
 *          pFunctionDataType - Contains the data type of the attribute to be written
 *          pFunctionData - Contains the actual value of the function to be written
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceControl_Send_WriteFunctions( uint8_t srcEP, afAddrType_t *dstAddr,
                                                          zclApplianceControlWriteFunctions_t *pPayload,
                                                          uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t i;
  uint8_t *pBuf;    // variable length payload
  uint8_t arrayRecordSize;  // number of bytes in record array
  uint16_t calculatedBufSize;
  ZStatus_t status;


  arrayRecordSize = zclGetDataTypeLength( pPayload->functionDataType );   // calculate length of pFunctionData field

  // get a buffer large enough to hold the whole packet
  calculatedBufSize = ( sizeof( zclApplianceControlWriteFunctions_t ) + arrayRecordSize - 2  );    // size of structure plus the size of the variable array

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = LO_UINT16(pPayload->functionID);
  pBuf[1] = HI_UINT16(pPayload->functionID);
  pBuf[2] = pPayload->functionDataType;
  for( i = 0; i < arrayRecordSize; ++i )
  {
    pBuf[3 + i] = pPayload->pFunctionData[i];
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_APPLIANCE_CONTROL,
                           COMMAND_APPLIANCE_CONTROL_WRITE_FUNCTIONS, TRUE,
                           ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );

  return ( status );
}

/*********************************************************************
 * @fn      zclApplianceControl_Send_OverloadPauseResume
 *
 * @brief   Request sent to server for Appliance Control Overload Pause Resume.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceControl_Send_OverloadPauseResume( uint8_t srcEP, afAddrType_t *dstAddr,
                                                               uint8_t disableDefaultRsp, uint8_t seqNum )
{
  // no payload

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_APPLIANCE_CONTROL,
                          COMMAND_APPLIANCE_CONTROL_OVERLOAD_PAUSE_RESUME, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 0, 0 );
}

/*********************************************************************
 * @fn      zclApplianceControl_Send_OverloadPause
 *
 * @brief   Request sent to server for Appliance Control Overload Pause.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceControl_Send_OverloadPause( uint8_t srcEP, afAddrType_t *dstAddr,
                                                         uint8_t disableDefaultRsp, uint8_t seqNum )
{
  // no payload

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_APPLIANCE_CONTROL,
                          COMMAND_APPLIANCE_CONTROL_OVERLOAD_PAUSE, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 0, 0 );
}

/*********************************************************************
 * @fn      zclApplianceControl_Send_OverloadWarning
 *
 * @brief   Request sent to server for Appliance Control Overload Warning.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   warningEvent - The ID of the events to be communicated to alert about possible overload
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceControl_Send_OverloadWarning( uint8_t srcEP, afAddrType_t *dstAddr,
                                                           uint8_t warningEvent,
                                                           uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[1]; // 1 byte payload

  buf[0] = warningEvent;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_APPLIANCE_CONTROL,
                          COMMAND_APPLIANCE_CONTROL_OVERLOAD_WARNING, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );
}

/*********************************************************************
 * @fn      zclApplianceControl_Send_SignalStateRsp
 *
 * @brief   Response sent to client for Appliance Control Signal State Response.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   applianceStatus - current status of the appliance
 * @param   remoteEnableFlagsDeviceStatus2 - current status of appliance correlated with remote control
 * @param   applianceStatus2 - detailed definition of appliance state
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceControl_Send_SignalStateRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                                          uint8_t applianceStatus, uint8_t remoteEnableFlagsDeviceStatus2,
                                                          uint24 applianceStatus2,
                                                          uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[5]; // 5 byte payload

  buf[0] = applianceStatus;
  buf[1] = remoteEnableFlagsDeviceStatus2;
  buf[2] = BREAK_UINT32( applianceStatus2, 0 );
  buf[3] = BREAK_UINT32( applianceStatus2, 1 );
  buf[4] = BREAK_UINT32( applianceStatus2, 2 );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_APPLIANCE_CONTROL,
                          COMMAND_APPLIANCE_CONTROL_SIGNAL_STATE_RSP, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );
}

/*********************************************************************
 * @fn      zclApplianceControl_Send_SignalStateNotification
 *
 * @brief   Response sent to client for Appliance Control Signal State Response.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   applianceStatus - current status of the appliance
 * @param   remoteEnableFlagsDeviceStatus2 - current status of appliance correlated with remote control
 * @param   applianceStatus2 - detailed definition of appliance state
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceControl_Send_SignalStateNotification( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                   uint8_t applianceStatus, uint8_t remoteEnableFlagsDeviceStatus2,
                                                                   uint24 applianceStatus2,
                                                                   uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[5]; // 5 byte payload

  buf[0] = applianceStatus;
  buf[1] = remoteEnableFlagsDeviceStatus2;
  buf[2] = BREAK_UINT32( applianceStatus2, 0 );
  buf[3] = BREAK_UINT32( applianceStatus2, 1 );
  buf[4] = BREAK_UINT32( applianceStatus2, 2 );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_APPLIANCE_CONTROL,
                          COMMAND_APPLIANCE_CONTROL_SIGNAL_STATE_NOTIFICATION, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );
}

/*********************************************************************
 * @fn      zclApplianceControl_FindCallbacks
 *
 * @brief   Find the callbacks for an endpoint
 *
 * @param   endpoint - endpoint to find the application callbacks for
 *
 * @return  pointer to the callbacks
 */
static zclApplianceControl_AppCallbacks_t *zclApplianceControl_FindCallbacks( uint8_t endpoint )
{
  zclApplianceControlCBRec_t *pCBs;

  pCBs = zclApplianceControlCBs;
  while ( pCBs != NULL )
  {
    if ( pCBs->endpoint == endpoint )
    {
      return ( pCBs->CBs );
    }
    pCBs = pCBs->next;
  }
  return ( (zclApplianceControl_AppCallbacks_t *)NULL );
}

/*********************************************************************
 * @fn      zclApplianceControl_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceControl_HdlIncoming( zclIncoming_t *pInMsg )
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
      stat = zclApplianceControl_HdlInSpecificCommands( pInMsg );
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
 * @fn      zclApplianceControl_HdlInSpecificCommands
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library

 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceControl_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  zclApplianceControl_AppCallbacks_t *pCBs;

  // make sure endpoint exists
  pCBs = zclApplianceControl_FindCallbacks( pInMsg->msg->endPoint );
  if ( pCBs == NULL )
  {
    return ( ZFailure );
  }

  stat = zclApplianceControl_ProcessInCmds( pInMsg, pCBs );

  return ( stat );
}

/*********************************************************************
 * @fn      zclApplianceControl_ProcessInCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis

 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceControl_ProcessInCmds( zclIncoming_t *pInMsg, zclApplianceControl_AppCallbacks_t *pCBs )
{
  ZStatus_t stat;

  // Client-to-Server
  if( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    switch( pInMsg->hdr.commandID )
    {
      case COMMAND_APPLIANCE_CONTROL_COMMAND_EXECUTION:
        stat = zclApplianceControl_ProcessInCmd_CommandExecution( pInMsg, pCBs );
        break;

      case COMMAND_APPLIANCE_CONTROL_SIGNAL_STATE:
        stat = zclApplianceControl_ProcessInCmd_SignalState( pInMsg, pCBs );
        break;

      case COMMAND_APPLIANCE_CONTROL_WRITE_FUNCTIONS:
        stat = zclApplianceControl_ProcessInCmd_WriteFunctions( pInMsg, pCBs );
        break;

      case COMMAND_APPLIANCE_CONTROL_OVERLOAD_PAUSE_RESUME:
        stat = zclApplianceControl_ProcessInCmd_OverloadPauseResume( pInMsg, pCBs );
        break;

      case COMMAND_APPLIANCE_CONTROL_OVERLOAD_PAUSE:
        stat = zclApplianceControl_ProcessInCmd_OverloadPause( pInMsg, pCBs );
        break;

      case COMMAND_APPLIANCE_CONTROL_OVERLOAD_WARNING:
        stat = zclApplianceControl_ProcessInCmd_OverloadWarning( pInMsg, pCBs );
        break;

      default:
        // Unknown command
        stat = ZFailure;
        break;
    }
  }
  // sent Server-to-Client
  else
  {
    switch( pInMsg->hdr.commandID )
    {
      case COMMAND_APPLIANCE_CONTROL_SIGNAL_STATE_RSP:
        stat = zclApplianceControl_ProcessInCmd_SignalStateRsp( pInMsg, pCBs );
        break;

      case COMMAND_APPLIANCE_CONTROL_SIGNAL_STATE_NOTIFICATION:
        stat = zclApplianceControl_ProcessInCmd_SignalStateNotification( pInMsg, pCBs );
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
 * @fn      zclApplianceControl_ProcessInCmd_SignalStateRsp
 *
 * @brief   Process in the received Signal State Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceControl_ProcessInCmd_SignalStateRsp( zclIncoming_t *pInMsg,
                                                                  zclApplianceControl_AppCallbacks_t *pCBs )
{
  zclApplianceControlSignalState_t cmd;

  if ( pCBs->pfnApplianceControl_SignalStateRsp )
  {
    cmd.applianceStatus = pInMsg->pData[0];
    cmd.remoteEnableFlagsDeviceStatus2 = pInMsg->pData[1];
    cmd.applianceStatus2 = BUILD_UINT32( pInMsg->pData[2], pInMsg->pData[3], pInMsg->pData[4], 0 );

    return ( pCBs->pfnApplianceControl_SignalStateRsp( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclApplianceControl_ProcessInCmd_SignalStateNotification
 *
 * @brief   Process in the received Signal State Notification cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceControl_ProcessInCmd_SignalStateNotification( zclIncoming_t *pInMsg,
                                                                           zclApplianceControl_AppCallbacks_t *pCBs )
{
  zclApplianceControlSignalState_t cmd;

  if ( pCBs->pfnApplianceControl_SignalStateNotification )
  {
    cmd.applianceStatus = pInMsg->pData[0];
    cmd.remoteEnableFlagsDeviceStatus2 = pInMsg->pData[1];
    cmd.applianceStatus2 = BUILD_UINT32( pInMsg->pData[2], pInMsg->pData[3], pInMsg->pData[4], 0 );

    return ( pCBs->pfnApplianceControl_SignalStateNotification( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclApplianceControl_ProcessInCmd_CommandExecution
 *
 * @brief   Process in the received Command Execution cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceControl_ProcessInCmd_CommandExecution( zclIncoming_t *pInMsg,
                                                                    zclApplianceControl_AppCallbacks_t *pCBs )
{
  zclApplianceControlCommandExecution_t cmd;

  if ( pCBs->pfnApplianceControl_CommandExecution )
  {
    cmd.commandID = pInMsg->pData[0];

    return ( pCBs->pfnApplianceControl_CommandExecution( pInMsg, &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclApplianceControl_ProcessInCmd_SignalState
 *
 * @brief   Process in the received Signal State cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceControl_ProcessInCmd_SignalState( zclIncoming_t *pInMsg,
                                                               zclApplianceControl_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnApplianceControl_SignalState )
  {
    // no payload
    return ( pCBs->pfnApplianceControl_SignalState( pInMsg ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclApplianceControl_ProcessInCmd_WriteFunctions
 *
 * @brief   Process in the received Write Functions cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceControl_ProcessInCmd_WriteFunctions( zclIncoming_t *pInMsg,
                                                                  zclApplianceControl_AppCallbacks_t *pCBs )
{
  uint8_t i;
  uint8_t calculatedArraySize;
  zclApplianceControlWriteFunctions_t cmd;
  ZStatus_t status;

  if ( pCBs->pfnApplianceControl_WriteFunctions )
  {
    calculatedArraySize = pInMsg->pDataLen - 3;   // calculate size of pFunctionData field

    cmd.pFunctionData = zcl_mem_alloc( calculatedArraySize );
    if ( !cmd.pFunctionData )
    {
      return ( ZMemError );  // no memory, return error
    }

    cmd.functionID = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
    cmd.functionDataType = pInMsg->pData[2];
    for( i = 0; i < calculatedArraySize; i++ )
    {
      cmd.pFunctionData[i] = pInMsg->pData[3 + i];
    }

    status = ( pCBs->pfnApplianceControl_WriteFunctions( pInMsg, &cmd ) );
    zcl_mem_free( cmd.pFunctionData );
    return status;
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclApplianceControl_ProcessInCmd_OverloadPauseResume
 *
 * @brief   Process in the received Overload Pause Resume cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceControl_ProcessInCmd_OverloadPauseResume( zclIncoming_t *pInMsg,
                                                                       zclApplianceControl_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnApplianceControl_OverloadPauseResume )
  {
    // no payload
    return ( pCBs->pfnApplianceControl_OverloadPauseResume( ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclApplianceControl_ProcessInCmd_OverloadPause
 *
 * @brief   Process in the received Overload Pause cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceControl_ProcessInCmd_OverloadPause( zclIncoming_t *pInMsg,
                                                                 zclApplianceControl_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnApplianceControl_OverloadPause )
  {
    // no payload
    return ( pCBs->pfnApplianceControl_OverloadPause( ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclApplianceControl_ProcessInCmd_OverloadWarning
 *
 * @brief   Process in the received Overload Warning cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceControl_ProcessInCmd_OverloadWarning( zclIncoming_t *pInMsg,
                                                                   zclApplianceControl_AppCallbacks_t *pCBs )
{
  zclApplianceControlOverloadWarning_t cmd;

  if ( pCBs->pfnApplianceControl_OverloadWarning )
  {
    cmd.warningEvent = pInMsg->pData[0];

    return ( pCBs->pfnApplianceControl_OverloadWarning( &cmd ) );
  }

  return ( ZFailure );
}

#endif // ZCL_APPLIANCE_CONTROL
/****************************************************************************
****************************************************************************/
