/**************************************************************************************************
  Filename:       zcl_general.c
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
#include "zcomdef.h"
#include "ti_zstack_config.h"
#include "zcl.h"
#include "zcl_general.h"

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  #include "stub_aps.h"
#endif
#include "bdb.h"

/*********************************************************************
 * MACROS
 */
#define locationTypeAbsolute( a )          ( (a) & LOCATION_TYPE_ABSOLUTE )
#define locationType2D( a )                ( (a) & LOCATION_TYPE_2_D )
#define locationTypeCoordinateSystem( a )  ( (a) & LOCATION_TYPE_COORDINATE_SYSTEM )

#ifdef ZCL_SCENES
#define zclGeneral_ScenesRemaingCapacity() ( ZCL_GENERAL_MAX_SCENES - zclGeneral_CountAllScenes() )
#endif // ZCL_SCENES

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct zclGenCBRec
{
  struct zclGenCBRec        *next;
  uint8_t                     endpoint; // Used to link it into the endpoint descriptor
  zclGeneral_AppCallbacks_t *CBs;     // Pointer to Callback function
} zclGenCBRec_t;

typedef struct zclGenSceneItem
{
  struct zclGenSceneItem    *next;
  uint8_t                     endpoint; // Used to link it into the endpoint descriptor
  zclGeneral_Scene_t        scene;    // Scene info
} zclGenSceneItem_t;

typedef struct zclGenAlarmItem
{
  struct zclGenAlarmItem    *next;
  uint8_t                     endpoint; // Used to link it into the endpoint descriptor
  zclGeneral_Alarm_t        alarm;    // Alarm info
} zclGenAlarmItem_t;

// Scene NV types
typedef struct
{
  uint16_t                    numRecs;
} nvGenScenesHdr_t;

typedef struct zclGenSceneNVItem
{
  uint8_t                     endpoint;
  zclGeneral_Scene_t        scene;
} zclGenSceneNVItem_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static zclGenCBRec_t *zclGenCBs = (zclGenCBRec_t *)NULL;
static uint8_t zclGenPluginRegisted = FALSE;

#if defined( ZCL_SCENES )
  #if !defined ( ZCL_STANDALONE )
    static zclGenSceneItem_t *zclGenSceneTable = (zclGenSceneItem_t *)NULL;
  #endif
#endif // ZCL_SCENES

#ifdef ZCL_ALARMS
static zclGenAlarmItem_t *zclGenAlarmTable = (zclGenAlarmItem_t *)NULL;
#endif // ZCL_ALARMS

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclGeneral_HdlIncoming( zclIncoming_t *pInMsg );
static ZStatus_t zclGeneral_HdlInSpecificCommands( zclIncoming_t *pInMsg );
static zclGeneral_AppCallbacks_t *zclGeneral_FindCallbacks( uint8_t endpoint );

// Device Configuration and Installation clusters
#ifdef ZCL_BASIC
static ZStatus_t zclGeneral_ProcessInBasic( zclIncoming_t *pInMsg, zclGeneral_AppCallbacks_t *pCBs );
#endif // ZCL_BASIC

#ifdef ZCL_IDENTIFY
static ZStatus_t zclGeneral_ProcessInIdentity( zclIncoming_t *pInMsg, zclGeneral_AppCallbacks_t *pCBs );
#endif // ZCL_IDENTIFY

// Groups and Scenes clusters
#ifdef ZCL_GROUPS
static ZStatus_t zclGeneral_ProcessInGroupsServer( zclIncoming_t *pInMsg );
static ZStatus_t zclGeneral_ProcessInGroupsClient( zclIncoming_t *pInMsg, zclGeneral_AppCallbacks_t *pCBs );
static ZStatus_t zclGeneral_AddGroup( uint8_t endPoint, aps_Group_t *group, uint8_t *pData );
#endif // ZCL_GROUPS

#ifdef ZCL_SCENES
static ZStatus_t zclGeneral_ProcessInScenesServer( zclIncoming_t *pInMsg, zclGeneral_AppCallbacks_t *pCBs );
static ZStatus_t zclGeneral_ProcessInScenesClient( zclIncoming_t *pInMsg, zclGeneral_AppCallbacks_t *pCBs );
#endif // ZCL_SCENES

// On/Off and Level Control Configuration clusters
#ifdef ZCL_ON_OFF
static ZStatus_t zclGeneral_ProcessInOnOff( zclIncoming_t *pInMsg, zclGeneral_AppCallbacks_t *pCBs );
#endif // ZCL_ONOFF

#ifdef ZCL_LEVEL_CTRL
static ZStatus_t zclGeneral_ProcessInLevelControl( zclIncoming_t *pInMsg, zclGeneral_AppCallbacks_t *pCBs );
#endif // ZCL_LEVEL_CTRL

// Alarms cluster
#ifdef ZCL_ALARMS
static ZStatus_t zclGeneral_ProcessInAlarmsServer( zclIncoming_t *pInMsg, zclGeneral_AppCallbacks_t *pCBs );
static ZStatus_t zclGeneral_ProcessInAlarmsClient( zclIncoming_t *pInMsg, zclGeneral_AppCallbacks_t *pCBs );
#endif // ZCL_ALARMS

// Location cluster
#ifdef ZCL_LOCATION
static ZStatus_t zclGeneral_ProcessInLocationServer( zclIncoming_t *pInMsg, zclGeneral_AppCallbacks_t *pCBs );
static ZStatus_t zclGeneral_ProcessInLocationClient( zclIncoming_t *pInMsg, zclGeneral_AppCallbacks_t *pCBs );
#endif // ZCL_LOCATION

#ifdef ZCL_SCENES
  #if !defined ( ZCL_STANDALONE )
    static uint8_t zclGeneral_ScenesInitNV( void );
    static void zclGeneral_ScenesSetDefaultNV( void );
    static void zclGeneral_ScenesWriteNV( void );
    static uint16_t zclGeneral_ScenesRestoreFromNV( void );
  #endif
#endif // ZCL_SCENES

