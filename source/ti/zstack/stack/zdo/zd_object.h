/**************************************************************************************************
  Filename:       ZDObject.h
  Revised:        $Date: 2015-01-22 13:22:52 -0800 (Thu, 22 Jan 2015) $
  Revision:       $Revision: 41965 $

  Description:    This file contains the interface to the Zigbee Device Object.


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

#ifndef ZDOBJECT_H
#define ZDOBJECT_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "nl_mede.h"
#include "zd_app.h"

/*********************************************************************
 * CONSTANTS
 */
#define ZDO_MAX_LQI_ITEMS     2

#define ZDO_MAX_NWKDISC_ITEMS   5
#define ZDO_MAX_RTG_ITEMS       10
#define ZDO_MAX_BIND_ITEMS      3


//Current version of the stack indicated in NodeDesc
#define STACK_COMPLIANCE_CURRENT_REV_POS     9
#define STACK_COMPLIANCE_CURRENT_REV      0x16   //Rev 22

//Minimum stack revision to validate/request certain features. Ej. update TC Link key
#define STACK_COMPL_REV_21                0x15

/*********************************************************************
 * TYPEDEFS
 */
typedef enum
{
  MODE_JOIN,
  MODE_RESUME,
//MODE_SOFT,      // Not supported yet
  MODE_HARD,
  MODE_REJOIN
} devStartModes_t;

typedef struct
{
  uint8_t  status;
  uint16_t nwkAddr;
  uint8_t  extAddr[Z_EXTADDR_LEN];
  uint8_t  numAssocDevs;
  uint8_t  startIndex;
  uint16_t devList[];
} ZDO_NwkIEEEAddrResp_t;

typedef struct
{
  uint8_t status;
  uint16_t nwkAddr;   // Network address of interest
  NodeDescriptorFormat_t nodeDesc;
} ZDO_NodeDescRsp_t;

typedef struct
{
  uint8_t status;
  uint16_t nwkAddr;   // Network address of interest
  NodePowerDescriptorFormat_t pwrDesc;
} ZDO_PowerRsp_t;

typedef struct
{
  uint8_t  status;
  uint16_t nwkAddr;   // Network address of interest
  SimpleDescriptionFormat_t simpleDesc;
} ZDO_SimpleDescRsp_t;

typedef struct
{
  uint8_t  status;
  uint16_t nwkAddr;   // Network address of interest
  uint8_t  cnt;
  uint8_t  epList[];
} ZDO_ActiveEndpointRsp_t;

typedef ZDO_ActiveEndpointRsp_t ZDO_MatchDescRsp_t;

typedef struct
{
  uint8_t  status;
  uint8_t  networkCount;
  uint8_t  startIndex;
  uint8_t  networkListCount;
  mgmtNwkDiscItem_t list[];
} ZDO_MgmNwkDiscRsp_t;

typedef struct
{
  uint8_t  status;
  uint8_t  neighborLqiEntries;
  uint8_t  startIndex;
  uint8_t  neighborLqiCount;
  ZDP_MgmtLqiItem_t list[];
} ZDO_MgmtLqiRsp_t;

typedef struct
{
  uint8_t  status;
  uint8_t  rtgCount;
  uint8_t  startIndex;
  uint8_t  rtgListCount;
  rtgItem_t list[];
} ZDO_MgmtRtgRsp_t;

typedef struct
{
  uint8_t  status;
  uint8_t  bindingCount;
  uint8_t  startIndex;
  uint8_t  bindingListCount;
  apsBindingItem_t list[];
} ZDO_MgmtBindRsp_t;

typedef struct
{
  uint8_t  status;
  uint16_t nwkAddr;   // Address of interest
  uint8_t  length;
  uint8_t  desc[];
} ZDO_UserDescRsp_t;

typedef struct
{
  uint8_t  status;
  uint16_t serverMask;
} ZDO_ServerDiscRsp_t;

typedef struct
{
  uint16_t      nwkAddr;
  uint8_t       extAddr[Z_EXTADDR_LEN];
  uint8_t       capabilities;
} ZDO_DeviceAnnce_t;

typedef struct
{
  uint8_t  extAddr[Z_EXTADDR_LEN];
} ZDO_ChildInfo_t;

typedef struct ZDO_ChildInfoList_s
{
  ZDO_ChildInfo_t child;
  struct ZDO_ChildInfoList_s *next;
} ZDO_ChildInfoList_t;

typedef struct
{
  uint8_t           numOfChildren;
  ZDO_ChildInfo_t childInfo[];
} ZDO_ParentAnnce_t;

typedef struct
{
  uint32_t channelMask;
  uint8_t scanDuration;
  uint8_t scanCount;
  uint8_t nwkUpdateId;
  int16_t nwkManagerAddr;
} ZDO_MgmtNwkUpdateReq_t;

