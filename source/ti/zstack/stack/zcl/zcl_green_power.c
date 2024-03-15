/**************************************************************************************************
  Filename:       zcl_green_power.c
  Revised:        $Date: 2015-09-10 09:36:48 -0700 (Thu, 10 Sep 2015) $
  Revision:       $Revision: 44493 $

  Description:    Zigbee Cluster Library - General.  This application receives all
                  ZCL messages and initially parses them before passing to application.


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
#include "zglobals.h"

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
#include "string.h"
#include "zcomdef.h"
#include "aps.h"
#include "bdb.h"
#include "zcl.h"
#include "zcl_green_power.h"
#include "gp_common.h"
#include "stub_aps.h"

#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
#include "util_timer.h"
#endif

/*********************************************************************
 * extern
 */
#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
extern ClockP_Struct gpAppDataSendClk;
extern ClockP_Handle gpAppDataSendClkHandle;
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

typedef struct zclGpCBRec
{
  struct zclGpCBRec        *next;
  uint8_t                     endpoint; // Used to link it into the endpoint descriptor
  zclGp_AppCallbacks_t     *CBs;      // Pointer to Callback function
} zclGpCBRec_t;


/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static zclGpCBRec_t *zclGpCBs = (zclGpCBRec_t *)NULL;
static uint8_t zclGpPluginRegisted = FALSE;
static gpNotificationMsg_t *pNotificationMsgHead = NULL;
static gpCmdPayloadMsg_t *pCmdPayloadMsgHead = NULL;

//static zclGenSceneItem_t *zclGenSceneTable = (zclGenSceneItem_t *)NULL;


/*********************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclGp_HdlIncoming( zclIncoming_t *pInMsg );
static ZStatus_t zclGp_HdlInSpecificCommands( zclIncoming_t *pInMsg );
static zclGp_AppCallbacks_t *zclGp_FindCallbacks( uint8_t endpoint );
static uint8_t gp_addPairedSinksToMsgQueue( gpdID_t* pGpdID, gpCmdPayloadMsg_t* pMsg );
static ZStatus_t zclGp_ProcessInBasicCombo( zclIncoming_t *pInMsg, zclGp_AppCallbacks_t *pCBs );
static ZStatus_t zclGp_ProcessInBasicProxy( zclIncoming_t *pInMsg, zclGp_AppCallbacks_t *pCBs );

/*********************************************************************
 * @fn      zclGp_RegisterCmdCallbacks
 *
 * @brief   Register an applications command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
ZStatus_t zclGp_RegisterCmdCallbacks( uint8_t endpoint, zclGp_AppCallbacks_t *callbacks )
{
  zclGpCBRec_t *pNewItem;
  zclGpCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( zclGpPluginRegisted == FALSE )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_GREEN_POWER,
                        ZCL_CLUSTER_ID_GREEN_POWER,
                        zclGp_HdlIncoming );

    zclGpPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclGpCBRec_t ) );
  if ( pNewItem == NULL )
    return (ZMemError);

  pNewItem->next = (zclGpCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if (  zclGpCBs == NULL )
  {
    zclGpCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclGpCBs;
    while ( pLoop->next != NULL )
      pLoop = pLoop->next;

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclGp_FindCallbacks
 *
 * @brief   Find the callbacks for an endpoint
 *
 * @param   endpoint - endpoint to find the application callbacks for
 *
 * @return  pointer to the callbacks
 */
static zclGp_AppCallbacks_t *zclGp_FindCallbacks( uint8_t endpoint )
{
  zclGpCBRec_t *pCBs;

  pCBs = zclGpCBs;
  while ( pCBs )
  {
    if ( pCBs->endpoint == endpoint )
      return ( pCBs->CBs );
    pCBs = pCBs->next;
  }
  return ( (zclGp_AppCallbacks_t *)NULL );
}