/*********************************************************************
 * @fn      zclGeneral_RegisterCmdCallbacks
 *
 * @brief   Register an applications command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
ZStatus_t zclGeneral_RegisterCmdCallbacks( uint8_t endpoint, zclGeneral_AppCallbacks_t *callbacks )
{
  zclGenCBRec_t *pNewItem;
  zclGenCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( zclGenPluginRegisted == FALSE )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_GENERAL_BASIC,
                        ZCL_CLUSTER_ID_GENERAL_MULTISTATE_VALUE_BASIC,
                        zclGeneral_HdlIncoming );

#ifdef ZCL_SCENES
    // Initialize the Scenes Table
    zclGeneral_ScenesInit();
#endif // ZCL_SCENES

    zclGenPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclGenCBRec_t ) );
  if ( pNewItem == NULL )
    return (ZMemError);

  pNewItem->next = (zclGenCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if (  zclGenCBs == NULL )
  {
    zclGenCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclGenCBs;
    while ( pLoop->next != NULL )
      pLoop = pLoop->next;

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}

#ifdef ZCL_IDENTIFY
/*********************************************************************
 * @fn      zclGeneral_SendIdentify
 *
 * @brief   Call to send out an Identify Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   identifyTime - how long the device will continue to identify itself (in seconds)
 * @param   seqNum - identification number for the transaction
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendIdentify( uint8_t srcEP, afAddrType_t *dstAddr,
                                   uint16_t identifyTime, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[2];

  buf[0] = LO_UINT16( identifyTime );
  buf[1] = HI_UINT16( identifyTime );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
                          COMMAND_IDENTIFY_IDENTIFY, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, 2, buf );
}

/*********************************************************************
 * @fn      zclGeneral_SendIdentifyTriggerEffect
 *
 * @brief   Call to send out a Trigger Effect Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   effectId - identify effect to use
 * @param   effectVariant - which variant of effect to be triggered
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - identification number for the transaction
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendIdentifyTriggerEffect( uint8_t srcEP, afAddrType_t *dstAddr,
                                                uint8_t effectId, uint8_t effectVariant,
                                                uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[2];

  buf[0] = effectId;
  buf[1] = effectVariant;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
                          COMMAND_IDENTIFY_TRIGGER_EFFECT, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, 2, buf );
}


/*********************************************************************
 * @fn      zclGeneral_SendIdentifyQueryResponse
 *
 * @brief   Call to send out an Identify Query Response Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   timeout - how long the device will continue to identify itself (in seconds)
 * @param   seqNum - identification number for the transaction
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendIdentifyQueryResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                                uint16_t timeout, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[2];

  buf[0] = LO_UINT16( timeout );
  buf[1] = HI_UINT16( timeout );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
                          COMMAND_IDENTIFY_IDENTIFY_QUERY_RESPONSE, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, 2, buf );
}
#endif // ZCL_IDENTIFY

#ifdef ZCL_GROUPS
/*********************************************************************
 * @fn      zclGeneral_SendGroupRequest
 *
 * @brief   Send a Group Request to a device.  You can also use the
 *          appropriate macro.
 *
 * @param   srcEP - Sending Apps endpoint
 * @param   dstAddr - where to send the request
 * @param   cmd - one of the following:
 *              COMMAND_GROUPS_VIEW_GROUP
 *              COMMAND_GROUPS_REMOVE_GROUP
 * @param   groupID -
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendGroupRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                       uint8_t cmd, uint16_t groupID, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[2];

  buf[0] = LO_UINT16( groupID );
  buf[1] = HI_UINT16( groupID );

  return ( zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_GROUPS,
                            cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                            disableDefaultRsp, 0, seqNum, 2, buf ) );
}

/*********************************************************************
 * @fn      zclGeneral_SendAddGroupRequestEx
 *
 * @brief   Send the Add Group Request to a device
 *
 * @param   srcEP - Sending Apps endpoint
 * @param   dstAddr - where to send the request
 * @param   cmd - one of the following:
 *                COMMAND_GROUPS_ADD_GROUP
 *                COMMAND_GROUPS_ADD_GROUP_IF_IDENTIFYING
 * @param   groupID - pointer to the group structure
 * @param   groupName - pointer to Group Name.  This is a Zigbee
 *          string data type, so the first byte is the length of the
 *          name (in bytes), then the name.
 * @param   isReqFromApp - Indicates where the request comes from application thread or stack thread
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendAddGroupRequestEx( uint8_t srcEP, afAddrType_t *dstAddr,
                                          uint8_t cmd, uint16_t groupID, uint8_t *groupName,
                                          uint8_t disableDefaultRsp, uint8_t seqNum, uint8_t isReqFromApp )
{
  uint8_t *buf;
  uint8_t *pBuf;
  uint8_t len;
  ZStatus_t status;

  len = 2;    // Group ID
  len += groupName[0] + 1;  // String + 1 for length

  buf = zcl_mem_alloc( len );
  if ( buf )
  {
    pBuf = buf;
    *pBuf++ = LO_UINT16( groupID );
    *pBuf++ = HI_UINT16( groupID );
    *pBuf++ = groupName[0]; // string length
    zcl_memcpy( pBuf, &(groupName[1]), groupName[0] );

  if(isReqFromApp)
  {
    status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_GROUPS,
                              cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                              disableDefaultRsp, 0, seqNum, len, buf );
  }
  else
  {
    status = zcl_StackSendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_GROUPS,
                                cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                                disableDefaultRsp, 0, seqNum, len, buf );
  }

    zcl_mem_free( buf );
  }
  else
    status = ZMemError;

  return ( status );
}

/*********************************************************************
 * @fn      zclGeneral_SendGroupGetMembershipRequest
 *
 * @brief   Send a Get Group Membership (Resposne) Command to a device
 *
 * @param   srcEP - Sending Apps endpoint
 * @param   dstAddr - where to send the request
 * @param   cmd - one of the following:
 *                COMMAND_GROUPS_GET_GROUP_MEMBERSHIP
 *                COMMAND_GROUPS_GET_GROUP_MEMBERSHIP_RESPONSE
 * @param   groupID - pointer to the group structure
 * @param   groupName - pointer to Group Name.  This is a Zigbee
 *          string data type, so the first byte is the length of the
 *          name (in bytes), then the name.
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendGroupGetMembershipRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                                    uint8_t cmd, uint8_t rspCmd, uint8_t direction, uint8_t capacity,
                                                    uint8_t grpCnt, uint16_t *grpList, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t *buf;
  uint8_t *pBuf;
  uint8_t len = 0;
  uint8_t i;
  ZStatus_t status;

  if ( rspCmd )
    len++;  // Capacity

  len++;  // Group Count
  len += sizeof ( uint16_t ) * grpCnt;  // Group List

  buf = zcl_mem_alloc( len );
  if ( buf )
  {
    pBuf = buf;
    if ( rspCmd )
      *pBuf++ = capacity;

    *pBuf++ = grpCnt;
    for ( i = 0; i < grpCnt; i++ )
    {
      *pBuf++ = LO_UINT16( grpList[i] );
      *pBuf++ = HI_UINT16( grpList[i] );
    }

    status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_GROUPS,
                              cmd, TRUE, direction,
                              disableDefaultRsp, 0, seqNum, len, buf );
    zcl_mem_free( buf );
  }
  else
    status = ZMemError;

  return ( status );
}

/*********************************************************************
 * @fn      zclGeneral_SendGroupResponse
 *
 * @brief   Send Group Response (not Group View Response)
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - either COMMAND_GROUPS_ADD_GROUP_RESPONSE or COMMAND_GROUPS_REMOVE_GROUP_RESPONSE
 * @param   status - group command status
 * @param   groupID - what group
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendGroupResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                        uint8_t cmd, uint8_t status, uint16_t groupID,
                                        uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[3];

  buf[0] = status;
  buf[1] = LO_UINT16( groupID );
  buf[2] = HI_UINT16( groupID );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_GROUPS,
                          cmd, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                          disableDefaultRsp, 0, seqNum, 3, buf );
}

/*********************************************************************
 * @fn      zclGeneral_SendGroupViewResponse
 *
 * @brief   Call to send Group Response Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - either COMMAND_GROUPS_ADD_GROUP_RESPONSE or COMMAND_GROUPS_REMOVE_GROUP_RESPONSE
 * @param   status - group command status
 * @param   grp - group info
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendGroupViewResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                            uint8_t status, aps_Group_t *grp, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t *buf;
  uint8_t len;
  ZStatus_t stat;

  len = 1 + 2 + 1; // Status + Group ID + name length

  if ( status == ZCL_STATUS_SUCCESS )
  {
    len += grp->name[0];  // String length
  }

  buf = zcl_mem_alloc( len );
  if ( buf )
  {
    buf[0] = status;
    buf[1] = LO_UINT16( grp->ID );
    buf[2] = HI_UINT16( grp->ID );

    if ( status == ZCL_STATUS_SUCCESS )
    {
      buf[3] = grp->name[0]; // string length
      zcl_memcpy( &buf[4], (&grp->name[1]), grp->name[0] );
    }
    else //ZCL_STATUS_NOT_FOUND
    {
      buf[3] = 0;
    }

    stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_GROUPS,
                            COMMAND_GROUPS_VIEW_GROUP_RESPONSE, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                            disableDefaultRsp, 0, seqNum, len, buf );
    zcl_mem_free( buf );
  }
  else
  {
    stat = ZMemError;
  }

  return ( stat );
}
#endif // ZCL_GROUPS

#ifdef ZCL_SCENES
/*********************************************************************
 * @fn      zclGeneral_SendAddSceneRequest
 *
 * @brief   Send the (Enhanced) Add Scene Request to a device. You can
 *           also use the appropriate macro.
 *
 * @param   srcEP - Sending Apps endpoint
 * @param   dstAddr - where to send the request
 * @param   scene - pointer to the scene structure
 * @param  cmd - COMMAND_SCENES_ADD_SCENE or COMMAND_SCENES_ENHANCED_ADD_SCENE
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendAddSceneRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                          uint8_t cmd, zclGeneral_Scene_t *scene,
                                          uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t *buf;
  uint8_t *pBuf;
  uint8_t len;
  ZStatus_t status;

  len = 2 + 1 + 2;    // Group ID + Scene ID + transition time
  len += scene->name[0] + 1; // String + 1 for length

  // Add something for the extension field length
  len += scene->extLen;

  buf = zcl_mem_alloc( len );
  if ( buf )
  {
    pBuf = buf;
    *pBuf++ = LO_UINT16( scene->groupID );
    *pBuf++ = HI_UINT16( scene->groupID );
    *pBuf++ = scene->ID;
    *pBuf++ = LO_UINT16( scene->transTime );
    *pBuf++ = HI_UINT16( scene->transTime );
    *pBuf++ = scene->name[0]; // string length
    zcl_memcpy( pBuf, &(scene->name[1]), scene->name[0] );
    pBuf += scene->name[0]; // move pass name

    // Add the extension fields
    if ( scene->extLen > 0 )
      zcl_memcpy( pBuf, scene->extField, scene->extLen );

    status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_SCENES,
                              cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                              disableDefaultRsp, 0, seqNum, len, buf );
    zcl_mem_free( buf );
  }
  else
    status = ZMemError;

  return ( status );
}

/*********************************************************************
 * @fn      zclGeneral_SendSceneRequest
 *
 * @brief   Send a Scene Request to a device.  You can also use the
 *          appropriate macro.
 *
 * @param   srcEP - Sending Apps endpoint
 * @param   dstAddr - where to send the request
 * @param   cmd - one of the following:
 *              COMMAND_SCENES_VIEW_SCENE
 *              COMMAND_SCENES_REMOVE_SCENE
 *              COMMAND_SCENES_REMOVE_ALL_SCENES
 *              COMMAND_SCENES_STORE_SCENE
 *              COMMAND_SCENES_RECALL_SCENE
 *              COMMAND_SCENES_GET_SCENE_MEMBERSHIP
 *              COMMAND_SCENES_ENHANCED_VIEW_SCENE
 * @param   groupID - group ID
 * @param   sceneID - scene ID (not applicable to COMMAND_SCENES_REMOVE_ALL_SCENES and
 *                    COMMAND_SCENES_GET_SCENE_MEMBERSHIP)
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendSceneRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                       uint8_t cmd, uint16_t groupID, uint8_t sceneID,
                                       uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[3];
  uint8_t len = 2;

  buf[0] = LO_UINT16( groupID );
  buf[1] = HI_UINT16( groupID );

  if ( cmd != COMMAND_SCENES_REMOVE_ALL_SCENES && cmd != COMMAND_SCENES_GET_SCENE_MEMBERSHIP )
  {
    buf[2] = sceneID;
    len++;
  }

  return ( zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_SCENES,
                            cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                            disableDefaultRsp, 0, seqNum, len, buf ) );
}

/*********************************************************************
 * @fn      zclGeneral_SendSceneResponse
 *
 * @brief   Send Group Response (not Group View Response)
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - either COMMAND_SCENES_ADD_SCENE_RESPONSE, COMMAND_SCENES_REMOVE_SCENE_RESPONSE
 *                COMMAND_SCENES_STORE_SCENE_RESPONSE, or COMMAND_SCENES_REMOVE_ALL_SCENES_RESPONSE
 * @param   status - scene command status
 * @param   groupID - what group
 * @param   sceneID - what scene (not applicable to COMMAND_SCENES_REMOVE_ALL_SCENES_RESPONSE)
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendSceneResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                        uint8_t cmd, uint8_t status, uint16_t groupID,
                                        uint8_t sceneID, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[4];
  uint8_t len = 1 + 2; // Status + Group ID

  buf[0] = status;
  buf[1] = LO_UINT16( groupID );
  buf[2] = HI_UINT16( groupID );

  if ( cmd != COMMAND_SCENES_REMOVE_ALL_SCENES_RESPONSE )
  {
    buf[3] = sceneID;
    len++;
  }

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_SCENES,
                          cmd, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                          disableDefaultRsp, 0, seqNum, len, buf );
}

/*********************************************************************
 * @fn      zclGeneral_SendSceneViewResponse
 *
 * @brief   Call to send Scene (Enahced) View Response Command. You can
 *           also use the appropriate macro.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - either COMMAND_SCENES_VIEW_SCENE_RESPONSE or COMMAND_SCENES_ENHANCED_VIEW_SCENE_RESPONSE
 * @param   status - scene command status
 * @param   scene - scene info
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendSceneViewRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                       uint8_t cmd, uint8_t status, zclGeneral_Scene_t *scene,
                                       uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t *buf;
  uint8_t *pBuf;
  uint8_t len = 1 + 2 + 1; // Status + Group ID + Scene ID
  ZStatus_t stat;

  if ( status == ZCL_STATUS_SUCCESS )
  {
    len += 2; // Transition Time
    len += scene->name[0] + 1; // string + 1 for length

    // Add something for the extension field length
    len += scene->extLen;
  }

  buf = zcl_mem_alloc( len );
  if ( buf )
  {
    pBuf = buf;
    *pBuf++ = status;
    *pBuf++ = LO_UINT16( scene->groupID );
    *pBuf++ = HI_UINT16( scene->groupID );
    *pBuf++ = scene->ID;
    if ( status == ZCL_STATUS_SUCCESS )
    {
      uint16_t transTime = scene->transTime;
      if ( cmd == COMMAND_SCENES_ENHANCED_VIEW_SCENE_RESPONSE )
      {
        // Transition time is in 1/10s
        transTime *= 10;
        transTime += scene->transTime100ms;
      }

      *pBuf++ = LO_UINT16( transTime );
      *pBuf++ = HI_UINT16( transTime );
      *pBuf++ = scene->name[0]; // string length
      if ( scene->name[0] != 0 )
      {
        zcl_memcpy( pBuf, &(scene->name[1]), scene->name[0] );
        pBuf += scene->name[0]; // move pass name
      }

      // Add the extension fields
      if ( scene->extLen > 0 )
        zcl_memcpy( pBuf, scene->extField, scene->extLen );
    }

    stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_SCENES,
                            cmd, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                            disableDefaultRsp, 0, seqNum, len, buf );
    zcl_mem_free( buf );
  }
  else
    stat = ZMemError;

  return ( stat );
}

/*********************************************************************
 * @fn      zclGeneral_SendSceneGetMembershipResponse
 *
 * @brief   Call to send Scene Get Membership Response Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   status - scene command status
 * @param   capacity - remaining capacity of the scene table
 * @param   sceneCnt - number of scenes in the scene list
 * @param   sceneList - list of scene IDs
 * @param   groupID - group ID that scene belongs to
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendSceneGetMembershipResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                                     uint8_t status, uint8_t capacity, uint8_t sceneCnt, uint8_t *sceneList,
                                                     uint16_t groupID, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t *buf;
  uint8_t *pBuf;
  uint8_t len = 1 + 1 + 2; // Status + Capacity + Group ID;
  uint8_t i;
  ZStatus_t stat;

  if ( status == ZCL_STATUS_SUCCESS )
  {
    len++; // Scene Count
    len += sceneCnt; // Scene List (Scene ID is a single octet)
  }

  buf = zcl_mem_alloc( len );
  if ( buf )
  {
    pBuf = buf;
    *pBuf++ = status;
    *pBuf++ = capacity;
    *pBuf++ = LO_UINT16( groupID );
    *pBuf++ = HI_UINT16( groupID );
    if ( status == ZCL_STATUS_SUCCESS )
    {
      *pBuf++ = sceneCnt;
      for ( i = 0; i < sceneCnt; i++ )
        *pBuf++ = sceneList[i];
    }

    stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_SCENES,
                            COMMAND_SCENES_GET_SCENE_MEMBERSHIP_RESPONSE, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, len, buf );
    zcl_mem_free( buf );
  }
  else
    stat = ZMemError;

  return ( stat );
}

#ifdef ZCL_LIGHT_LINK_ENHANCE
/*********************************************************************
 * @fn      zclGeneral_SendSceneCopy
 *
 * @brief   Send Scene Copy Request to a device
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where to send the request
 * @param   mode - how scene copy is to proceed
 * @param   groupIDFrom - group from which scene to be copied
 * @param   sceneIDFrom - scene from which scene to be copied
 * @param   groupIDTo - group to which scene to be copied
 * @param   sceneIDTo - scene to which scene to be copied
 * @param   disableDefaultRsp - disable Default Response command
 * @param   seqNum - the identification number for the transaction
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendSceneCopy( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint8_t mode, uint16_t groupIDFrom, uint8_t sceneIDFrom,
                                    uint16_t groupIDTo, uint8_t sceneIDTo,
                                    uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[7];

  buf[0] = mode;
  buf[1] = LO_UINT16( groupIDFrom );
  buf[2] = HI_UINT16( groupIDFrom );
  buf[3] = sceneIDFrom;
  buf[4] = LO_UINT16( groupIDTo );
  buf[5] = HI_UINT16( groupIDTo );
  buf[6] = sceneIDTo;

  return ( zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_SCENES,
                            COMMAND_SCENES_COPY_SCENE, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                            disableDefaultRsp, 0, seqNum, 7, buf ) );
}

/*********************************************************************
 * @fn      zclGeneral_SendSceneCopyResponse
 *
 * @brief   Send Scene Copy Response to a device
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where to send the request
 * @param   status - status of copy scene attemp
 * @param   groupIDFrom - group from which scene was copied
 * @param   sceneIDFrom - scene from which scene was copied
 * @param   disableDefaultRsp - disable Default Response command
 * @param   seqNum - the identification number for the transaction
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendSceneCopyResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                            uint8_t status, uint16_t groupIDFrom, uint8_t sceneIDFrom,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[4];

  buf[0] = status;
  buf[1] = LO_UINT16( groupIDFrom );
  buf[2] = HI_UINT16( groupIDFrom );
  buf[3] = sceneIDFrom;

  return ( zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_SCENES,
                            COMMAND_SCENES_COPY_SCENE_RESPONSE, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                            disableDefaultRsp, 0, seqNum, 4, buf ) );
}
#endif // ZCL_LIGHT_LINK_ENHANCE
#endif // ZCL_SCENES

#ifdef ZCL_ON_OFF
#ifdef ZCL_LIGHT_LINK_ENHANCE
/*********************************************************************
 * @fn      zclGeneral_SendOnOff_CmdOffWithEffect
 *
 * @brief   Call to send out an Off with Effect Command.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   effectId - fading effect to use when switching light off
 * @param   effectVariant - which variant of effect to be triggered
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendOnOff_CmdOffWithEffect( uint8_t srcEP, afAddrType_t *dstAddr,
                                                 uint8_t effectId, uint8_t effectVariant,
                                                 uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[2];

  buf[0] = effectId;
  buf[1] = effectVariant;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_ON_OFF,
                          COMMAND_ON_OFF_OFF_WITH_EFFECT, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, 2, buf );
}

/*********************************************************************
 * @fn      zclGeneral_SendOnOff_CmdOnWithTimedOff
 *
 * @brief   Call to send out an On with Timed Off Command.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   onOffCtrl - how the lamp is to be operated
 * @param   onTime - the length of time (in 1/10ths second) that the lamp is to remain on, before automatically turning off
 * @param   offWaitTime - the length of time (in 1/10ths second) that the lamp shall remain off, and guarded to prevent an on command turning the light back on.
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendOnOff_CmdOnWithTimedOff ( uint8_t srcEP, afAddrType_t *dstAddr,
                                                   zclOnOffCtrl_t onOffCtrl, uint16_t onTime, uint16_t offWaitTime,
                                                   uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[5];

  buf[0] = onOffCtrl.byte;
  buf[1] = LO_UINT16( onTime );
  buf[2] = HI_UINT16( onTime );
  buf[3] = LO_UINT16( offWaitTime );
  buf[4] = HI_UINT16( offWaitTime );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_ON_OFF,
                          COMMAND_ON_OFF_ON_WITH_TIMED_OFF, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, 5, buf );
}
#endif // ZCL_LIGHT_LINK_ENHANCE
#endif // ZCL_ON_OFF

#ifdef ZCL_LEVEL_CTRL
/*********************************************************************
 * @fn      zclGeneral_SendLevelControlMoveToLevelRequest
 *
 * @brief   Call to send out a Level Control Request. You can also use
 *          the appropriate macro.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - one of the following:
 *              COMMAND_LEVEL_MOVE_TO_LEVEL or
 *              COMMAND_LEVEL_MOVE_TO_LEVEL_WITH_ON_OFF
 * @param   level - what level to move to
 * @param   transitionTime - how long to take to get to the level (in seconds)
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendLevelControlMoveToLevelRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                                         uint8_t cmd, uint8_t level, uint16_t transTime,
                                                         uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[3];

  buf[0] = level;
  buf[1] = LO_UINT16( transTime );
  buf[2] = HI_UINT16( transTime );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
                          cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, 3, buf );
}

/*********************************************************************
 * @fn      zclGeneral_SendLevelControlMoveRequest
 *
 * @brief   Call to send out a Level Control Request. You can also use
 *          the appropriate macro.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - one of the following:
 *              COMMAND_LEVEL_MOVE or
 *              COMMAND_LEVEL_MOVE_WITH_ON_OFF
 * @param   moveMode - LEVEL_MOVE_UP or
 *                     LEVEL_MOVE_DOWN
 * @param   rate - number of steps to take per second
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendLevelControlMoveRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                                  uint8_t cmd, uint8_t moveMode, uint8_t rate,
                                                  uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[2];

  buf[0] = moveMode;
  buf[1] = rate;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
                          cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, 2, buf );
}

/*********************************************************************
 * @fn      zclGeneral_SendLevelControlStepRequest
 *
 * @brief   Call to send out a Level Control Request. You can also use
 *          the appropriate macro.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - one of the following:
 *              COMMAND_LEVEL_STEP
 *              COMMAND_LEVEL_STEP_WITH_ON_OFF
 * @param   stepMode - LEVEL_STEP_UP or
 *                     LEVEL_STEP_DOWN
 * @param   amount - number of levels to step
 * @param   transitionTime - time, in 1/10ths of a second, to take to perform the step
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendLevelControlStepRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                                  uint8_t cmd, uint8_t stepMode, uint8_t stepSize, uint16_t transTime,
                                                  uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[4];

  buf[0] = stepMode;
  buf[1] = stepSize;
  buf[2] = LO_UINT16( transTime );
  buf[3] = HI_UINT16( transTime );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
                          cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, 4, buf );
}

/*********************************************************************
 * @fn      zclGeneral_SendLevelControlStepRequest
 *
 * @brief   Call to send out a Level Control Request. You can also use
 *          the appropriate macro.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - one of the following:
 *              COMMAND_LEVEL_STOP
 *              COMMAND_LEVEL_STOP_WITH_ON_OFF
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendLevelControlStopRequest( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t cmd,
                                                  uint8_t disableDefaultRsp, uint8_t seqNum )
{
  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
                          cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, 0, NULL );
}
#endif // ZCL_LEVEL_CTRL

#ifdef ZCL_ALARMS
/*********************************************************************
 * @fn      zclGeneral_SendAlarm
 *
 * @brief   Call to send out an Alarm Request Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - COMMAND_ALARMS_ALARM
 * @param   alarmCode - code for the cause of the alarm
 * @param   clusterID - cluster whose attribute generate the alarm
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendAlarm( uint8_t srcEP, afAddrType_t *dstAddr,
                                uint8_t alarmCode, uint16_t clusterID,
                                uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[3];

  buf[0] = alarmCode;
  buf[1] = LO_UINT16( clusterID );
  buf[2] = HI_UINT16( clusterID );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_ALARMS,
                          COMMAND_ALARMS_ALARM, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                          disableDefaultRsp, 0, seqNum, 3, buf );
}

/*********************************************************************
 * @fn      zclGeneral_SendAlarmReset
 *
 * @brief   Call to send out an Alarm Reset Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   alarmCode - code for the cause of the alarm
 * @param   clusterID - cluster whose attribute generate the alarm
 *
 * @return  ZStatus_t
*/
ZStatus_t zclGeneral_SendAlarmReset( uint8_t srcEP, afAddrType_t *dstAddr,
                                     uint8_t alarmCode, uint16_t clusterID,
                                     uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[3];

  buf[0] = alarmCode;
  buf[1] = LO_UINT16( clusterID );
  buf[2] = HI_UINT16( clusterID );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_ALARMS,
                          COMMAND_ALARMS_RESET_ALARM, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, 3, buf );
}