typedef struct
{
  uint8_t status;
  uint32_t scannedChannels;
  uint16_t totalTransmissions;
  uint16_t transmissionFailures;
  uint8_t  listCount;
  uint8_t  energyValues[];
} ZDO_MgmtNwkUpdateNotify_t;

/*********************************************************************
 * Internal ZDO types
 */

enum
{
  ZDMATCH_REASON_START,
  ZDMATCH_REASON_TIMEOUT,
  ZDMATCH_REASON_UNBIND_RSP,
  ZDMATCH_REASON_BIND_RSP
};

typedef struct
{
  ZDEndDeviceBind_t ed1;
  ZDEndDeviceBind_t ed2;
  uint8_t  state;            // One of the above states
  uint8_t  sending;         // 0 - not sent, 1 - unbind, 2 bind - expecting response
  uint8_t  transSeq;
  uint8_t  ed1numMatched;
  uint16_t *ed1Matched;
  uint8_t  ed2numMatched;
  uint16_t *ed2Matched;
} ZDMatchEndDeviceBind_t;

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern ZDMatchEndDeviceBind_t *matchED;

extern uint8_t  zdoBindUnbindAuthAddr[Z_EXTADDR_LEN];
extern uint16_t zdoBindUnbindAuthClusterId;
extern uint8_t  zdoBindUnbindAuthEndpoint;
/*********************************************************************
 * FUNCTIONS - API
 */

/*
 * ZDO_Init - ZDObject and ZDApp Initialization.
 */
extern void ZDO_Init( void );


/*
 * ZDO_StartDevice - Start the device in a network.
 */
extern void ZDO_StartDevice( byte logicalType, devStartModes_t startMode,
                             byte beaconOrder, byte superframeOrder );

/*
 * ZDO_UpdateNwkStatus - Update nwk state in the apps
 */
extern void ZDO_UpdateNwkStatus( devStates_t state );

/*
 * ZDO_MatchEndDeviceBind - Match End Device Bind Requests
 */
extern void ZDO_MatchEndDeviceBind( ZDEndDeviceBind_t *bindReq );

/*********************************************************************
 * Call Back Functions from ZDProfile  - API
 */

extern byte ZDO_AnyClusterMatches(
                              byte ACnt, uint16_t *AList, byte BCnt, uint16_t *BList );

/*
 * ZDO_ProcessNodeDescRsp - Process the Node_Desc_rsp message.
 */
