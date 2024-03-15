/**************************************************************************************************
  Filename:       zcl_telecommunication.c
  Revised:        $Date: 2018-22-11 13:53:09 -0700 (Thur, 22 Nov 2018) $
  Revision:       $Revision: 34523 $

  Description:    Zigbee Cluster Library - Telecommunication Cluster


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
#include "zcl_telecommunication.h"

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  #include "stub_aps.h"
#endif

/*********************************************************************
 * MACROS
 */
#define TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE          2


/* Minimum packet size */
//Request by types minimum sizes
#define TELECOMMUNICATIONS_INFORMATION_REQ_BY_CONTENT_ID_SIZE   4
#define TELECOMMUNICATIONS_INFORMATION_REQ_MULTIPLE_ID          2
#define TELECOMMUNICATIONS_INFORMATION_REQ_ALL                  2
#define TELECOMMUNICATIONS_INFORMATION_REQ_BY_DEPTH             5

//ContentId + StatusFeedback
#define TELECOMMUNICATIONS_INFORMATION_NOTIFICATION_SIZE        3

//Base size of request preference response (preference type + Count)
#define TELECOMMUNICATIONS_INFORMATION_SEND_PREFERENCE_SIZE     3
//Base size of request preference response (status + preference type + Count)
#define TELECOMMUNICATIONS_INFORMATION_PREFERENCE_RSP_SIZE      4

#define TELECOMMUNICATIONS_INFORMATION_UPDATE_SIZE              3
#define TELECOMMUNICATIONS_INFORMATION_REQ_INFO_RSP_SIZE        2
#define TELECOMMUNICATIONS_INFORMATION_PUSH_INFO_SIZE           1

#define TELECOMMUNICATIONS_CHATTING_JOIN_REQ_SIZE               5
#define TELECOMMUNICATIONS_CHATTING_LEAVE_CHAT_REQ_SIZE         4
#define TELECOMMUNICATIONS_CHATTING_SWITCH_CHAIRMAN_RSP_SIZE    4
#define TELECOMMUNICATIONS_CHATTING_START_CHAT_REQ_SIZE         4
#define TELECOMMUNICATIONS_CHATTING_CHAT_MESSAGE_SIZE           8
#define TELECOMMUNICATIONS_CHATTING_GET_NODE_INFO_REQ_SIZE      4
#define TELECOMMUNICATIONS_CHATTING_START_CHAT_RSP_SIZE         3
#define TELECOMMUNICATIONS_CHATTING_JOIN_CHAT_RSP_SIZE          3
#define TELECOMMUNICATIONS_CHATTING_USER_LEFT_SIZE              5
#define TELECOMMUNICATIONS_CHATTING_SEARCH_CHAT_RSP_SIZE        1
#define TELECOMMUNICATIONS_CHATTING_SWITCH_CHAIRMAN_REQ_SIZE    2
#define TELECOMMUNICATIONS_CHATTING_SWITCH_CHAIRMAN_CNF_SIZE    2
#define TELECOMMUNICATIONS_CHATTING_SWITCH_CHAIRMAN_NOTIF_SIZE  7
#define TELECOMMUNICATIONS_CHATTING_GET_NODE_INFO_RSP_SIZE      5

#define TELECOMMUNICATIONS_VOICE_ESTABLISHMENT_REQ_SIZE         10
#define TELECOMMUNICATIONS_VOICE_TRANSMISSION_RSP_SIZE          2

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */
#ifdef ZCL_INFORMATION
typedef struct zclTelInformationCBRec
{
  struct zclTelInformationCBRec     *next;
  uint8_t                             endpoint;  // Used to link it into the endpoint descriptor
  zclTel_InformationAppCallbacks_t  *CBs;      // Pointer to Callback function
} zclTelInformationCBRec_t;
#endif

#ifdef ZCL_CHATTING
typedef struct zclTelChattingCBRec
{
  struct zclTelChattingCBRec        *next;
  uint8_t                             endpoint;  // Used to link it into the endpoint descriptor
  zclTel_ChattingAppCallbacks_t     *CBs;      // Pointer to Callback function
} zclTelChattingCBRec_t;
#endif

#ifdef ZCL_VOICE
typedef struct zclTel_VoiceAppCallbacks
{
  struct zclTel_VoiceAppCallbacks   *next;
  uint8_t                             endpoint;  // Used to link it into the endpoint descriptor
  zclTel_VoiceAppCallbacks_t        *CBs;      // Pointer to Callback function
} zclTelVoiceCBRec_t;
#endif
/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

#ifdef ZCL_INFORMATION
static zclTelInformationCBRec_t *zclTelInformationCBs = (zclTelInformationCBRec_t *)NULL;
static uint8_t zclTelInformationPluginRegisted = FALSE;
#endif

#ifdef ZCL_CHATTING
static zclTelChattingCBRec_t *zclTelChattingCBs = (zclTelChattingCBRec_t *)NULL;
static uint8_t zclTelChattingPluginRegisted = FALSE;
#endif

#ifdef ZCL_VOICE
static zclTelVoiceCBRec_t *zclTelVoiceCBs = (zclTelVoiceCBRec_t *)NULL;
static uint8_t zclTelVoicePluginRegisted = FALSE;
#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */

#if defined (ZCL_CHATTING) || defined (ZCL_VOICE) || defined (ZCL_INFORMATION)
/*
 * Callback from ZCL to process incoming Commands specific to this cluster library
 */
static ZStatus_t zclTel_HdlIncoming(  zclIncoming_t *pInMsg );

/*
 * Function to process incoming Commands specific to this cluster library
 */
static ZStatus_t zclTel_HdlInSpecificCommands( zclIncoming_t *pInMsg );
#endif

/*
 * Function to find the callbacks registered for this endpoint
 */
#ifdef ZCL_INFORMATION
static zclTel_InformationAppCallbacks_t *zclTel_InformationFindCallbacks( uint8_t endpoint );
#endif
#ifdef ZCL_CHATTING
static zclTel_ChattingAppCallbacks_t *zclTel_ChattingFindCallbacks( uint8_t endpoint );
#endif
#ifdef ZCL_VOICE
static zclTel_VoiceAppCallbacks_t *zclTel_VoiceFindCallbacks( uint8_t endpoint );
#endif
/*
 * Function to find the callbacks registered for this endpoint
 */
#ifdef ZCL_INFORMATION
static ZStatus_t zclTel_ProcessIn_InformationCmds( zclIncoming_t *pInMsg, zclTel_InformationAppCallbacks_t *pInformationCBs );
#endif
#ifdef ZCL_CHATTING
static ZStatus_t zclTel_ProcessIn_ChattingCmds( zclIncoming_t *pInMsg, zclTel_ChattingAppCallbacks_t *pChattingCBs );
#endif
#ifdef ZCL_VOICE
static ZStatus_t zclTel_ProcessIn_VoiceOverZigbeeCmds( zclIncoming_t *pInMsg, zclTel_VoiceAppCallbacks_t *pVoiceCBs );
#endif

#ifdef ZCL_INFORMATION
/*********************************************************************
 * @fn      zclTel_RegisterInformationCmdCallbacks
 *
 * @brief   Register an applications Information command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZSuccess if successful. ZMemError if not able to allocate
 */
ZStatus_t zclTel_RegisterInformationCmdCallbacks( uint8_t endpoint, zclTel_InformationAppCallbacks_t *callbacks )
{
  zclTelInformationCBRec_t *pNewItem;
  zclTelInformationCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( !zclTelInformationPluginRegisted )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                        ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                        zclTel_HdlIncoming );
    zclTelInformationPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclTelInformationCBRec_t ) );
  if ( pNewItem == NULL )
  {
    return ( ZMemError );
  }

  pNewItem->next = (zclTelInformationCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclTelInformationCBs == NULL )
  {
    zclTelInformationCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclTelInformationCBs;
    while ( pLoop->next != NULL )
    {
      pLoop = pLoop->next;
    }

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}
#endif

#ifdef ZCL_CHATTING
/*********************************************************************
 * @fn      zclTel_RegisterChattingCmdCallbacks
 *
 * @brief   Register an applications Chatting command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZSuccess if successful. ZMemError if not able to allocate
 */
ZStatus_t zclTel_RegisterChattingCmdCallbacks( uint8_t endpoint, zclTel_ChattingAppCallbacks_t *callbacks )
{
  zclTelChattingCBRec_t *pNewItem;
  zclTelChattingCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( !zclTelChattingPluginRegisted )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                        ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                        zclTel_HdlIncoming );
    zclTelChattingPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclTelChattingCBRec_t ) );
  if ( pNewItem == NULL )
  {
    return ( ZMemError );
  }

  pNewItem->next = (zclTelChattingCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclTelChattingCBs == NULL )
  {
    zclTelChattingCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclTelChattingCBs;
    while ( pLoop->next != NULL )
    {
      pLoop = pLoop->next;
    }

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}
#endif

#ifdef ZCL_VOICE
/*********************************************************************
 * @fn      zclTel_RegisterVoiceCmdCallbacks
 *
 * @brief   Register an applications Voice command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZSuccess if successful. ZMemError if not able to allocate
 */
ZStatus_t zclTel_RegisterVoiceCmdCallbacks( uint8_t endpoint, zclTel_VoiceAppCallbacks_t *callbacks )
{
  zclTelVoiceCBRec_t *pNewItem;
  zclTelVoiceCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( !zclTelVoicePluginRegisted )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_TELECOMMUNICATIONS_VOICE_OVER_ZIGBEE,
                        ZCL_CLUSTER_ID_TELECOMMUNICATIONS_VOICE_OVER_ZIGBEE,
                        zclTel_HdlIncoming );
    zclTelVoicePluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclTelVoiceCBRec_t ) );
  if ( pNewItem == NULL )
  {
    return ( ZMemError );
  }

  pNewItem->next = (zclTelVoiceCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclTelVoiceCBs == NULL )
  {
    zclTelVoiceCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclTelVoiceCBs;
    while ( pLoop->next != NULL )
    {
      pLoop = pLoop->next;
    }

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}
#endif