/*********************************************************************
 * @fn      zclGeneral_SendAlarmGetResponse
 *
 * @brief   Call to send out an Alarm Get Response Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   status - SUCCESS or NOT_FOUND
 * @param   alarmCode - code for the cause of the alarm
 * @param   clusterID - cluster whose attribute generate the alarm
 * @param   timeStamp - time at which the alarm occured
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendAlarmGetResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                           uint8_t status, uint8_t alarmCode, uint16_t clusterID,
                                           uint32_t timeStamp, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[8];
  uint8_t len = 1; // Status

  buf[0] = status;
  if ( status == ZCL_STATUS_SUCCESS )
  {
    len += 1 + 2 + 4; // Alarm code + Cluster ID + Time stamp
    buf[1] = alarmCode;
    buf[2] = LO_UINT16( clusterID );
    buf[3] = HI_UINT16( clusterID );
    zcl_buffer_uint32( &buf[4], timeStamp );
  }

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_ALARMS,
                          COMMAND_ALARMS_GET_ALARM_RESPONSE, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                          disableDefaultRsp, 0, seqNum, len, buf );
}

#ifdef SE_UK_EXT
/*********************************************************************
 * @fn      zclGeneral_SendAlarmGetEventLog
 *
 * @brief   Call to send out an Alarm Get Event Log Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pEventLog - pointer to Get Event Log Command
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendAlarmGetEventLog( uint8_t srcEP, afAddrType_t *dstAddr,
                                           zclGetEventLog_t *pEventLog,
                                           uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[10];

  buf[0] = pEventLog->logID;
  zcl_buffer_uint32( &buf[1], pEventLog->startTime );
  zcl_buffer_uint32( &buf[5], pEventLog->endTime );
  buf[9] = pEventLog->numEvents;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_ALARMS,
                          COMMAND_ALARMS_GET_EVENT_LOG, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                          disableDefaultRsp, 0, seqNum, 10, buf );
}

/*********************************************************************
 * @fn      zclGeneral_SendAlarmPublishEventLog
 *
 * @brief   Call to send out an Alarm Publish Event Log Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pEventLog - pointer to Publish Event Log Command
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendAlarmPublishEventLog( uint8_t srcEP, afAddrType_t *dstAddr,
                                               zclPublishEventLog_t *pEventLog,
                                               uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t *buf;
  uint8_t *pBuf;
  uint8_t bufLen;

  // Log ID + Command Index + Total Commands + (numSubLogs * ( Event ID + Event Time))
  bufLen = 1 + 1 + 1 + (pEventLog->numSubLogs * (1 + 4));

  buf = zcl_mem_alloc( bufLen );
  if ( buf == NULL )
  {
    return (ZMemError);
  }

  pBuf = buf;
  *pBuf++ = pEventLog->logID;
  *pBuf++ = pEventLog->cmdIndex;
  *pBuf++ = pEventLog->totalCmds;

  for ( uint8_t i = 0; i < pEventLog->numSubLogs; i++ )
  {
    zclEventLogPayload_t *pLogs = &(pEventLog->pLogs[i]);

    *pBuf++ = pLogs->eventId;
    pBuf = zcl_buffer_uint32( pBuf, pLogs->eventTime );
  }

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_ALARMS,
                          COMMAND_ALARMS_PUBLISH_EVENT_LOG, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, bufLen, buf );
}
#endif // SE_UK_EXT
#endif // ZCL_ALARMS

#ifdef ZCL_LOCATION
/*********************************************************************
 * @fn      zclGeneral_SendLocationSetAbsolute
 *
 * @brief   Call to send out a Set Absolute Location Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   absLoc - absolute location info
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendLocationSetAbsolute( uint8_t srcEP, afAddrType_t *dstAddr,
                                              zclLocationAbsolute_t *absLoc,
                                              uint8_t disableDefaultRsp, uint8_t seqNum )
{
   uint8_t buf[10]; // 5 fields (2 octects each)

   buf[0] = LO_UINT16( absLoc->coordinate1 );
   buf[1] = HI_UINT16( absLoc->coordinate1 );
   buf[2] = LO_UINT16( absLoc->coordinate2 );
   buf[3] = HI_UINT16( absLoc->coordinate2 );
   buf[4] = LO_UINT16( absLoc->coordinate3 );
   buf[5] = HI_UINT16( absLoc->coordinate3 );
   buf[6] = LO_UINT16( absLoc->power );
   buf[7] = HI_UINT16( absLoc->power );
   buf[8] = LO_UINT16( absLoc->pathLossExponent );
   buf[9] = HI_UINT16( absLoc->pathLossExponent );

   return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_LOCATION,
                           COMMAND_LOCATION_SET_ABSOLUTE, TRUE,
                           ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 10, buf );
}

/*********************************************************************
 * @fn      zclGeneral_SendLocationSetDevCfg
 *
 * @brief   Call to send out a Set Device Configuration Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   devCfg - device configuration info
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendLocationSetDevCfg( uint8_t srcEP, afAddrType_t *dstAddr,
                                            zclLocationDevCfg_t *devCfg,
                                            uint8_t disableDefaultRsp, uint8_t seqNum )
{
   uint8_t buf[9];  // 4 fields (2 octects each) + 1 field with 1 octect

   buf[0] = LO_UINT16( devCfg->power );
   buf[1] = HI_UINT16( devCfg->power );
   buf[2] = LO_UINT16( devCfg->pathLossExponent );
   buf[3] = HI_UINT16( devCfg->pathLossExponent );
   buf[4] = LO_UINT16( devCfg->calcPeriod );
   buf[5] = HI_UINT16( devCfg->calcPeriod );
   buf[6] = devCfg->numMeasurements;
   buf[7] = LO_UINT16( devCfg->reportPeriod );
   buf[8] = HI_UINT16( devCfg->reportPeriod );

   return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_LOCATION,
                           COMMAND_LOCATION_SET_DEV_CFG, TRUE,
                           ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 9, buf );
}

/*********************************************************************
 * @fn      zclGeneral_SendLocationGetDevCfg
 *
 * @brief   Call to send out a Get Device Configuration Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   targetAddr - device for which location parameters are being requested
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendLocationGetDevCfg( uint8_t srcEP, afAddrType_t *dstAddr,
                                            uint8_t *targetAddr, uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[8];

  zcl_memcpy( buf, targetAddr, 8 );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_LOCATION,
                          COMMAND_LOCATION_GET_DEV_CFG, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 8, buf );
}

/*********************************************************************
 * @fn      zclGeneral_SendLocationGetData
 *
 * @brief   Call to send out a Get Location Data Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   locaData - location information and channel parameters that are requested.
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendLocationGetData( uint8_t srcEP, afAddrType_t *dstAddr,
                                          zclLocationGetData_t *locData,
                                          uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[10]; // bitmap (1) + number responses (1) + IEEE Address (8)
  uint8_t *pBuf = buf;
  uint8_t len = 2; // bitmap + number responses

  *pBuf  = locData->absoluteOnly;
  *pBuf |= locData->recalculate << 1;
  *pBuf |= locData->brdcastIndicator << 2;
  *pBuf |= locData->brdcastResponse << 3;
  *pBuf |= locData->compactResponse << 4;
  pBuf++;  // move past the bitmap field

  *pBuf++ = locData->numResponses;

  if ( locData->brdcastIndicator == 0 )
  {
    zcl_memcpy( pBuf, locData->targetAddr, 8 );
    len += 8; // ieee addr
  }

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_LOCATION,
                          COMMAND_LOCATION_GET_DATA, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, len, buf );
}

/*********************************************************************
 * @fn      zclGeneral_SendLocationDevCfgResponse
 *
 * @brief   Call to send out a Device Configuration Response Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   devCfg - device's location parameters that are requested
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendLocationDevCfgResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                                 zclLocationDevCfgRsp_t *devCfg,
                                                 uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[10]; // 4 fields (2 octects each) + 2 fields (1 octect each)
  uint8_t len = 1; // Status

  buf[0] = devCfg->status;
  if ( devCfg->status == ZCL_STATUS_SUCCESS )
  {
    buf[1] = LO_UINT16( devCfg->data.power );
    buf[2] = HI_UINT16( devCfg->data.power );
    buf[3] = LO_UINT16( devCfg->data.pathLossExponent );
    buf[4] = HI_UINT16( devCfg->data.pathLossExponent );
    buf[5] = LO_UINT16( devCfg->data.calcPeriod );
    buf[6] = HI_UINT16( devCfg->data.calcPeriod );
    buf[7] = devCfg->data.numMeasurements;
    buf[8] = LO_UINT16( devCfg->data.reportPeriod );
    buf[9] = HI_UINT16( devCfg->data.reportPeriod );
    len += 9;
  }

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_LOCATION,
                          COMMAND_LOCATION_DEV_CFG_RSP, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, len, buf );
}

/*********************************************************************
 * @fn      zclGeneral_SendLocationData
 *
 * @brief   Call to send out location data
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   status - indicates whether response to request was successful or not
 * @param   locData - location information and channel parameters being sent

 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_SendLocationData( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t cmd,
                                       uint8_t status, zclLocationData_t *locData,
                                       uint8_t disableDefaultRsp, uint8_t seqNum )
{
  uint8_t buf[16];
  uint8_t *pBuf = buf;
  uint8_t len = 0;

  if ( cmd == COMMAND_LOCATION_DATA_RSP )
  {
    // Only response command includes a status field
    *pBuf++ = status;
    len++;
  }

  if ( cmd != COMMAND_LOCATION_DATA_RSP || status == ZCL_STATUS_SUCCESS )
  {
    // Notification or Response with successful status
    *pBuf++ = locData->type;
    *pBuf++ = LO_UINT16( locData->absLoc.coordinate1 );
    *pBuf++ = HI_UINT16( locData->absLoc.coordinate1 );
    *pBuf++ = LO_UINT16( locData->absLoc.coordinate2 );
    *pBuf++ = HI_UINT16( locData->absLoc.coordinate2 );
    len += 5;

    if ( locationType2D(locData->type) == 0 )
    {
      // 2D location doesn't have coordinate 3
      *pBuf++ = LO_UINT16( locData->absLoc.coordinate3 );
      *pBuf++ = HI_UINT16( locData->absLoc.coordinate3 );
      len += 2;
    }

    if ( cmd != COMMAND_LOCATION_COMPACT_DATA_NOTIF )
    {
      // Compact notification doesn't include these fields
      *pBuf++ = LO_UINT16( locData->absLoc.power );
      *pBuf++ = HI_UINT16( locData->absLoc.power );
      *pBuf++ = LO_UINT16( locData->absLoc.pathLossExponent );
      *pBuf++ = HI_UINT16( locData->absLoc.pathLossExponent );
      len += 4;
    }

    if ( locationTypeAbsolute(locData->type) == 0 )
    {
      // Absolute location doesn't include these fields
      if ( cmd != COMMAND_LOCATION_COMPACT_DATA_NOTIF )
      {
        // Compact notification doesn't include this field
        *pBuf++ = locData->calcLoc.locationMethod;
        len++;
      }

      *pBuf++ = locData->calcLoc.qualityMeasure;
      *pBuf++ = LO_UINT16( locData->calcLoc.locationAge );
      *pBuf++ = HI_UINT16( locData->calcLoc.locationAge );
      len += 3;
    }
  }

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GENERAL_LOCATION,
                          cmd, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                          disableDefaultRsp, 0, seqNum, len, buf );
}
#endif // ZCL_LOCATION

/*********************************************************************
 * @fn      zclGeneral_FindCallbacks
 *
 * @brief   Find the callbacks for an endpoint
 *
 * @param   endpoint - endpoint to find the application callbacks for
 *
 * @return  pointer to the callbacks
 */
static zclGeneral_AppCallbacks_t *zclGeneral_FindCallbacks( uint8_t endpoint )
{
  zclGenCBRec_t *pCBs;

  pCBs = zclGenCBs;
  while ( pCBs )
  {
    if ( pCBs->endpoint == endpoint )
      return ( pCBs->CBs );
    pCBs = pCBs->next;
  }
  return ( (zclGeneral_AppCallbacks_t *)NULL );
}