/*********************************************************************
 * @fn      zclGp_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGp_HdlIncoming( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  if ( StubAPS_InterPan( pInMsg->msg->srcAddr.panId, pInMsg->msg->srcAddr.endPoint ) )
    return ( stat ); // Cluster not supported thru Inter-PAN
#endif
  if ( zcl_ClusterCmd( pInMsg->hdr.fc.type ) )
  {
    // Is this a manufacturer specific command?
    if ( pInMsg->hdr.fc.manuSpecific == 0 )
    {
      stat = zclGp_HdlInSpecificCommands( pInMsg );
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
 * @fn      zclGp_HdlInSpecificCommands
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library

 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGp_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t stat;
  zclGp_AppCallbacks_t *pCBs;

  // make sure endpoint exists
  pCBs = zclGp_FindCallbacks( pInMsg->msg->endPoint );
  if ( pCBs == NULL )
    return ( ZFailure );

  switch ( pInMsg->msg->clusterId )
  {
    case ZCL_CLUSTER_ID_GREEN_POWER:
      if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
      {
        // Basic Combo handler
        stat = zclGp_ProcessInBasicCombo( pInMsg, pCBs );
      }
      else
      {
        // Basic Proxy handler
        stat = zclGp_ProcessInBasicProxy( pInMsg, pCBs );
      }
      break;

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}


/*********************************************************************
 * @fn      zclGp_ProcessInBasicCombo
 *
 * @brief   Process in the received server commands for the Basic Proxy.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGp_ProcessInBasicCombo( zclIncoming_t *pInMsg,
                                            zclGp_AppCallbacks_t *pCBs )
{
#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
  switch ( pInMsg->hdr.commandID )
  {
  case COMMAND_GP_NOTIFICATION:
      if(pCBs->pfnGpNotificationCmd)
      {
        zclGpNotification_t cmd;
        uint8_t  *pTempData;
        uint16_t options = 0;

        pTempData = pInMsg->pData;
        cmd.DataLen = pInMsg->pDataLen;

        //build options field
        options = (*pTempData & 0xFF);
        pTempData++;
        options |= ((*pTempData & 0xFF) << 8);
        pTempData++;


        //Get application ID
        cmd.gpdId.appID = options & 0x03;  //Filter the options fields in the notification

        //Application ID is 4 bytes
        if(cmd.gpdId.appID == GP_OPT_APP_ID_GPD)
        {
          OsalPort_memcpy(&cmd.gpdId.id.srcID, pTempData, sizeof(uint32_t));
          pTempData += sizeof(uint32_t);
          cmd.DataLen -= sizeof(uint32_t);
          cmd.GPDEndpoint = 0x00;  //invalid endpoint
        }
        //Application ID is 8 bytes
        else if(cmd.gpdId.appID == GP_OPT_APP_ID_IEEE)
        {
          OsalPort_memcpy(cmd.gpdId.id.gpdExtAddr, pTempData, Z_EXTADDR_LEN);
          pTempData += Z_EXTADDR_LEN;
          cmd.DataLen -= Z_EXTADDR_LEN;

          cmd.GPDEndpoint = *pTempData;
          pTempData++;
          cmd.DataLen--;
        }

        //Security frame counter
        cmd.securityFrameCounter = zcl_build_uint32(pTempData, sizeof(uint32_t));
        pTempData += sizeof(uint32_t);

        //Command ID
        cmd.gpCmdId = *pTempData;
        pTempData++;

        //Get command length
        cmd.DataLen = *pTempData;
        pTempData++;

        cmd.pData = pTempData;

        //Check if proxy info is present
        if(options & 0x4000)
        {
          pTempData += cmd.DataLen;

          cmd.GPPAddress = *pTempData;
          pTempData++;
          cmd.GPPAddress |= ((*pTempData) << 8);
          pTempData++;
          cmd.GPPLink = *pTempData;
        }
        else
        {
          cmd.GPPAddress = INVALID_NODE_ADDR;
          cmd.GPPLink = 0;
        }

        if(cmd.DataLen == 0)
        {
          cmd.pData = NULL;
        }

        pCBs->pfnGpNotificationCmd( &cmd );

      }
    break;

  case COMMAND_GP_COMMISSIONING_NOTIFICATION:
      if(pCBs->pfnGpCommissioningNotificationCmd)
      {
        zclGpCommissioningNotification_t cmd;
        uint8_t  *pTempData;

        pTempData = pInMsg->pData;
        cmd.DataLen = pInMsg->pDataLen;
        gp_bfParse_commissioningNotificationOptions(&cmd.options, *(uint16_t*)pTempData);
        pTempData += sizeof(uint16_t);

        //Get application ID
        cmd.gpdId.appID = cmd.options.appId;  //Filter the options fields in the notification

        //Application ID is 4 bytes
        if(cmd.gpdId.appID == GP_OPT_APP_ID_GPD)
        {
          OsalPort_memcpy(&cmd.gpdId.id.srcID, pTempData, sizeof(uint32_t));
          pTempData += sizeof(uint32_t);
          cmd.DataLen -= sizeof(uint32_t);
          cmd.GPDEndpoint = 0x00;  //invalid endpoint
        }
        //Application ID is 8 bytes
        else if(cmd.gpdId.appID == GP_OPT_APP_ID_IEEE)
        {
          OsalPort_memcpy(cmd.gpdId.id.gpdExtAddr, pTempData, Z_EXTADDR_LEN);
          pTempData += Z_EXTADDR_LEN;
          cmd.DataLen -= Z_EXTADDR_LEN;

          cmd.GPDEndpoint = *pTempData;
          pTempData++;
          cmd.DataLen--;
        }

        //Security frame counter
        cmd.securityFrameCounter = zcl_build_uint32(pTempData, sizeof(uint32_t));
        pTempData += sizeof(uint32_t);

        //Command ID
        cmd.gpCmdId = *pTempData;
        pTempData++;

        //Get command length
        cmd.DataLen = *pTempData;
        pTempData++;

        cmd.pData = pTempData;

        //Check if proxy info is present
        if(cmd.options.proxyInfoPresent == TRUE)
        {
          pTempData += cmd.DataLen;

          cmd.GPPAddress = *pTempData;
          pTempData++;
          cmd.GPPAddress |= ((*pTempData) << 8);
          pTempData++;
          cmd.GPPLink = *pTempData;
        }
        else
        {
          cmd.GPPAddress = INVALID_NODE_ADDR;
          cmd.GPPLink = 0;
        }

        if(cmd.DataLen == 0)
        {
          cmd.pData = NULL;
        }

        //Check if security processing failed
        if(cmd.options.securityProcessingFailed == TRUE )
        {
            cmd.MIC = zcl_build_uint32(pTempData, sizeof(uint32_t));
            pTempData += sizeof(uint32_t);
        }
        pCBs->pfnGpCommissioningNotificationCmd( &cmd );

      }
    break;

  case COMMAND_GP_PAIRING_CONFIGURATION:
      if ( pCBs->pfnGpPairingConfigCmd )
      {
        //The command should be bigger than actions and options, the remaining payload will be parsed later.
        if((pInMsg->pData == NULL) || (pInMsg->pDataLen <= 3))
        {
          return ZFailure;
        }

        zclGpPairingConfig_t cmd;

        zcl_InSeqNum = pInMsg->hdr.transSeqNum;

        cmd.srcAddr = &(pInMsg->msg->srcAddr);
        cmd.actions = *pInMsg->pData;
        pInMsg->pData++;
        zcl_memcpy( &cmd.options, (void*)pInMsg->pData, sizeof(cmd.options) );
        pInMsg->pData += sizeof(cmd.options);  // Move data pointer after options field

        cmd.pData = pInMsg->pData;
        pCBs->pfnGpPairingConfigCmd( &cmd );
      }
    break;

  case COMMAND_GP_SINK_TABLE_REQUEST:

    break;
  }
#else
  // Device does not support GPCB. Send response with status Unsup Cmd
  zclDefaultRspCmd_t defaultRspCmd;

  defaultRspCmd.commandID = pInMsg->hdr.commandID;
  defaultRspCmd.statusCode = ZCL_STATUS_UNSUP_CLUSTER_COMMAND;
  zcl_SendDefaultRspCmd( GREEN_POWER_INTERNAL_ENDPOINT, &pInMsg->msg->srcAddr,
                          ZCL_CLUSTER_ID_GREEN_POWER, &defaultRspCmd,
                          ZCL_FRAME_CLIENT_SERVER_DIR, TRUE, 0, zcl_InSeqNum );

#endif
  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclGp_ProcessInBasicProxy
 *
 * @brief   Process in the received client commands for the Basic Proxy.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGp_ProcessInBasicProxy( zclIncoming_t *pInMsg,
                                          zclGp_AppCallbacks_t *pCBs )
{
  switch ( pInMsg->hdr.commandID )
  {
  case COMMAND_GP_PAIRING:
    if ( pCBs->pfnGpPairingCmd )
    {
      zclGpPairing_t cmd;

      zcl_InSeqNum = pInMsg->hdr.transSeqNum;

      cmd.wasBroadcast = pInMsg->msg->wasBroadcast;
      cmd.srcAddr = &(pInMsg->msg->srcAddr);
      zcl_memcpy( cmd.options, (void*)pInMsg->pData, sizeof(cmd.options) );
      pInMsg->pData += sizeof(cmd.options);  // Move data pointer after options field

      cmd.pData = pInMsg->pData;
      pCBs->pfnGpPairingCmd( &cmd );
    }
    break;

  case COMMAND_GP_PROXY_TABLE_REQUEST:
    if ( pCBs->pfnGpProxyTableReqCmd )
    {
      zclGpTableRequest_t cmd;

      zcl_InSeqNum = pInMsg->hdr.transSeqNum;
      cmd.srcAddr = &(pInMsg->msg->srcAddr);
      zcl_memcpy( &cmd.options, (void*)pInMsg->pData, sizeof(cmd.options) );
      pInMsg->pData += sizeof(cmd.options);  // Move data pointer after options field

      cmd.pData = (void*)pInMsg->pData;
      pCBs->pfnGpProxyTableReqCmd( &cmd );

    }
    break;
  case COMMAND_GP_PROXY_COMMISSIONING_MODE:
    if(pCBs->pfnGpProxyCommissioningModeCmd)
    {
      zclGpProxyCommissioningMode_t cmd;

      zcl_memcpy( &cmd.options, (void*)pInMsg->pData, sizeof(cmd.options) );
      pInMsg->pData += sizeof(cmd.options);  // Move data pointer after options field

      cmd.pData = (void*)pInMsg->pData;

      if(pInMsg->msg->srcAddr.addrMode == afAddr16Bit)
      {
        cmd.srcAddr = pInMsg->msg->srcAddr.addr.shortAddr;
      }

      pCBs->pfnGpProxyCommissioningModeCmd( &cmd );
    }
    break;
  case COMMAND_GP_RESPONSE:
    if(pCBs->pfnGpResponseCommand)
    {
      zclGpResponse_t cmd;

      zcl_memcpy( &cmd.options, (void*)pInMsg->pData, sizeof(cmd.options) );
      pInMsg->pData += sizeof(cmd.options);  // Move data pointer after options field

      cmd.tempMasterShortAddr = OsalPort_buildUint16(pInMsg->pData);
      pInMsg->pData += sizeof(cmd.tempMasterShortAddr);  // Move data pointer after tempMaster field

      zcl_memcpy( &cmd.tempMasterTxChannel, (void*)pInMsg->pData, sizeof(cmd.tempMasterTxChannel) );
      pInMsg->pData += sizeof(cmd.tempMasterTxChannel);  // Move data pointer after options field

      cmd.dstAddr = pInMsg->msg->macDestAddr;

      cmd.pData = (void*)pInMsg->pData;

      pCBs->pfnGpResponseCommand( &cmd );
    }
    break;
  }
  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclGp_SendGpNotificationCommand
 *
 * @brief   Send the Green Power Notification Command to a device
 *
 * @param   pCmd - Pointer to GP Notification Struct
 *          secNum - Sequence number given by the GPDF
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGp_SendGpNotificationCommand( gpNotificationCmd_t *pCmd, uint8_t secNum )
{
  uint8_t *buf = NULL;
  uint8_t *pBuf = NULL;
  ZStatus_t status = ZFailure;
  uint16_t len = 11;  // options + GPD Sec Counter + Cmd ID + payloadLen + GPP Short Address + GPP-GPD link
  gpCmdPayloadMsg_t *pCmdPayloadMsgCurr = NULL;
  gpdID_t gpdID;

  // Check some stuff to calculate the packet len
  // If Application Id bitfield is 0b000
  if(GP_PAIRING_OPT_IS_APPLICATION_ID_GPD(pCmd->options))
  {
    len += 4;  // GPD ID
  }
  // If Application Id bitfield is 0b010
  else if(GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(pCmd->options))
  {
    len += 9;  // IEEE addr + EP
  }

  // Length of the command payload
  len += pCmd->payloadLen;

  buf = zcl_mem_alloc(len);
  if(buf != NULL)
  {
    pBuf = buf;
    zcl_memcpy( pBuf, &pCmd->options, sizeof(uint16_t));
    pBuf += sizeof(uint16_t);

    // If Application Id bitfield is 0b000
    if(GP_PAIRING_OPT_IS_APPLICATION_ID_GPD(pCmd->options))
    {
      gpdID.appID = GP_OPT_APP_ID_GPD;
      zcl_memcpy(&gpdID.id.srcID, &pCmd->gpdId, sizeof(uint32_t));
      zcl_memcpy(pBuf, &pCmd->gpdId, sizeof(uint32_t));
      pBuf += sizeof(uint32_t);
    }
    // If Application Id bitfield is 0b010
    else if(GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(pCmd->options))
    {
      gpdID.appID = GP_OPT_APP_ID_IEEE;
      zcl_memcpy(gpdID.id.gpdExtAddr, pCmd->gpdIEEE, Z_EXTADDR_LEN);
      zcl_memcpy(pBuf, pCmd->gpdIEEE, Z_EXTADDR_LEN);
      pBuf += Z_EXTADDR_LEN;
      *pBuf++ = pCmd->ep;
    }

    zcl_memcpy(pBuf, &pCmd->gpdSecCounter, sizeof(uint32_t));
    pBuf += sizeof(uint32_t);

    *pBuf++ = pCmd->cmdId;

    *pBuf++ = pCmd->payloadLen;
    if(pCmd->payloadLen > 0)
    {
      if(pCmd->cmdPayload == NULL)
      {
          return ZMemError;
      }
      zcl_memcpy(pBuf, pCmd->cmdPayload, pCmd->payloadLen);
      pBuf += pCmd->payloadLen;
      zcl_mem_free(pCmd->cmdPayload);
    }

    zcl_memcpy(pBuf, &pCmd->gppShortAddr, sizeof(uint16_t));
    pBuf += sizeof(uint16_t);

    *pBuf++ = pCmd->gppGpdLink;
    pCmdPayloadMsgCurr = gp_AddCmdPayloadMsgNode(&pCmdPayloadMsgHead, buf, len);
    if((pCmdPayloadMsgCurr != NULL))
    {
      pCmdPayloadMsgCurr->secNum = secNum;
      gp_addPairedSinksToMsgQueue(&gpdID, pCmdPayloadMsgCurr);

#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
      UtilTimer_setTimeout(gpAppDataSendClkHandle, GP_QUEUE_DATA_SEND_INTERVAL);
      UtilTimer_start(&gpAppDataSendClk);
#else
      OsalPortTimers_startTimer( zcl_TaskID, ZCL_DATABUF_SEND, 50 );
#endif
    }
  }
  else
    status = ZMemError;

  return status;
}

/*********************************************************************
 * @fn      zclGp_SendGpCommissioningNotificationCommand
 *
 * @brief   Send the Green Power Commissioning Notification Command to a device
 *
 * @param   pCmd - Pointer to GP Commissioning Notification Struct
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGp_SendGpCommissioningNotificationCommand( gpCommissioningNotificationCmd_t *pCmd, uint8_t secNumber, gpdID_t* pGpdID, uint8_t* entry )
{
  uint8_t *buf = NULL;
  uint8_t *pBuf = NULL;
  ZStatus_t status = ZSuccess;
  uint16_t len = 11;  // options + GPD Sec Counter + Cmd ID + payloadLen + GPP Short Address + GPP-GPD link

  // Check some stuff to calculate the packet len
  // If Application Id bitfield is 0b000
  if(GP_PAIRING_OPT_IS_APPLICATION_ID_GPD(pCmd->options))
  {
    len += 4;  // GPD ID
  }
  // If Application Id bitfield is 0b010
  else if(GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(pCmd->options))
  {
    len += 9;  // IEEE addr + EP
  }

  if(GP_CNTF_GET_SEC_FAIL(HI_UINT16(pCmd->options)) == TRUE)
  {
    len += 4; // MIC
  }


  // This is an encrypted gp success frame
  if(((pCmd->options & (1 << 9)) == FALSE) && (pCmd->cmdId == GP_SUCCESS_COMMAND_ID) ||
    (((pCmd->options & (1 << 9)) == TRUE) && (pCmd->payloadLen == 0x01)))
  {
    pCmd->payloadLen = 0x00;
  }

  // Length of the command payload
  len += pCmd->payloadLen;
  buf = zcl_mem_alloc(len);
  if(buf)
  {
    pBuf = buf;

    zcl_memcpy(pBuf, &pCmd->options, sizeof(uint16_t));
    pBuf += sizeof(uint16_t);

    // If Application Id bitfield is 0b000
    if(GP_PAIRING_OPT_IS_APPLICATION_ID_GPD(pCmd->options))
    {
      zcl_memcpy( pBuf, &pCmd->gpdId, sizeof(uint32_t));
      pBuf += sizeof(uint32_t);
    }
    // If Application Id bitfield is 0b010
    else if(GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(pCmd->options))
    {
      zcl_memcpy(pBuf, &pCmd->gpdIEEE, Z_EXTADDR_LEN);
      pBuf += Z_EXTADDR_LEN;
      *pBuf++ = pCmd->ep;
    }

    zcl_memcpy(pBuf, &pCmd->gpdSecCounter, sizeof(uint32_t));
    pBuf += sizeof(uint32_t);

    *pBuf++ = pCmd->cmdId;

    *pBuf++ = pCmd->payloadLen;
    if(pCmd->payloadLen > 0)
    {
      if(pCmd->cmdPayload == NULL)
      {
        return ZMemError;
      }
      zcl_memcpy(pBuf, pCmd->cmdPayload, pCmd->payloadLen);
      pBuf += pCmd->payloadLen;
      zcl_mem_free(pCmd->cmdPayload);
    }

    zcl_memcpy(pBuf, &pCmd->gppShortAddr, sizeof(uint16_t));
    pBuf += sizeof(uint16_t);

    *pBuf++ = pCmd->gppGpdLink;

    if(GP_CNTF_GET_SEC_FAIL(HI_UINT16(pCmd->options)) == TRUE)
    {
      zcl_memcpy(pBuf, &pCmd->mic, sizeof(uint32_t));
    }

    commissioningNotificationMsg.len = len;
    commissioningNotificationMsg.pbuf = buf;
    commissioningNotificationMsg.addr.endPoint = GREEN_POWER_INTERNAL_ENDPOINT;
    commissioningNotificationMsg.addr.panId = _NIB.nwkPanId;
    // Sequence number of alias frames, must be module 12. A.3.6.3.3.2 Derivation
    // of alias sequence number
    commissioningNotificationMsg.seqNum = secNumber - 12;
    if(gp_unicastCommunication)
    {
      commissioningNotificationMsg.addr.addrMode = afAddr16Bit;
      commissioningNotificationMsg.addr.addr.shortAddr = gp_commissionerAddress;
    }
    else
    {
      commissioningNotificationMsg.addr.addrMode = afAddrBroadcast;
      commissioningNotificationMsg.addr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVRXON;
    }
    // If there is an entry and has an assigned alias
    if((entry != NULL) && (PROXY_TBL_GET_ASSIGNED_ALIAS(entry[PROXY_TBL_OPT + 1])))
    {
      gp_u16CastPointer((uint8_t*)&commissioningNotificationMsg.aliasNwk, &entry[PROXY_TBL_ALIAS]);
    }
    else
    {
      // Derive Alias
      commissioningNotificationMsg.aliasNwk =  gp_aliasDerivation(pGpdID);
    }
    // Set the timer to wait for stub to send queued messages before
    // sending  ZCL commissioning notification
    UtilTimer_setTimeout(gpAppDataSendClkHandle, GP_QUEUE_DATA_SEND_INTERVAL);
    UtilTimer_start(&gpAppDataSendClk);
  }
  else
    status = ZMemError;

  return (status);
}

#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
/*********************************************************************
 * @fn      zclGp_SendGpSinkTableResponse
 *
 * @brief   Send the Green Power Sink Table Response to a device
 *
 * @param   dstAddr - where to send the request
 * @param   groupID - pointer to the group structure
 * @param   groupName - pointer to Group Name.  This is a Zigbee
 *          string data type, so the first byte is the length of the
 *          name (in bytes), then the name.
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGp_SendGpSinkTableResponse( afAddrType_t *dstAddr, zclGpTableResponse_t *rsp,
                                          uint8_t seqNum )
{
  uint8_t *buf = NULL;
  uint8_t *pBuf = NULL;
  uint8_t *pEntry = NULL;

  ZStatus_t status;
  uint16_t entryLen = 0;
  uint16_t len;

  if ( rsp->entry != NULL )
  {
    pEntry = rsp->entry;
    zcl_memcpy( &entryLen, pEntry, sizeof ( uint16_t ) );
    pEntry += sizeof ( uint16_t );
  }

  len = entryLen + 4;

  buf = zcl_mem_alloc( len );
  if ( buf )
  {
    pBuf = buf;
    *pBuf++ = rsp->status;
    *pBuf++ = rsp->tableEntriesTotal;
    *pBuf++ = rsp->startIndex;
    *pBuf++ = rsp->entriesCount;
    if ( rsp->entry != NULL )
    {
      zcl_memcpy( pBuf, pEntry, entryLen );
    }

    status = zcl_SendCommand( GREEN_POWER_INTERNAL_ENDPOINT, dstAddr, ZCL_CLUSTER_ID_GREEN_POWER,
                              COMMAND_GP_SINK_TABLE_RESPONSE, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                              TRUE, 0, seqNum, len, buf );
    zcl_mem_free( buf );
  }
  else
    status = ZMemError;

  return ( status );
}

/*********************************************************************
 * @fn      zclGp_SendGpPairing
 *
 * @brief   Send the Green Power pairing command
 *
 * @param   dstAddr - The destination address
 * @param   pEntry - Pointer to Sink Table entry
 * @param   actions - Pairing Actions
 * @param   secFrameCnt - Security Frame Counter
 * @param   seqNum - The identification number for the transaction
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGp_SendGpPairing(uint8_t* pEntry, uint8_t actions,
                              uint32_t secFrameCnt, uint8_t seqNum)
{
    ZStatus_t status = FAILURE;
    gpSinkTableOptions_t sinkOptions;
    gpSecurityOptions_t sinkSecurityOptions;
    gpPairingOptions_t pairingOptions;
    uint8_t *buf = NULL;
    uint8_t *pBuf = NULL;
    uint8_t len = 4;   // options field len + forwarding radius

    afAddrType_t dstAddr;

    dstAddr.addrMode = afAddrBroadcast;
    dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVZCZR;
    dstAddr.endPoint = GREEN_POWER_INTERNAL_ENDPOINT;

    zcl_memset(&pairingOptions, 0, sizeof(gpPairingOptions_t));
    zcl_memcpy(&sinkOptions, &pEntry[SINK_TBL_OPT], sizeof(uint16_t));
    if(sinkOptions.securityUse == TRUE)
    {
      sinkSecurityOptions.securityLevel = SEC_PARAMETERS_GET_SEC_OPT_SECURITY_LVL(pEntry[SINK_TBL_SEC_OPT]);
      sinkSecurityOptions.securityKeyType = SEC_PARAMETERS_GET_SEC_OPT_SECURITY_KEY_TYP(pEntry[SINK_TBL_SEC_OPT]);
    }
    else
    {
      sinkSecurityOptions.securityLevel = 0;
      sinkSecurityOptions.securityKeyType = 0;
    }
    pairingOptions.appId = sinkOptions.appId;
    pairingOptions.addSink = (GP_PAIRING_CONFIG_ACTION_IS_EXTEND(actions) | GP_PAIRING_CONFIG_ACTION_IS_REPLACE(actions));
    pairingOptions.removeGpd = (GP_PAIRING_CONFIG_ACTION_IS_REMOVE_PAIRING(actions) | GP_PAIRING_CONFIG_ACTION_IS_REMOVE_GPD(actions));
    pairingOptions.communicationMode = sinkOptions.communicationMode;
    pairingOptions.gpdFixed = sinkOptions.gpdFixed;
    pairingOptions.sequenceNumberCap = sinkOptions.sequenceNumberCap;
    pairingOptions.securityLevel = sinkSecurityOptions.securityLevel;
    pairingOptions.securityKeyType = sinkSecurityOptions.securityKeyType;
    pairingOptions.assignedAliasPresent = sinkOptions.assignedAlias;
    pairingOptions.forwardingRadiusPresent = TRUE;

    // If Application Id bitfield is 0b000
    if(GP_PAIRING_OPT_IS_APPLICATION_ID_GPD(pairingOptions.appId))
    {
      len += sizeof(uint32_t);
    }
    // If Application Id bitfield is 0b010
    else if(GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(pairingOptions.appId))
    {
      len += Z_EXTADDR_LEN + sizeof(uint8_t);
    }

    if(pairingOptions.removeGpd == FALSE)
    {
      if((pairingOptions.communicationMode == GP_OPT_COMMUNICATION_MODE_FULL_UNICAST) ||
           (pairingOptions.communicationMode == GP_OPT_COMMUNICATION_MODE_LIGHT_UNICAST))
      {
        len += Z_EXTADDR_LEN + sizeof( uint16_t );
      }

      if(pairingOptions.communicationMode == GP_OPT_COMMUNICATION_MODE_GRPCAST_DGROUP_ID)
      {
        len += sizeof(uint16_t);
      }
      else if(pairingOptions.communicationMode == GP_OPT_COMMUNICATION_MODE_GRPCAST_GROUP_ID)
      {
        len += sizeof(uint32_t);
      }
      len += sizeof(uint8_t);  // Device ID

      if(sinkOptions.securityUse && sinkSecurityOptions.securityLevel >= GP_SECURITY_LVL_4FC_4MIC)
      {
        pairingOptions.securityFrameCounterPresent = TRUE;
        pairingOptions.securityKeyPresent = TRUE;
        len += sizeof(uint32_t) + SEC_KEY_LEN;
      }
      else if(sinkOptions.sequenceNumberCap || pairingOptions.addSink)
      {
          pairingOptions.securityFrameCounterPresent = TRUE;
          len += sizeof(uint32_t);
      }

      if(pairingOptions.assignedAliasPresent)
      {
        len += sizeof(uint16_t);
      }
    }

    buf = zcl_mem_alloc(len);
    if(buf)
    {
      pBuf = buf;

      pBuf = zcl_memcpy(pBuf, &pairingOptions, sizeof(gpPairingOptions_t));

      // Options bitfield
      // If Application Id bitfield is 0b000
      if(GP_PAIRING_OPT_IS_APPLICATION_ID_GPD(pairingOptions.appId))
      {
        pBuf = zcl_memcpy(pBuf, &pEntry[SINK_TBL_SRC_ID], sizeof(uint32_t));
      }
      // If Application Id bitfield is 0b010
      else if(GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(pairingOptions.appId))
      {
        pBuf = zcl_memcpy(pBuf, &pEntry[SINK_TBL_GPD_ID], Z_EXTADDR_LEN);
        pBuf = zcl_memcpy(pBuf, &pEntry[SINK_TBL_GPD_EP], sizeof(uint8_t));
      }

      // Add a GPD
      if(pairingOptions.removeGpd == FALSE)
      {
        if ((pairingOptions.communicationMode == GP_OPT_COMMUNICATION_MODE_FULL_UNICAST) ||
            (pairingOptions.communicationMode == GP_OPT_COMMUNICATION_MODE_LIGHT_UNICAST))
        {
          uint16_t shortAddr;
          pBuf = osal_cpyExtAddr(pBuf, NLME_GetExtAddr());
          shortAddr = NLME_GetShortAddr();
          *pBuf++ = LO_UINT16(shortAddr);
          *pBuf++ = HI_UINT16(shortAddr);
        }

        if(pairingOptions.communicationMode == GP_OPT_COMMUNICATION_MODE_GRPCAST_DGROUP_ID)
        {
          pBuf = zcl_memcpy(pBuf, &pEntry[SINK_TBL_1ST_GRP_ADDR], sizeof(uint16_t));
        }
        else if ( pairingOptions.communicationMode == GP_OPT_COMMUNICATION_MODE_GRPCAST_GROUP_ID )
        {
          pBuf = zcl_memcpy(pBuf, &pEntry[SINK_TBL_1ST_GRP_ADDR], sizeof(uint16_t));
        }
        pBuf = zcl_memcpy(pBuf, (uint8_t*)&pEntry[SINK_TBL_DEVICE_ID], sizeof(uint8_t));

        if (sinkOptions.securityUse && sinkSecurityOptions.securityLevel >= GP_SECURITY_LVL_4FC_4MIC)
        {
          pBuf = zcl_memcpy(pBuf, &pEntry[SINK_TBL_SEC_FRAME], sizeof(uint32_t));
          pBuf = zcl_memcpy(pBuf, &pEntry[SINK_TBL_GPD_KEY], SEC_KEY_LEN);
        }
        else if(sinkOptions.sequenceNumberCap ||
                pairingOptions.addSink)
        {
          pBuf = zcl_memcpy(pBuf, &secFrameCnt, sizeof(uint32_t));
        }

        if(pairingOptions.assignedAliasPresent)
        {
          if(pairingOptions.communicationMode == GP_OPT_COMMUNICATION_MODE_GRPCAST_GROUP_ID)
          {
            pBuf = zcl_memcpy( pBuf, &pEntry[SINK_TBL_2ND_GRP_ADDR], sizeof( uint16_t ) );
          }
          else if((pairingOptions.communicationMode == GP_OPT_COMMUNICATION_MODE_FULL_UNICAST) ||
                  (pairingOptions.communicationMode == GP_OPT_COMMUNICATION_MODE_LIGHT_UNICAST))
          {
            pBuf = zcl_memcpy( pBuf, &pEntry[SINK_TBL_ALIAS], sizeof( uint16_t ) );
          }
        }
        pBuf = zcl_memcpy( pBuf, &pEntry[SINK_TBL_RADIUS], sizeof( uint8_t ) );
      }

      status = zcl_SendCommand( GREEN_POWER_INTERNAL_ENDPOINT, &dstAddr, ZCL_CLUSTER_ID_GREEN_POWER,
                                COMMAND_GP_PAIRING, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                                TRUE, 0, seqNum, len, buf );
      zcl_mem_free( buf );
    }
    return ( status );
}

/*********************************************************************
 * @fn      zclGp_SendGpProxyCommissioningModeCommand
 *
 * @brief   Send the Green Power Proxy Commissioning Mode Command to a device
 *
 * @param   pAddr - Remote device address
 * @param   pCmd - Pointer to GP Proxy Commissioning Mode Struct
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGp_SendGpProxyCommissioningModeCommand( afAddrType_t *pAddr,
                                           gpProxyCommissioningModeCmd_t *pCmd )
{
  ZStatus_t status = FAILURE;
  uint8_t *buf = NULL;
  uint8_t *pBuf = NULL;
  uint8_t len = 1; //  sizeof options bitfield

  if(pCmd->options.onCommissioningWindowExpires == TRUE)
  {
    len += 2;  // sizeof commissioning windows time
  }
  if(pCmd->options.channelPresent == TRUE)
  {
    len += 1;  // sizeof channel
  }

  buf = zcl_mem_alloc(len);
  if(buf)
  {
    pBuf = buf;

    pBuf = zcl_memcpy(pBuf, &pCmd->options, sizeof (uint8_t));
    if(pCmd->options.onCommissioningWindowExpires == TRUE)
    {
      pBuf = zcl_memcpy(pBuf, &pCmd->commissioningWindow, sizeof(uint16_t));
    }

    if(pCmd->options.channelPresent == TRUE)
    {
      pBuf = zcl_memcpy(pBuf, &pCmd->channel, sizeof(uint8_t));
    }

    status = zcl_SendCommand( GREEN_POWER_INTERNAL_ENDPOINT, pAddr, ZCL_CLUSTER_ID_GREEN_POWER,
                      COMMAND_GP_PROXY_COMMISSIONING_MODE, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                      TRUE, 0, zcl_getFrameCounter() , len, buf );

    zcl_mem_free(buf);
  }
  else
    status = ZMemError;

  return(status);
}

/*********************************************************************
 * @fn      zclGp_SendGpResponseCommand
 *
 * @brief   Send the Green Power Proxy Response Command to a device
 *
 * @param   pAddr - Remote device address
 * @param   pCmd - Pointer to GP Response Struct
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGp_SendGpResponseCommand( afAddrType_t *pAddr, gpResponseCmd_t *pCmd )
{
  ZStatus_t status = FAILURE;
  uint8_t *buf = NULL;
  uint8_t *pBuf = NULL;
  uint8_t len = 6;   // options field + temp master short addr + temp master rx ch + gpd cmdId + octet count

  if(GP_PAIRING_OPT_IS_APPLICATION_ID_GPD(pCmd->options))
  {
    len += sizeof(uint32_t);
  }
  else if( GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(pCmd->options))
  {
    len += Z_EXTADDR_LEN + sizeof(uint8_t);
  }

  len += pCmd->payloadLen;
  buf = zcl_mem_alloc(len);
  if(buf)
  {
    pBuf = buf;
    pBuf = zcl_memcpy(pBuf, &pCmd->options, sizeof(uint8_t));
    pBuf = zcl_memcpy(pBuf, &pCmd->tempMasterShortAddr, sizeof(uint16_t));
    pBuf = zcl_memcpy(pBuf, &pCmd->tempMasterChannel, sizeof(uint8_t));

    if(GP_PAIRING_OPT_IS_APPLICATION_ID_GPD(pCmd->options))
    {
      pBuf = zcl_memcpy(pBuf, &pCmd->gpdSrcId, sizeof(uint32_t));
    }
    else if(GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(pCmd->options))
    {
      pBuf = zcl_memcpy(pBuf, &pCmd->gpdIEEE, Z_EXTADDR_LEN);
      pBuf = zcl_memcpy(pBuf, &pCmd->ep, sizeof(uint8_t));
    }

    pBuf = zcl_memcpy(pBuf, &pCmd->cmdId, sizeof(uint8_t));
    pBuf = zcl_memcpy(pBuf, &pCmd->payloadLen, sizeof(uint8_t));
    pBuf = zcl_memcpy(pBuf, pCmd->cmdPayload, pCmd->payloadLen);

    status = zcl_SendCommand(GREEN_POWER_INTERNAL_ENDPOINT, pAddr, ZCL_CLUSTER_ID_GREEN_POWER,
                             COMMAND_GP_RESPONSE, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                             TRUE, 0,  zcl_getFrameCounter() , len, buf );

    zcl_mem_free(buf);
  }
  else
    status = ZMemError;

  return (status);
}
#endif

/*********************************************************************
 * @fn      zclGp_SendGpProxyTableResponse
 *
 * @brief   Send the Green Power Proxy Table Response to a device
 *
 * @param   dstAddr - where to send the request
 * @param   groupID - pointer to the group structure
 * @param   groupName - pointer to Group Name.  This is a Zigbee
 *          string data type, so the first byte is the length of the
 *          name (in bytes), then the name.
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGp_SendGpProxyTableResponse( afAddrType_t *dstAddr, zclGpTableResponse_t *rsp,
                                          uint8_t seqNum )
{
  uint8_t *buf = NULL;
  uint8_t *pBuf = NULL;
  uint8_t *pEntry = NULL;

  ZStatus_t status;
  uint16_t entryLen = 0;
  uint16_t len;

  if(rsp->entry != NULL)
  {
    pEntry = rsp->entry;
    zcl_memcpy( &entryLen, pEntry, sizeof(uint16_t));
    pEntry += sizeof (uint16_t);
  }

  len = entryLen + 4;

  buf = zcl_mem_alloc(len);
  if (buf)
  {
    pBuf = buf;
    *pBuf++ = rsp->status;
    *pBuf++ = rsp->tableEntriesTotal;
    *pBuf++ = rsp->startIndex;
    *pBuf++ = rsp->entriesCount;
    if(rsp->entry != NULL)
    {
      zcl_memcpy(pBuf, pEntry, entryLen);
    }

    status = zcl_SendCommand(GREEN_POWER_INTERNAL_ENDPOINT, dstAddr, ZCL_CLUSTER_ID_GREEN_POWER,
                             COMMAND_GP_PROXY_TABLE_RESPONSE, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                             TRUE, 0, seqNum, len, buf);
    zcl_mem_free(buf);
  }
  else
    status = ZMemError;

  return (status);
}

 /*********************************************************************
 * @fn          gp_addPairedSinksToMsgQueue
 *
 * @brief       General function to add Notification Msg to Queue for all
 *              Sinks paired to the given GPD
 *
 * @param       pGpdID - Pointer to Green Power Device Id struct
 *              pMsg  - Pointer to Gp Notification message
 *
 * @return
 */