extern void ZDO_ProcessNodeDescRsp( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ProcessNodeDescReq - Process the Node_Desc_req message.
 */
extern void ZDO_ProcessNodeDescReq( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ProcessPowerDescReq - Process the Power_Desc_req message.
 */
extern void ZDO_ProcessPowerDescReq( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ProcessSimpleDescReq - Process the Simple_Desc_req message
 */
extern void ZDO_ProcessSimpleDescReq( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ProcessSimpleDescRsp - Process the Simple_Desc_rsp message
 */
extern void ZDO_ProcessSimpleDescRsp( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ProcessActiveEPReq - Process the Active_EP_req message
 */
extern void ZDO_ProcessActiveEPReq( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ProcessMatchDescReq - Process the Match_Desc_req message
 */
extern void ZDO_ProcessMatchDescReq( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ProcessServerDiscRsp - Process the Server_Discovery_rsp message.
 */
void ZDO_ProcessServerDiscRsp( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ProcessServerDiscReq - Process the Server_Discovery_req message.
 */
void ZDO_ProcessServerDiscReq( zdoIncomingMsg_t *inMsg );

/*********************************************************************
 * Internal ZDO interfaces
 */

extern uint8_t ZDMatchSendState( uint8_t reason, uint8_t status, uint8_t TransSeq );

/*********************************************************************
 * Call Back Functions from APS  - API
 */

/*
 * ZDO_EndDeviceTimeoutCB - Called when the binding timer expires
 */
extern void ZDO_EndDeviceTimeoutCB( void );

/*********************************************************************
 * Optional Management Messages
 */

/*
 * ZDO_ProcessMgmtNwkDiscReq - Called to parse the incoming
 * Management Network Discover Response
 */
extern void ZDO_ProcessMgmNwkDiscRsp( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ProcessMgmtNwkDiscReq - Called to parse the incoming
 * Management LQI Request
 */
extern void ZDO_ProcessMgmtNwkDiscReq( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_FinishProcessingMgmtNwkDiscReq - Called to parse the incoming
 * Management LQI Request
 */
extern void ZDO_FinishProcessingMgmtNwkDiscReq( void );
/*
 * ZDO_ProcessMgmtNwkUpdateReq - Parse the Mgmt_NWK_Update_req message
 *
 * returns none
 */
extern void ZDO_ParseMgmtNwkUpdateReq( zdoIncomingMsg_t *inMsg, ZDO_MgmtNwkUpdateReq_t *pReq );

/*
 * ZDO_ProcessMgmtLqiReq - Called to parse the incoming
 * Management LQI Request
 */
extern void ZDO_ProcessMgmtLqiReq( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ProcessMgmtRtgReq - Called to parse the incoming
 * Management Routing Request
 */
extern void ZDO_ProcessMgmtRtgReq( zdoIncomingMsg_t *inMsg );

extern void ZDO_ProcessMgmtBindReq( zdoIncomingMsg_t *inMsg );

extern void ZDO_ProcessMgmtBindRsp( zdoIncomingMsg_t *inMsg );

extern void ZDO_ProcessMgmtDirectJoinReq( zdoIncomingMsg_t *inMsg );

extern void ZDO_ProcessMgmtLeaveReq( zdoIncomingMsg_t *inMsg );

extern void ZDO_ProcessMgmtPermitJoinReq( zdoIncomingMsg_t *inMsg );

extern void ZDO_ProcessUserDescReq( zdoIncomingMsg_t *inMsg );

extern void ZDO_ProcessUserDescSet( zdoIncomingMsg_t *inMsg );

extern void ZDO_ProcessDeviceAnnce( zdoIncomingMsg_t *inMsg );

extern void ZDO_ProcessParentAnnce( zdoIncomingMsg_t *inMsg );

extern void ZDO_ProcessParentAnnceRsp( zdoIncomingMsg_t *inMsg );

extern void ZDO_BuildSimpleDescBuf( uint8_t *buf, SimpleDescriptionFormat_t *desc );

extern uint8_t ZDO_ParseSimpleDescBuf( uint8_t *buf, SimpleDescriptionFormat_t *desc );

extern void ZDO_UpdateAddrManager( uint16_t nwkAddr, uint8_t *extAddr );

/*
 * ZDO_ParseAddrRsp - Parse the NWK_addr_rsp and IEEE_addr_rsp messages
 *
 * returns a pointer to parsed structures.  This structure was
 *          allocated using OsalPort_malloc, so it must be freed
 *          by the calling function [OsalPort_free()].
 */
extern ZDO_NwkIEEEAddrResp_t *ZDO_ParseAddrRsp( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ParseNodeDescRsp - Parse the Node_Desc_rsp message
 */
extern void ZDO_ParseNodeDescRsp( zdoIncomingMsg_t *inMsg, ZDO_NodeDescRsp_t *pNDRsp );

/*
 * ZDO_ParsePowerDescRsp - Parse the Power_Desc_rsp message
 */
extern void ZDO_ParsePowerDescRsp( zdoIncomingMsg_t *inMsg, ZDO_PowerRsp_t *pNPRsp );

/*
 * ZDO_ParseSimpleDescRsp - Parse the Simple_Desc_rsp message
 *   NOTE: The pAppInClusterList and pAppOutClusterList fields
 *         in the SimpleDescriptionFormat_t structure are allocated
 *         and the calling function needs to free [osal_msg_free()]
 *         these buffers.
 */
extern void ZDO_ParseSimpleDescRsp( zdoIncomingMsg_t *inMsg, ZDO_SimpleDescRsp_t *pSimpleDescRsp );

/*
 * ZDO_ParseEPListRsp - Parse the Active_EP_rsp or Match_Desc_rsp message
 *
 * returns a pointer to parsed structures.  This structure was
 *          allocated using OsalPort_malloc, so it must be freed
 *          by the calling function [OsalPort_free()].
 */
extern ZDO_ActiveEndpointRsp_t *ZDO_ParseEPListRsp( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ParseBindRsp - Parse the Bind_rsp, Unbind_rsp or End_Device_Bind_rsp message.
 *     Parameter: a - pointer to the message to parse [zdoIncomingMsg_t *]
 *
 *  returns the status field of the message.
 */
#define ZDO_ParseBindRsp(a) ((uint8_t)(*(a->asdu)))

/*
 * ZDO_ParseMgmNwkDiscRsp - Parse the Mgmt_NWK_Disc_rsp message
 *
 * returns a pointer to parsed response structure (NULL if not allocated).
 *          This structure was allocated using OsalPort_malloc, so it must be freed
 *          by the calling function [OsalPort_free()].
 */
extern ZDO_MgmNwkDiscRsp_t *ZDO_ParseMgmNwkDiscRsp( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ParseMgmtLqiRsp - Parse the Mgmt_Lqi_rsp message
 *
 * returns a pointer to parsed response structure (NULL if not allocated).
 *          This structure was allocated using OsalPort_malloc, so it must be freed
 *          by the calling function [OsalPort_free()].
 */
extern ZDO_MgmtLqiRsp_t *ZDO_ParseMgmtLqiRsp( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ParseMgmtRtgRsp - Parse the Mgmt_Rtg_rsp message
 *
 * returns a pointer to parsed response structure (NULL if not allocated).
 *          This structure was allocated using OsalPort_malloc, so it must be freed
 *          by the calling function [OsalPort_free()].
 */
extern ZDO_MgmtRtgRsp_t *ZDO_ParseMgmtRtgRsp( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ParseMgmtBindRsp - Parse the Mgmt_Bind_rsp message
 *
 * returns a pointer to parsed response structure (NULL if not allocated).
 *          This structure was allocated using OsalPort_malloc, so it must be freed
 *          by the calling function [OsalPort_free()].
 */
extern ZDO_MgmtBindRsp_t *ZDO_ParseMgmtBindRsp( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ParseMgmtDirectJoinRsp - Parse the Mgmt_Direct_Join_rsp message.
 *     Parameter: a - pointer to the message to parse [zdoIncomingMsg_t *]
 *
 *  returns the status field of the message.
 */
#define ZDO_ParseMgmtDirectJoinRsp(a) ((uint8_t)(*(a->asdu)))

/*
 * ZDO_ParseMgmtLeaveRsp - Parse the Mgmt_Leave_rsp message.
 *     Parameter: a - pointer to the message to parse [zdoIncomingMsg_t *]
 *
 *  returns the status field of the message.
 */
#define ZDO_ParseMgmtLeaveRsp(a) ((uint8_t)(*(a->asdu)))

/*
 * ZDO_ParseMgmtPermitJoinRsp - Parse the Mgmt_Permit_Join_rsp message.
 *     Parameter: a - pointer to the message to parse [zdoIncomingMsg_t *]
 *
 *  returns the status field of the message.
 */
#define ZDO_ParseMgmtPermitJoinRsp(a) ((uint8_t)(*(a->asdu)))

/*
 * ZDO_ParseUserDescRsp - Parse the User_Desc_rsp message
 *
 * returns a pointer to parsed response structure (NULL if not allocated).
 *          This structure was allocated using OsalPort_malloc, so it must be freed
 *          by the calling function [OsalPort_free()].
 */
extern ZDO_UserDescRsp_t *ZDO_ParseUserDescRsp( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ParseServerDiscRsp - Parse the Server_Discovery_rsp message
 *
 */
extern void ZDO_ParseServerDiscRsp( zdoIncomingMsg_t *inMsg, ZDO_ServerDiscRsp_t *pRsp );

/*
 * ZDO_ParseEndDeviceBindReq - Parse the User_Desc_rsp message
 *
 *   NOTE:  The clusters lists in bindReq are allocated in this
 *          function and must be freed by that calling function.
 */
extern void ZDO_ParseEndDeviceBindReq( zdoIncomingMsg_t *inMsg, ZDEndDeviceBind_t *bindReq );

/*
 * ZDO_ParseBindUnbindReq - Parses the Bind_req or Unbind_req messages
 */
extern void ZDO_ParseBindUnbindReq( zdoIncomingMsg_t *inMsg, ZDO_BindUnbindReq_t *pReq );

/*
 * ZDApp_ProcessBindUnbindReq - Called to process a Bind_req or Unbind_req message
 */
extern void ZDO_ProcessBindUnbindReq( zdoIncomingMsg_t *inMsg, ZDO_BindUnbindReq_t *pReq );

/*
 * ZDO_ParseUserDescConf - Parse the User_Desc_conf message.
 *     Parameter: a - pointer to the message to parse [zdoIncomingMsg_t *]
 *
 *  returns the status field of the message.
 */
#define ZDO_ParseUserDescConf(a) ((uint8_t)(*(a->asdu)))

/*
 * ZDO_ParseDeviceAnnce - Called to parse an End_Device_annce message
 */
extern void ZDO_ParseDeviceAnnce( zdoIncomingMsg_t *inMsg, ZDO_DeviceAnnce_t *pAnnce );

/*
 * ZDO_ParseParentAnnce - Called to parse a Parent_annce and Parent_annce_rsp message
 */
extern ZDO_ParentAnnce_t *ZDO_ParseParentAnnce( zdoIncomingMsg_t *inMsg );

/*
 * ZDO_ParseMgmtNwkUpdateNotify - Parse the Mgmt_NWK_Update_notify message
 *
 * returns a pointer to parsed response structure (NULL if not allocated).
 *          This structure was allocated using OsalPort_malloc, so it must be freed
 *          by the calling function [OsalPort_free()].
 */
extern ZDO_MgmtNwkUpdateNotify_t *ZDO_ParseMgmtNwkUpdateNotify( zdoIncomingMsg_t *inMsg );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZDOBJECT_H */