#ifdef ZCL_INFORMATION
/*********************************************************************
 * @fn      zclTel_InformationFindCallbacks
 *
 * @brief   Find the Information Cluster callbacks for an endpoint
 *
 * @param   endpoint
 *
 * @return  pointer to the callbacks
 */
static zclTel_InformationAppCallbacks_t *zclTel_InformationFindCallbacks( uint8_t endpoint )
{
  zclTelInformationCBRec_t *pCBs;

  pCBs = zclTelInformationCBs;
  while ( pCBs != NULL )
  {
    if ( pCBs->endpoint == endpoint )
    {
      return ( pCBs->CBs );
    }

    pCBs = pCBs->next;
  }

  return ( (zclTel_InformationAppCallbacks_t *)NULL );
}
#endif

#ifdef ZCL_CHATTING
/*********************************************************************
 * @fn      zclTel_ChattingFindCallbacks
 *
 * @brief   Find the Chatting Cluster callbacks for an endpoint
 *
 * @param   endpoint
 *
 * @return  pointer to the callbacks
 */
static zclTel_ChattingAppCallbacks_t *zclTel_ChattingFindCallbacks( uint8_t endpoint )
{
  zclTelChattingCBRec_t *pCBs;

  pCBs = zclTelChattingCBs;
  while ( pCBs != NULL )
  {
    if ( pCBs->endpoint == endpoint )
    {
      return ( pCBs->CBs );
    }

    pCBs = pCBs->next;
  }

  return ( (zclTel_ChattingAppCallbacks_t *)NULL );
}
#endif

#ifdef ZCL_VOICE
/*********************************************************************
 * @fn      zclTel_VoiceFindCallbacks
 *
 * @brief   Find the Voice Cluster callbacks for an endpoint
 *
 * @param   endpoint
 *
 * @return  pointer to the callbacks
 */
static zclTel_VoiceAppCallbacks_t *zclTel_VoiceFindCallbacks( uint8_t endpoint )
{
  zclTelVoiceCBRec_t *pCBs;

  pCBs = zclTelVoiceCBs;
  while ( pCBs != NULL )
  {
    if ( pCBs->endpoint == endpoint )
    {
      return ( pCBs->CBs );
    }

    pCBs = pCBs->next;
  }

  return ( (zclTel_VoiceAppCallbacks_t *)NULL );
}
#endif