/*********************************************************************
 * @fn      zclGeneral_HdlIncoming
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
static ZStatus_t zclGeneral_HdlIncoming( zclIncoming_t *pInMsg )
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
      stat = zclGeneral_HdlInSpecificCommands( pInMsg );
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
 * @fn      zclGeneral_HdlInSpecificCommands
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library

 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGeneral_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t stat;
  zclGeneral_AppCallbacks_t *pCBs;

  // make sure endpoint exists
  pCBs = zclGeneral_FindCallbacks( pInMsg->msg->endPoint );
  if ( pCBs == NULL )
    return ( ZFailure );

  switch ( pInMsg->msg->clusterId )
  {
#ifdef ZCL_BASIC
    case ZCL_CLUSTER_ID_GENERAL_BASIC:
      stat = zclGeneral_ProcessInBasic( pInMsg, pCBs );
      break;
#endif // ZCL_BASIC

#ifdef ZCL_IDENTIFY
    case ZCL_CLUSTER_ID_GENERAL_IDENTIFY:
      stat = zclGeneral_ProcessInIdentity( pInMsg, pCBs );
      break;
#endif // ZCL_IDENTIFY

#ifdef ZCL_GROUPS
    case ZCL_CLUSTER_ID_GENERAL_GROUPS:
      if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
        stat = zclGeneral_ProcessInGroupsServer( pInMsg );
      else
        stat = zclGeneral_ProcessInGroupsClient( pInMsg, pCBs );
      break;
#endif // ZCL_GROUPS

#ifdef ZCL_SCENES
    case ZCL_CLUSTER_ID_GENERAL_SCENES:
      if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
        stat = zclGeneral_ProcessInScenesServer( pInMsg, pCBs );
      else
        stat = zclGeneral_ProcessInScenesClient( pInMsg, pCBs );
      break;
#endif // ZCL_SCENES

#ifdef ZCL_ON_OFF
    case ZCL_CLUSTER_ID_GENERAL_ON_OFF:
      stat = zclGeneral_ProcessInOnOff( pInMsg, pCBs );
      break;
#endif // ZCL_ON_OFF

#ifdef ZCL_LEVEL_CTRL
    case ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL:
      stat = zclGeneral_ProcessInLevelControl( pInMsg, pCBs );
      break;
#endif // ZCL_LEVEL_CTRL

#ifdef ZCL_ALARMS
    case ZCL_CLUSTER_ID_GENERAL_ALARMS:
      if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
        stat = zclGeneral_ProcessInAlarmsServer( pInMsg, pCBs );
      else
        stat = zclGeneral_ProcessInAlarmsClient( pInMsg, pCBs );
      break;
#endif // ZCL_ALARMS

#ifdef ZCL_LOCATION
    case ZCL_CLUSTER_ID_GENERAL_LOCATION:
      if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
        stat = zclGeneral_ProcessInLocationServer( pInMsg, pCBs );
      else
        stat = zclGeneral_ProcessInLocationClient( pInMsg, pCBs );
      break;
#endif // ZCL_LOCATION

    case ZCL_CLUSTER_ID_GENERAL_POWER_CFG:
    case ZCL_CLUSTER_ID_GENERAL_DEVICE_TEMP_CONFIG:
    case ZCL_CLUSTER_ID_GENERAL_ON_OFF_SWITCH_CONFIGURATION:
    case ZCL_CLUSTER_ID_GENERAL_TIME:
    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

#ifdef ZCL_BASIC
/*********************************************************************
 * @fn      zclGeneral_ProcessInBasic
 *
 * @brief   Process in the received Basic Command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGeneral_ProcessInBasic( zclIncoming_t *pInMsg,
                                            zclGeneral_AppCallbacks_t *pCBs )
{
  if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    if ( pInMsg->hdr.commandID > COMMAND_BASIC_RESET_TO_FACTORY_DEFAULTS )
      return ( ZFailure );   // Error ignore the command

    if ( pCBs->pfnBasicReset )
      pCBs->pfnBasicReset();
  }
  // no Client command

  return ( ZSuccess );
}
#endif // ZCL_BASIC

#ifdef ZCL_IDENTIFY
/*********************************************************************
 * @fn      zclGeneral_ProcessInIdentity
 *
 * @brief   Process in the received Identity Command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGeneral_ProcessInIdentity( zclIncoming_t *pInMsg,
                                               zclGeneral_AppCallbacks_t *pCBs )
{
  if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED == 1)

    if ( pInMsg->hdr.commandID == COMMAND_IDENTIFY_IDENTIFY )
    {
      uint16_t identifyTime;

      identifyTime = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );

      bdb_ZclIdentifyCmdInd( identifyTime, pInMsg->msg->endPoint);
    }
#endif
#ifdef ZCL_READ
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED == 1)
    else if ( pInMsg->hdr.commandID == COMMAND_IDENTIFY_IDENTIFY_QUERY )
#else
    if ( pInMsg->hdr.commandID == COMMAND_IDENTIFY_IDENTIFY_QUERY )
#endif
    {
      uint16_t identifyTime = 0;

      if(pCBs->pfnIdentifyQuery != NULL)
      {
        zclIdentifyQuery_t cmd;
        cmd.srcAddr = &(pInMsg->msg->srcAddr);
        pCBs->pfnIdentifyQuery(&cmd);
      }

      // Retrieve Identify Time
      zcl_ReadAttrData( pInMsg->msg->endPoint, pInMsg->msg->clusterId,
                        ATTRID_IDENTIFY_IDENTIFY_TIME, (uint8_t *)&identifyTime, NULL );

      // Is device identifying itself?
      if ( identifyTime > 0 )
      {
        zclGeneral_SendIdentifyQueryResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                              identifyTime, true, pInMsg->hdr.transSeqNum );
        return ( ZCL_STATUS_CMD_HAS_RSP );
      }
    }
#endif

#ifdef ZCL_LIGHT_LINK_ENHANCE
    else if ( pInMsg->hdr.commandID == COMMAND_IDENTIFY_TRIGGER_EFFECT )
    {
      if ( pCBs->pfnIdentifyTriggerEffect )
      {
        zclIdentifyTriggerEffect_t cmd;

        cmd.srcAddr = &(pInMsg->msg->srcAddr);
        cmd.effectId = pInMsg->pData[0];
        cmd.effectVariant = pInMsg->pData[1];

        pCBs->pfnIdentifyTriggerEffect( &cmd );
      }
    }
#endif //ZCL_LIGHT_LINK_ENHANCE
    else if ( pInMsg->hdr.commandID == COMMAND_IDENTIFY_IDENTIFY )
        {
          if(pCBs->pfnIdentify != NULL)
          {
            zclIdentify_t cmd;
            cmd.srcAddr = &(pInMsg->msg->srcAddr);
            cmd.identifyTime = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
            pCBs->pfnIdentify( &cmd );
          }
        }
    else
    {
      return ( ZFailure );   // Error ignore the command
    }
  }
  else // Client Command
  {
    if ( pInMsg->hdr.commandID > COMMAND_IDENTIFY_IDENTIFY_QUERY_RESPONSE )
      return ( ZFailure );   // Error ignore the command
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
    zclIdentifyQueryRsp_t rsp;

    rsp.srcAddr = &(pInMsg->msg->srcAddr);
    rsp.timeout = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );

    bdb_ZclIdentifyQueryCmdInd( &rsp );

#endif
    if ( pCBs->pfnIdentifyQueryRsp )
    {
      zclIdentifyQueryRsp_t rsp;

      rsp.srcAddr = &(pInMsg->msg->srcAddr);
      rsp.timeout = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );

      pCBs->pfnIdentifyQueryRsp( &rsp );
    }
  }
  return ( ZSuccess );
}
#endif // ZCL_IDENTIFY

#ifdef ZCL_GROUPS
/*********************************************************************
 * @fn      zclGeneral_AddGroup
 *
 * @brief   Add a Group.
 *
 * @param   endPoint - application endpoint
 * @param   group - group to be added
 * @param   pData - pointer to the group info
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGeneral_AddGroup( uint8_t endPoint, aps_Group_t *group, uint8_t *pData )
{
  uint8_t nameLen;
  uint8_t nameSupport = FALSE;

  pData += 2;   // Move past group ID
  nameLen = *pData++;
#ifdef ZCL_READ
  // Retrieve Name Support attribute
  zcl_ReadAttrData( endPoint, ZCL_CLUSTER_ID_GENERAL_GROUPS,
                    ATTRID_GROUPS_NAME_SUPPORT, &nameSupport, NULL );
#endif // ZCL_READ

  if ( nameSupport )
  {
    if ( nameLen > (APS_GROUP_NAME_LEN-1) )
       nameLen = (APS_GROUP_NAME_LEN-1);
    group->name[0] = nameLen;
    zcl_memcpy( &(group->name[1]), pData, nameLen );
  }

  return ( aps_AddGroup( endPoint, group, true ) );
}

/*********************************************************************
 * @fn      zclGeneral_ProcessInGroupsServer
 *
 * @brief   Process in the received Groups Command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGeneral_ProcessInGroupsServer( zclIncoming_t *pInMsg )
{
  aps_Group_t group;
  aps_Group_t *pGroup;
  uint8_t *pData;
  uint8_t status;
  uint8_t grpCnt;
  uint8_t grpRspCnt = 0;
  uint16_t *grpList;
  uint16_t identifyTime = 0;
  uint8_t i;
  ZStatus_t stat = ZSuccess;

  zcl_memset( (uint8_t*)&group, 0, sizeof( aps_Group_t ) );

  pData = pInMsg->pData;
  group.ID = BUILD_UINT16( pData[0], pData[1] );
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_GROUPS_ADD_GROUP:
      status = zclGeneral_AddGroup( pInMsg->msg->endPoint, &group, pData );
      if ( status != ZSuccess )
      {
        if ( status == ZApsDuplicateEntry )
        {
          status = ZCL_STATUS_DUPLICATE_EXISTS;
        }
        else
        {
          status = ZCL_STATUS_INSUFFICIENT_SPACE;
        }
      }

      //Only respond if unicast
      if ( UNICAST_MSG( pInMsg->msg ) )
      {
        zclGeneral_SendGroupAddResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                         status, group.ID, true, pInMsg->hdr.transSeqNum );
        stat = ZCL_STATUS_CMD_HAS_RSP;
      }
      break;

    case COMMAND_GROUPS_VIEW_GROUP:
      //Only respond if unicast
      if ( UNICAST_MSG( pInMsg->msg ) )
      {
        pGroup = aps_FindGroup( pInMsg->msg->endPoint, group.ID );
        if ( pGroup )
        {
          status = ZCL_STATUS_SUCCESS;
        }
        else
        {
          // Group not found
          status = ZCL_STATUS_NOT_FOUND;
          pGroup = &group;
        }
        zclGeneral_SendGroupViewResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                          status, pGroup, true, pInMsg->hdr.transSeqNum );
        stat = ZCL_STATUS_CMD_HAS_RSP;
      }
      break;

    case COMMAND_GROUPS_GET_GROUP_MEMBERSHIP:
      {
        grpCnt = *pData++;

        // Allocate space for the group list
        grpList = zcl_mem_alloc( sizeof( uint16_t ) * APS_MAX_GROUPS );
        if ( grpList != NULL )
        {
          if ( grpCnt == 0 )
          {
            // Find out all the groups of which the endpoint is a member.
            grpRspCnt = aps_FindAllGroupsForEndpoint( pInMsg->msg->endPoint, grpList );
          }
          else
          {
            // Find out the groups (in the list) of which the endpoint is a member.
            for ( i = 0; i < grpCnt; i++ )
            {
              group.ID = BUILD_UINT16( pData[0], pData[1] );
              pData += 2;

              if ( aps_FindGroup( pInMsg->msg->endPoint, group.ID ) )
                grpList[grpRspCnt++] = group.ID;
            }
          }

          if ( grpCnt == 0 ||  grpRspCnt != 0 )
          {
            zclGeneral_SendGroupGetMembershipResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                                       aps_GroupsRemaingCapacity(), grpRspCnt,
                                                       grpList, true, pInMsg->hdr.transSeqNum );
          }

          zcl_mem_free( grpList );
        }
        else
        {
          // Couldn't allocate space for the group list -- send a Default Response command back.
          zclDefaultRspCmd_t defaultRspCmd;

          defaultRspCmd.commandID = pInMsg->hdr.commandID;
          defaultRspCmd.statusCode = ZCL_STATUS_INSUFFICIENT_SPACE;
          zcl_SendDefaultRspCmd( pInMsg->msg->endPoint, &(pInMsg->msg->srcAddr),
                                 pInMsg->msg->clusterId, &defaultRspCmd,
                                 ZCL_FRAME_SERVER_CLIENT_DIR, true, 0, pInMsg->hdr.transSeqNum );
        }

        stat = ZCL_STATUS_CMD_HAS_RSP;
      }
      break;

    case COMMAND_GROUPS_REMOVE_GROUP:
#if defined ( ZCL_SCENES )
      zclGeneral_RemoveAllScenes( pInMsg->msg->endPoint, group.ID );
#endif
      if ( aps_RemoveGroup( pInMsg->msg->endPoint, group.ID ) )
      {
        status = ZCL_STATUS_SUCCESS;
      }
      else
      {
        status = ZCL_STATUS_NOT_FOUND;
      }

      //Only respond if unicast
      if ( UNICAST_MSG( pInMsg->msg ) )
      {
        zclGeneral_SendGroupRemoveResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                            status, group.ID, true, pInMsg->hdr.transSeqNum );
        stat = ZCL_STATUS_CMD_HAS_RSP;
      }
      break;

    case COMMAND_GROUPS_REMOVE_ALL_GROUPS:
      {
        uint8_t numGroups;
        uint16_t groupList[APS_MAX_GROUPS];

        if ( 0 != ( numGroups = aps_FindAllGroupsForEndpoint( pInMsg->msg->endPoint, groupList ) ) )
        {
          for ( i = 0; i < numGroups; i++ )
          {
#if defined ( ZCL_SCENES )
            zclGeneral_RemoveAllScenes( pInMsg->msg->endPoint, groupList[i] );
#endif
          }

          aps_RemoveAllGroup( pInMsg->msg->endPoint );
        }
      }
      break;

    case COMMAND_GROUPS_ADD_GROUP_IF_IDENTIFYING:
#ifdef ZCL_READ
      // Retrieve Identify Time
      zcl_ReadAttrData( pInMsg->msg->endPoint, ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
                        ATTRID_IDENTIFY_IDENTIFY_TIME, (uint8_t *)&identifyTime, NULL );
#endif

      // Is device identifying itself?
      if ( identifyTime > 0 )
      {
        zclGeneral_AddGroup( pInMsg->msg->endPoint, &group, pData );
      }
      break;

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclGeneral_ProcessInGroupsClient
 *
 * @brief   Process in the received Groups Command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGeneral_ProcessInGroupsClient( zclIncoming_t *pInMsg,
                                                   zclGeneral_AppCallbacks_t *pCBs )
{
  aps_Group_t group;
  uint8_t *pData = pInMsg->pData;
  uint8_t grpCnt;
  uint8_t nameLen;
  zclGroupRsp_t rsp;
  uint8_t i;
  ZStatus_t stat = ZSuccess;

  zcl_memset( (uint8_t*)&group, 0, sizeof( aps_Group_t ) );
  zcl_memset( (uint8_t*)&rsp, 0, sizeof( zclGroupRsp_t ) );

  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_GROUPS_ADD_GROUP_RESPONSE:
    case COMMAND_GROUPS_VIEW_GROUP_RESPONSE:
    case COMMAND_GROUPS_REMOVE_GROUP_RESPONSE:
      rsp.status = *pData++;
      group.ID = BUILD_UINT16( pData[0], pData[1] );

      if ( rsp.status == ZCL_STATUS_SUCCESS && pInMsg->hdr.commandID == COMMAND_GROUPS_VIEW_GROUP_RESPONSE )
      {
        pData += 2;   // Move past ID
        nameLen = *pData++;
        if ( nameLen > (APS_GROUP_NAME_LEN-1) )
          nameLen = (APS_GROUP_NAME_LEN-1);
        group.name[0] = nameLen;
        zcl_memcpy( &(group.name[1]), pData, nameLen );
        rsp.grpName = group.name;
      }

      if ( pCBs->pfnGroupRsp )
      {
        rsp.srcAddr = &(pInMsg->msg->srcAddr);
        rsp.cmdID = pInMsg->hdr.commandID;
        rsp.grpCnt = 1;
        rsp.grpList = &group.ID;
        rsp.capacity = 0;

        pCBs->pfnGroupRsp( &rsp );
      }
      break;

    case COMMAND_GROUPS_GET_GROUP_MEMBERSHIP_RESPONSE:
      {
        uint16_t *grpList = NULL;
        rsp.capacity = *pData++;
        grpCnt = *pData++;

        if ( grpCnt > 0 )
        {
          // Allocate space for the group list
          grpList = zcl_mem_alloc( sizeof( uint16_t ) * grpCnt );
          if ( grpList != NULL )
          {
            rsp.grpCnt = grpCnt;
            for ( i = 0; i < grpCnt; i++ )
            {
              grpList[i] = BUILD_UINT16( pData[0], pData[1] );
              pData += 2;
            }
          }
        }

        if ( pCBs->pfnGroupRsp )
        {
          rsp.srcAddr = &(pInMsg->msg->srcAddr);
          rsp.cmdID = pInMsg->hdr.commandID;
          rsp.grpList = grpList;

          pCBs->pfnGroupRsp( &rsp );
        }

        if ( grpList != NULL )
        {
          zcl_mem_free( grpList );
        }
      }
      break;

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}
#endif // ZCL_GROUPS

#if defined( ZCL_SCENES )
#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn      zclGeneral_AddScene
 *
 * @brief   Add a scene for an endpoint
 *
 * @param   endpoint -
 * @param   scene - new scene item
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_AddScene( uint8_t endpoint, zclGeneral_Scene_t *scene )
{
  zclGenSceneItem_t *pNewItem;
  zclGenSceneItem_t *pLoop;

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclGenSceneItem_t ) );
  if ( pNewItem == NULL )
    return ( ZMemError );

  // Fill in the plugin record.
  pNewItem->next = (zclGenSceneItem_t *)NULL;
  pNewItem->endpoint = endpoint;
  zcl_memcpy( (uint8_t*)&(pNewItem->scene), (uint8_t*)scene, sizeof ( zclGeneral_Scene_t ));

  // Find spot in list
  if (  zclGenSceneTable == NULL )
  {
    zclGenSceneTable = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclGenSceneTable;
    while ( pLoop->next != NULL )
      pLoop = pLoop->next;

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  // Update NV
  zclGeneral_ScenesWriteNV();

  return ( ZSuccess );
}
#endif // ZCL_STANDALONE

#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn      zclGeneral_FindScene
 *
 * @brief   Find a scene with endpoint and sceneID
 *
 * @param   endpoint -
 * @param   groupID - what group the scene belongs to
 * @param   sceneID - ID to look for scene
 *
 * @return  a pointer to the scene information, NULL if not found
 */