static uint8_t gp_addPairedSinksToMsgQueue( gpdID_t* pGpdID, gpCmdPayloadMsg_t* pMsg )
{
  uint8_t freeSinkEntry[LSINK_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  uint8_t status;
  uint8_t i;
  uint8_t currEntry[PROXY_TBL_LEN];
  afAddrType_t dstAddr = {0};
  gpNotificationMsg_t *pNotificationMsgCurr = NULL;

  for(i = 0; i < GPP_MAX_PROXY_TABLE_ENTRIES ; i++)
  {
    status = gp_getProxyTableByIndex(i, currEntry);
    if(status == NV_OPER_FAILED)
    {
      break;
    }

    // if the entry is empty
    if(status == NV_INVALID_DATA)
    {
      continue;
    }

    if(GP_TBL_COMP_APPLICATION_ID( pGpdID->appID, currEntry[PROXY_TBL_OPT]))
    {
      if(zcl_memcmp(&pGpdID->id.srcID, &currEntry[PROXY_TBL_GPD_ID + 4], sizeof(uint32_t)))
      {
        // Entry found
        break;
      }
      else if(zcl_memcmp(pGpdID->id.gpdExtAddr, &currEntry[PROXY_TBL_GPD_ID], Z_EXTADDR_LEN))
      {
        // Entry found
        break;
      }
    }
  }
  // proxy table entry not found
  if(status == NV_INVALID_DATA &&
     i >= GPP_MAX_PROXY_TABLE_ENTRIES)
  {
    return status;
  }

  dstAddr.endPoint = GREEN_POWER_INTERNAL_ENDPOINT;
  dstAddr.panId = _NIB.nwkPanId;

  // Add notifications for Light Weight Unicast
  if(PROXY_TBL_GET_LIGHTWIGHT_UNICAST(currEntry[PROXY_TBL_OPT]))
  {
    dstAddr.addrMode = afAddr16Bit;
    if (!zcl_memcmp(freeSinkEntry, &currEntry[PROXY_TBL_1ST_LSINK_ADDR], LSINK_ADDR_LEN))
    {
      zcl_memcpy(&dstAddr.addr.shortAddr, &currEntry[PROXY_TBL_1ST_LSINK_ADDR + Z_EXTADDR_LEN], sizeof(uint16_t));
      pNotificationMsgCurr = gp_AddNotificationMsgNode(&pNotificationMsgHead, pMsg);
      if (pNotificationMsgCurr != NULL)
      {
        pNotificationMsgCurr->nwkAlias = _NIB.nwkDevAddress;
        pNotificationMsgCurr->addr = dstAddr;
        pNotificationMsgCurr->secNum = pMsg->secNum;
      }
    }
    if (!zcl_memcmp(freeSinkEntry, &currEntry[PROXY_TBL_2ND_LSINK_ADDR], LSINK_ADDR_LEN))
    {
      zcl_memcpy(&dstAddr.addr.shortAddr, &currEntry[PROXY_TBL_2ND_LSINK_ADDR + Z_EXTADDR_LEN], sizeof (uint16_t));
      pNotificationMsgCurr = gp_AddNotificationMsgNode(&pNotificationMsgHead, pMsg);
      if(pNotificationMsgCurr != NULL)
      {
        pNotificationMsgCurr->nwkAlias = _NIB.nwkDevAddress;
        pNotificationMsgCurr->addr = dstAddr;
        pNotificationMsgCurr->secNum = pMsg->secNum;
      }
    }
  }

  // Add notifications for Derived Groupcast
  if(PROXY_TBL_GET_DGROUP(currEntry[PROXY_TBL_OPT]))
  {
    dstAddr.addrMode = afAddrGroup;
    dstAddr.addr.shortAddr = gp_aliasDerivation(pGpdID);
    pNotificationMsgCurr = gp_AddNotificationMsgNode(&pNotificationMsgHead, pMsg);
    if(pNotificationMsgCurr != NULL)
    {
      if(PROXY_TBL_GET_ASSIGNED_ALIAS(currEntry[PROXY_TBL_OPT + 1]))
      {
        zcl_memcpy(&pNotificationMsgCurr->nwkAlias, &currEntry[PROXY_TBL_ALIAS], sizeof(uint16_t));
      }
      else
      {
        pNotificationMsgCurr->nwkAlias = gp_aliasDerivation(pGpdID);
      }
      pNotificationMsgCurr->addr = dstAddr;
      pNotificationMsgCurr->secNum = pMsg->secNum;
    }
  }

  // Add notifications for Commissioned Groupcast
  if(PROXY_TBL_GET_CGROUP(currEntry[PROXY_TBL_OPT + 1]))
  {
    dstAddr.addrMode = afAddrGroup;
    if(GET_BIT(&currEntry[PROXY_TBL_GRP_TBL_ENTRIES], PROXY_TBL_1ST_GRP_BIT))
    {
      zcl_memcpy(&dstAddr.addr.shortAddr, &currEntry[PROXY_TBL_1ST_GRP_ADDR], sizeof(uint16_t));
      pNotificationMsgCurr = gp_AddNotificationMsgNode(&pNotificationMsgHead, pMsg);
      if(pNotificationMsgCurr != NULL)
      {
        zcl_memcpy((uint8_t*)&pNotificationMsgCurr->nwkAlias, &currEntry[PROXY_TBL_1ST_GRP_ADDR + sizeof(uint16_t)], sizeof(uint16_t));
        pNotificationMsgCurr->addr = dstAddr;
        pNotificationMsgCurr->secNum = pMsg->secNum - 9;
      }
    }
    if(GET_BIT(&currEntry[PROXY_TBL_GRP_TBL_ENTRIES], PROXY_TBL_2ND_GRP_BIT))
    {
      zcl_memcpy(&dstAddr.addr.shortAddr, &currEntry[PROXY_TBL_2ND_GRP_ADDR], sizeof(uint16_t));
      pNotificationMsgCurr = gp_AddNotificationMsgNode(&pNotificationMsgHead, pMsg);
      if(pNotificationMsgCurr != NULL)
      {
        zcl_memcpy((uint8_t*)&pNotificationMsgCurr->nwkAlias,&currEntry[PROXY_TBL_2ND_GRP_ADDR + sizeof(uint16_t)], sizeof(uint16_t));
        pNotificationMsgCurr->addr = dstAddr;
        pNotificationMsgCurr->secNum = pMsg->secNum - 9;
      }
    }
  }
  return status;
}

/*********************************************************************
 * @fn      gp_CreateNotificationMsgList
 *
 * @brief   Create Notification Msg List for paired sinks if empty
 *
 * @param   pHead - pointer to a pointer of the list head
 *
 * @return  none
 */
void gp_CreateNotificationMsgList( gpNotificationMsg_t **pHead )
{

  // Create the list if empty
  if(*pHead == NULL)
  {
    *pHead = (gpNotificationMsg_t*)zcl_mem_alloc(sizeof(gpNotificationMsg_t));

    if(*pHead != NULL)
    {
      (*pHead)->pNext = NULL;
    }
  }
  return;
}

/*********************************************************************
 * @fn      gp_CreateCmdPayloadMsgList
 *
 * @brief   Create Notification Msg List for paired sinks if empty
 *
 * @param   pHead - pointer to a pointer of the list head
 *
 * @return  none
 */
void gp_CreateCmdPayloadMsgList( gpCmdPayloadMsg_t **pHead )
{
  // Create the list if empty
  if(*pHead == NULL)
  {
    *pHead = (gpCmdPayloadMsg_t*)zcl_mem_alloc(sizeof(gpCmdPayloadMsg_t));

    if(*pHead != NULL)
    {
      ((gpCmdPayloadMsg_t*)*pHead)->pMsg = NULL;
      (*pHead)->pNext = NULL;
    }
  }
  return;
}

/*********************************************************************
 * @fn      gp_AddNotificationMsgNode
 *
 * @brief   Add node to Notification Msg list
 *
 * @param   pHead - pointer to a pointer of the list head
 *
 * @return  pointer to new node
 */
gpNotificationMsg_t* gp_AddNotificationMsgNode( gpNotificationMsg_t **pHead, gpCmdPayloadMsg_t *pMsg )
{
  gpNotificationMsg_t **pCurr;

    // Create respondent list if empty
  if(*pHead == NULL)
  {
    gp_CreateNotificationMsgList(pHead);
    ((gpNotificationMsg_t*) *pHead)->pMsg = (uint8_t*)pMsg;
    return *pHead;
  }

  pCurr = &((*pHead)->pNext);

  while(*pCurr != NULL)
  {
    pCurr = &((*pCurr)->pNext);
  }

  *pCurr = (gpNotificationMsg_t*)zcl_mem_alloc(sizeof(gpNotificationMsg_t));

  if ( *pCurr )
  {
    if (  (*pCurr)->pNext != NULL )
    {
      (*pCurr)->pNext = NULL;
    }
    ((gpNotificationMsg_t*)*pCurr)->pMsg = (uint8_t*)pMsg;
  }
  return *pCurr;
}

/*********************************************************************
 * @fn      gp_AddCmdPayloadMsgNode
 *
 * @brief   Add node to Notification Msg list
 *
 * @param   pHead - pointer to a pointer of the list head
 *
 * @return  pointer to new node
 */
gpCmdPayloadMsg_t* gp_AddCmdPayloadMsgNode( gpCmdPayloadMsg_t **pHead, uint8_t* pBuf, uint8_t len )
{
  gpCmdPayloadMsg_t **pCurr;

    // Create respondent list if empty
  if(*pHead == NULL)
  {
    gp_CreateCmdPayloadMsgList(pHead);

    if(pHead != NULL)
    {
      (((gpCmdPayloadMsg_t*) *pHead)->lenght) = len;
      (((gpCmdPayloadMsg_t*) *pHead)->pMsg) = (uint8_t*)pBuf;
      return *pHead;
    }
    else
    {
      return NULL;
    }
  }

  pCurr = &((*pHead)->pNext);

  while(*pCurr != NULL)
  {
    pCurr = &((*pCurr)->pNext);
  }

  *pCurr = (gpCmdPayloadMsg_t*)zcl_mem_alloc(sizeof(gpCmdPayloadMsg_t));

  if ( *pCurr )
  {
    if ( (*pCurr)->pNext != NULL )
    {
      (*pCurr)->pNext = NULL;
    }
    (((gpCmdPayloadMsg_t*) *pCurr)->lenght) = len;
    (((gpCmdPayloadMsg_t*) *pCurr)->pMsg) = (uint8_t*)pBuf;
  }

  return *pCurr;
}

/*********************************************************************
 * @fn      gp_GetHeadNotificationMsg
 *
 * @brief   Returns head pointer for  finding and binding respondent list
 *
 * @param   none
 *
 * @return  pointer to head
 */
gpNotificationMsg_t* gp_GetHeadNotificationMsg(void)
{
  return(pNotificationMsgHead);
}

/*********************************************************************
 * @fn      gp_GetPHeadNotification
 *
 * @brief   Returns head pointer for  finding and binding respondent list
 *
 * @param   none
 *
 * @return  pointer to head
 */
gpNotificationMsg_t** gp_GetPHeadNotification(void)
{
  return(&pNotificationMsgHead);
}

/*********************************************************************
 * @fn      gp_GetHeadCmdPayloadMsg
 *
 * @brief   Returns head pointer for  finding and binding respondent list
 *
 * @param   none
 *
 * @return  pointer to head
 */
gpCmdPayloadMsg_t* gp_GetHeadCmdPayloadMsg(void)
{
  return(pCmdPayloadMsgHead);
}

/*********************************************************************
 * @fn      gp_GetHeadCmdPayloadMsg
 *
 * @brief   Returns head pointer for  finding and binding respondent list
 *
 * @param   none
 *
 * @return  pointer to head
 */
gpCmdPayloadMsg_t** gp_GetPHeadCmdPayload(void)
{
  return(&pCmdPayloadMsgHead);
}

/*********************************************************************
 * @fn      gp_NotificationMsgClean
 *
 * @brief   This function free reserved memory for respondent list
 *
 * @param   pHead - begin of the Notification Msg list
 *
 * @return  status
 */
void gp_NotificationMsgClean( gpNotificationMsg_t **pHead )
{
  gpNotificationMsg_t **pCurr;
  gpNotificationMsg_t **pNext;

  if(*pHead == NULL)
  {
    return;
  }

  pCurr = pHead;

  if(*pCurr != NULL)
  {
    pNext = &((*pCurr)->pNext);
    while( *pNext != NULL )
    {
      if(((gpNotificationMsg_t* )*pCurr)->pMsg ==
         ((gpNotificationMsg_t* ) *pNext )->pMsg)
      {
        break;
      }
      pNext = &((*pNext)->pNext);
    }

    if(*pNext == NULL)
    {
      gp_CmdPayloadMsgClean(gp_GetPHeadCmdPayload());
    }
    zcl_mem_free(*pCurr);
    *pCurr = (gpNotificationMsg_t*)NULL;
    pCurr = pNext;
  }
  *pHead = *pCurr;
}

/*********************************************************************
 * @fn      gp_CmdPayloadMsgClean
 *
 * @brief   This function free reserved memory for respondent list
 *
 * @param   pHead - begin of the Notification Msg list
 *
 * @return
 */
void gp_CmdPayloadMsgClean( gpCmdPayloadMsg_t **pHead )
{
  gpCmdPayloadMsg_t **pCurr;
  gpCmdPayloadMsg_t **pNext;

  if(*pHead == NULL)
  {
    return;
  }

  pCurr = pHead;

  if(*pCurr != NULL)
  {
    pNext = &((*pCurr)->pNext);
    zcl_mem_free(((gpCmdPayloadMsg_t*)*pCurr)->pMsg);
    ((gpCmdPayloadMsg_t*)*pCurr)->pMsg = NULL;
    zcl_mem_free(*pCurr);
    *pCurr = (gpCmdPayloadMsg_t*)NULL;
    pCurr = pNext;
  }
  pHead = pCurr;
}

/*********************************************************************
 * @fn      gp_ResetProxyTblEntry
 *
 * @brief   This function removes data of the given proxy table entry
 *
 * @param   entry - proxy table entry to be removed
 *
 * @return
 */
void gp_ResetProxyTblEntry( uint8_t* entry )
{
  memset(entry, 0xFF, PROXY_TBL_1ST_LSINK_ADDR);
  memset(&entry[PROXY_TBL_1ST_LSINK_ADDR],0xFF,PROXY_TBL_LEN - PROXY_TBL_1ST_LSINK_ADDR);
  entry[PROXY_TBL_GPD_EP] = 0;
}

/*********************************************************************
 * @fn      gp_ResetSinkTblEntry
 *
 * @brief   This function removes data of the given sink table entry
 *
 * @param   entry - sink table entry to be removed
 *
 * @return
 */
void gp_ResetSinkTblEntry( uint8_t* entry )
{
  memset(entry, 0xFF, SINK_TBL_ENTRY_LEN);
}

#endif

/***************************************************************************
****************************************************************************/