#if defined (ZCL_CHATTING) || defined (ZCL_VOICE) || defined (ZCL_INFORMATION)
/*********************************************************************
 * @fn      zclTel_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclTel_HdlIncoming(  zclIncoming_t *pInMsg )
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
      stat = zclTel_HdlInSpecificCommands( pInMsg );
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
 * @fn      zclTel_HdlInSpecificCommands
 *
 * @brief   Function to process incoming Commands specific
 *          to this cluster library

 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclTel_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;

#ifdef ZCL_INFORMATION
  zclTel_InformationAppCallbacks_t *pInformationCBs;
#endif
#ifdef ZCL_CHATTING
  zclTel_ChattingAppCallbacks_t *pChattingCBs;
#endif
#ifdef ZCL_VOICE
  zclTel_VoiceAppCallbacks_t *pVoiceCBs;
#endif

  // Make sure endpoint exists
#ifdef ZCL_INFORMATION
  pInformationCBs = zclTel_InformationFindCallbacks( pInMsg->msg->endPoint );
  if ( pInformationCBs == NULL )
  {
    return ( ZFailure );
  }
#endif
#ifdef ZCL_CHATTING
  pChattingCBs = zclTel_ChattingFindCallbacks( pInMsg->msg->endPoint );
  if ( pChattingCBs == NULL )
  {
    return ( ZFailure );
  }
#endif
#ifdef ZCL_VOICE
  pVoiceCBs = zclTel_VoiceFindCallbacks( pInMsg->msg->endPoint );
  if ( pVoiceCBs == NULL )
  {
    return ( ZFailure );
  }
#endif

  switch ( pInMsg->msg->clusterId )
  {

#ifdef ZCL_INFORMATION
    case ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION:
      stat = zclTel_ProcessIn_InformationCmds( pInMsg, pInformationCBs );
    break;
#endif
#ifdef ZCL_CHATTING
    case ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING:
      stat = zclTel_ProcessIn_ChattingCmds( pInMsg, pChattingCBs );
    break;
#endif
#ifdef ZCL_VOICE
    case ZCL_CLUSTER_ID_TELECOMMUNICATIONS_VOICE_OVER_ZIGBEE:
      stat = zclTel_ProcessIn_VoiceOverZigbeeCmds( pInMsg, pVoiceCBs );
    break;
#endif

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}
#endif

#ifdef ZCL_INFORMATION
/*********************************************************************
 * @fn      zclTel_ProcessIn_InformationCmds
 *
 * @brief   Callback from ZCL to process incoming Commands for information cluster
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclTel_ProcessIn_InformationCmds( zclIncoming_t *pInMsg, zclTel_InformationAppCallbacks_t *pInformationCBs )
{
  ZStatus_t stat;

  // Client-to-Server
  if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    switch(pInMsg->hdr.commandID)
    {
      case COMMAND_TEL_INFORMATION_REQ_INFO:
        if(pInformationCBs->pfnRequestInformation)
        {
          stat = pInformationCBs->pfnRequestInformation(pInMsg);
        }
      break;
      case COMMAND_TEL_INFORMATION_PUSH_INFO_RSP:
        if(pInformationCBs->pfnPushInformationRsp)
        {
          stat = pInformationCBs->pfnPushInformationRsp(pInMsg);
        }
      break;
      case COMMAND_TEL_INFORMATION_SEND_PREF:
        if(pInformationCBs->pfnSendPreference)
        {
          stat = pInformationCBs->pfnSendPreference(pInMsg);
        }
      break;
      case COMMAND_TEL_INFORMATION_REQ_PREF_RSP:
        if(pInformationCBs->pfnRequestPreferenceRsp)
        {
          stat = pInformationCBs->pfnRequestPreferenceRsp(pInMsg);
        }
      break;
      case COMMAND_TEL_INFORMATION_UPDATE:
        if(pInformationCBs->pfnUpdate)
        {
          stat = pInformationCBs->pfnUpdate(pInMsg);
        }
      break;
      case COMMAND_TEL_INFORMATION_DELETE:
        if(pInformationCBs->pfnDelete)
        {
          stat = pInformationCBs->pfnDelete(pInMsg);
        }
      break;
      case COMMAND_TEL_INFORMATION_CONF_NODE_DESC:
        if(pInformationCBs->pfnConfigureNodeDescription)
        {
          stat = pInformationCBs->pfnConfigureNodeDescription(pInMsg);
        }
      break;
      case COMMAND_TEL_INFORMATION_CONF_DELIVERY_ENABLE:
        if(pInformationCBs->pfnConfigureDeliveryEnable)
        {
          stat = pInformationCBs->pfnConfigureDeliveryEnable(pInMsg);
        }
      break;
      case COMMAND_TEL_INFORMATION_CONF_PUSH_INFO_TIMER:
        if(pInformationCBs->pfnConfigurePushInformationTimer)
        {
          stat = pInformationCBs->pfnConfigurePushInformationTimer(pInMsg);
        }
      break;
      case COMMAND_TEL_INFORMATION_CONF_SET_ROOT_ID:
        if(pInformationCBs->pfnConfigureSetRootID)
        {
          stat = pInformationCBs->pfnConfigureSetRootID(pInMsg);
        }
      break;
      default:
      return( ZFailure );
    }
  }
  // Server-to-Client
  else
  {
    switch(pInMsg->hdr.commandID)
    {
      case COMMAND_TEL_INFORMATION_REQ_INFO_RSP:
        if(pInformationCBs->pfnRequestInformationRsp)
        {
          stat = pInformationCBs->pfnRequestInformationRsp(pInMsg);
        }
      break;
      case COMMAND_TEL_INFORMATION_PUSH_INFO:
        if(pInformationCBs->pfnPushInformation)
        {
          stat = pInformationCBs->pfnPushInformation(pInMsg);
        }
      break;
      case COMMAND_TEL_INFORMATION_SEND_PREF_RSP:
        if(pInformationCBs->pfnSendPreferenceRsp)
        {
          stat = pInformationCBs->pfnSendPreferenceRsp(pInMsg);
        }
      break;
      case COMMAND_TEL_INFORMATION_SERVER_REQ_PREF:
        if(pInformationCBs->pfnServerRequestPreference)
        {
          stat = pInformationCBs->pfnServerRequestPreference(pInMsg);
        }
      break;
      case COMMAND_TEL_INFORMATION_REQ_PREF_CONFIR:
        if(pInformationCBs->pfnRequestPreferenceConfirmation)
        {
          stat = pInformationCBs->pfnRequestPreferenceConfirmation(pInMsg);
        }
      break;
      case COMMAND_TEL_INFORMATION_UPDATE_RSP:
        if(pInformationCBs->pfnConfigureDeliveryEnable)
        {
          stat = pInformationCBs->pfnUpdateRsp(pInMsg);
        }
      break;
      case COMMAND_TEL_INFORMATION_DELETE_RSP:
        if(pInformationCBs->pfnDeleteRsp)
        {
          stat = pInformationCBs->pfnDeleteRsp(pInMsg);
        }
      break;
      default:
      return( ZFailure );
    }
  }
  return stat;
}
#endif


#ifdef ZCL_CHATTING
/*********************************************************************
 * @fn      zclTel_ProcessIn_ChattingCmds
 *
 * @brief   Callback from ZCL to process incoming Commands for chatting cluster
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclTel_ProcessIn_ChattingCmds( zclIncoming_t *pInMsg, zclTel_ChattingAppCallbacks_t *pChattingCBs )
{
  ZStatus_t stat;

  // Client-to-Server
  if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    switch(pInMsg->hdr.commandID)
    {
      case COMMAND_TEL_CHATTING_JOIN_CHAT_REQ:
        if(pChattingCBs->pfnJoinChatReq)
        {
          stat = pChattingCBs->pfnJoinChatReq(pInMsg);
        }
      break;
      case COMMAND_TEL_CHATTING_LEAVE_CHAT_REQ:
        if(pChattingCBs->pfnLeaveChatReq)
        {
          stat = pChattingCBs->pfnLeaveChatReq(pInMsg);
        }
      break;
      case COMMAND_TEL_CHATTING_SEARCH_CHAT_REQ:
        if(pChattingCBs->pfnSearchChatReq)
        {
          stat = pChattingCBs->pfnSearchChatReq(pInMsg);
        }
      break;
      case COMMAND_TEL_CHATTING_SWITCH_CHAIRMAN_RSP:
        if(pChattingCBs->pfnSwitchChairmanRsp)
        {
          stat = pChattingCBs->pfnSwitchChairmanRsp(pInMsg);
        }
      break;
      case COMMAND_TEL_CHATTING_START_CHAT_REQ:
        if(pChattingCBs->pfnStartChatReq)
        {
          stat = pChattingCBs->pfnStartChatReq(pInMsg);
        }
      break;
      case COMMAND_TEL_CHATTING_CHAT_MESSAGE:
        if(pChattingCBs->pfnChatMessage)
        {
          stat = pChattingCBs->pfnChatMessage(pInMsg);
        }
      break;
      case COMMAND_TEL_CHATTING_GET_NODE_INFO_REQ:
        if(pChattingCBs->pfnGetNodeInformationReq)
        {
          stat = pChattingCBs->pfnGetNodeInformationReq(pInMsg);
        }
      break;
      default:
      return( ZFailure );
    }
  }
  // Server-to-Client
  else
  {
    switch(pInMsg->hdr.commandID)
    {
      case COMMAND_TEL_CHATTING_START_CHAT_RSP:
        if(pChattingCBs->pfnStartChatRsp)
        {
          stat = pChattingCBs->pfnStartChatRsp(pInMsg);
        }
      break;
      case COMMAND_TEL_CHATTING_JOIN_CHAT_RSP:
        if(pChattingCBs->pfnJoinChatRsp)
        {
          stat = pChattingCBs->pfnJoinChatRsp(pInMsg);
        }
      break;
      case COMMAND_TEL_CHATTING_USER_LEFT:
        if(pChattingCBs->pfnUserLeft)
        {
          stat = pChattingCBs->pfnUserLeft(pInMsg);
        }
      break;
      case COMMAND_TEL_CHATTING_USER_JOINED:
        if(pChattingCBs->pfnUserJoined)
        {
          stat = pChattingCBs->pfnUserJoined(pInMsg);
        }
      break;
      case COMMAND_TEL_CHATTING_SEARCH_CHAT_RSP:
        if(pChattingCBs->pfnSearchChatRsp)
        {
          stat = pChattingCBs->pfnSearchChatRsp(pInMsg);
        }
      break;
      case COMMAND_TEL_CHATTING_SWITCH_CHAIRMAN_REQ:
        if(pChattingCBs->pfnSwitchChairmanReq)
        {
          stat = pChattingCBs->pfnSwitchChairmanReq(pInMsg);
        }
      break;
      case COMMAND_TEL_CHATTING_SWITCH_CHAIRMAN_CONFIRM:
        if(pChattingCBs->pfnSwitchChairmanConfirm)
        {
          stat = pChattingCBs->pfnSwitchChairmanConfirm(pInMsg);
        }
      break;
      case COMMAND_TEL_CHATTING_SWITCH_CHAIRMAN_NOTIFICATION:
        if(pChattingCBs->pfnSwitchChairmanNotification)
        {
          stat = pChattingCBs->pfnSwitchChairmanNotification(pInMsg);
        }
      break;
      case COMMAND_TEL_CHATTING_GET_NODE_INFO_RSP:
        if(pChattingCBs->pfnGetNodeInformationRsp)
        {
          stat = pChattingCBs->pfnGetNodeInformationRsp(pInMsg);
        }
      break;
      default:
      return( ZFailure );
    }
  }
  return stat;
}
#endif


#ifdef ZCL_VOICE
/*********************************************************************
 * @fn      zclTel_ProcessIn_VoiceOverZigbeeCmds
 *
 * @brief   Callback from ZCL to process incoming Commands for Voice Over Zigbee cluster
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclTel_ProcessIn_VoiceOverZigbeeCmds( zclIncoming_t *pInMsg, zclTel_VoiceAppCallbacks_t *pVoiceCBs )
{
  ZStatus_t stat;

  // Client-to-Server
  if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    switch(pInMsg->hdr.commandID)
    {

      case COMMAND_TEL_VOICE_ESTABLISHMENT_REQ:
        if(pVoiceCBs->pfnEstablishmentRequest)
        {
          stat = pVoiceCBs->pfnEstablishmentRequest(pInMsg);
        }
      break;
      case COMMAND_TEL_VOICE_VOICE_TRANSMISSION:
        if(pVoiceCBs->pfnVoiceTransmission)
        {
          stat = pVoiceCBs->pfnVoiceTransmission(pInMsg);
        }
      break;
      case COMMAND_TEL_VOICE_VOICE_TRANSMISSION_COMPLETION:
        if(pVoiceCBs->pfnVoiceTransmissionCompletion)
        {
          stat = pVoiceCBs->pfnVoiceTransmissionCompletion(pInMsg);
        }
      break;
      case COMMAND_TEL_VOICE_CONTROL_RSP:
        if(pVoiceCBs->pfnControlRsp)
        {
          stat = pVoiceCBs->pfnControlRsp(pInMsg);
        }
      break;

      default:
      return( ZFailure );
    }
  }
  // Server-to-Client
  else
  {
    switch(pInMsg->hdr.commandID)
    {
      case COMMAND_TEL_VOICE_VOICE_TRANSMISSION_RSP:
        if(pVoiceCBs->pfnVoiceTransmissionRsp)
        {
          stat = pVoiceCBs->pfnVoiceTransmissionRsp(pInMsg);
        }
      break;
      case COMMAND_TEL_VOICE_ESTABLISHMENT_RSP:
        if(pVoiceCBs->pfnEstablishmentRsp)
        {
          stat = pVoiceCBs->pfnEstablishmentRsp(pInMsg);
        }
      break;
      case COMMAND_TEL_VOICE_CONTROL:
        if(pVoiceCBs->pfnControl)
        {
          stat = pVoiceCBs->pfnControl(pInMsg);
        }
      break;

      default:
      return( ZFailure );
    }
  }
  return stat;
}

#endif



#ifdef ZCL_INFORMATION
/*********************************************************************
 * @fn      zclTel_InfoSendRequestInformation
 *
 * @brief   Call to send out a Request Information command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Request Information structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendRequestInformation( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_RequestInfo_t *pCmd,
                                    uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t bufLen;
  uint8_t *buf = NULL;
  uint8_t *tempBuf = NULL;
  ZStatus_t stat = ZSuccess;

  switch(pCmd->InquiryId)
  {
    case TELECOMMUNICATIONS_INFORMATION_INQUIRY_ID_REQUEST_BY_CONTENT_ID:
      bufLen = TELECOMMUNICATIONS_INFORMATION_REQ_BY_CONTENT_ID_SIZE;
    break;
    case TELECOMMUNICATIONS_INFORMATION_INQUIRY_ID_REQUEST_MULTIPLE_ID:
      bufLen = TELECOMMUNICATIONS_INFORMATION_REQ_MULTIPLE_ID;
      bufLen += (pCmd->RequestInfoPayload.MultipleContent.NumContent)*(sizeof(uint16_t));
    break;
    case TELECOMMUNICATIONS_INFORMATION_INQUIRY_ID_REQUEST_ALL:
      bufLen = TELECOMMUNICATIONS_INFORMATION_REQ_ALL;
    break;
    case TELECOMMUNICATIONS_INFORMATION_INQUIRY_ID_REQUEST_BY_DEPTH:
      bufLen = TELECOMMUNICATIONS_INFORMATION_REQ_BY_DEPTH;
    break;
    default:
      return ZInvalidParameter;
  }

  buf = zcl_mem_alloc(bufLen);
  if(buf == NULL)
  {
    return ZMemError;
  }

  tempBuf = buf;

  tempBuf[0] = pCmd->InquiryId;
  tempBuf[1] = pCmd->DataType;

  tempBuf += 2;

  switch(pCmd->InquiryId)
  {
    case TELECOMMUNICATIONS_INFORMATION_INQUIRY_ID_REQUEST_BY_CONTENT_ID:
      tempBuf[0] = LO_UINT16(pCmd->RequestInfoPayload.ReqAContentByContentId);
      tempBuf[1] = HI_UINT16(pCmd->RequestInfoPayload.ReqAContentByContentId);
    break;
    case TELECOMMUNICATIONS_INFORMATION_INQUIRY_ID_REQUEST_MULTIPLE_ID:
      {
        uint8_t i;
        uint16_t  *pContenId = pCmd->RequestInfoPayload.MultipleContent.ContentIDs;
        for (i=0; i < pCmd->RequestInfoPayload.MultipleContent.NumContent; i++)
        {
          tempBuf[0] = LO_UINT16(*pContenId);
          tempBuf[1] = HI_UINT16(*pContenId);
          tempBuf += TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
          pContenId += TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
        }
      }
    break;
    case TELECOMMUNICATIONS_INFORMATION_INQUIRY_ID_REQUEST_ALL:
      //This is just a place holder for the InquiryId type. This does not have additional payload.
    break;
    case TELECOMMUNICATIONS_INFORMATION_INQUIRY_ID_REQUEST_BY_DEPTH:
          tempBuf[0] = LO_UINT16(pCmd->RequestInfoPayload.ReqByDepth.StartId);
          tempBuf[1] = HI_UINT16(pCmd->RequestInfoPayload.ReqByDepth.StartId);
          tempBuf[2] = pCmd->RequestInfoPayload.ReqByDepth.Depth;
    break;
    default:
      return ZInvalidParameter;
  }

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                          COMMAND_TEL_INFORMATION_REQ_INFO, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                          seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;

}



/*********************************************************************
 * @fn      zclTel_InfoSendPushInformationRsp
 *
 * @brief   Call to send out a Push Information Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Push Information Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendPushInformationRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_PushInformationRsp_t *pCmd,
                                    uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t bufLen;
  uint8_t *buf = NULL;
  uint8_t *tempBuf = NULL;
  uint8_t i;
  ZStatus_t stat = ZSuccess;
  zcl_Tel_Info_Notification_t *pNotifications;

  bufLen = pCmd->NumNotifications * TELECOMMUNICATIONS_INFORMATION_NOTIFICATION_SIZE;

  buf = zcl_mem_alloc(bufLen);

  if(buf == NULL)
  {
    return ZMemError;
  }
  tempBuf = buf;

  pNotifications = pCmd->Notifications;
  for (i=0; i < pCmd->NumNotifications; i++)
  {
    tempBuf[0] = LO_UINT16(pNotifications->ContentId);
    tempBuf[1] = HI_UINT16(pNotifications->ContentId);
    tempBuf[2] = pNotifications->StatusFeedback;
    tempBuf += TELECOMMUNICATIONS_INFORMATION_NOTIFICATION_SIZE;
    pNotifications += TELECOMMUNICATIONS_INFORMATION_NOTIFICATION_SIZE;
  }

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                        COMMAND_TEL_INFORMATION_PUSH_INFO_RSP, TRUE,
                        ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;

}


/*********************************************************************
 * @fn      zclTel_InfoSendPreference
 *
 * @brief   Call to send out a Send Preference command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Push Information Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendPreference( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_SendPreference_t *pCmd,
                                    uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t bufLen = TELECOMMUNICATIONS_INFORMATION_SEND_PREFERENCE_SIZE;
  uint8_t *buf = NULL;
  uint8_t *tempBuf = NULL;
  uint8_t i;
  ZStatus_t stat = ZSuccess;

  if(pCmd->PreferenceType == TELECOMMUNICATIONS_INFORMATION_PREFERENCE_TYPE_MULTIPLE_CONTENT_ID)
  {
    bufLen += (pCmd->PreferenceCount * TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE);

    buf = zcl_mem_alloc(bufLen);

    if(buf == NULL)
    {
      return ZMemError;
    }
    tempBuf = buf;

    tempBuf[0] = LO_UINT16(pCmd->PreferenceType);
    tempBuf[1] = HI_UINT16(pCmd->PreferenceType);
    tempBuf[2] = pCmd->PreferenceCount;

    tempBuf += TELECOMMUNICATIONS_INFORMATION_SEND_PREFERENCE_SIZE;

    for (i=0; i<pCmd->PreferenceCount; i++)
    {
          tempBuf[0] = LO_UINT16(*pCmd->PreferencePayload.PrefMultipleContentId);
          tempBuf[1] = HI_UINT16(*pCmd->PreferencePayload.PrefMultipleContentId);
          tempBuf += TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
    }
  }
  else if(pCmd->PreferenceType == TELECOMMUNICATIONS_INFORMATION_PREFERENCE_TYPE_MULTIPLE_OCTECT_STR)
  {
    uint8_t *pOctStr = pCmd->PreferencePayload.PrefMultipleOctectStr;
    for(i=0; i < pCmd->PreferenceCount; i++)
    {
      //Increase the length of the string and count also the byte that tells the length
      bufLen += *pOctStr + 1;
      pOctStr+= *pOctStr + 1;
    }

    buf = zcl_mem_alloc(bufLen);

    if(buf == NULL)
    {
      return ZMemError;
    }

    tempBuf = buf;
    tempBuf[0] = LO_UINT16(pCmd->PreferenceType);
    tempBuf[1] = HI_UINT16(pCmd->PreferenceType);
    tempBuf[2] = pCmd->PreferenceCount;

    tempBuf += TELECOMMUNICATIONS_INFORMATION_SEND_PREFERENCE_SIZE;

    zcl_memcpy(tempBuf, pCmd->PreferencePayload.PrefMultipleOctectStr, bufLen - TELECOMMUNICATIONS_INFORMATION_SEND_PREFERENCE_SIZE);
  }

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                        COMMAND_TEL_INFORMATION_REQ_PREF_RSP, TRUE,
                        ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;

}



/*********************************************************************
 * @fn      zclTel_InfoSendReqPreferenceRsp
 *
 * @brief   Call to send out a Send Request Preference Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Request Preference Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendReqPreferenceRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_ReqPreferenceRsp_t *pCmd,
                                    uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint16_t bufLen = TELECOMMUNICATIONS_INFORMATION_PREFERENCE_RSP_SIZE;
  uint8_t  *buf = NULL;
  uint8_t  *tempBuf = NULL;
  ZStatus_t stat;
  uint8_t i;

  if(pCmd->PreferenceType == TELECOMMUNICATIONS_INFORMATION_PREFERENCE_TYPE_MULTIPLE_CONTENT_ID)
  {
    bufLen += (pCmd->PreferenceCount * TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE);

    buf = zcl_mem_alloc(bufLen);

    if(buf == NULL)
    {
      return ZMemError;
    }
    tempBuf = buf;

    tempBuf[0] = pCmd->StatusFeedback;
    tempBuf[1] = LO_UINT16(pCmd->PreferenceType);
    tempBuf[2] = HI_UINT16(pCmd->PreferenceType);
    tempBuf[3] = pCmd->PreferenceCount;

    tempBuf += TELECOMMUNICATIONS_INFORMATION_PREFERENCE_RSP_SIZE;

    for (i=0; i<pCmd->PreferenceCount; i++)
    {
          tempBuf[0] = LO_UINT16(*pCmd->PreferencePayload.PrefMultipleContentId);
          tempBuf[1] = HI_UINT16(*pCmd->PreferencePayload.PrefMultipleContentId);
          tempBuf += TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
    }
  }
  else if(pCmd->PreferenceType == TELECOMMUNICATIONS_INFORMATION_PREFERENCE_TYPE_MULTIPLE_OCTECT_STR)
  {
    uint8_t *pOctStr = pCmd->PreferencePayload.PrefMultipleOctectStr;
    for(i=0; i < pCmd->PreferenceCount; i++)
    {
      //Increase the length of the string and count also the byte that tells the length
      bufLen += *pOctStr + 1;
      pOctStr+= *pOctStr + 1;
    }

    buf = zcl_mem_alloc(bufLen);

    if(buf == NULL)
    {
      return ZMemError;
    }

    tempBuf = buf;
    tempBuf[0] = pCmd->StatusFeedback;
    tempBuf[1] = LO_UINT16(pCmd->PreferenceType);
    tempBuf[2] = HI_UINT16(pCmd->PreferenceType);
    tempBuf[3] = pCmd->PreferenceCount;

    tempBuf += TELECOMMUNICATIONS_INFORMATION_PREFERENCE_RSP_SIZE;

    zcl_memcpy(tempBuf, pCmd->PreferencePayload.PrefMultipleOctectStr, bufLen - TELECOMMUNICATIONS_INFORMATION_PREFERENCE_RSP_SIZE);
  }

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                        COMMAND_TEL_INFORMATION_REQ_PREF_RSP, TRUE,
                        ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;

}


/*********************************************************************
 * @fn      zclTel_InfoSendUpdate
 *
 * @brief   Call to send out a Send Update command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Update structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendUpdate( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_Update_t *pCmd,
                                    uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint16_t bufLen = TELECOMMUNICATIONS_INFORMATION_UPDATE_SIZE;
  uint8_t  *buf = NULL;
  uint8_t  *tempBuf = NULL;
  ZStatus_t stat;
  uint8_t i,j;

  for(i=0; i < pCmd->MultipleContent.NumberSingleContent; i++)
  {
    uint16_t tempLen;

    //Add ContentId, DataTypeId and LongCharacterLen, number of children
    bufLen += 6;
    tempLen = BUILD_UINT16(*(pCmd->MultipleContent.SingleContent->TitleString),*(pCmd->MultipleContent.SingleContent->TitleString++));
    bufLen += tempLen;
    bufLen += pCmd->MultipleContent.SingleContent->NumberOfChildren * TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
  }

  buf = zcl_mem_alloc(bufLen);

  if(buf == NULL)
  {
    return ZMemError;
  }
  tempBuf = buf;

  tempBuf[0] = pCmd->AccessControl;
  tempBuf[1] = pCmd->OptionField;
  tempBuf[2] = pCmd->MultipleContent.NumberSingleContent;
  tempBuf += 3;

  for(i=0; i < pCmd->MultipleContent.NumberSingleContent; i++)
  {
    uint16_t* pChildrenContent = NULL;
    uint16_t tempLen;

    tempBuf[0] = LO_UINT16(pCmd->MultipleContent.SingleContent->ContentID);
    tempBuf[1] = HI_UINT16(pCmd->MultipleContent.SingleContent->ContentID);
    tempBuf += TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
    tempBuf[2] = pCmd->MultipleContent.SingleContent->DataTypeID;
    tempBuf++;
    tempBuf[3] = pCmd->MultipleContent.SingleContent->TitleString[0];
    tempBuf[4] = pCmd->MultipleContent.SingleContent->TitleString[1];
    tempBuf += 2;

    //Get the length of the string
    tempLen = BUILD_UINT16(*(pCmd->MultipleContent.SingleContent->TitleString),*(pCmd->MultipleContent.SingleContent->TitleString++));

    //Copy the title String, length of the title string is in the first 2 bytes
    tempBuf = zcl_memcpy(tempBuf, &pCmd->MultipleContent.SingleContent->TitleString[2], tempLen);

    pChildrenContent = pCmd->MultipleContent.SingleContent->ChildContentId;

    for(j=0; j < pCmd->MultipleContent.SingleContent->NumberOfChildren; j++)
    {
      tempBuf[0] = LO_UINT16(*pChildrenContent);
      tempBuf[1] = HI_UINT16(*pChildrenContent);
      tempBuf   += TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
      pChildrenContent += TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
    }
  }

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                        COMMAND_TEL_INFORMATION_UPDATE, TRUE,
                        ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;
}


/*********************************************************************
 * @fn      zclTel_InfoSendDelete
 *
 * @brief   Call to send out a Send Delete command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to delete structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendDelete( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_Delete_t *pCmd,
                                    uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint16_t bufLen = 1;
  uint8_t  *buf = NULL;
  uint8_t  *tempBuf = NULL;
  ZStatus_t stat;
  uint8_t i;
  uint16_t* pChildrenContent;

  bufLen += pCmd->NumberContentIDs * TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;

  buf = zcl_mem_alloc(bufLen);

  if(buf == NULL)
  {
    return ZMemError;
  }
  tempBuf = buf;

  tempBuf[0] = pCmd->DeletionOption;
  tempBuf++;

  pChildrenContent = pCmd->ContentIDs;

  for(i=0; i < pCmd->NumberContentIDs; i++)
  {
    tempBuf[0] = LO_UINT16(*pChildrenContent);
    tempBuf[1] = HI_UINT16(*pChildrenContent);
    tempBuf          += TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
    pChildrenContent += TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
  }

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                        COMMAND_TEL_INFORMATION_DELETE, TRUE,
                        ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;
}



/*********************************************************************
 * @fn      zclTel_InfoSendConfigureNodeDescription
 *
 * @brief   Call to send out a Send Configure Node Description command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pDescription - Description to update the node description attribute of remote device.
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendConfigureNodeDescription( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint8_t *pDescription,
                                    uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t  *buf = NULL;
  uint8_t  bufLen;
  ZStatus_t stat;

  bufLen = *pDescription + 1;
  buf = zcl_mem_alloc(bufLen);
  if(buf == NULL)
  {
    return ZMemError;
  }

  zcl_memcpy(buf, pDescription, bufLen);

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                      COMMAND_TEL_INFORMATION_CONF_NODE_DESC, TRUE,
                      ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                      seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;
}

/*********************************************************************
 * @fn      zclTel_InfoSendConfigureDeliveryEnable
 *
 * @brief   Call to send out a Send Configure Delivery Enable command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   EnableFlag - Flag to set the Delivery Enable attribute on the remote device.
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendConfigureDeliveryEnable( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint8_t EnableFlag, uint8_t disableDefaultRsp, uint8_t seqNum )
{

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                      COMMAND_TEL_INFORMATION_CONF_DELIVERY_ENABLE, TRUE,
                      ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                      seqNum, 1, &EnableFlag );

}


/*********************************************************************
 * @fn      zclTel_InfoSendConfigurePushInformationTimer
 *
 * @brief   Call to send out a Send Configure Push Information Timer command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   Timer - Time to set the Push Information Timer attribute on the remote device.
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendConfigurePushInformationTimer( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint32_t Timer, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t  tempTimer[4];

  tempTimer[0] = BREAK_UINT32(Timer,0);
  tempTimer[1] = BREAK_UINT32(Timer,1);
  tempTimer[2] = BREAK_UINT32(Timer,2);
  tempTimer[3] = BREAK_UINT32(Timer,3);

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                          COMMAND_TEL_INFORMATION_CONF_PUSH_INFO_TIMER, TRUE,
                      ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                      seqNum, 4, tempTimer );

}

/*********************************************************************
 * @fn      zclTel_InfoSendConfigureSetRootID
 *
 * @brief   Call to send out a Send Configure Set Root ID command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   RootID - RootID attribute to set on the remote device.
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendConfigureSetRootID( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint16_t RootID, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t  tempRootId[2];

  tempRootId[0] = BREAK_UINT32(RootID,0);
  tempRootId[1] = BREAK_UINT32(RootID,1);

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                          COMMAND_TEL_INFORMATION_CONF_SET_ROOT_ID, TRUE,
                      ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                      seqNum, 2, tempRootId );

}


/*********************************************************************
 * @fn      zclTel_InfoSendRequestInformationResponse
 *
 * @brief   Call to send out a Send Request Information Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Request Information Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendRequestInformationResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_ReqInfoRsp_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint16_t bufLen = TELECOMMUNICATIONS_INFORMATION_REQ_INFO_RSP_SIZE;
  uint8_t  *buf = NULL;
  uint8_t  *tempBuf = NULL;
  ZStatus_t stat;
  uint8_t i,j;

  for(i=0; i < pCmd->NumberSingleContent; i++)
  {
    uint16_t tempLen;
    //Add ContentId, DataTypeId and LongCharacterLen, number of children
    bufLen += 6;
    tempLen = BUILD_UINT16(*(pCmd->SingleContent.TitleString),*(pCmd->SingleContent.TitleString++));
    bufLen += tempLen;
    bufLen += pCmd->SingleContent.NumberOfChildren * TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
  }

  buf = zcl_mem_alloc(bufLen);

  if(buf == NULL)
  {
    return ZMemError;
  }
  tempBuf = buf;

  tempBuf[0] = pCmd->NumberSingleContent;
  tempBuf[1] = pCmd->StatusFeedback;
  tempBuf += 2;

  for(i=0; i < pCmd->NumberSingleContent; i++)
  {
    uint16_t* pChildrenContent = NULL;
    uint16_t tempLen;

    tempBuf[0] = LO_UINT16(pCmd->SingleContent.ContentID);
    tempBuf[1] = HI_UINT16(pCmd->SingleContent.ContentID);
    tempBuf += TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
    tempBuf[2] = pCmd->SingleContent.DataTypeID;
    tempBuf++;
    tempBuf[3] = pCmd->SingleContent.TitleString[1];
    tempBuf[4] = pCmd->SingleContent.TitleString[0];
    tempBuf += 2;

    //Get the length of the string
    tempLen = BUILD_UINT16(*(pCmd->SingleContent.TitleString),*(pCmd->SingleContent.TitleString++));

    //Copy the title String, length of the title string is in the first 2 bytes
    tempBuf = zcl_memcpy(tempBuf, &pCmd->SingleContent.TitleString[2], tempLen);

    pChildrenContent = pCmd->SingleContent.ChildContentId;

    for(j=0; j < pCmd->SingleContent.NumberOfChildren; j++)
    {
      tempBuf[0] = LO_UINT16(*pChildrenContent);
      tempBuf[1] = HI_UINT16(*pChildrenContent);
      tempBuf   += TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
      pChildrenContent += TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
    }
  }

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                        COMMAND_TEL_INFORMATION_REQ_INFO_RSP, TRUE,
                        ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;

}


/*********************************************************************
 * @fn      zclTel_InfoSendPushInformation
 *
 * @brief   Call to send out a Send Push Information command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to push Information structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendPushInformation( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_PushInfo_t* pCmd, uint8_t disableDefaultRsp,
                                    uint8_t seqNum )
{
  uint16_t bufLen = 0;
  uint8_t  *buf = NULL;
  uint8_t  *tempBuf = NULL;
  ZStatus_t stat;
  uint8_t i,j;

  for(i=0; i < pCmd->MultipleContent.NumberSingleContent; i++)
  {
    uint16_t tempLen;

    //Add ContentId, DataTypeId and LongCharacterLen, number of children
    bufLen += 6;
    tempLen = BUILD_UINT16(*(pCmd->MultipleContent.SingleContent->TitleString),*(pCmd->MultipleContent.SingleContent->TitleString++));
    bufLen += tempLen;
    bufLen += pCmd->MultipleContent.SingleContent->NumberOfChildren * TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
  }

  buf = zcl_mem_alloc(bufLen);

  if(buf == NULL)
  {
    return ZMemError;
  }
  tempBuf = buf;

  for(i=0; i < pCmd->MultipleContent.NumberSingleContent; i++)
  {
    uint16_t* pChildrenContent = NULL;
    uint16_t tempLen;

    tempBuf[0] = LO_UINT16(pCmd->MultipleContent.SingleContent->ContentID);
    tempBuf[1] = HI_UINT16(pCmd->MultipleContent.SingleContent->ContentID);
    tempBuf += TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
    tempBuf[2] = pCmd->MultipleContent.SingleContent->DataTypeID;
    tempBuf++;
    tempBuf[3] = pCmd->MultipleContent.SingleContent->TitleString[0];
    tempBuf[4] = pCmd->MultipleContent.SingleContent->TitleString[1];
    tempBuf += 2;

    //Get the length of the string
    tempLen = BUILD_UINT16(*(pCmd->MultipleContent.SingleContent->TitleString),*(pCmd->MultipleContent.SingleContent->TitleString++));

    //Copy the title String, length of the title string is in the first 2 bytes
    tempBuf = zcl_memcpy(tempBuf, &pCmd->MultipleContent.SingleContent->TitleString[2], tempLen);

    pChildrenContent = pCmd->MultipleContent.SingleContent->ChildContentId;

    for(j=0; j < pCmd->MultipleContent.SingleContent->NumberOfChildren; j++)
    {
      tempBuf[0] = LO_UINT16(*pChildrenContent);
      tempBuf[1] = HI_UINT16(*pChildrenContent);
      tempBuf   += TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
      pChildrenContent += TELECOMMUNICATIONS_INFORMATION_CONTENT_ID_SIZE;
    }
  }

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                        COMMAND_TEL_INFORMATION_UPDATE, TRUE,
                        ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;
}


/*********************************************************************
 * @fn      zclTel_InfoSendPreferenceRsp
 *
 * @brief   Call to send out a Send Preference Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Send Preference Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendPreferenceRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_PreferenceRsp_t* pCmd, uint8_t disableDefaultRsp,
                                    uint8_t seqNum )
{

  return  zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                          COMMAND_TEL_INFORMATION_SEND_PREF_RSP, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                          seqNum, pCmd->NumberStatus, pCmd->StatusFeedback );
}


/*********************************************************************
 * @fn      zclTel_InfoSendServerRequestPreference
 *
 * @brief   Call to send out a Send Server Request Preference command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendServerRequestPreference( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint8_t disableDefaultRsp, uint8_t seqNum )
{

  return  zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                          COMMAND_TEL_INFORMATION_SERVER_REQ_PREF, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                          seqNum, 0, NULL );
}

/*********************************************************************
 * @fn      zclTel_InfoSendPreferenceConfirmation
 *
 * @brief   Call to send out a Send Preference Confirmation command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Send Preference Confirmation structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendPreferenceConfirmation( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_PreferenceConfirm_t* pCmd, uint8_t disableDefaultRsp,
                                    uint8_t seqNum )
{

  return  zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                          COMMAND_TEL_INFORMATION_REQ_PREF_CONFIR, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                          seqNum, pCmd->NumberStatus, pCmd->StatusFeedback );
}



/*********************************************************************
 * @fn      zclTel_InfoSendUpdateRsp
 *
 * @brief   Call to send out a Send Delete Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Update Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendUpdateRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_UpdateRsp_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t bufLen;
  uint8_t *buf = NULL;
  uint8_t *tempBuf = NULL;
  uint8_t i;
  ZStatus_t stat = ZSuccess;
  zcl_Tel_Info_Notification_t *pNotifications;

  bufLen = pCmd->NumNotifications * TELECOMMUNICATIONS_INFORMATION_NOTIFICATION_SIZE;

  buf = zcl_mem_alloc(bufLen);

  if(buf == NULL)
  {
    return ZMemError;
  }
  tempBuf = buf;

  pNotifications = pCmd->Notifications;
  for (i=0; i < pCmd->NumNotifications; i++)
  {
    tempBuf[0] = LO_UINT16(pNotifications->ContentId);
    tempBuf[1] = HI_UINT16(pNotifications->ContentId);
    tempBuf[2] = pNotifications->StatusFeedback;
    tempBuf += TELECOMMUNICATIONS_INFORMATION_NOTIFICATION_SIZE;
    pNotifications += TELECOMMUNICATIONS_INFORMATION_NOTIFICATION_SIZE;
  }

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                        COMMAND_TEL_INFORMATION_UPDATE_RSP, TRUE,
                        ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;
}


/*********************************************************************
 * @fn      zclTel_InfoSendDeleteRsp
 *
 * @brief   Call to send out a Send Delete Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Delete Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendDeleteRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_DeleteRsp_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t bufLen;
  uint8_t *buf = NULL;
  uint8_t *tempBuf = NULL;
  uint8_t i;
  ZStatus_t stat = ZSuccess;
  zcl_Tel_Info_Notification_t *pNotifications;

  bufLen = pCmd->NumNotifications * TELECOMMUNICATIONS_INFORMATION_NOTIFICATION_SIZE;

  buf = zcl_mem_alloc(bufLen);

  if(buf == NULL)
  {
    return ZMemError;
  }
  tempBuf = buf;

  pNotifications = pCmd->Notifications;
  for (i=0; i < pCmd->NumNotifications; i++)
  {
    tempBuf[0] = LO_UINT16(pNotifications->ContentId);
    tempBuf[1] = HI_UINT16(pNotifications->ContentId);
    tempBuf[2] = pNotifications->StatusFeedback;
    tempBuf += TELECOMMUNICATIONS_INFORMATION_NOTIFICATION_SIZE;
    pNotifications += TELECOMMUNICATIONS_INFORMATION_NOTIFICATION_SIZE;
  }

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION,
                        COMMAND_TEL_INFORMATION_DELETE_RSP, TRUE,
                        ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;
}

#endif

#ifdef ZCL_CHATTING

/*********************************************************************
 * @fn      zclTel_ChatJoinChatReq
 *
 * @brief   Call to send out a Send Join Chat Request command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Join Request structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_ChatJoinChatReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_JoinReq_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t bufLen = TELECOMMUNICATIONS_CHATTING_JOIN_REQ_SIZE;
  uint8_t *buf = NULL;
  uint8_t *tempBuf = NULL;
  ZStatus_t stat = ZSuccess;

  bufLen += pCmd->Nickname[0];

  buf = tempBuf = zcl_mem_alloc(bufLen);

  if(buf == NULL)
  {
    return ZMemError;
  }

  *tempBuf++ = LO_UINT16(pCmd->U_ID);
  *tempBuf++ = HI_UINT16(pCmd->U_ID);

  // Copy length of nickname and length byte
  tempBuf = zcl_memcpy(tempBuf, &pCmd->Nickname[0], pCmd->Nickname[0] + 1);

  *tempBuf++ = LO_UINT16(pCmd->C_ID);
  *tempBuf++ = HI_UINT16(pCmd->C_ID);

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                        COMMAND_TEL_CHATTING_JOIN_CHAT_REQ, TRUE,
                        ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;
}

/*********************************************************************
 * @fn      zclTel_ChatLeaveChatReq
 *
 * @brief   Call to send out a Send Leave Chat Request command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Leave Chat Request structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_ChatLeaveChatReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_LeaveChatReq_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[TELECOMMUNICATIONS_CHATTING_LEAVE_CHAT_REQ_SIZE];

  buf[0] = LO_UINT16(pCmd->C_ID);
  buf[1] = HI_UINT16(pCmd->C_ID);
  buf[2] = LO_UINT16(pCmd->U_ID);
  buf[3] = HI_UINT16(pCmd->U_ID);

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                        COMMAND_TEL_CHATTING_LEAVE_CHAT_REQ, TRUE,
                        ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                        seqNum, TELECOMMUNICATIONS_CHATTING_LEAVE_CHAT_REQ_SIZE, buf );
}


/*********************************************************************
 * @fn      zclTel_ChatSearchChatReq
 *
 * @brief   Call to send out a Send Search Chat Request command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Leave Chat Request structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_ChatSearchChatReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                   uint8_t disableDefaultRsp, uint8_t seqNum )
{

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                        COMMAND_TEL_CHATTING_SEARCH_CHAT_REQ, TRUE,
                        ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                        seqNum, 0, NULL );
}


/*********************************************************************
 * @fn      zclTel_ChatSwitchChairmanRsp
 *
 * @brief   Call to send out a Send Switch Chairman Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Switch Chairman Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_ChatSwitchChairmanRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_SwitchChairmanRsp_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[TELECOMMUNICATIONS_CHATTING_SWITCH_CHAIRMAN_RSP_SIZE];

  buf[0] = LO_UINT16(pCmd->C_ID);
  buf[1] = HI_UINT16(pCmd->C_ID);
  buf[2] = LO_UINT16(pCmd->U_ID);
  buf[3] = HI_UINT16(pCmd->U_ID);

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                        COMMAND_TEL_CHATTING_SWITCH_CHAIRMAN_RSP, TRUE,
                        ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                        seqNum, TELECOMMUNICATIONS_CHATTING_SWITCH_CHAIRMAN_RSP_SIZE, buf );
}


/*********************************************************************
 * @fn      zclTel_ChatStartChatReq
 *
 * @brief   Call to send out a Send Start Chat Request command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Start Chat Request structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_ChatStartChatReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_StartChatReq_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t bufLen = TELECOMMUNICATIONS_CHATTING_START_CHAT_REQ_SIZE;
  uint8_t *buf = NULL;
  uint8_t *tempBuf = NULL;
  ZStatus_t stat = ZSuccess;

  bufLen += pCmd->Name[0] + pCmd->Nickname[0];

  buf = tempBuf = zcl_mem_alloc(bufLen);

  if(buf == NULL)
  {
    return ZMemError;
  }

  // Copy length of name and length byte
  tempBuf = zcl_memcpy(tempBuf, &pCmd->Name[0], pCmd->Name[0] + 1);
  *tempBuf++ = LO_UINT16(pCmd->U_ID);
  *tempBuf++ = HI_UINT16(pCmd->U_ID);

  // Copy length of nickname and length byte
  tempBuf = zcl_memcpy(tempBuf, &pCmd->Nickname[0], pCmd->Nickname[0] + 1);

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                        COMMAND_TEL_CHATTING_START_CHAT_REQ, TRUE,
                        ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;
}

/*********************************************************************
 * @fn      zclTel_ChatChatMessage
 *
 * @brief   Call to send out a Send Chat Message command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Chat Message structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_ChatChatMessage( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_ChatMessage_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t bufLen = TELECOMMUNICATIONS_CHATTING_CHAT_MESSAGE_SIZE;
  uint8_t *buf = NULL;
  uint8_t *tempBuf = NULL;
  ZStatus_t stat = ZSuccess;

  bufLen += pCmd->Nickname[0] + pCmd->Message[0];

  buf = tempBuf = zcl_mem_alloc(bufLen);

  if(buf == NULL)
  {
    return ZMemError;
  }

  *tempBuf++ = LO_UINT16(pCmd->Dest_U_ID);
  *tempBuf++ = HI_UINT16(pCmd->Dest_U_ID);
  *tempBuf++ = LO_UINT16(pCmd->Src_U_ID);
  *tempBuf++ = HI_UINT16(pCmd->Src_U_ID);
  *tempBuf++ = LO_UINT16(pCmd->C_ID);
  *tempBuf++ = HI_UINT16(pCmd->C_ID);

  // Copy length of nickname and length byte
  tempBuf = zcl_memcpy(tempBuf, &pCmd->Nickname[0], pCmd->Nickname[0] + 1);

  // Copy length of message and length byte
  tempBuf = zcl_memcpy(tempBuf, &pCmd->Message[0], pCmd->Message[0] + 1);

  stat =  zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                         COMMAND_TEL_CHATTING_CHAT_MESSAGE, TRUE,
                         ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                         seqNum, bufLen, buf );
  zcl_mem_free(buf);

  return stat;
}

/*********************************************************************
 * @fn      zclTel_ChatGetNodeInfoReq
 *
 * @brief   Call to send out a Send Get Node Information Request command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Get Node Information Request structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_ChatGetNodeInfoReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_GetNodeInfoReq_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{

  uint8_t buf[TELECOMMUNICATIONS_CHATTING_GET_NODE_INFO_REQ_SIZE];

  buf[0] = LO_UINT16(pCmd->C_ID);
  buf[1] = HI_UINT16(pCmd->C_ID);
  buf[2] = LO_UINT16(pCmd->U_ID);
  buf[3] = HI_UINT16(pCmd->U_ID);


  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                        COMMAND_TEL_CHATTING_GET_NODE_INFO_REQ, TRUE,
                        ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                        seqNum, TELECOMMUNICATIONS_CHATTING_GET_NODE_INFO_REQ_SIZE, buf );
}



/*********************************************************************
 * @fn      zclTel_ChatStartChatRsp
 *
 * @brief   Call to send out a Send Start Chat Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Start Chat Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_ChatStartChatRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_StartChatRsp_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t bufLen = 1;
  uint8_t buf[TELECOMMUNICATIONS_CHATTING_START_CHAT_RSP_SIZE];

  buf[0] = pCmd->status;

  if(pCmd->status == ZSuccess)
  {
    bufLen += 2;
    buf[1] = LO_UINT16(pCmd->C_ID);
    buf[2] = HI_UINT16(pCmd->C_ID);
  }

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                        COMMAND_TEL_CHATTING_START_CHAT_RSP, TRUE,
                        ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );
}

/*********************************************************************
 * @fn      zclTel_ChatJoinChatRsp
 *
 * @brief   Call to send out a Send Join Chat Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Start Chat Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_ChatJoinChatRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_JoinChatRsp_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t bufLen = TELECOMMUNICATIONS_CHATTING_JOIN_CHAT_RSP_SIZE;
  uint8_t *buf = NULL;
  uint8_t *tempBuf = NULL;
  uint8_t i;
  ZStatus_t stat = ZSuccess;
  ChatUser_t *ChatUsers = NULL;

  if(pCmd->status == ZSuccess && pCmd->numberChatUsers)
  {
    //Increase length of buffer by number of U_IDs in the list
    bufLen += pCmd->numberChatUsers * 2;

    ChatUsers = pCmd->ChatUsers;

    //Increase length of buffer by the nickname length and length byte
    for(i=0; i < pCmd->numberChatUsers; i++)
    {
      uint8_t *ptr = (uint8_t*)ChatUsers;
      bufLen += ChatUsers->Nickname[0] + 1;
      // Point to next chat user
      ptr += sizeof(ChatRoom_t);
      ChatUsers = (ChatUser_t*)ptr;
    }
  }

  buf = tempBuf = zcl_mem_alloc(bufLen);

  if(buf == NULL)
  {
    return ZMemError;
  }

  *tempBuf++ = pCmd->status;
  *tempBuf++ = LO_UINT16(pCmd->C_ID);
  *tempBuf++ = HI_UINT16(pCmd->C_ID);

  ChatUsers = pCmd->ChatUsers;

  // Copy the nicknames
  if(pCmd->status == ZSuccess){
    for(i=0; i < pCmd->numberChatUsers; i++)
    {
      uint8_t* ptr = (uint8_t*)ChatUsers;
      *tempBuf++ = LO_UINT16(ChatUsers->U_ID);
      *tempBuf++ = HI_UINT16(ChatUsers->U_ID);
      tempBuf = zcl_memcpy(tempBuf, ChatUsers->Nickname, ChatUsers->Nickname[0] + 1);

      // Point to next chat user
      ptr += sizeof(ChatRoom_t);
      ChatUsers = (ChatUser_t*)ptr;
    }
  }

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                        COMMAND_TEL_CHATTING_JOIN_CHAT_RSP, TRUE,
                        ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;
}


/*********************************************************************
 * @fn      zclTel_ChatUserLeftJoined
 *
 * @brief   Call to send out a Send User Left or User Joined command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to User Left or Join structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_ChatUserLeftJoined( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_UserLeftJoined_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t bufLen = TELECOMMUNICATIONS_CHATTING_USER_LEFT_SIZE;
  uint8_t *buf = NULL;
  uint8_t *tempBuf = NULL;
  ZStatus_t stat = ZSuccess;
  uint16_t cmdId;

  if(pCmd->didUserLeft)
  {
    cmdId = COMMAND_TEL_CHATTING_USER_LEFT;
  }
  else
  {
    cmdId = COMMAND_TEL_CHATTING_USER_JOINED;
  }

  //Increase buffer length by the nickname length (length byte included in USER_LEFT_SIZE)
  bufLen += pCmd->ChatUser.Nickname[0];

  buf = tempBuf = zcl_mem_alloc(bufLen);

  if(buf == NULL)
  {
    return ZMemError;
  }

  *tempBuf++ = LO_UINT16(pCmd->C_ID);
  *tempBuf++ = HI_UINT16(pCmd->C_ID);
  *tempBuf++ = LO_UINT16(pCmd->ChatUser.U_ID);
  *tempBuf++ = HI_UINT16(pCmd->ChatUser.U_ID);

  tempBuf = zcl_memcpy(tempBuf, pCmd->ChatUser.Nickname, pCmd->ChatUser.Nickname[0] + 1);

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                        cmdId, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;
}

/*********************************************************************
 * @fn      zclTel_ChatSearchChatRsp
 *
 * @brief   Call to send out a Send Search Chat Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Search Chat Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_ChatSearchChatRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_SearchChatRsp_t *pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t bufLen = TELECOMMUNICATIONS_CHATTING_SEARCH_CHAT_RSP_SIZE;
  uint8_t *buf = NULL;
  uint8_t *tempBuf = NULL;
  uint8_t i;
  ZStatus_t stat = ZSuccess;
  ChatRoom_t *ChatRoom = NULL;

  if(pCmd->numberChatRooms)
  {
    //Increase length of buffer by number of U_IDs in the list
    bufLen += pCmd->numberChatRooms * 2;

    ChatRoom = pCmd->ChatRoom;

    //Increase length of buffer by the name length
    for(i=0; i < pCmd->numberChatRooms; i++)
    {
      uint8_t* ptr = (uint8_t*)ChatRoom;
      bufLen += ChatRoom->Name[0] + 1;
      // Point to next chat room
      ptr += sizeof(ChatRoom_t);
      ChatRoom = (ChatRoom_t*)ptr;
    }
  }

  buf = tempBuf = zcl_mem_alloc(bufLen);

  if(buf == NULL)
  {
    return ZMemError;
  }

  *tempBuf++ = pCmd->options;

  ChatRoom = pCmd->ChatRoom;

  //Copy the chat room names
  for(i=0; i < pCmd->numberChatRooms; i++)
  {
    uint8_t* ptr = (uint8_t*)ChatRoom;

    *tempBuf++ = LO_UINT16(ChatRoom->C_ID);
    *tempBuf++ = HI_UINT16(ChatRoom->C_ID);
    tempBuf = zcl_memcpy(tempBuf, ChatRoom->Name, ChatRoom->Name[0] + 1);

    //Point to next chat room
    ptr += sizeof(ChatRoom_t);
    ChatRoom = (ChatRoom_t*)ptr;
  }

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                        COMMAND_TEL_CHATTING_SEARCH_CHAT_RSP, TRUE,
                        ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;
}

/*********************************************************************
 * @fn      zclTel_ChatSwitchChairmanReq
 *
 * @brief   Call to send out a Send Switch Chairman Request command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   C_ID - ID of the chat room where the chairman is requested to be changed
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_ChatSwitchChairmanReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint16_t C_ID, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[TELECOMMUNICATIONS_CHATTING_SWITCH_CHAIRMAN_REQ_SIZE];

  buf[0] = LO_UINT16(C_ID);
  buf[1] = HI_UINT16(C_ID);

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                        COMMAND_TEL_CHATTING_SWITCH_CHAIRMAN_REQ, TRUE,
                        ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                        seqNum, TELECOMMUNICATIONS_CHATTING_SWITCH_CHAIRMAN_REQ_SIZE, buf );
}


/*********************************************************************
 * @fn      zclTel_ChatSwitchChairmanCnf
 *
 * @brief   Call to send out a Send Switch Chairman Confirm command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Switch Chairman Confirm structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_ChatSwitchChairmanCnf( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_SwitchChairmanCnf_t *pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t bufLen = TELECOMMUNICATIONS_CHATTING_SWITCH_CHAIRMAN_CNF_SIZE;
  uint8_t *buf = NULL;
  uint8_t *tempBuf = NULL;
  uint8_t i;
  ZStatus_t stat = ZSuccess;
  NodeInfo_t *NodeInfo = NULL;

  if(pCmd->numberNodeInfo)
  {
    //Increase length of buffer by number of nodes (each node room must have U_ID, ShortAddr and endpoint)
    bufLen += pCmd->numberNodeInfo * 5;

    NodeInfo = pCmd->NodeInfo;

    //increase length of buffer by the Nickname length in nodeInformation
    for(i=0; i < pCmd->numberNodeInfo; i++)
    {
      uint8_t* ptr = (uint8_t*)NodeInfo;
      // Length of nickname size plus length byte
      bufLen += NodeInfo->Nickname[0] + 1;
      //Point to next node information
      ptr += sizeof(NodeInfo_t);
      NodeInfo = (NodeInfo_t*)ptr;
    }
  }

  buf = tempBuf = zcl_mem_alloc(bufLen);

  if(buf == NULL)
  {
    return ZMemError;
  }

  *tempBuf++ = LO_UINT16(pCmd->C_ID);
  *tempBuf++ = HI_UINT16(pCmd->C_ID);

  NodeInfo = pCmd->NodeInfo;

  //Copy the nodes information
  for(i=0; i < pCmd->numberNodeInfo; i++)
  {
    uint8_t* ptr = (uint8_t*)NodeInfo;

    *tempBuf++ = LO_UINT16(NodeInfo->U_ID);
    *tempBuf++ = HI_UINT16(NodeInfo->U_ID);
    *tempBuf++ = LO_UINT16(NodeInfo->ShortAddr);
    *tempBuf++ = HI_UINT16(NodeInfo->ShortAddr);
    *tempBuf++ = NodeInfo->endpoint;
    tempBuf = zcl_memcpy(tempBuf, NodeInfo->Nickname, NodeInfo->Nickname[0] + 1);

    //Point to next node information
    ptr += sizeof(NodeInfo_t);
    NodeInfo = (NodeInfo_t*)ptr;
  }

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                        COMMAND_TEL_CHATTING_SWITCH_CHAIRMAN_CONFIRM, TRUE,
                        ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;
}


/*********************************************************************
 * @fn      zclTel_ChatSwitchChairmanNotification
 *
 * @brief   Call to send out a Send Switch Chairman Notification command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Switch Chairman Notification structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_ChatSwitchChairmanNotification( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_SwitchChairmanNotification_t *pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[TELECOMMUNICATIONS_CHATTING_SWITCH_CHAIRMAN_NOTIF_SIZE];

  buf[0] = LO_UINT16(pCmd->C_ID);
  buf[1] = HI_UINT16(pCmd->C_ID);
  buf[2] = LO_UINT16(pCmd->U_ID);
  buf[3] = HI_UINT16(pCmd->U_ID);
  buf[4] = LO_UINT16(pCmd->ShortAddr);
  buf[5] = HI_UINT16(pCmd->ShortAddr);
  buf[6] = pCmd->endpoint;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                          COMMAND_TEL_CHATTING_SWITCH_CHAIRMAN_NOTIFICATION, TRUE,
                        ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                        seqNum, TELECOMMUNICATIONS_CHATTING_SWITCH_CHAIRMAN_NOTIF_SIZE, buf );
}


/*********************************************************************
 * @fn      zclTel_ChatGetNodeInfoRsp
 *
 * @brief   Call to send out a Send Get Node Information Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Get Node Information Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_ChatGetNodeInfoRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_GetNodeInfoRsp_t *pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t bufLen = TELECOMMUNICATIONS_CHATTING_GET_NODE_INFO_RSP_SIZE;
  uint8_t *buf = NULL;
  uint8_t *tempBuf = NULL;
  ZStatus_t stat = ZSuccess;

  if(pCmd->status == ZSuccess)
  {
    //If success, then add ShortAddr, Endpoint and length field of Nickname
    bufLen += 4 + pCmd->Nickname[0];
  }

  buf = tempBuf = zcl_mem_alloc(bufLen);

  if(buf == NULL)
  {
    return ZMemError;
  }

  *tempBuf++ = pCmd->status;
  *tempBuf++ = LO_UINT16(pCmd->C_ID);
  *tempBuf++ = HI_UINT16(pCmd->C_ID);
  *tempBuf++ = LO_UINT16(pCmd->U_ID);
  *tempBuf++ = HI_UINT16(pCmd->U_ID);

  if(pCmd->status == ZSuccess)
  {
      *tempBuf++ = LO_UINT16(pCmd->shortAddr);
      *tempBuf++ = HI_UINT16(pCmd->shortAddr);
      *tempBuf++ = pCmd->endpoint;

      zcl_memcpy(tempBuf, &pCmd->Nickname[0], pCmd->Nickname[0] + 1);
  }


  stat =  zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING,
                        COMMAND_TEL_CHATTING_GET_NODE_INFO_RSP, TRUE,
                        ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

  zcl_mem_free(buf);

  return stat;
}
#endif

#ifdef ZCL_VOICE
/*********************************************************************
 * @fn      zclTel_VoiceEstablishmentRequest
 *
 * @brief   Call to send out a Send Establishment Request command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Establishment Request structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_VoiceEstablishmentRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Voice_GetNodeInfoRsp_t *pCmd, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[TELECOMMUNICATIONS_VOICE_ESTABLISHMENT_REQ_SIZE];
  uint8_t bufLen = 5;
  uint8_t*  pBuf;

  buf[0] = pCmd->Flag;
  buf[1] = pCmd->CodecType;
  buf[2] = pCmd->SampleFreq;
  buf[3] = pCmd->CodecRate;
  buf[4] = pCmd->ServiceType;
  pBuf = &buf[5];
  if(pCmd->Flag & TELECOMMUNICATIONS_VOICE_CODEC_TYPE_S1_BIT)
  {
    bufLen++;
    *pBuf = pCmd->CodecTypeS1;
    pBuf++;
  }
  if(pCmd->Flag & TELECOMMUNICATIONS_VOICE_CODEC_TYPE_S2_BIT)
  {
    bufLen++;
    *pBuf = pCmd->CodecTypeS2;
    pBuf++;
  }
  if(pCmd->Flag & TELECOMMUNICATIONS_VOICE_CODEC_TYPE_S3_BIT)
  {
    bufLen++;
    *pBuf = pCmd->CodecTypeS3;
    pBuf++;
  }
  if(pCmd->Flag & TELECOMMUNICATIONS_VOICE_COMPRESSION_BIT)
  {
    bufLen += 2;
    *pBuf = pCmd->CompType;
    pBuf++;
    *pBuf = pCmd->CompRate;
    pBuf++;
  }

 return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_VOICE_OVER_ZIGBEE,
                        COMMAND_TEL_VOICE_ESTABLISHMENT_REQ, TRUE,
                        ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );

}


/*********************************************************************
 * @fn      zclTel_VoiceTransmission
 *
 * @brief   Call to send out a Send Voice Transmission command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pVoiceData - pointer voice data
 * @param   voiceDataLen - Length of the voice data
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_VoiceTransmission( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint8_t* pVoiceData, uint16_t voiceDataLen, uint8_t disableDefaultRsp, uint8_t seqNum )
{

return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_VOICE_OVER_ZIGBEE,
                        COMMAND_TEL_VOICE_VOICE_TRANSMISSION, TRUE,
                        ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                        seqNum, voiceDataLen, pVoiceData );


}


/*********************************************************************
 * @fn      zclTel_VoiceTransmissionCompletion
 *
 * @brief   Call to send out a Send Voice Transmission Completion command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_VoiceTransmissionCompletion( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint8_t disableDefaultRsp, uint8_t seqNum )
{
  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_VOICE_OVER_ZIGBEE,
                        COMMAND_TEL_VOICE_VOICE_TRANSMISSION_COMPLETION, TRUE,
                        ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                        seqNum, 0, NULL );

}



/*********************************************************************
 * @fn      zclTel_VoiceControlRsp
 *
 * @brief   Call to send out a Send ControlRsp command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   ACK_NACK - pointer voice data
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_VoiceControlRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t ACK_NACK,
                                    uint8_t disableDefaultRsp, uint8_t seqNum )
{

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_VOICE_OVER_ZIGBEE,
                        COMMAND_TEL_VOICE_CONTROL_RSP, TRUE,
                        ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                        seqNum, 1, &ACK_NACK );

}


/*********************************************************************
 * @fn      zclTel_VoiceTransmissionRsp
 *
 * @brief   Call to send out a Send Voice Transmission Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   zclSeqNumFailed - ZCL Sequence Number of the frame failed.
 * @param   ErrorFlag - Error reason: 0x00 Failure to decode voice data, 0x01 Wrong order of voice data
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_VoiceTransmissionRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint8_t zclSeqNumFailed, uint8_t ErrorFlag, uint8_t disableDefaultRsp, uint8_t seqNum )
{

  uint8_t buf[TELECOMMUNICATIONS_VOICE_TRANSMISSION_RSP_SIZE];
  buf[0] = zclSeqNumFailed;
  buf[1] = ErrorFlag;

    return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_VOICE_OVER_ZIGBEE,
                        COMMAND_TEL_VOICE_VOICE_TRANSMISSION_RSP, TRUE,
                        ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                        seqNum, TELECOMMUNICATIONS_VOICE_TRANSMISSION_RSP_SIZE, buf );
}

/*********************************************************************
 * @fn      zclTel_VoiceEstablishmentRsp
 *
 * @brief   Call to send out a Send Voice Establishment Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   ACK_NACK - ACK=0x01 NAK=0x00. Set to 0, if codec type not supported and provide codectype supported in field CodecType.
 * @param   CodecType - Codec Type supported. Only used if ACK_NACK is set to 0.
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_VoiceEstablishmentRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint8_t ACK_NACK, uint8_t CodecType, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[2];
  uint8_t bufLen = 1;

  buf[0] = ACK_NACK;

  if(ACK_NACK == 0)
  {
    buf[1] = CodecType;
    bufLen ++;
  }

    return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_VOICE_OVER_ZIGBEE,
                        COMMAND_TEL_VOICE_ESTABLISHMENT_RSP, TRUE,
                        ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                        seqNum, bufLen, buf );
}


/*********************************************************************
 * @fn      zclTel_VoiceControl
 *
 * @brief   Call to send out a Send Control command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   ControlType -  indicates the control options, including the play operation (0x01), the stop operation (0x02), and the disconnection operation (0x03)
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_VoiceControl( uint8_t srcEP, afAddrType_t *dstAddr,
                                     uint8_t ControlType, uint8_t disableDefaultRsp, uint8_t seqNum )
{
      return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_TELECOMMUNICATIONS_VOICE_OVER_ZIGBEE,
                        COMMAND_TEL_VOICE_CONTROL, TRUE,
                        ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                        seqNum, 1, &ControlType );

}
#endif


/********************************************************************************************
*********************************************************************************************/