zclGeneral_Scene_t *zclGeneral_FindScene( uint8_t endpoint, uint16_t groupID, uint8_t sceneID )
{
  zclGenSceneItem_t *pLoop;

  // Look for end of list
  pLoop = zclGenSceneTable;
  while ( pLoop )
  {
    if ( (pLoop->endpoint == endpoint || endpoint == 0xFF)
        && pLoop->scene.groupID == groupID && pLoop->scene.ID == sceneID )
    {
      return ( &(pLoop->scene) );
    }
    pLoop = pLoop->next;
  }

  return ( (zclGeneral_Scene_t *)NULL );
}
#endif // ZCL_STANDALONE

#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn      zclGeneral_FindAllScenesForGroup
 *
 * @brief   Find all the scenes with groupID
 *
 * @param   endpoint - endpoint to look for
 * @param   sceneList - List to hold scene IDs (should hold APS_MAX_SCENES entries)
 *
 * @return  number of scenes copied to sceneList
 */
uint8_t zclGeneral_FindAllScenesForGroup( uint8_t endpoint, uint16_t groupID, uint8_t *sceneList )
{
  zclGenSceneItem_t *pLoop;
  uint8_t cnt = 0;

  // Look for end of list
  pLoop = zclGenSceneTable;
  while ( pLoop )
  {
    if ( pLoop->endpoint == endpoint && pLoop->scene.groupID == groupID )
      sceneList[cnt++] = pLoop->scene.ID;
    pLoop = pLoop->next;
  }
  return ( cnt );
}
#endif // ZCL_STANDALONE

#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn      zclGeneral_RemoveScene
 *
 * @brief   Remove a scene with endpoint and sceneID
 *
 * @param   endpoint -
 * @param   groupID - what group the scene belongs to
 * @param   sceneID - ID to look for scene
 *
 * @return  TRUE if removed, FALSE if not found
 */
uint8_t zclGeneral_RemoveScene( uint8_t endpoint, uint16_t groupID, uint8_t sceneID )
{
  zclGenSceneItem_t *pLoop;
  zclGenSceneItem_t *pPrev;

  // Look for end of list
  pLoop = zclGenSceneTable;
  pPrev = NULL;
  while ( pLoop )
  {
    if ( pLoop->endpoint == endpoint
        && pLoop->scene.groupID == groupID && pLoop->scene.ID == sceneID )
    {
      if ( pPrev == NULL )
        zclGenSceneTable = pLoop->next;
      else
        pPrev->next = pLoop->next;

      // Free the memory
      zcl_mem_free( pLoop );

      // Update NV
      zclGeneral_ScenesWriteNV();

      return ( TRUE );
    }
    pPrev = pLoop;
    pLoop = pLoop->next;
  }

  return ( FALSE );
}
#endif // ZCL_STANDALONE

#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn      zclGeneral_RemoveAllScenes
 *
 * @brief   Remove all scenes with endpoint and group Id
 *
 * @param   endpoint -
 * @param   groupID - ID to look for group
 *
 * @return  none
 */
void zclGeneral_RemoveAllScenes( uint8_t endpoint, uint16_t groupID )
{
  zclGenSceneItem_t *pLoop;
  zclGenSceneItem_t *pPrev;
  zclGenSceneItem_t *pNext;

  // Look for end of list
  pLoop = zclGenSceneTable;
  pPrev = NULL;
  while ( pLoop )
  {
    if ( pLoop->endpoint == endpoint && pLoop->scene.groupID == groupID )
    {
      if ( pPrev == NULL )
        zclGenSceneTable = pLoop->next;
      else
        pPrev->next = pLoop->next;
      pNext = pLoop->next;

      // Free the memory
      zcl_mem_free( pLoop );
      pLoop = pNext;
    }
    else
    {
      pPrev = pLoop;
      pLoop = pLoop->next;
    }
  }

  // Update NV
  zclGeneral_ScenesWriteNV();
}
#endif // ZCL_STANDALONE

#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn      zclGeneral_CountScenes
 *
 * @brief   Count the number of scenes for an endpoint
 *
 * @param   endpoint -
 *
 * @return  number of scenes assigned to an endpoint
 */
uint8_t zclGeneral_CountScenes( uint8_t endpoint )
{
  zclGenSceneItem_t *pLoop;
  uint8_t cnt = 0;

  // Look for end of list
  pLoop = zclGenSceneTable;
  while ( pLoop )
  {
    if ( pLoop->endpoint == endpoint  )
      cnt++;
    pLoop = pLoop->next;
  }
  return ( cnt );
}
#endif

#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn      zclGeneral_CountAllScenes
 *
 * @brief   Count the total number of scenes
 *
 * @param   none
 *
 * @return  number of scenes
 */
uint8_t zclGeneral_CountAllScenes( void )
{
  zclGenSceneItem_t *pLoop;
  uint8_t cnt = 0;

  // Look for end of list
  pLoop = zclGenSceneTable;
  while ( pLoop )
  {
    cnt++;
    pLoop = pLoop->next;
  }
  return ( cnt );
}
#endif // ZCL_STANDALONE

/*********************************************************************
 * @fn      zclGeneral_ReadSceneCountCB
 *
 * @brief   Read the number of scenes currently in the device's
 *          scene table (i.e., the Scene Count attribute).
 *
 *          Note: This function gets called only when the pointer
 *                'dataPtr' to the Scene Count attribute value is
 *                NULL in the attribute database registered with
 *                the ZCL.
 *
 * @param   clusterId - cluster that attribute belongs to
 * @param   attrId - attribute to be read or written
 * @param   oper - ZCL_OPER_LEN, ZCL_OPER_READ, or ZCL_OPER_WRITE
 * @param   pValue - pointer to attribute value
 * @param   pLen - pointer to length of attribute value read
 *
 * @return  status
 */
ZStatus_t zclGeneral_ReadSceneCountCB( uint16_t clusterId, uint16_t attrId,
                                       uint8_t oper, uint8_t *pValue, uint16_t *pLen )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;

  // This callback function should only be called for the Scene Count attribute
  switch ( oper )
  {
    case ZCL_OPER_LEN:
      *pLen = 1; // uint8_t
      break;

    case ZCL_OPER_READ:
      *pValue = zclGeneral_CountAllScenes();

      if ( pLen != NULL )
      {
        *pLen = 1;
      }
      break;

    case ZCL_OPER_WRITE:
      // Fall through

    default:
      status = ZCL_STATUS_SOFTWARE_FAILURE; // should never get here!
      break;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclGeneral_ProcessInScenesServer
 *
 * @brief   Process in the received Scenes Command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGeneral_ProcessInScenesServer( zclIncoming_t *pInMsg,
                                                   zclGeneral_AppCallbacks_t *pCBs )
{
  zclGeneral_Scene_t scene;
  zclGeneral_Scene_t *pScene;
  uint8_t *pData = pInMsg->pData;
  uint8_t nameLen;
  uint8_t status;
  uint8_t sceneCnt = 0;
  uint8_t *sceneList = NULL;
  uint8_t sendRsp = TRUE;
  uint8_t nameSupport = FALSE;
  ZStatus_t stat = ZSuccess;

  zcl_memset( (uint8_t*)&scene, 0, sizeof( zclGeneral_Scene_t ) );

  scene.groupID = BUILD_UINT16( pData[0], pData[1] );
  pData += 2;   // Move past group ID
  scene.ID = *pData++;

  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SCENES_ADD_SCENE:
#ifdef ZCL_LIGHT_LINK_ENHANCE
    case COMMAND_SCENES_ENHANCED_ADD_SCENE:
#endif // ZCL_LIGHT_LINK_ENHANCE
      // Parse the rest of the incoming message
      scene.transTime = BUILD_UINT16( pData[0], pData[1] );
      pData += 2;

      if ( pInMsg->hdr.commandID == COMMAND_SCENES_ENHANCED_ADD_SCENE )
      {
        // Received transition time is in 1/10 second
        scene.transTime100ms = scene.transTime % 10;
        scene.transTime /= 10;
      }

      nameLen= *pData++; // Name length
#ifdef ZCL_READ
      // Retrieve Name Support attribute
      zcl_ReadAttrData( pInMsg->msg->endPoint, ZCL_CLUSTER_ID_GENERAL_SCENES,
                        ATTRID_SCENES_NAME_SUPPORT, &nameSupport, NULL );
#endif // ZCL_READ
      if ( nameSupport )
      {
        if ( nameLen > (ZCL_GENERAL_SCENE_NAME_LEN-1) )
          nameLen = (ZCL_GENERAL_SCENE_NAME_LEN-1);
        scene.name[0] = nameLen;
        zcl_memcpy( &(scene.name[1]), pData, nameLen );
      }

      pData += nameLen; // move past name, use original length

      scene.extLen = pInMsg->pDataLen - ( (uint16_t)( pData - pInMsg->pData ) );
      if ( scene.extLen > 0 )
      {
        // Copy the extention field(s)
        if ( scene.extLen > ZCL_GENERAL_SCENE_EXT_LEN )
        {
          scene.extLen = ZCL_GENERAL_SCENE_EXT_LEN;
        }
        zcl_memset(scene.extField, 0xFF,ZCL_GENERAL_SCENE_EXT_LEN);
        zcl_memcpy( scene.extField, pData, scene.extLen );
      }

      if ( scene.groupID == 0x0000 ||
           aps_FindGroup( pInMsg->msg->endPoint, scene.groupID ) != NULL )
      {
        // Either the Scene doesn't belong to a Group (Group ID = 0x0000) or it
        // does and the corresponding Group exits
        pScene = zclGeneral_FindScene( pInMsg->msg->endPoint, scene.groupID, scene.ID );
        if ( pScene || ( zclGeneral_CountAllScenes() < ZCL_GENERAL_MAX_SCENES ) )
        {
          status = ZCL_STATUS_SUCCESS;
          if ( pScene != NULL )
          {
            // The Scene already exists so update it
            pScene->transTime = scene.transTime;

            if(nameSupport)
            {
                zcl_memcpy( pScene->name, scene.name, ZCL_GENERAL_SCENE_NAME_LEN );
            }

            // Use the new extention field(s)
            zcl_memcpy( pScene->extField, scene.extField, scene.extLen );
            pScene->extLen = scene.extLen;

            // Save Scenes
            zclGeneral_ScenesSave();
          }
          else
          {
            // The Scene doesn't exist so add it
            zclGeneral_AddScene( pInMsg->msg->endPoint, &scene );
          }
        }
        else
        {
          status = ZCL_STATUS_INSUFFICIENT_SPACE; // The Scene Table is full
        }
      }
      else
      {
        status = ZCL_STATUS_INVALID_FIELD; // The Group is not in the Group Table
      }

      if ( UNICAST_MSG( pInMsg->msg ) )
      {
        if ( pInMsg->hdr.commandID == COMMAND_SCENES_ADD_SCENE )
        {
          zclGeneral_SendSceneAddResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                          status, scene.groupID, scene.ID,
                                          true, pInMsg->hdr.transSeqNum );
        }
#ifdef ZCL_LIGHT_LINK_ENHANCE
        else // COMMAND_SCENES_ENHANCED_ADD_SCENE
        {
          zclGeneral_SendSceneEnhancedAddResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                                  status, scene.groupID, scene.ID,
                                                  true, pInMsg->hdr.transSeqNum );
        }
#endif // ZCL_LIGHT_LINK_ENHANCE
      }
      stat = ZCL_STATUS_CMD_HAS_RSP;

      break;

    case COMMAND_SCENES_VIEW_SCENE:
#ifdef ZCL_LIGHT_LINK_ENHANCE
    case COMMAND_SCENES_ENHANCED_VIEW_SCENE:
#endif // ZCL_LIGHT_LINK_ENHANCE
      pScene = zclGeneral_FindScene( pInMsg->msg->endPoint, scene.groupID, scene.ID );
      if ( pScene != NULL )
      {
        status = ZCL_STATUS_SUCCESS;
      }
      else
      {
        // Scene not found
        if ( scene.groupID != 0x0000 &&
             aps_FindGroup( pInMsg->msg->endPoint, scene.groupID ) == NULL )
        {
          status = ZCL_STATUS_INVALID_FIELD; // The Group is not in the Group Table
        }
        else
        {
          status = ZCL_STATUS_NOT_FOUND;
        }
        pScene = &scene;
      }

      if ( UNICAST_MSG( pInMsg->msg ) )
      {
        if ( pInMsg->hdr.commandID == COMMAND_SCENES_VIEW_SCENE )
        {
          zclGeneral_SendSceneViewResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                           status, pScene, true, pInMsg->hdr.transSeqNum );
        }
#ifdef ZCL_LIGHT_LINK_ENHANCE
        else
        {
          zclGeneral_SendSceneEnhancedViewResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                                   status, pScene, true, pInMsg->hdr.transSeqNum );
        }
#endif // ZCL_LIGHT_LINK_ENHANCE
      }
      stat = ZCL_STATUS_CMD_HAS_RSP;
      break;

    case COMMAND_SCENES_REMOVE_SCENE:
      if ( zclGeneral_RemoveScene( pInMsg->msg->endPoint, scene.groupID, scene.ID ) )
      {
        status = ZCL_STATUS_SUCCESS;
      }
      else
      {
        // Scene not found
        if ( aps_FindGroup( pInMsg->msg->endPoint, scene.groupID ) == NULL )
        {
          // The Group is not in the Group Table
          status = ZCL_STATUS_INVALID_FIELD;
        }
        else
        {
          status = ZCL_STATUS_NOT_FOUND;
        }
      }

      if ( UNICAST_MSG( pInMsg->msg ) )
      {
        // Addressed to this device (not to a group) - send a response back
        zclGeneral_SendSceneRemoveResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                            status, scene.groupID,
                                            scene.ID, true, pInMsg->hdr.transSeqNum );
      }
      stat = ZCL_STATUS_CMD_HAS_RSP;
      break;

    case COMMAND_SCENES_REMOVE_ALL_SCENES:
      if ( scene.groupID == 0x0000 ||
           aps_FindGroup( pInMsg->msg->endPoint, scene.groupID ) != NULL )
      {
        zclGeneral_RemoveAllScenes( pInMsg->msg->endPoint, scene.groupID );
        status = ZCL_STATUS_SUCCESS;
      }
      else
      {
        status = ZCL_STATUS_INVALID_FIELD;
      }

      if ( UNICAST_MSG( pInMsg->msg ) )
      {
        // Addressed to this device (not to a group) - send a response back
        zclGeneral_SendSceneRemoveAllResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                               status, scene.groupID, true, pInMsg->hdr.transSeqNum );
      }
      stat = ZCL_STATUS_CMD_HAS_RSP;
      break;

    case COMMAND_SCENES_STORE_SCENE:
      if ( scene.groupID == 0x0000 ||
           aps_FindGroup( pInMsg->msg->endPoint, scene.groupID ) != NULL )
      {
        // Either the Scene doesn't belong to a Group (Group ID = 0x0000) or it
        // does and the corresponding Group exits
        pScene = zclGeneral_FindScene( pInMsg->msg->endPoint, scene.groupID, scene.ID );
        if ( pScene || ( zclGeneral_CountAllScenes() < ZCL_GENERAL_MAX_SCENES ) )
        {
          uint8_t sceneChanged = FALSE;

          status = ZCL_STATUS_SUCCESS;
          if ( pScene == NULL )
          {
            // Haven't been added yet
            pScene = &scene;
          }

          if ( pCBs->pfnSceneStoreReq )
          {
            zclSceneReq_t req;

            req.srcAddr = &(pInMsg->msg->srcAddr);
            req.scene = pScene;

            // Get the latest Scene info
            if ( pCBs->pfnSceneStoreReq( &req ) )
            {
              sceneChanged = TRUE;
            }
          }

          if ( pScene == &scene )
          {
            // The Scene doesn't exist so add it
            zclGeneral_AddScene( pInMsg->msg->endPoint, &scene );
          }
          else if ( sceneChanged )
          {
            // The Scene already exists so update only NV
            zclGeneral_ScenesSave();
          }
        }
        else
        {
          status = ZCL_STATUS_INSUFFICIENT_SPACE; // The Scene Table is full
        }
      }
      else
      {
        status = ZCL_STATUS_INVALID_FIELD; // The Group is not in the Group Table
      }

      if ( UNICAST_MSG( pInMsg->msg ) )
      {
        // Addressed to this device (not to a group) - send a response back
        zclGeneral_SendSceneStoreResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                           status, scene.groupID, scene.ID,
                                           true, pInMsg->hdr.transSeqNum );
      }
      stat = ZCL_STATUS_CMD_HAS_RSP;
      break;

    case COMMAND_SCENES_RECALL_SCENE:
      pScene = zclGeneral_FindScene( pInMsg->msg->endPoint, scene.groupID, scene.ID );
      if ( pScene && pCBs->pfnSceneRecallReq )
      {
        zclSceneReq_t req;

        req.srcAddr = &(pInMsg->msg->srcAddr);
        req.scene = pScene;

        pCBs->pfnSceneRecallReq( &req );
      }
      // No response
      break;

    case COMMAND_SCENES_GET_SCENE_MEMBERSHIP:
      // Find all the Scenes corresponding to the Group ID
      if ( scene.groupID == 0x0000 ||
           aps_FindGroup( pInMsg->msg->endPoint, scene.groupID ) != NULL )
      {
        // Allocate space for the scene list
        sceneList = zcl_mem_alloc( ZCL_GENERAL_MAX_SCENES );
        if ( sceneList != NULL )
        {
          sceneCnt = zclGeneral_FindAllScenesForGroup( pInMsg->msg->endPoint,
                                                       scene.groupID, sceneList );
          status = ZCL_STATUS_SUCCESS;
          if ( ! UNICAST_MSG( pInMsg->msg ) )
          {
            // Addressed to the Group - ONLY send a response if an entry within the
            // Scene Table corresponds to the Group ID
            if ( sceneCnt == 0 )
            {
              sendRsp = FALSE;
            }
          }
        }
        else
        {
          // Couldn't allocate space for the scene list!
          status = ZCL_STATUS_INSUFFICIENT_SPACE;
        }
      }
      else
      {
        // The Group is not in the Group Table - send a response back
        status = ZCL_STATUS_INVALID_FIELD;
      }

#ifdef ZCL_LIGHT_LINK_ENHANCE //ZLL allows response to unicast only
      if ( sendRsp && UNICAST_MSG( pInMsg->msg ) )
#else
      if ( sendRsp )
#endif //ZCL_LIGHT_LINK_ENHANCE
      {
        zclGeneral_SendSceneGetMembershipResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                    status, zclGeneral_ScenesRemaingCapacity(), sceneCnt, sceneList,
                                    scene.groupID, true, pInMsg->hdr.transSeqNum );
      }

      if ( sceneList != NULL )
        zcl_mem_free( sceneList );

      stat = ZCL_STATUS_CMD_HAS_RSP;
      break;

#ifdef ZCL_LIGHT_LINK_ENHANCE
    case COMMAND_SCENES_COPY_SCENE:
      {
        uint8_t mode;
        uint16_t groupIDFrom, groupIDTo;
        uint8_t sceneIDFrom, sceneIDTo;

        pData = pInMsg->pData; // different payload format

        mode = *pData++;
        groupIDFrom = BUILD_UINT16( pData[0], pData[1] ); // from group ID
        pData += 2;
        sceneIDFrom = *pData++; // from scene ID
        groupIDTo = BUILD_UINT16( pData[0], pData[1] ); // to group ID
        pData += 2;
        if ( (mode & SCENE_COPY_MODE_ALL_BIT) == 0 )
        {
          sceneIDTo = *pData++; // to scene ID
        }

        // Make sure the groups exist
        if ( ( aps_FindGroup( pInMsg->msg->endPoint, groupIDFrom ) != NULL ) &&
             ( aps_FindGroup( pInMsg->msg->endPoint, groupIDTo ) != NULL ) )
        {
          // Allocate space for the scene list
          sceneList = zcl_mem_alloc( (mode & SCENE_COPY_MODE_ALL_BIT) ? ZCL_GENERAL_MAX_SCENES : 1 );
          if ( sceneList == NULL )
          {
            status = ZCL_STATUS_INSUFFICIENT_SPACE; // Couldn't allocate space for the scene list!
          }
          else
          {
            status = ZCL_STATUS_SUCCESS;
            if ( mode & SCENE_COPY_MODE_ALL_BIT ) // Copy all scenes
            {
              sceneCnt = zclGeneral_FindAllScenesForGroup( pInMsg->msg->endPoint,
                                                           groupIDFrom, sceneList );
            }
            else // Copy single scene
            {
              // Make sure the scene exists
              pScene = zclGeneral_FindScene( pInMsg->msg->endPoint, groupIDFrom, sceneIDFrom );
              if ( pScene != NULL )
              {
                sceneList[0] = sceneIDFrom;
                sceneCnt = 1;
              }
              else
              {
                status = ZCL_STATUS_INVALID_FIELD; // Scene not found
              }
            }
          }

          if ( status == ZCL_STATUS_SUCCESS )
          {
            uint8_t numScenesToAdd = 0;
            uint8_t i;
            for ( i = 0; i < sceneCnt; i++ )
            {
              if ( zclGeneral_FindScene( pInMsg->msg->endPoint, groupIDTo, sceneList[i] ) == NULL )
              {
                numScenesToAdd++;
              }
            }
            if ( zclGeneral_ScenesRemaingCapacity() >= numScenesToAdd )
            {
              // Copy the scenes
              for ( i = 0; i < sceneCnt; i++ )
              {
                // Ignore scene ID from and scene ID to fields
                pScene = zclGeneral_FindScene( pInMsg->msg->endPoint, groupIDFrom, sceneList[i] );
                if ( pScene != NULL )
                {
                  zclGeneral_Scene_t *pToScene;
                  scene = *pScene;
                  scene.groupID = groupIDTo;
                  scene.ID = ( (mode & SCENE_COPY_MODE_ALL_BIT) ? sceneList[i] : sceneIDTo );

                  pToScene = zclGeneral_FindScene( pInMsg->msg->endPoint, groupIDTo, scene.ID );
                  if( pToScene != NULL )
                  {
                    zclGeneral_RemoveScene( pInMsg->msg->endPoint, groupIDTo, scene.ID );
                  }
                  // Add the scene
                  zclGeneral_AddScene( pInMsg->msg->endPoint, &scene );
                }
              }
            }
            else
            {
              status = ZCL_STATUS_INSUFFICIENT_SPACE; // The Scene Table is full
            }
          }
        }
        else
        {
          status = ZCL_STATUS_INVALID_FIELD; // The Group is not in the Group Table
        }

        if ( UNICAST_MSG( pInMsg->msg ) )
        {
          if ( pScene == NULL )
          {
            pScene = &scene;
          }
          // Addressed to this device (not to a group) - send a response back
          zclGeneral_SendSceneCopyResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                            status, pScene->groupID, pScene->ID,
                                            true, pInMsg->hdr.transSeqNum );
        }

        if ( sceneList != NULL )
        {
          zcl_mem_free( sceneList );
        }
      }

      stat = ZCL_STATUS_CMD_HAS_RSP;
      break;
#endif // ZCL_LIGHT_LINK_ENHANCE

    default:
      stat = ZFailure;
    break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclGeneral_ProcessInScenesClient
 *
 * @brief   Process in the received Scenes Command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGeneral_ProcessInScenesClient( zclIncoming_t *pInMsg,
                                                   zclGeneral_AppCallbacks_t *pCBs )
{
  zclGeneral_Scene_t scene;
  uint8_t *pData = pInMsg->pData;
  uint8_t nameLen;
  zclSceneRsp_t rsp;
  uint8_t i;
  ZStatus_t stat = ZSuccess;

  zcl_memset( (uint8_t*)&scene, 0, sizeof( zclGeneral_Scene_t ) );
  zcl_memset( (uint8_t*)&rsp, 0, sizeof( zclSceneRsp_t ) );

  // Get the status field first
  rsp.status = *pData++;

  if ( pInMsg->hdr.commandID == COMMAND_SCENES_GET_SCENE_MEMBERSHIP_RESPONSE )
  {
    rsp.capacity = *pData++;
  }

  scene.groupID = BUILD_UINT16( pData[0], pData[1] );
  pData += 2;   // Move past group ID

  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SCENES_VIEW_SCENE_RESPONSE:
      // Parse the rest of the incoming message
      scene.ID = *pData++; // Not applicable to Remove All Response command
      scene.transTime = BUILD_UINT16( pData[0], pData[1] );
      pData += 2;
      nameLen = *pData++; // Name length
      if ( nameLen > (ZCL_GENERAL_SCENE_NAME_LEN-1) )
        nameLen = (ZCL_GENERAL_SCENE_NAME_LEN-1);

      scene.name[0] = nameLen;
      zcl_memcpy( &(scene.name[1]), pData, nameLen );

      pData += nameLen; // move past name, use original length

      //*** Do something with the extension field(s)

      // Fall through to callback - break is left off intentionally

    case COMMAND_SCENES_ADD_SCENE_RESPONSE:
    case COMMAND_SCENES_REMOVE_SCENE_RESPONSE:
    case COMMAND_SCENES_REMOVE_ALL_SCENES_RESPONSE:
    case COMMAND_SCENES_STORE_SCENE_RESPONSE:
#ifdef ZCL_LIGHT_LINK_ENHANCE
    case COMMAND_SCENES_ENHANCED_ADD_SCENE_RESPONSE:
    case COMMAND_SCENES_ENHANCED_VIEW_SCENE_RESPONSE:
    case COMMAND_SCENES_COPY_SCENE_RESPONSE:
#endif
      if ( pCBs->pfnSceneRsp )
      {
        if ( pInMsg->hdr.commandID != COMMAND_SCENES_REMOVE_ALL_SCENES_RESPONSE )
        {
          scene.ID = *pData++;
        }
        rsp.srcAddr = &(pInMsg->msg->srcAddr);
        rsp.cmdID = pInMsg->hdr.commandID;
        rsp.scene = &scene;

        pCBs->pfnSceneRsp( &rsp );
      }
      break;

    case COMMAND_SCENES_GET_SCENE_MEMBERSHIP_RESPONSE:
      {
        uint8_t *sceneList = NULL;

        if ( rsp.status == ZCL_STATUS_SUCCESS )
        {
          uint8_t sceneCnt = *pData++;

          if ( sceneCnt > 0 )
          {
            // Allocate space for the scene list
            sceneList = zcl_mem_alloc( sceneCnt );
            if ( sceneList != NULL )
            {
              rsp.sceneCnt = sceneCnt;
              for ( i = 0; i < sceneCnt; i++ )
                sceneList[i] = *pData++;
            }
          }
        }

        if ( pCBs->pfnSceneRsp )
        {
          rsp.srcAddr = &(pInMsg->msg->srcAddr);
          rsp.cmdID = pInMsg->hdr.commandID;
          rsp.sceneList = sceneList;
          rsp.scene = &scene;

          pCBs->pfnSceneRsp( &rsp);
        }

        if ( sceneList != NULL )
        {
          zcl_mem_free( sceneList );
        }
      }
      break;

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}
#endif // ZCL_SCENES

#ifdef ZCL_ON_OFF
/*********************************************************************
 * @fn      zclGeneral_ProcessInCmdOnOff
 *
 * @brief   Process in the received On/Off Command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGeneral_ProcessInOnOff( zclIncoming_t *pInMsg,
                                            zclGeneral_AppCallbacks_t *pCBs )
{
  ZStatus_t stat = ZSuccess;

  if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    switch ( pInMsg->hdr.commandID )
    {
      case COMMAND_ON_OFF_OFF:
      case COMMAND_ON_OFF_ON:
      case COMMAND_ON_OFF_TOGGLE:
        if ( pCBs->pfnOnOff )
        {
          pCBs->pfnOnOff( pInMsg->hdr.commandID );
        }
        break;

#ifdef ZCL_LIGHT_LINK_ENHANCE
      case COMMAND_ON_OFF_OFF_WITH_EFFECT:
        if ( pCBs->pfnOnOff_OffWithEffect )
        {
          zclOffWithEffect_t cmd;

          cmd.srcAddr = &(pInMsg->msg->srcAddr);
          cmd.effectId = pInMsg->pData[0];
          cmd.effectVariant = pInMsg->pData[1];

          pCBs->pfnOnOff_OffWithEffect( &cmd );
        }
        break;

      case COMMAND_ON_OFF_ON_WITH_RECALL_GLOBAL_SCENE:
        if ( pCBs->pfnOnOff_OnWithRecallGlobalScene )
        {
          pCBs->pfnOnOff_OnWithRecallGlobalScene();
        }
        break;

      case COMMAND_ON_OFF_ON_WITH_TIMED_OFF:
        if ( pCBs->pfnOnOff_OnWithTimedOff )
        {
          zclOnWithTimedOff_t cmd;

          cmd.onOffCtrl.byte = pInMsg->pData[0];
          cmd.onTime = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );
          cmd.offWaitTime = BUILD_UINT16( pInMsg->pData[3], pInMsg->pData[4] );
          pCBs->pfnOnOff_OnWithTimedOff( &cmd );
        }
        break;
#endif // ZCL_LIGHT_LINK_ENHANCE

      default:
        stat = ZFailure;
        break;
    }
  }
  // no Client command

  return ( stat );
}
#endif // ZCL_ON_OFF

#ifdef ZCL_LEVEL_CTRL
/*********************************************************************
 * @fn      zclGeneral_ProcessInLevelControl
 *
 * @brief   Process in the received Level Control Command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGeneral_ProcessInLevelControl( zclIncoming_t *pInMsg,
                                                   zclGeneral_AppCallbacks_t *pCBs )
{
  uint8_t withOnOff = FALSE;
  ZStatus_t stat = ZSuccess;

  if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    switch ( pInMsg->hdr.commandID )
    {
      case COMMAND_LEVEL_MOVE_TO_LEVEL_WITH_ON_OFF:
        withOnOff = TRUE;
        // fall through
      case COMMAND_LEVEL_MOVE_TO_LEVEL:
        if ( pCBs->pfnLevelControlMoveToLevel )
        {
          zclLCMoveToLevel_t cmd;

          cmd.level = pInMsg->pData[0];

          if ( ( cmd.level >= ATTR_LEVEL_MIN_LEVEL ) &&
               ( cmd.level <= ATTR_LEVEL_MAX_LEVEL ) )
          {
            cmd.transitionTime = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );
            cmd.withOnOff = withOnOff;

            pCBs->pfnLevelControlMoveToLevel( &cmd );
          }
          else
          {
            // level range requested is invalid
            stat = ZCL_STATUS_INVALID_VALUE;
          }
        }
        break;

      case COMMAND_LEVEL_MOVE_WITH_ON_OFF:
        withOnOff = TRUE;
        // fall through
      case COMMAND_LEVEL_MOVE:
        if ( pCBs->pfnLevelControlMove )
        {
          zclLCMove_t cmd;

          cmd.moveMode = pInMsg->pData[0];
          cmd.rate = pInMsg->pData[1];
          cmd.withOnOff = withOnOff;

          pCBs->pfnLevelControlMove( &cmd );
        }
        break;

      case COMMAND_LEVEL_STEP_WITH_ON_OFF:
        withOnOff = TRUE;
        // fall through
      case COMMAND_LEVEL_STEP:
        if ( pCBs->pfnLevelControlStep )
        {
          zclLCStep_t cmd;

          cmd.stepMode = pInMsg->pData[0];
          cmd.amount =  pInMsg->pData[1];
          cmd.transitionTime = BUILD_UINT16( pInMsg->pData[2], pInMsg->pData[3] );
          cmd.withOnOff = withOnOff;

          pCBs->pfnLevelControlStep( &cmd );
        }
        break;

      case COMMAND_LEVEL_STOP_WITH_ON_OFF:
          withOnOff = TRUE;
      case COMMAND_LEVEL_STOP:
        // fall through
        if ( pCBs->pfnLevelControlStop )
        {
            zclLCStop_t cmd;
            cmd.withOnOff = withOnOff;

            pCBs->pfnLevelControlStop( &cmd );
        }
        break;

      case COMMAND_LEVEL_MOVE_TO_CLOSEST_FREQUENCY:
          if ( pCBs->pfnLevelControlMoveFreq )
          {
              zclLCMoveFreq_t cmd;
              cmd.freq = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );

              pCBs->pfnLevelControlMoveFreq( &cmd );
          }
          break;

      default:
        stat = ZFailure;
        break;
    }
  }
  // no Client command

  return ( stat );
}
#endif // ZCL_LEVEL_CTRL

#ifdef ZCL_ALARMS
/*********************************************************************
 * @fn      zclGeneral_AddAlarm
 *
 * @brief   Add an alarm for a cluster
 *
 * @param   endpoint -
 * @param   alarm - new alarm item
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_AddAlarm( uint8_t endpoint, zclGeneral_Alarm_t *alarm )
{
  zclGenAlarmItem_t *pNewItem;
  zclGenAlarmItem_t *pLoop;

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclGenAlarmItem_t ) );
  if ( pNewItem == NULL )
    return ( ZMemError );

  // Fill in the plugin record.
  pNewItem->next = (zclGenAlarmItem_t *)NULL;
  pNewItem->endpoint =  endpoint;
  zcl_memcpy( (uint8_t*)(&pNewItem->alarm), (uint8_t*)alarm, sizeof ( zclGeneral_Alarm_t ) );

  // Find spot in list
  if (  zclGenAlarmTable == NULL )
  {
    zclGenAlarmTable = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclGenAlarmTable;
    while ( pLoop->next != NULL )
      pLoop = pLoop->next;

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclGeneral_FindAlarm
 *
 * @brief   Find an alarm with alarmCode and clusterID
 *
 * @param   endpoint -
 * @param   groupID - what group the scene belongs to
 * @param   sceneID - ID to look for scene
 *
 * @return  a pointer to the alarm information, NULL if not found
 */
zclGeneral_Alarm_t *zclGeneral_FindAlarm( uint8_t endpoint, uint8_t alarmCode, uint16_t clusterID )
{
  zclGenAlarmItem_t *pLoop;

  // Look for the alarm
  pLoop = zclGenAlarmTable;
  while ( pLoop )
  {
    if ( pLoop->endpoint == endpoint &&
         pLoop->alarm.code == alarmCode && pLoop->alarm.clusterID == clusterID )
    {
      return ( &(pLoop->alarm) );
    }
    pLoop = pLoop->next;
  }

  return ( (zclGeneral_Alarm_t *)NULL );
}

/*********************************************************************
 * @fn      zclGeneral_FindEarliestAlarm
 *
 * @brief   Find an alarm with the earliest timestamp
 *
 * @param   endpoint -
 *
 * @return  a pointer to the alarm information, NULL if not found
 */
zclGeneral_Alarm_t *zclGeneral_FindEarliestAlarm( uint8_t endpoint )
{
  zclGenAlarmItem_t *pLoop;
  zclGenAlarmItem_t earliestAlarm;
  zclGenAlarmItem_t *pEarliestAlarm = &earliestAlarm;

  pEarliestAlarm->alarm.timeStamp = 0xFFFFFFFF;

  // Look for alarm with earliest time
  pLoop = zclGenAlarmTable;
  while ( pLoop )
  {
    if ( pLoop->endpoint == endpoint &&
         pLoop->alarm.timeStamp < pEarliestAlarm->alarm.timeStamp )
    {
      pEarliestAlarm = pLoop;
    }
    pLoop = pLoop->next;
  }

  if ( pEarliestAlarm->alarm.timeStamp != 0xFFFFFFFF )
    return ( &(pEarliestAlarm->alarm) );

  // No alarm
  return ( (zclGeneral_Alarm_t *)NULL );
}

/*********************************************************************
 * @fn      zclGeneral_ResetAlarm
 *
 * @brief   Remove an alarm with alarmCode and clusterID
 *
 * @param   endpoint -
 * @param   alarmCode -
 * @param   clusterID -
 *
 * @return  TRUE if removed, FALSE if not found
 */
void zclGeneral_ResetAlarm( uint8_t endpoint, uint8_t alarmCode, uint16_t clusterID )
{
  zclGenAlarmItem_t *pLoop;
  zclGenAlarmItem_t *pPrev;

  // Look for end of list
  pLoop = zclGenAlarmTable;
  pPrev = NULL;
  while ( pLoop )
  {
    if ( pLoop->endpoint == endpoint &&
         pLoop->alarm.code == alarmCode && pLoop->alarm.clusterID == clusterID )
    {
      if ( pPrev == NULL )
        zclGenAlarmTable = pLoop->next;
      else
        pPrev->next = pLoop->next;

      // Free the memory
      zcl_mem_free( pLoop );

      // Notify the Application so that if the alarm condition still active then
      // a new notification will be generated, and a new alarm record will be
      // added to the alarm log
      // zclGeneral_NotifyReset( alarmCode, clusterID ); // callback function?
      return;
    }
    pPrev = pLoop;
    pLoop = pLoop->next;
  }
}

/*********************************************************************
 * @fn      zclGeneral_ResetAllAlarms
 *
 * @brief   Remove all alarms with endpoint
 *
 * @param   endpoint -
 * @param   notifyApp -
 *
 * @return  none
 */
void zclGeneral_ResetAllAlarms( uint8_t endpoint, uint8_t notifyApp )
{
  zclGenAlarmItem_t *pLoop;
  zclGenAlarmItem_t *pPrev;
  zclGenAlarmItem_t *pNext;

  // Look for end of list
  pLoop = zclGenAlarmTable;
  pPrev = NULL;
  while ( pLoop )
  {
    if (  pLoop->endpoint == endpoint )
    {
      if ( pPrev == NULL )
        zclGenAlarmTable = pLoop->next;
      else
        pPrev->next = pLoop->next;

      pNext = pLoop->next;

      // Free the memory
      zcl_mem_free( pLoop );

      pLoop = pNext;
    }
    else
    {
      pPrev = pLoop;
      pLoop = pLoop->next;
    }
  }

  if ( notifyApp )
  {
    // Notify the Application so that if any alarm conditions still active then
    // a new notification will be generated, and a new alarm record will be
    // added to the alarm log
    // zclGeneral_NotifyResetAll(); // callback function?
  }
}

/*********************************************************************
 * @fn      zclGeneral_ProcessInAlarmsServer
 *
 * @brief   Process in the received Alarms Command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGeneral_ProcessInAlarmsServer( zclIncoming_t *pInMsg,
                                                   zclGeneral_AppCallbacks_t *pCBs )
{
  zclAlarm_t alarm;
  zclGeneral_Alarm_t *pAlarm;
  uint8_t *pData = pInMsg->pData;
  ZStatus_t stat = ZSuccess;

  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_ALARMS_RESET_ALARM:
      if ( pCBs->pfnAlarm )
      {
        alarm.cmdID = pInMsg->hdr.commandID;
        alarm.alarmCode = pData[0];
        alarm.clusterID = BUILD_UINT16( pData[1], pData[2] );

        pCBs->pfnAlarm( pInMsg->hdr.fc.direction, &alarm );
      }
      else
      {
        stat = ZCL_STATUS_FAILURE;
      }
      break;

    case COMMAND_ALARMS_RESET_ALL_ALARMS:
      if ( pCBs->pfnAlarm )
      {
        alarm.cmdID = pInMsg->hdr.commandID;

        pCBs->pfnAlarm( pInMsg->hdr.fc.direction, &alarm );
      }
      else
      {
        stat = ZCL_STATUS_FAILURE;
      }
      break;

    case COMMAND_ALARMS_GET_ALARM:
      if ( pCBs->pfnAlarm )
      {
        alarm.srcAddr = &(pInMsg->msg->srcAddr);
        alarm.cmdID = pInMsg->hdr.commandID;

        pCBs->pfnAlarm( pInMsg->hdr.fc.direction, &alarm );

        pAlarm = zclGeneral_FindEarliestAlarm( pInMsg->msg->endPoint );
        if ( pAlarm )
        {
          // Send a response back
          zclGeneral_SendAlarmGetResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                           ZCL_STATUS_SUCCESS, pAlarm->code,
                                           pAlarm->clusterID, pAlarm->timeStamp,
                                           true, pInMsg->hdr.transSeqNum );
          // Remove the entry from the Alarm table
          zclGeneral_ResetAlarm( pInMsg->msg->endPoint, pAlarm->code, pAlarm->clusterID );
        }
        else
        {
          // Send a response back
          zclGeneral_SendAlarmGetResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                           ZCL_STATUS_NOT_FOUND, 0, 0, 0,
                                           true, pInMsg->hdr.transSeqNum );
        }
        stat = ZCL_STATUS_CMD_HAS_RSP;
      }
      else
      {
        stat = ZCL_STATUS_FAILURE;
      }
      break;

    case COMMAND_ALARMS_RESET_ALARM_LOG:
      if ( pCBs->pfnAlarm )
      {
        alarm.cmdID = pInMsg->hdr.commandID;

        pCBs->pfnAlarm( pInMsg->hdr.fc.direction, &alarm );

        zclGeneral_ResetAllAlarms( pInMsg->msg->endPoint, FALSE );
      }
      else
      {
        stat = ZCL_STATUS_FAILURE;
      }
      break;

#ifdef SE_UK_EXT
    case COMMAND_ALARMS_PUBLISH_EVENT_LOG:
      if ( pCBs->pfnPublishEventLog )
      {
        zclPublishEventLog_t eventLog;

        eventLog.logID = *pData++;
        eventLog.cmdIndex = *pData++;
        eventLog.totalCmds = *pData++;

        // First try to find out number of Sub Log Payloads
        eventLog.numSubLogs = (pInMsg->pDataLen-3)/(1+4); // event ID + event time
        if ( eventLog.numSubLogs > 0 )
        {
          // Try to alloc space for Log Payload
          eventLog.pLogs = (zclEventLogPayload_t *)zcl_mem_alloc( sizeof( zclEventLogPayload_t ) *
                                                                   eventLog.numSubLogs );
          if ( eventLog.pLogs != NULL )
          {
            // Copy Log Payload
            for ( uint8_t i = 0; i < eventLog.numSubLogs; i++ )
            {
              eventLog.pLogs[i].eventId = *pData++;
              eventLog.pLogs[i].eventTime = zcl_build_uint32( pData, 4 );
              pData += 4;
            }
          }
          else
          {
            stat = ZCL_STATUS_SOFTWARE_FAILURE;
          }
        }
        else
        {
          eventLog.pLogs = NULL;
        }

        if ( stat == ZSuccess )
        {
          pCBs->pfnPublishEventLog( &(pInMsg->msg->srcAddr), &eventLog );
        }

        if ( eventLog.pLogs != NULL )
        {
          zcl_mem_free( eventLog.pLogs );
        }
      }
      break;
#endif // SE_UK_EXT

    default:
      stat = ZCL_STATUS_UNSUP_CLUSTER_COMMAND;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclGeneral_ProcessInAlarmsClient
 *
 * @brief   Process in the received Alarms Command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGeneral_ProcessInAlarmsClient( zclIncoming_t *pInMsg,
                                                   zclGeneral_AppCallbacks_t *pCBs )
{
  uint8_t *pData = pInMsg->pData;
  zclAlarm_t alarm;
  ZStatus_t stat = ZSuccess;

  zcl_memset( (uint8_t*)&alarm, 0, sizeof( zclAlarm_t ) );

  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_ALARMS_ALARM:
      if ( pCBs->pfnAlarm )
      {
        alarm.srcAddr = &(pInMsg->msg->srcAddr);
        alarm.cmdID = pInMsg->hdr.commandID;
        alarm.alarmCode = pData[0];
        alarm.clusterID = BUILD_UINT16( pData[1], pData[2] );

        pCBs->pfnAlarm( pInMsg->hdr.fc.direction, &alarm );
      }
      else
      {
        stat = ZCL_STATUS_FAILURE;
      }
      break;

    case COMMAND_ALARMS_GET_ALARM_RESPONSE:
      if ( pCBs->pfnAlarm )
      {
        alarm.srcAddr = &(pInMsg->msg->srcAddr);
        alarm.cmdID = pInMsg->hdr.commandID;
        alarm.alarmCode = *pData++;
        alarm.clusterID = BUILD_UINT16( pData[0], pData[1] );

        pCBs->pfnAlarm( pInMsg->hdr.fc.direction, &alarm );
      }
      else
      {
        stat = ZCL_STATUS_FAILURE;
      }
      break;

#ifdef SE_UK_EXT
    case COMMAND_ALARMS_GET_EVENT_LOG:
      if ( pCBs->pfnGetEventLog )
      {
        zclGetEventLog_t eventLog;

        eventLog.logID = *pData++;
        eventLog.startTime = zcl_build_uint32( pData, 4 );
        pData += 4;
        eventLog.endTime = zcl_build_uint32( pData, 4 );
        pData += 4;
        eventLog.numEvents = *pData;

        pCBs->pfnGetEventLog( pInMsg->msg->endPoint, &(pInMsg->msg->srcAddr),
                              &eventLog, pInMsg->hdr.transSeqNum );
      }
      break;
#endif // SE_UK_EXT

    default:
      stat = ZCL_STATUS_UNSUP_CLUSTER_COMMAND;
      break;
  }

  return ( stat );
}
#endif // ZCL_ALARMS

#ifdef ZCL_LOCATION
/*********************************************************************
 * @fn      zclGeneral_ProcessInLocationServer
 *
 * @brief   Process in the received Location Command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGeneral_ProcessInLocationServer( zclIncoming_t *pInMsg,
                                                     zclGeneral_AppCallbacks_t *pCBs )
{
  uint8_t *pData = pInMsg->pData;
  zclLocation_t cmd;
  ZStatus_t stat = ZSuccess;

  zcl_memset( (uint8_t*)&cmd, 0, sizeof( zclLocation_t ) );

  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_LOCATION_SET_ABSOLUTE:
      cmd.un.absLoc.coordinate1 = BUILD_UINT16( pData[0], pData[1] );
      pData += 2;
      cmd.un.absLoc.coordinate2 = BUILD_UINT16( pData[0], pData[1] );
      pData += 2;
      cmd.un.absLoc.coordinate3 = BUILD_UINT16( pData[0], pData[1] );
      pData += 2;
      cmd.un.absLoc.power = BUILD_UINT16( pData[0], pData[1] );
      pData += 2;
      cmd.un.absLoc.pathLossExponent = BUILD_UINT16( pData[0], pData[1] );

      if ( pCBs->pfnLocation )
      {
        cmd.srcAddr = &(pInMsg->msg->srcAddr);
        cmd.cmdID = pInMsg->hdr.commandID;

        // Update the absolute location info
        pCBs->pfnLocation( &cmd );
      }
      break;

    case COMMAND_LOCATION_SET_DEV_CFG:
      cmd.un.devCfg.power = BUILD_UINT16( pData[0], pData[1] );
      pData += 2;
      cmd.un.devCfg.pathLossExponent = BUILD_UINT16( pData[0], pData[1] );
      pData += 2;
      cmd.un.devCfg.calcPeriod = BUILD_UINT16( pData[0], pData[1] );
      pData += 2;
      cmd.un.devCfg.numMeasurements = *pData++;
      cmd.un.devCfg.reportPeriod = BUILD_UINT16( pData[0], pData[1] );

      if ( pCBs->pfnLocation )
      {
        cmd.srcAddr = &(pInMsg->msg->srcAddr);
        cmd.cmdID = pInMsg->hdr.commandID;

        // Update the device configuration info
        pCBs->pfnLocation( &cmd );
      }
      break;

    case COMMAND_LOCATION_GET_DEV_CFG:
      cmd.un.ieeeAddr = pData;

      if ( pCBs->pfnLocation )
      {
        cmd.srcAddr = &(pInMsg->msg->srcAddr);
        cmd.cmdID = pInMsg->hdr.commandID;
        cmd.seqNum = pInMsg->hdr.transSeqNum;

        // Retreive the Device Configuration
        pCBs->pfnLocation( &cmd );
      }
      stat = ZCL_STATUS_CMD_HAS_RSP;
      break;

    case COMMAND_LOCATION_GET_DATA:
      cmd.un.loc.bitmap.locByte = *pData++;
      cmd.un.loc.numResponses = *pData++;

      if ( cmd.un.loc.brdcastResponse == 0 ) // command is sent as a unicast
        zcl_memcpy( cmd.un.loc.targetAddr, pData, 8 );

      if ( pCBs->pfnLocation )
      {
        cmd.srcAddr = &(pInMsg->msg->srcAddr);
        cmd.cmdID = pInMsg->hdr.commandID;
        cmd.seqNum = pInMsg->hdr.transSeqNum;

        // Retreive the Location Data
        pCBs->pfnLocation( &cmd );
      }
      stat = ZCL_STATUS_CMD_HAS_RSP;
      break;

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclGeneral_ProcessInLocationDataRsp
 *
 * @brief   Process in the received Location Command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static void zclGeneral_ProcessInLocationDataRsp( zclIncoming_t *pInMsg,
                                                 zclGeneral_AppCallbacks_t *pCBs )
{
  uint8_t *pData = pInMsg->pData;
  zclLocationRsp_t rsp;

  zcl_memset( (uint8_t*)&rsp, 0, sizeof( zclLocationRsp_t ) );

  if ( pCBs->pfnLocationRsp )
  {
    if ( pInMsg->hdr.commandID == COMMAND_LOCATION_DATA_RSP )
      rsp.un.loc.status = *pData++;

    if ( pInMsg->hdr.commandID != COMMAND_LOCATION_DATA_RSP ||
         rsp.un.loc.status == ZCL_STATUS_SUCCESS )
    {
      rsp.un.loc.data.type = *pData++;
      rsp.un.loc.data.absLoc.coordinate1 = BUILD_UINT16( pData[0], pData[1] );
      pData += 2;
      rsp.un.loc.data.absLoc.coordinate2 = BUILD_UINT16( pData[0], pData[1] );
      pData += 2;

      if ( locationType2D( rsp.un.loc.data.type ) == 0 )
      {
        rsp.un.loc.data.absLoc.coordinate3 = BUILD_UINT16( pData[0], pData[1] );
        pData += 2;
      }

      if ( pInMsg->hdr.commandID != COMMAND_LOCATION_COMPACT_DATA_NOTIF )
      {
        rsp.un.loc.data.absLoc.power = BUILD_UINT16( pData[0], pData[1] );
        pData += 2;
        rsp.un.loc.data.absLoc.pathLossExponent = BUILD_UINT16( pData[0], pData[1] );
        pData += 2;
      }

      if ( locationTypeAbsolute( rsp.un.loc.data.type ) == 0 )
      {
        if ( pInMsg->hdr.commandID != COMMAND_LOCATION_COMPACT_DATA_NOTIF )
          rsp.un.loc.data.calcLoc.locationMethod = *pData++;

        rsp.un.loc.data.calcLoc.qualityMeasure = *pData++;
        rsp.un.loc.data.calcLoc.locationAge = BUILD_UINT16( pData[0], pData[1] );
      }
    }

    rsp.srcAddr = &(pInMsg->msg->srcAddr);
    rsp.cmdID = pInMsg->hdr.commandID;

    // Notify the Application
    pCBs->pfnLocationRsp( &rsp );
  }
}

/*********************************************************************
 * @fn      zclGeneral_ProcessInLocationClient
 *
 * @brief   Process in the received Location Command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclGeneral_ProcessInLocationClient( zclIncoming_t *pInMsg,
                                                     zclGeneral_AppCallbacks_t *pCBs )
{
  uint8_t *pData = pInMsg->pData;
  zclLocationRsp_t rsp;
  ZStatus_t stat = ZSuccess;

  zcl_memset( (uint8_t*)&rsp, 0, sizeof( zclLocationRsp_t ) );

  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_LOCATION_DEV_CFG_RSP:
      if ( pCBs->pfnLocationRsp )
      {
        rsp.un.devCfg.status = *pData++;
        if ( rsp.un.devCfg.status == ZCL_STATUS_SUCCESS )
        {
          rsp.un.devCfg.data.power = BUILD_UINT16( pData[0], pData[1] );
          pData += 2;
          rsp.un.devCfg.data.pathLossExponent = BUILD_UINT16( pData[0], pData[1] );
          pData += 2;
          rsp.un.devCfg.data.calcPeriod = BUILD_UINT16( pData[0], pData[1] );
          pData += 2;
          rsp.un.devCfg.data.numMeasurements = *pData++;
          rsp.un.devCfg.data.reportPeriod = BUILD_UINT16( pData[0], pData[1] );

          rsp.srcAddr = &(pInMsg->msg->srcAddr);
          rsp.cmdID = pInMsg->hdr.commandID;

          // Notify the Application
          pCBs->pfnLocationRsp( &rsp );
        }
      }
      break;

    case COMMAND_LOCATION_DATA_RSP:
    case COMMAND_LOCATION_DATA_NOTIF:
    case COMMAND_LOCATION_COMPACT_DATA_NOTIF:
      zclGeneral_ProcessInLocationDataRsp( pInMsg, pCBs );
      break;

    case COMMAND_LOCATION_RSSI_PING:
      if ( pCBs->pfnLocationRsp )
      {
        rsp.un.locationType = *pData;

        rsp.srcAddr = &(pInMsg->msg->srcAddr);
        rsp.cmdID = pInMsg->hdr.commandID;

        // Notify the Application
        pCBs->pfnLocationRsp( &rsp );
      }
      break;

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}
#endif // ZCL_LOCATION

#ifdef ZCL_SCENES
#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn      zclGeneral_ScenesInitNV
 *
 * @brief   Initialize the NV Scene Table Items
 *
 * @param   none
 *
 * @return  number of scenes
 */
static uint8_t zclGeneral_ScenesInitNV( void )
{
  uint8_t  status;
  uint16_t size;

  size = (uint16_t)((sizeof ( nvGenScenesHdr_t ))
                  + ( sizeof( zclGenSceneNVItem_t ) * ZCL_GENERAL_MAX_SCENES ));

  status = zcl_nv_item_init( ZCD_NV_SCENE_TABLE, size, NULL );

  if ( status != ZSUCCESS )
  {
    zclGeneral_ScenesSetDefaultNV();
  }

  return status;
}
#endif // ZCL_STANDALONE

#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn          zclGeneral_ScenesSetDefaultNV
 *
 * @brief       Write the defaults to NV
 *
 * @param       none
 *
 * @return      none
 */
static void zclGeneral_ScenesSetDefaultNV( void )
{
  nvGenScenesHdr_t hdr;

  // Initialize the header
  hdr.numRecs = 0;

  // Save off the header
  zcl_nv_write( ZCD_NV_SCENE_TABLE, 0, sizeof( nvGenScenesHdr_t ), &hdr );
}
#endif // ZCL_STANDALONE

#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn          zclGeneral_ScenesWriteNV
 *
 * @brief       Save the Scene Table in NV
 *
 * @param       none
 *
 * @return      none
 */
static void zclGeneral_ScenesWriteNV( void )
{
  nvGenScenesHdr_t hdr;
  zclGenSceneItem_t *pLoop;
  zclGenSceneNVItem_t item;

  hdr.numRecs = 0;

  // Look for end of list
  pLoop = zclGenSceneTable;
  while ( pLoop )
  {
    // Build the record
    item.endpoint = pLoop->endpoint;
    zcl_memcpy( &(item.scene), &(pLoop->scene), sizeof ( zclGeneral_Scene_t ) );

    // Save the record to NV
    zcl_nv_write( ZCD_NV_SCENE_TABLE,
            (uint16_t)((sizeof( nvGenScenesHdr_t )) + (hdr.numRecs * sizeof ( zclGenSceneNVItem_t ))),
                    sizeof ( zclGenSceneNVItem_t ), &item );

    hdr.numRecs++;

    pLoop = pLoop->next;
  }

  // Save off the header
  zcl_nv_write( ZCD_NV_SCENE_TABLE, 0, sizeof( nvGenScenesHdr_t ), &hdr );
}
#endif // ZCL_STANDALONE

#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn          zclGeneral_ScenesRestoreFromNV
 *
 * @brief       Restore the Scene table from NV
 *
 * @param       none
 *
 * @return      Number of entries restored
 */
static uint16_t zclGeneral_ScenesRestoreFromNV( void )
{
  uint16_t x;
  nvGenScenesHdr_t hdr;

  zclGenSceneNVItem_t item;
  uint16_t numAdded = 0;

  if ( zcl_nv_read( ZCD_NV_SCENE_TABLE, 0, sizeof(nvGenScenesHdr_t), &hdr ) == ZSuccess )
  {
    // Read in the device list
    for ( x = 0; x < hdr.numRecs; x++ )
    {
      if ( zcl_nv_read( ZCD_NV_SCENE_TABLE,
                (uint16_t)(sizeof(nvGenScenesHdr_t) + (x * sizeof ( zclGenSceneNVItem_t ))),
                                  sizeof ( zclGenSceneNVItem_t ), &item ) == ZSUCCESS )
      {
        // Add the scene
        if ( zclGeneral_AddScene( item.endpoint, &(item.scene) ) == ZSuccess )
        {
          numAdded++;
        }
      }
    }
  }

  return ( numAdded );
}
#endif // ZCL_STANDALONE

#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn          zclGeneral_ScenesInit
 *
 * @brief       Initialize the scenes table
 *
 * @param       none
 *
 * @return      none
 */
void zclGeneral_ScenesInit( void )
{
  // Initialize NV items
  zclGeneral_ScenesInitNV();

  // Restore the Scene table
  zclGeneral_ScenesRestoreFromNV();
}
#endif // ZCL_STANDALONE

#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn          zclGeneral_ScenesSave
 *
 * @brief       Save the scenes table
 *
 * @param       none
 *
 * @return      none
 */
void zclGeneral_ScenesSave( void )
{
  // Update NV
  zclGeneral_ScenesWriteNV();
}
#endif // ZCL_STANDALONE

#endif // ZCL_SCENES

/***************************************************************************
****************************************************************************/
