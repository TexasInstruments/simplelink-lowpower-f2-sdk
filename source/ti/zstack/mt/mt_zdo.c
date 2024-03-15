/**************************************************************************************************
  Filename:       MT_ZDO.c
  Revised:        $Date: 2015-02-05 17:15:13 -0800 (Thu, 05 Feb 2015) $
  Revision:       $Revision: 42371 $

  Description:    MonitorTest functions for the ZDO layer.

  Copyright 2004-2015 Texas Instruments Incorporated.

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

#ifdef MT_ZDO_FUNC

/**************************************************************************************************
 * INCLUDES
 **************************************************************************************************/
#include "zcomdef.h"
#include "rom_jt_154.h"
#include "osal_nv.h"
#include "mt.h"
#include "mt_zdo.h"
#include "addr_mgr.h"
#include "aps_mede.h"
#include "zd_config.h"
#include "zd_profile.h"
#include "zd_object.h"
#include "zd_app.h"
#include "aps_groups.h"
#include "bdb_interface.h"

#if defined ( MT_ZDO_EXTENSIONS )
  #include "rtg.h"
#endif
#if defined ( MT_SYS_KEY_MANAGEMENT ) || defined ( MT_ZDO_EXTENSIONS )
  #include "zd_sec_mgr.h"
#endif

#include "nwk_util.h"

/**************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
#define MT_ZDO_END_DEVICE_ANNCE_IND_LEN   0x0D
#define MT_ZDO_ADDR_RSP_LEN               0x0D
#define MT_ZDO_BIND_UNBIND_RSP_LEN        0x03
#define MT_ZDO_BEACON_IND_LEN             21
#define MT_ZDO_BEACON_IND_PACK_LEN        (255 - SPI_0DATA_MSG_LEN)
#define MT_ZDO_JOIN_CNF_LEN               5

// Message must pack nwk addr, entire (not just pointer to) ieee addr, and packet cost, so the
// sizeof(zdoConcentratorInd_t) is not usable.
#define MT_ZDO_CONCENTRATOR_IND_LEN      (2 + Z_EXTADDR_LEN + 1)

#define MTZDO_RESPONSE_BUFFER_LEN   100

#define MTZDO_MAX_MATCH_CLUSTERS    16
#define MTZDO_MAX_ED_BIND_CLUSTERS  15

// Conversion from ZDO Cluster Id to the RPC AREQ Id is direct as follows:
#define MT_ZDO_CID_TO_AREQ_ID(CId)  ((uint8_t)(CId) | 0x80)

#define MT_ZDO_STATUS_LEN   1

#define MT_ZDO_EXT_RX_IDLE_SET 1
#define MT_ZDO_EXT_RX_IDLE_RX_ON_CONFIG 2
#define MT_ZDO_EXT_RX_IDLE_SLEEPY_CONFIG 3

#if defined ( MT_ZDO_EXTENSIONS )
typedef struct
{
  uint16_t            ami;
  uint16_t            keyNvId;   // index to the Link Key table in NV
  ZDSecMgr_Authentication_Option authenticateOption;
} ZDSecMgrEntry_t;
#endif  // MT_ZDO_EXTENSIONS

/**************************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************************/
uint32_t _zdoCallbackSub;
uint8_t *pBeaconIndBuf = NULL;

/**************************************************************************************************
 * LOCAL VARIABLES
 **************************************************************************************************/
bool ignoreIndication = FALSE;

/**************************************************************************************************
 * LOCAL FUNCTIONS
 **************************************************************************************************/
#if defined (MT_ZDO_FUNC)
static void MT_ZdoNWKAddressRequest(uint8_t *pBuf);
static void MT_ZdoIEEEAddrRequest(uint8_t *pBuf);
static void MT_ZdoNodeDescRequest(uint8_t *pBuf);
static void MT_ZdoPowerDescRequest(uint8_t *pBuf);
static void MT_ZdoSimpleDescRequest(uint8_t *pBuf);
static void MT_ZdoActiveEpRequest(uint8_t *pBuf);
static void MT_ZdoMatchDescRequest(uint8_t *pBuf);
static void MT_ZdoComplexDescRequest(uint8_t *pBuf);
static void MT_ZdoUserDescRequest(uint8_t *pBuf);
static void MT_ZdoEndDevAnnce(uint8_t *pBuf);
static void MT_ZdoUserDescSet(uint8_t *pBuf);
static void MT_ZdoServiceDiscRequest(uint8_t *pBuf);
static void MT_ZdoEndDevBindRequest(uint8_t *pBuf);
static void MT_ZdoBindRequest(uint8_t *pBuf);
static void MT_ZdoUnbindRequest(uint8_t *pBuf);
#if defined ( MT_SYS_KEY_MANAGEMENT )
static void MT_ZdoSetLinkKey(uint8_t *pBuf);
static void MT_ZdoRemoveLinkKey(uint8_t *pBuf);
static void MT_ZdoGetLinkKey(uint8_t *pBuf);
#endif /* MT_SYS_KEY_MANAGEMENT */
static void MT_ZdoNetworkDiscoveryReq(uint8_t *pBuf);
static void MT_ZdoJoinReq(uint8_t *pBuf);
/* Call back function */
void *MT_ZdoNwkDiscoveryCnfCB ( void *pStr );
void *MT_ZdoBeaconIndCB ( void *pStr );
void *MT_ZdoJoinCnfCB ( void *pStr );
#if defined (MT_ZDO_MGMT)
static void MT_ZdoMgmtLqiRequest(uint8_t *pBuf);
static void MT_ZdoMgmtRtgRequest(uint8_t *pBuf);
static void MT_ZdoMgmtBindRequest(uint8_t *pBuf);
static void MT_ZdoMgmtLeaveRequest(uint8_t *pBuf);
static void MT_ZdoMgmtDirectJoinRequest(uint8_t *pBuf);
static void MT_ZdoMgmtPermitJoinRequest(uint8_t *pBuf);
static void MT_ZdoMgmtNwkUpdateRequest(uint8_t *pBuf);
static void MT_ZdoMgmtNwkDiscRequest(uint8_t *pBuf);
#endif /* MT_ZDO_MGMT */
static void MT_ZdoSendData( uint8_t *pBuf );
static void MT_ZdoNwkAddrOfInterestReq( uint8_t *pBuf );
static void MT_ZdoStartupFromApp(uint8_t *pBuf);
static void MT_ZdoRegisterForZDOMsg(uint8_t *pBuf);
static void MT_ZdoRemoveRegisteredCB(uint8_t *pBuf);
#endif /* MT_ZDO_FUNC */

#if defined (MT_ZDO_CB_FUNC)
static uint8_t MT_ZdoHandleExceptions( afIncomingMSGPacket_t *pData, zdoIncomingMsg_t *inMsg );
void MT_ZdoAddrRspCB( ZDO_NwkIEEEAddrResp_t *pMsg, uint16_t clusterID );
void MT_ZdoEndDevAnnceCB( ZDO_DeviceAnnce_t *pMsg, uint16_t srcAddr );
void MT_ZdoBindUnbindRspCB( uint16_t clusterID, uint16_t srcAddr, uint8_t status );
void* MT_ZdoSrcRtgCB( void *pStr );
static void *MT_ZdoConcentratorIndCB(void *pStr);
static void *MT_ZdoLeaveInd(void *vPtr);
void *MT_ZdoTcDeviceInd( void *params );
void *MT_ZdoPermitJoinInd( void *duration );
#endif /* MT_ZDO_CB_FUNC */

#if defined ( MT_ZDO_EXTENSIONS )
#if ( ZG_BUILD_COORDINATOR_TYPE )
static void MT_ZdoSecUpdateNwkKey( uint8_t *pBuf );
static void MT_ZdoSecSwitchNwkKey( uint8_t *pBuf );
#endif // ZDO_COORDINATOR
static void MT_ZdoSecAddLinkKey( uint8_t *pBuf );
static void MT_ZdoSecEntryLookupExt( uint8_t *pBuf );
static void MT_ZdoSecDeviceRemove( uint8_t *pBuf );
static void MT_ZdoExtRouteDisc( uint8_t *pBuf );
static void MT_ZdoExtRouteCheck( uint8_t *pBuf );
static void MT_ZdoExtRemoveGroup( uint8_t *pBuf );
static void MT_ZdoExtRemoveAllGroup( uint8_t *pBuf );
static void MT_ZdoExtFindAllGroupsEndpoint( uint8_t *pBuf );
static void MT_ZdoExtFindGroup( uint8_t *pBuf );
static void MT_ZdoExtAddGroup( uint8_t *pBuf );
static void MT_ZdoExtCountAllGroups( uint8_t *pBuf );
static void MT_ZdoExtRxIdle( uint8_t *pBuf );
static void MT_ZdoExtNwkInfo( uint8_t *pBuf );
static void MT_ZdoExtSecApsRemoveReq( uint8_t *pBuf );
static void MT_ZdoExtSetParams( uint8_t *pBuf );
extern ZStatus_t ZDSecMgrEntryLookupExt( uint8_t* extAddr, ZDSecMgrEntry_t** entry );
#endif // MT_ZDO_EXTENSIONS

#if defined (MT_ZDO_FUNC)
/***************************************************************************************************
 * @fn      MT_ZdoInit
 *
 * @brief   MT ZDO initialization
 *
 * @param   none
 *
 * @return  none
 ***************************************************************************************************/
void MT_ZdoInit(void)
{
#ifdef MT_ZDO_CB_FUNC
  /* Register with ZDO for indication callbacks */
  ZDO_RegisterForZdoCB(ZDO_SRC_RTG_IND_CBID, &MT_ZdoSrcRtgCB);
  ZDO_RegisterForZdoCB(ZDO_CONCENTRATOR_IND_CBID, &MT_ZdoConcentratorIndCB);
  ZDO_RegisterForZdoCB(ZDO_LEAVE_IND_CBID, &MT_ZdoLeaveInd);
  ZDO_RegisterForZdoCB(ZDO_PERMIT_JOIN_CBID, &MT_ZdoPermitJoinInd);
  ZDO_RegisterForZdoCB(ZDO_TC_DEVICE_CBID, &MT_ZdoTcDeviceInd);
#endif
}

/***************************************************************************************************
 * @fn      MT_ZdoCommandProcessing
 *
 * @brief
 *
 *   Process all the ZDO commands that are issued by test tool
 *
 * @param   pBuf - pointer to the msg buffer
 *
 *          | LEN  | CMD0  | CMD1  |  DATA  |
 *          |  1   |   1   |   1   |  0-255 |
 *
 * @return  status
 ***************************************************************************************************/
uint8_t MT_ZdoCommandProcessing(uint8_t* pBuf)
{
  uint8_t status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
#if defined ( ZDO_NWKADDR_REQUEST )
    case MT_ZDO_NWK_ADDR_REQ:
      MT_ZdoNWKAddressRequest(pBuf);
      break;
#endif

#if defined ( ZDO_IEEEADDR_REQUEST )
    case MT_ZDO_IEEE_ADDR_REQ:
      MT_ZdoIEEEAddrRequest(pBuf);
      break;
#endif

#if defined ( ZDO_NODEDESC_REQUEST )
    case MT_ZDO_NODE_DESC_REQ:
      MT_ZdoNodeDescRequest(pBuf);
      break;
#endif

#if defined ( ZDO_POWERDESC_REQUEST )
    case MT_ZDO_POWER_DESC_REQ:
      MT_ZdoPowerDescRequest(pBuf);
      break;
#endif

#if defined ( ZDO_SIMPLEDESC_REQUEST )
    case MT_ZDO_SIMPLE_DESC_REQ:
      MT_ZdoSimpleDescRequest(pBuf);
      break;
#endif

#if defined ( ZDO_ACTIVEEP_REQUEST )
    case MT_ZDO_ACTIVE_EP_REQ:
      MT_ZdoActiveEpRequest(pBuf);
      break;
#endif

#if defined ( ZDO_MATCH_REQUEST )
    case MT_ZDO_MATCH_DESC_REQ:
      MT_ZdoMatchDescRequest(pBuf);
      break;
#endif

#if defined ( ZDO_COMPLEXDESC_REQUEST )
    case MT_ZDO_COMPLEX_DESC_REQ:
      MT_ZdoComplexDescRequest(pBuf);
      break;
#endif

#if defined ( ZDO_USERDESC_REQUEST )
    case MT_ZDO_USER_DESC_REQ:
      MT_ZdoUserDescRequest(pBuf);
      break;
#endif

#if defined ( ZDO_ENDDEVICE_ANNCE )
    case MT_ZDO_END_DEV_ANNCE:
      MT_ZdoEndDevAnnce(pBuf);
      break;
#endif

#if defined ( ZDO_USERDESCSET_REQUEST )
    case MT_ZDO_USER_DESC_SET:
      MT_ZdoUserDescSet(pBuf);
      break;
#endif

#if defined ( ZDO_SERVERDISC_REQUEST )
    case MT_ZDO_SERVICE_DISC_REQ:
      MT_ZdoServiceDiscRequest(pBuf);
      break;
#endif

#if defined ( ZDO_ENDDEVICEBIND_REQUEST )
    case MT_ZDO_END_DEV_BIND_REQ:
      MT_ZdoEndDevBindRequest(pBuf);
      break;
#endif

#if defined ( ZDO_BIND_UNBIND_REQUEST )
    case MT_ZDO_BIND_REQ:
      MT_ZdoBindRequest(pBuf);
      break;
#endif

#if defined ( ZDO_BIND_UNBIND_REQUEST )
    case MT_ZDO_UNBIND_REQ:
      MT_ZdoUnbindRequest(pBuf);
      break;
#endif

#if defined ( MT_SYS_KEY_MANAGEMENT )
    case MT_ZDO_SET_LINK_KEY:
      MT_ZdoSetLinkKey(pBuf);
      break;

    case MT_ZDO_REMOVE_LINK_KEY:
      MT_ZdoRemoveLinkKey(pBuf);
      break;

    case MT_ZDO_GET_LINK_KEY:
      MT_ZdoGetLinkKey(pBuf);
      break;
#endif // MT_SYS_KEY_MANAGEMENT

#if defined ( ZDO_MANUAL_JOIN )
    case MT_ZDO_NWK_DISCOVERY_REQ:
      MT_ZdoNetworkDiscoveryReq(pBuf);
      break;

    case MT_ZDO_JOIN_REQ:
      MT_ZdoJoinReq(pBuf);
      break;
#endif

#if defined ( ZDO_MGMT_NWKDISC_REQUEST )
    case MT_ZDO_MGMT_NWKDISC_REQ:
      MT_ZdoMgmtNwkDiscRequest(pBuf);
      break;
#endif

#if defined ( ZDO_MGMT_LQI_REQUEST )
    case MT_ZDO_MGMT_LQI_REQ:
      MT_ZdoMgmtLqiRequest(pBuf);
      break;
#endif

#if defined ( ZDO_MGMT_RTG_REQUEST )
    case MT_ZDO_MGMT_RTG_REQ:
      MT_ZdoMgmtRtgRequest(pBuf);
      break;
#endif

#if defined ( ZDO_MGMT_BIND_REQUEST )
    case MT_ZDO_MGMT_BIND_REQ:
      MT_ZdoMgmtBindRequest(pBuf);
      break;
#endif

#if defined ( ZDO_MGMT_LEAVE_REQUEST )
    case MT_ZDO_MGMT_LEAVE_REQ:
      MT_ZdoMgmtLeaveRequest(pBuf);
      break;
#endif

#if defined ( ZDO_MGMT_JOINDIRECT_REQUEST )
    case MT_ZDO_MGMT_DIRECT_JOIN_REQ:
      MT_ZdoMgmtDirectJoinRequest(pBuf);
      break;
#endif

#if defined ( ZDO_MGMT_PERMIT_JOIN_REQUEST )
    case MT_ZDO_MGMT_PERMIT_JOIN_REQ:
      MT_ZdoMgmtPermitJoinRequest(pBuf);
      break;
#endif

#if defined ( ZDO_MGMT_NWKUPDATE_REQUEST )
    case MT_ZDO_MGMT_NWK_UPDATE_REQ:
      MT_ZdoMgmtNwkUpdateRequest(pBuf);
      break;
#endif

#if defined ( ZDO_NETWORKSTART_REQUEST )
    case MT_ZDO_STARTUP_FROM_APP:
      MT_ZdoStartupFromApp(pBuf);
      break;
#endif

    case MT_ZDO_SEND_DATA:
      MT_ZdoSendData( pBuf );
      break;

    case MT_ZDO_NWK_ADDR_OF_INTEREST_REQ:
      MT_ZdoNwkAddrOfInterestReq( pBuf );
      break;

    case MT_ZDO_MSG_CB_REGISTER:
      MT_ZdoRegisterForZDOMsg(pBuf);
      break;

    case MT_ZDO_MSG_CB_REMOVE:
      MT_ZdoRemoveRegisteredCB(pBuf);
      break;

#if defined ( MT_ZDO_EXTENSIONS )
#if ( ZG_BUILD_COORDINATOR_TYPE )
    case MT_ZDO_EXT_UPDATE_NWK_KEY:
      MT_ZdoSecUpdateNwkKey( pBuf );
      break;

    case MT_ZDO_EXT_SWITCH_NWK_KEY:
      MT_ZdoSecSwitchNwkKey( pBuf );
      break;
#endif // ZDO_COORDINATOR
    case MT_ZDO_SEC_ADD_LINK_KEY:
      MT_ZdoSecAddLinkKey( pBuf );
      break;

    case MT_ZDO_SEC_ENTRY_LOOKUP_EXT:
      MT_ZdoSecEntryLookupExt( pBuf );
      break;

    case MT_ZDO_SEC_DEVICE_REMOVE:
       MT_ZdoSecDeviceRemove( pBuf );
       break;

    case MT_ZDO_EXT_ROUTE_DISC:
       MT_ZdoExtRouteDisc( pBuf );
       break;

    case MT_ZDO_EXT_ROUTE_CHECK:
       MT_ZdoExtRouteCheck( pBuf );
       break;

    case MT_ZDO_EXT_REMOVE_GROUP:
      MT_ZdoExtRemoveGroup( pBuf );
      break;

    case MT_ZDO_EXT_REMOVE_ALL_GROUP:
      MT_ZdoExtRemoveAllGroup( pBuf );
      break;

    case MT_ZDO_EXT_FIND_ALL_GROUPS_ENDPOINT:
      MT_ZdoExtFindAllGroupsEndpoint( pBuf );
      break;

    case MT_ZDO_EXT_FIND_GROUP:
      MT_ZdoExtFindGroup( pBuf );
      break;

    case MT_ZDO_EXT_ADD_GROUP:
      MT_ZdoExtAddGroup( pBuf );
      break;

    case MT_ZDO_EXT_COUNT_ALL_GROUPS:
      MT_ZdoExtCountAllGroups( pBuf );
      break;

    case MT_ZDO_EXT_RX_IDLE:
      MT_ZdoExtRxIdle( pBuf );
      break;

    case MT_ZDO_EXT_NWK_INFO:
      MT_ZdoExtNwkInfo( pBuf );
      break;

    case MT_ZDO_EXT_SEC_APS_REMOVE_REQ:
      MT_ZdoExtSecApsRemoveReq( pBuf );
      break;

    case MT_ZDO_FORCE_CONCENTRATOR_CHANGE:
      ZDApp_ForceConcentratorChange();
      break;

    case MT_ZDO_EXT_SET_PARAMS:
      MT_ZdoExtSetParams( pBuf );
      break;
#endif  // MT_ZDO_EXTENSIONS

    default:
      status = MT_RPC_ERR_COMMAND_ID;
      break;
  }

  return status;
}

/***************************************************************************************************
 * @fn      MT_ZdoNwkAddrReq
 *
 * @brief   Handle a nwk address request.
 *
 * @param   pData  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoNWKAddressRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  uint8_t reqType;
  uint8_t startIndex;
  uint8_t *pExtAddr;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* parse parameters */
  pExtAddr = pBuf;
  pBuf += Z_EXTADDR_LEN;

  /* Request type */
  reqType = *pBuf++;

  /* Start index */
  startIndex = *pBuf;

  retValue = (uint8_t)ZDP_NwkAddrReq(pExtAddr, reqType, startIndex, 0);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);

#if defined ( MT_ZDO_EXTENSIONS )
  {
    // Force a response message if the ext address matches a child of this device
    associated_devices_t *pAssoc;
    uint8_t buf[21];
    uint8_t *pBuf = buf;

    if ( (ZSTACK_ROUTER_BUILD)
      && (((pAssoc = AssocGetWithExt( pExtAddr )) != NULL)
             && (pAssoc->nodeRelation == CHILD_RFD)) )
    {
      uint16_t nwkAddr = NLME_GetShortAddr();

      *pBuf++ = LO_UINT16(nwkAddr);
      *pBuf++ = HI_UINT16(nwkAddr);
      *pBuf++ = 0;
      *pBuf++ = LO_UINT16(NWK_addr_rsp);
      *pBuf++ = HI_UINT16(NWK_addr_rsp);
      *pBuf++ = 0;
      *pBuf++ = 0;
      *pBuf++ = LO_UINT16(nwkAddr);
      *pBuf++ = HI_UINT16(nwkAddr);

      *pBuf++ = ZSuccess;

      pBuf = osal_cpyExtAddr( pBuf, pExtAddr );

      *pBuf++ = LO_UINT16( pAssoc->shortAddr );
      *pBuf++ = HI_UINT16( pAssoc->shortAddr );
      *pBuf = 0;

      MT_BuildAndSendZToolResponse( ((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_ZDO),
                                  MT_ZDO_MSG_CB_INCOMING, 21, buf );
    }
  }
#endif
}

/***************************************************************************************************
 * @fn      MT_ZdoIEEEAddrRequest
 *
 * @brief   Handle a IEEE address request.
 *
 * @param   pData  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoIEEEAddrRequest (uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  uint16_t shortAddr;
  uint8_t reqType;
  uint8_t startIndex;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Dev address */
  shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* request type */
  reqType = *pBuf++;

  /* start index */
  startIndex = *pBuf;

  retValue = (uint8_t)ZDP_IEEEAddrReq(shortAddr, reqType, startIndex, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoNodeDescRequest
 *
 * @brief   Handle a Node Descriptor request.
 *
 * @param   pData  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoNodeDescRequest (uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr;
  uint16_t shortAddr;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Destination address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Network address of interest */
  shortAddr = OsalPort_buildUint16( pBuf );

  retValue = (uint8_t)ZDP_NodeDescReq( &destAddr, shortAddr, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoPowerDescRequest
 *
 * @brief   Handle a Power Descriptor request.
 *
 * @param   pData  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoPowerDescRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr;
  uint16_t shortAddr;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Dev address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Network address of interest */
  shortAddr = OsalPort_buildUint16( pBuf );

  retValue = (uint8_t)ZDP_PowerDescReq( &destAddr, shortAddr, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoSimpleDescRequest
 *
 * @brief   Handle a Simple Descriptor request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoSimpleDescRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  uint8_t epInt;
  zAddrType_t destAddr;
  uint16_t shortAddr;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Dev address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Network address of interest */
  shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* endpoint/interface */
  epInt = *pBuf;

  retValue = (uint8_t)ZDP_SimpleDescReq( &destAddr, shortAddr, epInt, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoActiveEpRequest
 *
 * @brief   Handle a Active EP request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoActiveEpRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr;
  uint16_t shortAddr;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Dev address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Network address of interest */
  shortAddr = OsalPort_buildUint16( pBuf );

  retValue = (uint8_t)ZDP_ActiveEPReq( &destAddr, shortAddr, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoMatchDescRequest
 *
 * @brief   Handle a Match Descriptor request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoMatchDescRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue = 0;
  uint8_t i, numInClusters, numOutClusters;
  uint16_t profileId;
  zAddrType_t destAddr;
  uint16_t shortAddr;
  uint16_t inClusters[MTZDO_MAX_MATCH_CLUSTERS], outClusters[MTZDO_MAX_MATCH_CLUSTERS];

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Dev address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Network address of interest */
  shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Profile ID */
  profileId = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* NumInClusters */
  numInClusters = *pBuf++;
  if ( numInClusters <= MTZDO_MAX_MATCH_CLUSTERS )
  {
    /* IN clusters */
    for ( i = 0; i < numInClusters; i++ )
    {
      inClusters[i] = OsalPort_buildUint16( pBuf );
      pBuf += 2;
    }
  }
  else
  {
    retValue = ZDP_INVALID_REQTYPE;
  }

  /* NumOutClusters */
  numOutClusters = *pBuf++;
  if ( numOutClusters <= MTZDO_MAX_MATCH_CLUSTERS )
  {
    /* OUT Clusters */
    for ( i = 0; i < numOutClusters; i++ )
    {
      outClusters[i] = OsalPort_buildUint16( pBuf );
      pBuf += 2;
    }
  }
  else
  {
    retValue = ZDP_INVALID_REQTYPE;
  }

  if ( retValue == 0 )
  {
    retValue = (uint8_t)ZDP_MatchDescReq( &destAddr, shortAddr, profileId, numInClusters,
                                       inClusters, numOutClusters, outClusters, 0);
  }

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoComplexDescRequest
 *
 * @brief   Handle a Complex Descriptor request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoComplexDescRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr;
  uint16_t shortAddr;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Dev address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Network address of interest */
  shortAddr = OsalPort_buildUint16( pBuf );

  retValue = (uint8_t)ZDP_ComplexDescReq( &destAddr, shortAddr, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoUserDescRequest
 *
 * @brief   Handle a User Descriptor request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoUserDescRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr;
  uint16_t shortAddr;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Dev address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Network address of interest */
  shortAddr = OsalPort_buildUint16( pBuf );

  retValue = (uint8_t)ZDP_UserDescReq( &destAddr, shortAddr, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoEndDevAnnce
 *
 * @brief   Handle a End Device Announce Descriptor request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoEndDevAnnce(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  uint16_t shortAddr;
  uint8_t *pIEEEAddr;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* network address */
  shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* extended address */
  pIEEEAddr = pBuf;
  pBuf += Z_EXTADDR_LEN;

  retValue = (uint8_t)ZDP_DeviceAnnce( shortAddr, pIEEEAddr, *pBuf, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoUserDescSet
 *
 * @brief   Handle a User Descriptor Set.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoUserDescSet(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr;
  uint16_t shortAddr;
  UserDescriptorFormat_t userDesc;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Dev address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Network address of interest */
  shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* User descriptor */
  userDesc.len = *pBuf++;
  OsalPort_memcpy( userDesc.desc, pBuf, userDesc.len );

  retValue = (uint8_t)ZDP_UserDescSet( &destAddr, shortAddr, &userDesc, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoServiceDiscRequest
 *
 * @brief   Handle a Server Discovery request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoServiceDiscRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  uint16_t serviceMask;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Service Mask */
  serviceMask = OsalPort_buildUint16( pBuf );

  retValue = (uint8_t)ZDP_ServerDiscReq( serviceMask, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoEndDevBindRequest
 *
 * @brief   Handle a End Device Bind request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoEndDevBindRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue = 0;
  uint8_t i, epInt, numInClusters, numOutClusters;
  zAddrType_t destAddr;
  uint16_t shortAddr;
  uint16_t profileID, inClusters[MTZDO_MAX_ED_BIND_CLUSTERS], outClusters[MTZDO_MAX_ED_BIND_CLUSTERS];

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Dev address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Local coordinator of the binding */
  shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* For now, skip past the extended address */
  pBuf += Z_EXTADDR_LEN;

  /* Endpoint */
  epInt = *pBuf++;

  /* Profile ID */
  profileID = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* NumInClusters */
  numInClusters = *pBuf++;
  if ( numInClusters <= MTZDO_MAX_ED_BIND_CLUSTERS )
  {
    for ( i = 0; i < numInClusters; i++ )
    {
      inClusters[i] = OsalPort_buildUint16( pBuf );
      pBuf += 2;
    }
  }
  else
  {
    retValue = ZDP_INVALID_REQTYPE;
  }

  /* NumOutClusters */
  numOutClusters = *pBuf++;
  if ( numOutClusters <= MTZDO_MAX_ED_BIND_CLUSTERS )
  {
    for ( i = 0; i < numOutClusters; i++ )
    {
      outClusters[i] = OsalPort_buildUint16( pBuf );
      pBuf += 2;
    }
  }
  else
  {
    retValue = ZDP_INVALID_REQTYPE;
  }

  if ( retValue == 0 )
  {
    retValue = (uint8_t)ZDP_EndDeviceBindReq( &destAddr, shortAddr, epInt, profileID,
                                          numInClusters, inClusters, numOutClusters, outClusters, 0);
  }

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoBindRequest
 *
 * @brief   Handle a Bind request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoBindRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr, devAddr;
  uint8_t *pSrcAddr, *ptr;
  uint8_t srcEPInt, dstEPInt;
  uint16_t clusterID;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Dev address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* SrcAddress */
  pSrcAddr = pBuf;
  pBuf += Z_EXTADDR_LEN;

  /* SrcEPInt */
  srcEPInt = *pBuf++;

  /* ClusterID */
  clusterID = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Destination Address mode */
  devAddr.addrMode = *pBuf++;

  /* Destination Address */
  if ( devAddr.addrMode == Addr64Bit )
  {
    ptr = pBuf;
    osal_cpyExtAddr( devAddr.addr.extAddr, ptr );
  }
  else
  {
    devAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  }
  /* The short address occupies LSB two bytes */
  pBuf += Z_EXTADDR_LEN;

  /* DstEPInt */
  dstEPInt = *pBuf;

  retValue = (uint8_t)ZDP_BindReq( &destAddr, pSrcAddr, srcEPInt, clusterID, &devAddr, dstEPInt, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoUnbindRequest
 *
 * @brief   Handle a Unbind request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoUnbindRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr, devAddr;
  uint8_t *pSrcAddr, *ptr;
  uint8_t srcEPInt, dstEPInt;
  uint16_t clusterID;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* dev address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* SrcAddress */
  pSrcAddr = pBuf;
  pBuf += Z_EXTADDR_LEN;

  /* SrcEPInt */
  srcEPInt = *pBuf++;

  /* ClusterID */
  clusterID = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Destination Address mode */
  devAddr.addrMode = *pBuf++;

  /* Destination Address */
  if ( devAddr.addrMode == Addr64Bit )
  {
    ptr = pBuf;
    osal_cpyExtAddr( devAddr.addr.extAddr, ptr );
  }
  else
  {
    devAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  }
  /* The short address occupies LSB two bytes */
  pBuf += Z_EXTADDR_LEN;

  /* dstEPInt */
  dstEPInt = *pBuf;

  retValue = (uint8_t)ZDP_UnbindReq( &destAddr, pSrcAddr, srcEPInt, clusterID, &devAddr, dstEPInt, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

#if defined (MT_SYS_KEY_MANAGEMENT)
/***************************************************************************************************
 * @fn      MT_ZdoSetLinkKey
 *
 * @brief   Set an application or trust center link key.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoSetLinkKey(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  uint8_t *pExtAddr;
  uint8_t *pKey;
  uint16_t shortAddr;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* ShortAddr */
  shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Extended Addr */
  pExtAddr = pBuf;
  pBuf += Z_EXTADDR_LEN;

  /* Key data */
  pKey = pBuf;

  retValue = (uint8_t)ZDSecMgrAddLinkKey( shortAddr, pExtAddr, pKey);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoRemoveLinkKey
 *
 * @brief   Remove an application or trust center link key.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoRemoveLinkKey(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue = ZNwkUnknownDevice;
  uint8_t *pExtAddr;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* ShortAddr */
  pExtAddr = pBuf;

  if( ( ZG_BUILD_COORDINATOR_TYPE ) && ( ZG_DEVICE_COORDINATOR_TYPE ))
  {
    uint16_t tempIndex;
    APSME_TCLinkKeyNVEntry_t TCLKDevEntry;
    uint8_t found;
                                                   //Reset the frame counter associated to this device  TCLinkKeyRAMEntry
    tempIndex = APSME_SearchTCLinkKeyEntry(pExtAddr,&found, &TCLKDevEntry);

    if(found)
    {
      uint16_t i;

      i = tempIndex;

      TCLinkKeyRAMEntry[i].txFrmCntr = 0;
      TCLinkKeyRAMEntry[i].rxFrmCntr = 0;

      if(TCLKDevEntry.IcIndex & APSME_IC_SET_MASK)
      {
        APSME_EraseICEntry(&TCLKDevEntry.IcIndex);
      }

      memset(&TCLKDevEntry,0,sizeof(APSME_TCLinkKeyNVEntry_t));
      osal_nv_write_ex(ZCD_NV_EX_TCLK_TABLE,
                       tempIndex,
                       sizeof(APSME_TCLinkKeyNVEntry_t),
                       &TCLKDevEntry );

      retValue = ZSuccess;
    }
  }
  else
  {
    retValue = ZDSecMgrDeviceRemoveByExtAddr( pExtAddr );
  }
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoGetLinkKey
 *
 * @brief   Get the application link key.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoGetLinkKey(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  uint8_t *pExtAddr;
  uint8_t *retBuf = NULL;
  uint8_t len;
  APSME_ApsLinkKeyNVEntry_t *pApsLinkKey = NULL;
  uint16_t apsLinkKeyNvId;

  // parse header
  len = pBuf[MT_RPC_POS_LEN];
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  // Extended Address
  pExtAddr = pBuf;

  // Check for illegal extended address -- indicating network address lookup
  if ( ( pExtAddr[2] == 0xFE ) && ( pExtAddr[3] == 0xFF ) &&
       ( len >= Z_EXTADDR_LEN )            )
  {
    uint16_t nwkAddr;
    nwkAddr = OsalPort_buildUint16( pExtAddr );
    (void)AddrMgrExtAddrLookup( nwkAddr, pExtAddr );
  }

  // Fetch the key NV ID
  retValue = APSME_LinkKeyNVIdGet( pExtAddr, &apsLinkKeyNvId );

  if (retValue == ZSuccess)
  {
    if ((pApsLinkKey = (APSME_ApsLinkKeyNVEntry_t *)OsalPort_malloc(sizeof(APSME_ApsLinkKeyNVEntry_t))) != NULL)
    {
      // retrieve key from NV
      if (osal_nv_read( apsLinkKeyNvId, 0,
                       sizeof(APSME_ApsLinkKeyNVEntry_t), pApsLinkKey) != SUCCESS)
      {
        retValue = ZNwkUnknownDevice;
      }
    }
    else
    {
      retValue = ZNwkUnknownDevice;
    }
  }

  // Construct the response message
  len = MT_ZDO_STATUS_LEN + Z_EXTADDR_LEN + SEC_KEY_LEN; // status + extAddr + key
  if ((retBuf = (uint8_t *)OsalPort_malloc(len)) != NULL)
  {
    if (retValue == ZSuccess)
    {
      // Extended Address
      OsalPort_memcpy( &(retBuf[1]), pExtAddr, Z_EXTADDR_LEN );

      // Key data
      OsalPort_memcpy( &(retBuf[1 + Z_EXTADDR_LEN]), pApsLinkKey->key, SEC_KEY_LEN );
    }
    else
    {
      // Failed case - set the rest fields to all FF
      memset( &(retBuf[1]), 0xFF, Z_EXTADDR_LEN + SEC_KEY_LEN );
    }

    retBuf[0] = retValue;  // Status

    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, len, retBuf);

    // clear retBuf because it contains key data and free allocated memory
    memset(retBuf, 0x00, len);

    OsalPort_free(retBuf);
  }

  // clear copy of key in RAM
  if (pApsLinkKey != NULL)
  {
    memset(pApsLinkKey, 0x00, sizeof(APSME_ApsLinkKeyNVEntry_t));

    OsalPort_free(pApsLinkKey);
  }

  return;
}
#endif // MT_SYS_KEY_MANAGEMENT

#if defined (MT_ZDO_MGMT)
/***************************************************************************************************
 * @fn      MT_ZdoMgmtNwkDiscRequest
 *
 * @brief   Handle a Mgmt Nwk Discovery request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoMgmtNwkDiscRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr;
  uint32_t scanChannels;
  uint8_t scanDuration, startIndex;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Dev address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Scan Channels */
  scanChannels = OsalPort_buildUint32(pBuf, 4);
  pBuf += 4;

  /* Scan Duration */
  scanDuration = *pBuf++;

  /* Start Index */
  startIndex = *pBuf;

  retValue = (uint8_t)ZDP_MgmtNwkDiscReq( &destAddr, scanChannels, scanDuration, startIndex, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoMgmtLqiRequest
 *
 * @brief   Handle a Mgmt Lqi request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoMgmtLqiRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr;
  uint8_t startIndex;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Dev address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Start Index */
  startIndex = *pBuf;

  retValue = (uint8_t)ZDP_MgmtLqiReq( &destAddr, startIndex, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoMgmtRtgRequest
 *
 * @brief   Handle a Mgmt Rtg request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoMgmtRtgRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr;
  uint8_t startIndex;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Dev Address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Start Index */
  startIndex = *pBuf;

  retValue = (byte)ZDP_MgmtRtgReq( &destAddr, startIndex, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoMgmtBindRequest
 *
 * @brief   Handle a Mgmt Bind request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoMgmtBindRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr;
  uint8_t startIndex;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Dev Address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Start Index */
  startIndex = *pBuf;

  retValue = (uint8_t)ZDP_MgmtBindReq( &destAddr, startIndex, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoMgmtLeaveRequest
 *
 * @brief   Handle a Mgmt Leave request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoMgmtLeaveRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr;
  uint8_t *pIEEEAddr;
  uint8_t removeChildren;
  uint8_t rejoin;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Destination Address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* IEEE address */
  pIEEEAddr = pBuf;
  pBuf += Z_EXTADDR_LEN;

  /* Rejoin if bit0 is set */
  rejoin = ( *pBuf & 0x01 ) ? TRUE : FALSE;

  /* Remove Children if bit1 is set */
  removeChildren = ( *pBuf & 0x02 ) ? TRUE : FALSE;

  retValue = (byte)ZDP_MgmtLeaveReq( &destAddr, pIEEEAddr, removeChildren, rejoin, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoMgmtDirectJoinRequest
 *
 * @brief   Handle a Mgmt Direct Join request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoMgmtDirectJoinRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr;
  uint8_t *deviceAddr;
  uint8_t capInfo;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Destination Address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Device Address */
  deviceAddr = pBuf;
  pBuf += Z_EXTADDR_LEN;

  /* Capability information */
  capInfo = *pBuf;

  retValue = (uint8_t)ZDP_MgmtDirectJoinReq( &destAddr, deviceAddr, capInfo, 0);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoMgmtPermitJoinRequest
 *
 * @brief   Handle a Mgmt Permit Join request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoMgmtPermitJoinRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t dataLn;
  uint8_t retValue;
  uint8_t duration;
  uint8_t tcSignificance;
  zAddrType_t destAddr;

  /* parse header */
  dataLn = pBuf[MT_RPC_POS_LEN];
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if ( 4 == dataLn )
  {
    /* addrMode was hardwired up to Z-Stack 2.6.0 */
    destAddr.addrMode = afAddr16Bit;
  }
  else
  {
    /* addrMode is a parameter after Z-Stack 2.6.0 */
    destAddr.addrMode = *pBuf++;
  }

  /* Destination Address */
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Duration */
  duration = *pBuf++;

  /* Trust center significance */
  tcSignificance = *pBuf;

  ignoreIndication = TRUE;
  retValue = (uint8_t)ZDP_MgmtPermitJoinReq( &destAddr, duration, tcSignificance, 0);
  ignoreIndication = FALSE;

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoMgmtNwkUpdateRequest
 *
 * @brief   Handle a Mgmt Nwk Update request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoMgmtNwkUpdateRequest(uint8_t *pBuf)
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr;
  uint32_t channelMask;
  uint8_t scanDuration, scanCount;
  uint16_t nwkManagerAddr;

    /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Destination address */
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Destination address mode */
  destAddr.addrMode = *pBuf++;

  channelMask = OsalPort_buildUint32(pBuf, 4);
  pBuf += 4;

  /* Scan duration */
  scanDuration = *pBuf++;

  /* Scan count */
  scanCount = *pBuf++;

  /* NWK manager address */
  nwkManagerAddr = OsalPort_buildUint16( pBuf );

  /* Send the Management Network Update request */
  retValue = (uint8_t)ZDP_MgmtNwkUpdateReq( &destAddr, channelMask, scanDuration,
                                          scanCount, _NIB.nwkUpdateId+1, nwkManagerAddr );

  /*
    Since we don't recevied our own broadcast messages, we should
    send a unicast copy of the message to ourself.
  */
  if ( destAddr.addrMode == AddrBroadcast )
  {
    destAddr.addrMode = Addr16Bit;
    destAddr.addr.shortAddr = _NIB.nwkDevAddress;
    retValue = (uint8_t) ZDP_MgmtNwkUpdateReq( &destAddr, channelMask, scanDuration,
                                             scanCount, _NIB.nwkUpdateId+1, nwkManagerAddr );
  }

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}
#endif /* MT_ZDO_MGMT */

/***************************************************************************************************
 * @fn      MT_ZdoSendData
 *
 * @brief   Handle a ZDO Send Data request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoSendData( uint8_t *pBuf )
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr;
  uint8_t transSeq;
  uint8_t len;
  uint16_t cmd;

    /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Destination address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Destination address mode */
  transSeq = *pBuf++;

  /* cmd */
  cmd = OsalPort_buildUint16( pBuf );
  pBuf += 2;
  len = *pBuf++;

  /* Send the Generic ZDO message request */
  retValue = (uint8_t)ZDP_SendData( &transSeq, &destAddr, cmd, len, pBuf, 0 );
  /*
    Since we don't recevied our own broadcast messages, we should
    send a unicast copy of the message to ourself.
  */
  if ( destAddr.addrMode == AddrBroadcast )
  {
    destAddr.addrMode = Addr16Bit;
    destAddr.addr.shortAddr = _NIB.nwkDevAddress;
    retValue = (uint8_t)ZDP_SendData( &transSeq, &destAddr, cmd, len, pBuf, 0 );
  }

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoNwkAddrOfInterestReq
 *
 * @brief   Handle a ZDO Network Address of Interest request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoNwkAddrOfInterestReq( uint8_t *pBuf )
{
  uint8_t cmdId;
  uint8_t retValue;
  zAddrType_t destAddr;
  uint16_t nwkAddr;
  uint8_t cmd;

    /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Destination address */
  destAddr.addrMode = Addr16Bit;
  destAddr.addr.shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  /* Network Address of Interest */
  nwkAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;

  cmd = *pBuf++;

  /* Send the Generic ZDO message request */
  retValue = (uint8_t)ZDP_NWKAddrOfInterestReq( &destAddr, nwkAddr, cmd, 0 );


  /*
    Since we don't recevied our own broadcast messages, we should
    send a unicast copy of the message to ourself.
  */
  if ( destAddr.addrMode == AddrBroadcast )
  {
    destAddr.addrMode = Addr16Bit;
    destAddr.addr.shortAddr = _NIB.nwkDevAddress;
    retValue = (uint8_t)ZDP_NWKAddrOfInterestReq( &destAddr, nwkAddr, cmd, 0 );
  }

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_ZdoStartupFromApp
 *
 * @brief   Handle a Startup from App request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoStartupFromApp(uint8_t *pBuf)
{
  uint8_t cmd0, cmd1, retValue;
  retValue = ZSuccess;

  /* parse header */
  cmd0 = pBuf[MT_RPC_POS_CMD0];
  cmd1 = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if(ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE)
  {
    bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_FORMATION);
  }
  else if (ZG_BUILD_JOINING_TYPE && ZG_DEVICE_JOINING_TYPE)
  {
    bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_STEERING);
  }
  else
  {
     retValue = ZFailure;
  }

  if (MT_RPC_CMD_SREQ == (cmd0 & MT_RPC_CMD_TYPE_MASK))
  {
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP|(uint8_t)MT_RPC_SYS_ZDO), cmd1,1, &retValue);
  }
}


/***************************************************************************************************
 * @fn      MT_ZdoNetworkDiscoveryReq
 *
 * @brief   Handle a ZDO Network Discovery request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoNetworkDiscoveryReq(uint8_t *pBuf)
{
  uint8_t  retValue = ZFailure;
  uint8_t  cmdId;
  uint32_t scanChannels;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Packet format */
  /* scan channels (4) | scan duration (1) */

  /* Scan channels */
  scanChannels = OsalPort_buildUint32(pBuf, 4);
  pBuf += 4;

  retValue = ZDApp_NetworkDiscoveryReq(scanChannels, *pBuf);

  // Register ZDO callback for MT to handle the network discovery confirm
  // and beacon notification confirm
  ZDO_RegisterForZdoCB( ZDO_NWK_DISCOVERY_CNF_CBID, &MT_ZdoNwkDiscoveryCnfCB );
  ZDO_RegisterForZdoCB( ZDO_BEACON_NOTIFY_IND_CBID, &MT_ZdoBeaconIndCB );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue );
}


/***************************************************************************************************
 * @fn      MT_ZdoJoinReq
 *
 * @brief   Handle a ZDO Join request.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 ***************************************************************************************************/
static void MT_ZdoJoinReq(uint8_t *pBuf)
{
  uint8_t  retValue = ZFailure;
  uint8_t  cmdId;
  uint16_t panId;
  uint16_t chosenParent;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Packet format */
  /* channel     (1) | panID (2) | extendedPanID (8) | chosenParent (2) |
   * parentDepth (1) | stackProfile  (1)
   */

  panId        = OsalPort_buildUint16( &pBuf[1] );
  chosenParent = OsalPort_buildUint16( &pBuf[11] );

  retValue = ZDApp_JoinReq(pBuf[0], panId, &(pBuf[3]), chosenParent, pBuf[13], pBuf[14]);

  /* Register for MT to receive Join Confirm */
  ZDO_RegisterForZdoCB( ZDO_JOIN_CNF_CBID, &MT_ZdoJoinCnfCB );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO), cmdId, 1, &retValue );

}


/***************************************************************************************************
 * @fn          MT_ZdoNwkDiscoveryCnfCB
 *
 * @brief       Send an indication to inform host device the completion of
 *              network discovery scan
 *
 * @param       pStr - pointer to a parameter and a structure of parameters
 *
 * @return      void
 ***************************************************************************************************/
void *MT_ZdoNwkDiscoveryCnfCB ( void *pStr )
{
  /* pStr: status (uint8_t) */
  /* Packet Format */
  /* Status (1) */

  // Scan completed. De-register the callback with ZDO
  ZDO_DeregisterForZdoCB( ZDO_NWK_DISCOVERY_CNF_CBID );
  ZDO_DeregisterForZdoCB( ZDO_BEACON_NOTIFY_IND_CBID );

  // Send the buffered beacon indication
  MT_ZdoBeaconIndCB ( NULL );

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_ZDO),
                                         MT_ZDO_NWK_DISCOVERY_CNF, 1, pStr);
  return NULL;
}

/***************************************************************************************************
 * @fn          MT_ZdoBeaconIndCB
 *
 * @brief       Send an indication to host device of a beacon notification
 *
 * @param       pStr -  pointer to a parameter and a structure of parameters
 *
 * @return      void
 ***************************************************************************************************/
void *MT_ZdoBeaconIndCB ( void *pStr )
{
  zdoBeaconInd_t *pBeacon = pStr;
  uint8_t *pTmp;

  /* Packet Format */
  /* devCnt (1) | device #1 (21) | device #2 (21) |... | device #n (21) */

  if( pStr != NULL)
  {
    if( pBeaconIndBuf == NULL )
    {
      // If pBeaconIndBuf has not been allocated yet
      // allocate memory now with MAX_UART_TX_BUFF
      if( NULL == (pBeaconIndBuf = (uint8_t *)OsalPort_malloc(MT_ZDO_BEACON_IND_PACK_LEN)))
      {
        // Memory failure
        return NULL;
      }
      pBeaconIndBuf[0] = 0; // First byte is devCnt. Initialize to 0.
    }

    // Fill in the buffer with the beacon indication
    pTmp = pBeaconIndBuf + (1 + pBeaconIndBuf[0] * MT_ZDO_BEACON_IND_LEN);
    *pTmp++ = LO_UINT16(pBeacon->sourceAddr);
    *pTmp++ = HI_UINT16(pBeacon->sourceAddr);
    *pTmp++ = LO_UINT16(pBeacon->panID);
    *pTmp++ = HI_UINT16(pBeacon->panID);
    *pTmp++ = pBeacon->logicalChannel;
    *pTmp++ = pBeacon->permitJoining;
    *pTmp++ = pBeacon->routerCapacity;
    *pTmp++ = pBeacon->deviceCapacity;
    *pTmp++ = pBeacon->protocolVersion;
    *pTmp++ = pBeacon->stackProfile;
    *pTmp++ = pBeacon->LQI;
    *pTmp++ = pBeacon->depth;
    *pTmp++ = pBeacon->updateID;
    OsalPort_memcpy( pTmp, pBeacon->extendedPanID, Z_EXTADDR_LEN);

    pBeaconIndBuf[0] += 1; // Increment the devCnt

    // Check if the buffer can fit in another beacon
    if( ((pBeaconIndBuf[0] + 1) * MT_ZDO_BEACON_IND_LEN + 1) > MT_ZDO_BEACON_IND_PACK_LEN )
    {
      // Packet full, send the packet over MT
      MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_ZDO),
                                   MT_ZDO_BEACON_NOTIFY_IND,
                                   (pBeaconIndBuf[0] * MT_ZDO_BEACON_IND_LEN + 1), pBeaconIndBuf);
      pBeaconIndBuf[0] = 0; // Reset the devCnt back to zero
    }
  }
  else
  {
    if( (pBeaconIndBuf != NULL) && (pBeaconIndBuf[0] != 0) )
    {
      // End of beacon indication, send the packet over MT
      MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_ZDO),
                                   MT_ZDO_BEACON_NOTIFY_IND,
                                   (pBeaconIndBuf[0] * MT_ZDO_BEACON_IND_LEN + 1), pBeaconIndBuf);
    }
    // Free the allocated memory
    if(pBeaconIndBuf != NULL)
    {
      OsalPort_free(pBeaconIndBuf);
      pBeaconIndBuf = NULL;
    }
  }

  return NULL;
}

/***************************************************************************************************
 * @fn          MT_ZdoJoinCnfCB
 *
 * @brief       Handle the ZDO Join Confirm from ZDO
 *
 * @param       pStr - pointer to a parameter and a structure of parameters
 *
 * @return      void
 ***************************************************************************************************/
void *MT_ZdoJoinCnfCB ( void *pStr )
{
  /* pStr: zdoJoinCnf_t* */
  /* Packet Format */
  /* Status (1) | device addr (2) | parent addr (2) */

  uint8_t buf[MT_ZDO_JOIN_CNF_LEN];
  zdoJoinCnf_t *joinCnf = pStr;

  /* Join Complete. De-register the callback with ZDO */
  ZDO_DeregisterForZdoCB( ZDO_JOIN_CNF_CBID );

  buf[0] = joinCnf->status;
  buf[1] = LO_UINT16( joinCnf->deviceAddr );
  buf[2] = HI_UINT16( joinCnf->deviceAddr );
  buf[3] = LO_UINT16( joinCnf->parentAddr );
  buf[4] = HI_UINT16( joinCnf->parentAddr );

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_ZDO),
                               MT_ZDO_JOIN_CNF, MT_ZDO_JOIN_CNF_LEN, buf);

  return NULL;
}

/*************************************************************************************************
 * @fn      MT_ZdoRegisterForZDOMsg(pBuf);
 *
 * @brief   MT proxy for ZDO_RegisterForZDOMsg.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 *************************************************************************************************/
void MT_ZdoRegisterForZDOMsg(uint8_t *pBuf)
{
  uint8_t cmd0, cmd1, tmp;
  uint16_t cId;

  /* parse header */
  cmd0 = pBuf[MT_RPC_POS_CMD0];
  cmd1 = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  cId = OsalPort_buildUint16( pBuf );
  tmp = ZDO_RegisterForZDOMsg(MT_TaskID, cId);

  if (MT_RPC_CMD_SREQ == (cmd0 & MT_RPC_CMD_TYPE_MASK))
  {
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP|(uint8_t)MT_RPC_SYS_ZDO), cmd1, 1, &tmp);
  }
}

/*************************************************************************************************
 * @fn      MT_ZdoRemoveRegisteredCB(pBuf);
 *
 * @brief   MT proxy for ZDO_RemoveRegisteredCB.
 *
 * @param   pBuf  - MT message data
 *
 * @return  void
 *************************************************************************************************/
void MT_ZdoRemoveRegisteredCB(uint8_t *pBuf)
{
  uint8_t cmd0, cmd1, tmp;
  uint16_t cId;

  /* parse header */
  cmd0 = pBuf[MT_RPC_POS_CMD0];
  cmd1 = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  cId = OsalPort_buildUint16( pBuf );
  tmp = ZDO_RemoveRegisteredCB(MT_TaskID, cId);

  if (MT_RPC_CMD_SREQ == (cmd0 & MT_RPC_CMD_TYPE_MASK))
  {
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP|(uint8_t)MT_RPC_SYS_ZDO), cmd1, 1, &tmp);
  }
}

#endif /* MT_ZDO_FUNC */


/***************************************************************************************************
 * Callback handling function
 ***************************************************************************************************/

#if defined (MT_ZDO_CB_FUNC)

/***************************************************************************************************
 * @fn      MT_ZdoStateChangeCB
 *
 * @brief   Handle state change OSAL message from ZDO.
 *
 * @param   pMsg  - Message data
 *
 * @return  void
 */
void MT_ZdoStateChangeCB(OsalPort_EventHdr *pMsg)
{
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_STATE_CHANGE_IND, 1, &pMsg->status);
}

/***************************************************************************************************
 * @fn     MT_ZdoDirectCB()
 *
 * @brief  ZDO direct callback.  Build an MT message directly from the
 *         over-the-air ZDO message.
 *
 * @param  pData - Incoming AF frame.
 *
 * @return  none
 ***************************************************************************************************/
void MT_ZdoDirectCB( afIncomingMSGPacket_t *pData, zdoIncomingMsg_t *inMsg )
{
  uint8_t len, *pBuf;
  uint16_t origClusterId;

  // save original value because MT_ZdoHandleExceptions() function could modify pData->clusterId
  origClusterId = pData->clusterId;

  // Is the message an exception or not a response?
  if ( MT_ZdoHandleExceptions( pData, inMsg ) || ( (origClusterId & ZDO_RESPONSE_BIT) == 0 ) )
  {
    return;  // Handled somewhere else or not needed.
  }

  /* ZDO data starts after one-byte sequence number and the msg buffer length includes
   * two bytes for srcAddr.
   */
  len = pData->cmd.DataLength - 1 + sizeof(uint16_t);

  if (NULL != (pBuf = (uint8_t *)OsalPort_malloc(len)))
  {
    uint8_t id = MT_ZDO_CID_TO_AREQ_ID(pData->clusterId);

    pBuf[0] = LO_UINT16(pData->srcAddr.addr.shortAddr);
    pBuf[1] = HI_UINT16(pData->srcAddr.addr.shortAddr);

    /* copy ZDO data, skipping one-byte sequence number */
    OsalPort_memcpy(pBuf+2, (pData->cmd.Data + 1), pData->cmd.DataLength-1);

    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_ZDO), id, len, pBuf);
    OsalPort_free(pBuf);
  }
}

/***************************************************************************************************
 * @fn     MT_ZdoHandleExceptions()
 *
 * @brief  Handles all messages that are an exception to the generic MT ZDO Response.
 *
 * @param  pData - Incoming AF frame.
 *
 * @return  TRUE if handled by this function, FALSE if not
 ***************************************************************************************************/
static uint8_t MT_ZdoHandleExceptions( afIncomingMSGPacket_t *pData, zdoIncomingMsg_t *inMsg )
{
  uint8_t ret = TRUE;
  ZDO_NwkIEEEAddrResp_t *nwkRsp;
  ZDO_DeviceAnnce_t devAnnce;
  uint8_t doDefault = FALSE;

  switch ( inMsg->clusterID )
  {
    case NWK_addr_rsp:
    case IEEE_addr_rsp:
      if ( NULL != (nwkRsp = ZDO_ParseAddrRsp(inMsg)) )
      {
        if ( nwkRsp->status == ZDO_SUCCESS )
        {
          MT_ZdoAddrRspCB( nwkRsp, inMsg->clusterID );
        }
        OsalPort_free( nwkRsp );
      }
      break;

    case Device_annce:
      ZDO_ParseDeviceAnnce( inMsg, &devAnnce );
      MT_ZdoEndDevAnnceCB( &devAnnce, inMsg->srcAddr.addr.shortAddr );
      break;

    case Simple_Desc_rsp:
      if ( pData->cmd.DataLength > 5 )
      {
        ret = FALSE;
      }
      else
      {
        doDefault = TRUE;
      }
      break;

    default:
      ret = FALSE;
      break;
  }

  if ( doDefault )
  {
    ret = FALSE;
    pData->clusterId = MtZdoDef_rsp;
    pData->cmd.DataLength = 2;
  }

  return ( ret );
}

/***************************************************************************************************
 * @fn      MT_ZdoAddrRspCB
 *
 * @brief   Handle IEEE or nwk address response OSAL message from ZDO.
 *
 * @param   pMsg  - Message data
 *
 * @return  void
 */
void MT_ZdoAddrRspCB( ZDO_NwkIEEEAddrResp_t *pMsg, uint16_t clusterID )
{
  uint8_t   listLen, len, *pBuf;

  /* both ZDO_NwkAddrResp_t and ZDO_IEEEAddrResp_t must be the same */

  /* get length, sanity check length */
  listLen = pMsg->numAssocDevs;

  /* calculate msg length */
  len = MT_ZDO_ADDR_RSP_LEN + (listLen * sizeof(uint16_t));

  /* get buffer */
  if (NULL != (pBuf = (uint8_t *)OsalPort_malloc(len)))
  {
    uint8_t id = MT_ZDO_CID_TO_AREQ_ID(clusterID);
    uint8_t *pTmp = pBuf;

    *pTmp++ = pMsg->status;

    osal_cpyExtAddr(pTmp, pMsg->extAddr);
    pTmp += Z_EXTADDR_LEN;

    *pTmp++ = LO_UINT16(pMsg->nwkAddr);
    *pTmp++ = HI_UINT16(pMsg->nwkAddr);

    *pTmp++ = pMsg->startIndex;
    *pTmp++ = listLen;

    MT_Word2Buf(pTmp, pMsg->devList, listLen);

    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_ZDO), id, len, pBuf);
    OsalPort_free(pBuf);
  }
}

/***************************************************************************************************
 * @fn      MT_ZdoEndDevAnnceCB
 *
 * @brief   Handle end device announce OSAL message from ZDO.
 *
 * @param   pMsg  - Message data
 *
 * @return  void
 */
void MT_ZdoEndDevAnnceCB( ZDO_DeviceAnnce_t *pMsg, uint16_t srcAddr )
{
  uint8_t *pBuf;

  if (NULL != (pBuf = (uint8_t *)OsalPort_malloc(MT_ZDO_END_DEVICE_ANNCE_IND_LEN)))
  {
    uint8_t *pTmp = pBuf;

    *pTmp++ = LO_UINT16(srcAddr);
    *pTmp++ = HI_UINT16(srcAddr);

    *pTmp++ = LO_UINT16(pMsg->nwkAddr);
    *pTmp++ = HI_UINT16(pMsg->nwkAddr);

    osal_cpyExtAddr(pTmp, pMsg->extAddr);
    pTmp += Z_EXTADDR_LEN;

    *pTmp = pMsg->capabilities;

    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_ZDO),
                                         MT_ZDO_END_DEVICE_ANNCE_IND,
                                         MT_ZDO_END_DEVICE_ANNCE_IND_LEN, pBuf);
    OsalPort_free(pBuf);
  }
}

/***************************************************************************************************
 * @fn      MT_ZdoSrcRtgCB
 *
 * @brief   Handle Src Route from ZDO.
 *
 * @param   pStr  - pointer to the data structure for the src route
 *
 * @return  void*
 */
void* MT_ZdoSrcRtgCB( void *pStr )
{
  uint8_t len, *pBuf;
  zdoSrcRtg_t *pSrcRtg = pStr;

  // srcAddr (2) + relayCnt (1) + relayList( relaycnt * 2 )
  len = 2 + 1 + pSrcRtg->relayCnt * sizeof(uint16_t);

  if (NULL != (pBuf = (uint8_t *)OsalPort_malloc(len)))
  {
    uint8_t idx, *pTmp = pBuf;
    uint16_t *pRelay;

    // Packet payload
    *pTmp++ = LO_UINT16(pSrcRtg->srcAddr);
    *pTmp++ = HI_UINT16(pSrcRtg->srcAddr);
    *pTmp++ = pSrcRtg->relayCnt;

    // Relay List
    if( ( pRelay = pSrcRtg->pRelayList ) != NULL )
    {
      for( idx = 0; idx < pSrcRtg->relayCnt; idx ++ )
      {
        *pTmp++ = LO_UINT16(*pRelay);
        *pTmp++ = HI_UINT16(*pRelay);
        pRelay++;
      }
    }
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_ZDO),
                                         MT_ZDO_SRC_RTG_IND, len, pBuf);
    OsalPort_free(pBuf);
  }

  return NULL;
}

/***************************************************************************************************
 * @fn          MT_ZdoConcentratorIndCB
 *
 * @brief       Handle the ZDO Concentrator Indication callback from the ZDO.
 *
 * @param       pStr - pointer to a parameter and a structure of parameters
 *
 * @return      NULL
 ***************************************************************************************************/
void *MT_ZdoConcentratorIndCB(void *pStr)
{
  uint8_t buf[MT_ZDO_CONCENTRATOR_IND_LEN], *pTmp = buf;
  zdoConcentratorInd_t *pInd = (zdoConcentratorInd_t *)pStr;

  *pTmp++ = LO_UINT16(pInd->nwkAddr);
  *pTmp++ = HI_UINT16(pInd->nwkAddr);
  pTmp = OsalPort_memcpy(pTmp, pInd->extAddr, Z_EXTADDR_LEN);
  *pTmp = pInd->pktCost;

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_ZDO),
                                    MT_ZDO_CONCENTRATOR_IND_CB, MT_ZDO_CONCENTRATOR_IND_LEN, buf);
  return NULL;
}

/***************************************************************************************************
 * @fn          MT_ZdoLeaveInd
 *
 * @brief       Handle the ZDO Leave Indication callback from the ZDO.
 *
 * @param       vPtr - Pointer to the received Leave Indication message.
 *
 * @return      NULL
 ***************************************************************************************************/
static void *MT_ZdoLeaveInd(void *vPtr)
{
  NLME_LeaveInd_t *pInd = (NLME_LeaveInd_t *)vPtr;
  uint8_t buf[sizeof(NLME_LeaveInd_t)];

  buf[0] = LO_UINT16(pInd->srcAddr);
  buf[1] = HI_UINT16(pInd->srcAddr);
  (void)OsalPort_memcpy(buf+2, pInd->extAddr, Z_EXTADDR_LEN);
  buf[2+Z_EXTADDR_LEN] = pInd->request;
  buf[3+Z_EXTADDR_LEN] = pInd->removeChildren;
  buf[4+Z_EXTADDR_LEN] = pInd->rejoin;

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_LEAVE_IND, 5+Z_EXTADDR_LEN, buf);
  return NULL;
}

/***************************************************************************************************
 * @fn          MT_ZdoTcDeviceInd
 *
 * @brief       Handle the ZDO TC Device Indication callback from the ZDO.
 *
 * @param       params - pointer to Trust Center joining device information
 *
 * @return      NULL
 ***************************************************************************************************/
void *MT_ZdoTcDeviceInd( void *params )
{
  ZDO_TC_Device_t *pDev = (ZDO_TC_Device_t *)params;
  uint8_t buf[12];

  buf[0] = LO_UINT16( pDev->nwkAddr );
  buf[1] = HI_UINT16( pDev->nwkAddr );
  (void)OsalPort_memcpy( &buf[2], pDev->extAddr, Z_EXTADDR_LEN);
  buf[2+Z_EXTADDR_LEN] = LO_UINT16( pDev->parentAddr );
  buf[3+Z_EXTADDR_LEN] = HI_UINT16( pDev->parentAddr );

  MT_BuildAndSendZToolResponse( ((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_TC_DEVICE_IND, 12, buf );

  return ( NULL );
}

/***************************************************************************************************
 * @fn          MT_ZdoPermitJoinInd
 *
 * @brief       Handle the ZDO Permit Join Indication callback from the ZDO.
 *
 * @param       duration - permit join duration
 *
 * @return      NULL
 ***************************************************************************************************/
void *MT_ZdoPermitJoinInd( void *duration )
{
  if ( ignoreIndication == FALSE )
  {
    // The following condition was moved here from NLME_PermitJoiningSet. It was removed there to
    // support calling the callback for every time a permit join command is processed, and let the
    // callback decide how to act.
    if ((( *(uint8_t*)duration == 0x00 ) && ( NLME_PermitJoining )) || (( *(uint8_t*)duration != 0x00 ) && ( ! NLME_PermitJoining )))
    {
      MT_BuildAndSendZToolResponse( ((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_PERMIT_JOIN_IND, 1, (uint8_t *)duration );
    }
  }

  return ( NULL );
}
#endif // MT_ZDO_CB_FUNC

/***************************************************************************************************
 * @fn      MT_ZdoSendMsgCB
 *
 * @brief   Proxy the ZDO_SendMsgCBs one message at a time.
 *
 * @param   pMsg  - Message data
 *
 * @return  void
 */
void MT_ZdoSendMsgCB(zdoIncomingMsg_t *pMsg)
{
  uint8_t len = pMsg->asduLen + 9;
  uint8_t *pBuf = (uint8_t *)OsalPort_malloc(len);

  if (pBuf != NULL)
  {
    uint8_t *pTmp = pBuf;

    // Assuming exclusive use of network short addresses.
    *pTmp++ = LO_UINT16(pMsg->srcAddr.addr.shortAddr);
    *pTmp++ = HI_UINT16(pMsg->srcAddr.addr.shortAddr);
    *pTmp++ = pMsg->wasBroadcast;
    *pTmp++ = LO_UINT16(pMsg->clusterID);
    *pTmp++ = HI_UINT16(pMsg->clusterID);
    *pTmp++ = pMsg->SecurityUse;
    *pTmp++ = pMsg->TransSeq;
    // Skipping asduLen since it can be deduced from the RPC packet length.
    *pTmp++ = LO_UINT16(pMsg->macDestAddr);
    *pTmp++ = HI_UINT16(pMsg->macDestAddr);
    (void)OsalPort_memcpy(pTmp, pMsg->asdu, pMsg->asduLen);

    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_ZDO),
                                         MT_ZDO_MSG_CB_INCOMING, len, pBuf);

    OsalPort_free(pBuf);
  }
}


#if defined ( MT_ZDO_EXTENSIONS )
#if ( ZG_BUILD_COORDINATOR_TYPE )
/***************************************************************************************************
 * @fn          MT_ZdoSecUpdateNwkKey
 *
 * @brief       Handle the ZDO Security Update Network Key extension message
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoSecUpdateNwkKey( uint8_t *pBuf )
{
  uint16_t dstAddr;
  uint8_t keySeqNum;
  uint8_t status;

  pBuf += MT_RPC_FRAME_HDR_SZ;

  dstAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;
  keySeqNum = *pBuf++;

  status = ZDSecMgrUpdateNwkKey( pBuf, keySeqNum, dstAddr );

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_EXT_UPDATE_NWK_KEY, 1, &status );
}

/***************************************************************************************************
 * @fn          MT_ZdoSecSwitchNwkKey
 *
 * @brief       Handle the ZDO Security Switch Network Key extension message
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoSecSwitchNwkKey( uint8_t *pBuf )
{
  uint16_t dstAddr;
  uint8_t keySeqNum;
  uint8_t status;

  pBuf += MT_RPC_FRAME_HDR_SZ;

  dstAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;
  keySeqNum = *pBuf++;

  status = ZDSecMgrSwitchNwkKey( keySeqNum, dstAddr );

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_EXT_SWITCH_NWK_KEY, 1, &status );
}
#endif // ZG_BUILD_COORDINATOR_TYPE

/***************************************************************************************************
 * @fn          MT_ZdoSecAddLinkKey
 *
 * @brief       Handle the ZDO Security Add Link Key extension message
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoSecAddLinkKey( uint8_t *pBuf )
{
  uint16_t shortAddr;
  uint8_t *pExtAddr;
  uint8_t status;

  pBuf += MT_RPC_FRAME_HDR_SZ;

  shortAddr = OsalPort_buildUint16( pBuf );
  pBuf += 2;
  pExtAddr = pBuf;
  pBuf += Z_EXTADDR_LEN;

  status = ZDSecMgrAddLinkKey( shortAddr, pExtAddr, pBuf );

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_SEC_ADD_LINK_KEY, 1, &status );
}

/***************************************************************************************************
 * @fn          MT_ZdoSecEntryLookupExt
 *
 * @brief       Handle the ZDO Security Entry Lookup Extended extension message
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoSecEntryLookupExt( uint8_t *pBuf )
{
  ZDSecMgrEntry_t *pEntry = NULL;
  uint8_t buf[6] = {0};

  pBuf += MT_RPC_FRAME_HDR_SZ;

  // lookup entry index for specified EXT address
  buf[0] = ZDSecMgrEntryLookupExt( pBuf, &pEntry );
  if ( pEntry )
  {
    buf[1] = LO_UINT16( pEntry->ami );
    buf[2] = HI_UINT16( pEntry->ami );
    buf[3] = LO_UINT16( pEntry->keyNvId );
    buf[4] = HI_UINT16( pEntry->keyNvId );
    buf[5] = (uint8_t)pEntry->authenticateOption;
  }

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_SEC_ENTRY_LOOKUP_EXT, 6, buf );
}

/***************************************************************************************************
 * @fn          MT_ZdoSecDeviceRemove
 *
 * @brief       Handle the ZDO Security Remove Device extension message
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoSecDeviceRemove( uint8_t *pBuf )
{
  ZStatus_t status;

  pBuf += MT_RPC_FRAME_HDR_SZ;

  // lookup entry index for specified EXT address
  status = ZDSecMgrDeviceRemoveByExtAddr( pBuf );

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_SEC_DEVICE_REMOVE, 1, &status );
}

/***************************************************************************************************
 * @fn          MT_ZdoExtRouteDisc
 *
 * @brief       Handle the ZDO Route Discovery extension message
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoExtRouteDisc( uint8_t *pBuf )
{
  ZStatus_t status;
  uint16_t dstAddr;

  pBuf += MT_RPC_FRAME_HDR_SZ;

  dstAddr = OsalPort_buildUint16( pBuf );

  status = NLME_RouteDiscoveryRequest( dstAddr, pBuf[2], pBuf[3] );


  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_EXT_ROUTE_DISC, 1, &status );
}

/***************************************************************************************************
 * @fn          MT_ZdoExtRouteCheck
 *
 * @brief       Handle the ZDO Route Check extension message
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoExtRouteCheck( uint8_t *pBuf )
{
  ZStatus_t status;
  uint16_t dstAddr;

  pBuf += MT_RPC_FRAME_HDR_SZ;

  dstAddr = OsalPort_buildUint16( pBuf );

  status = RTG_CheckRtStatus( dstAddr, pBuf[2], pBuf[3] );

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_EXT_ROUTE_CHECK, 1, &status );
}

/***************************************************************************************************
 * @fn          MT_ZdoExtRemoveGroup
 *
 * @brief       Handle the ZDO extension Remove Group message
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoExtRemoveGroup( uint8_t *pBuf )
{
  ZStatus_t status;
  uint8_t endpoint;
  uint16_t groupID;

  pBuf += MT_RPC_FRAME_HDR_SZ;

  endpoint = *pBuf++;
  groupID = OsalPort_buildUint16( pBuf );

  if ( aps_RemoveGroup( endpoint, groupID ) )
  {
    status = ZSuccess;
  }
  else
  {
    status = ZFailure;
  }

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_EXT_REMOVE_GROUP, 1, &status );
}

/***************************************************************************************************
 * @fn          MT_ZdoExtRemoveAllGroup
 *
 * @brief       Handle the ZDO extension Remove All Groups message
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoExtRemoveAllGroup( uint8_t *pBuf )
{
  ZStatus_t status = ZSuccess;

  pBuf += MT_RPC_FRAME_HDR_SZ;

  aps_RemoveAllGroup( *pBuf );

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_EXT_REMOVE_GROUP, 1, &status );
}

/***************************************************************************************************
 * @fn          MT_ZdoExtFindAllGroupsEndpoint
 *
 * @brief       Handle the ZDO extension Find All Groups for Endpoint message
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoExtFindAllGroupsEndpoint( uint8_t *pBuf )
{
  uint16_t groupList[ APS_MAX_GROUPS ];
  uint8_t groups;
  uint8_t msgLen;
  uint8_t *pMsg;

  pBuf += MT_RPC_FRAME_HDR_SZ;

  groups = aps_FindAllGroupsForEndpoint( *pBuf, groupList );

  msgLen = 1 + (2 * groups);
  pMsg = OsalPort_malloc( msgLen );
  if ( pMsg )
  {
    uint8_t x;
    uint8_t *pBuf = pMsg;

    *pBuf++ = groups;
    for ( x = 0; x < groups; x++ )
    {
      *pBuf++ = LO_UINT16( groupList[x] );
      *pBuf++ = HI_UINT16( groupList[x] );
    }

    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_EXT_FIND_ALL_GROUPS_ENDPOINT, msgLen, pMsg );
    OsalPort_free( pMsg );
  }
  else
  {
    groups = 0;
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_EXT_FIND_ALL_GROUPS_ENDPOINT, 1, &groups );
  }
}

/***************************************************************************************************
 * @fn          MT_ZdoExtFindGroup
 *
 * @brief       Handle the ZDO extension Find Group message
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoExtFindGroup( uint8_t *pBuf )
{
  uint8_t endpoint;
  uint16_t groupID;
  aps_Group_t *pGroup;
  uint8_t buf[1+2+APS_GROUP_NAME_LEN] = {0};

  pBuf += MT_RPC_FRAME_HDR_SZ;

  endpoint = *pBuf++;
  groupID = OsalPort_buildUint16( pBuf );

  pGroup = aps_FindGroup( endpoint, groupID );
  if ( pGroup  )
  {
    buf[0] = ZSuccess;
    buf[1] = LO_UINT16( pGroup->ID );
    buf[2] = HI_UINT16( pGroup->ID );
    buf[3] = pGroup->name[0];
    OsalPort_memcpy( &buf[4], &pGroup->name[1], buf[3] );
  }
  else
  {
    buf[0] = ZFailure;
  }

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                      MT_ZDO_EXT_FIND_GROUP, (1+2+APS_GROUP_NAME_LEN), buf );
}

/***************************************************************************************************
 * @fn          MT_ZdoExtAddGroup
 *
 * @brief       Handle the ZDO extension Add Group message
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoExtAddGroup( uint8_t *pBuf )
{
  ZStatus_t status = ZSuccess;
  aps_Group_t group = {0};
  uint8_t endpoint;

  pBuf += MT_RPC_FRAME_HDR_SZ;

  endpoint = *pBuf++;
  group.ID = OsalPort_buildUint16( pBuf );
  group.name[0] = pBuf[2];
  if ( group.name[0] > (APS_GROUP_NAME_LEN-1) )
  {
    group.name[0] = (APS_GROUP_NAME_LEN-1);
  }

  if (group.name[0] > 0 )
  {
    OsalPort_memcpy( &group.name[1], &pBuf[3], group.name[0] );
  }

  status = aps_AddGroup( endpoint, &group, true );

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_EXT_ADD_GROUP, 1, &status );
}

/***************************************************************************************************
 * @fn          MT_ZdoExtCountAllGroups
 *
 * @brief       Handle the ZDO extension Count All Groups message
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoExtCountAllGroups( uint8_t *pBuf )
{
  ZStatus_t status = 0;

  pBuf += MT_RPC_FRAME_HDR_SZ;

  status = (ZStatus_t)aps_CountAllGroups();

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_EXT_COUNT_ALL_GROUPS, 1, &status );
}


/***************************************************************************************************
 * @fn          MT_ZdoExtRxIdle
 *
 * @brief       Handle the ZDO extension Get/Set RxOnIdle to ZMac message
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoExtRxIdle( uint8_t *pBuf )
{
  uint8_t setFlag;
  uint8_t setValue;

  pBuf += MT_RPC_FRAME_HDR_SZ;

  setFlag = *pBuf++;
  setValue = *pBuf++;

  if ( setFlag == MT_ZDO_EXT_RX_IDLE_SET )
  {
    ZMacSetReq( ZMacRxOnIdle, &setValue );
  }
  else if ( setFlag == MT_ZDO_EXT_RX_IDLE_RX_ON_CONFIG )
  {
    ZMacSetReq( ZMacRxOnIdle, &setValue );
#if ( RFD_RX_ALWAYS_ON_CAPABLE == TRUE )
    zgRxAlwaysOn = TRUE;
#endif
  }
  else if ( setFlag == MT_ZDO_EXT_RX_IDLE_SLEEPY_CONFIG )
  {
    ZMacSetReq( ZMacRxOnIdle, &setValue );
    zgRxAlwaysOn = FALSE;
  }
  else
  {
    ZMacGetReq( ZMacRxOnIdle, &setValue );
  }

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_EXT_RX_IDLE, 1, &setValue );
}

/***************************************************************************************************
 * @fn          MT_ZdoExtNwkInfo
 *
 * @brief       Handle the ZDO extension Network Info Request message
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoExtNwkInfo( uint8_t *pBuf )
{
  uint8_t buf[24];
  uint8_t *pMsg;

  pMsg = buf;

  *pMsg++ = LO_UINT16( _NIB.nwkDevAddress );
  *pMsg++ = HI_UINT16( _NIB.nwkDevAddress );

  *pMsg++ = devState;
  *pMsg++ = LO_UINT16( _NIB.nwkPanId );
  *pMsg++ = HI_UINT16( _NIB.nwkPanId );
  *pMsg++ = LO_UINT16( _NIB.nwkCoordAddress );
  *pMsg++ = HI_UINT16( _NIB.nwkCoordAddress );
  OsalPort_memcpy( pMsg, _NIB.extendedPANID, 8 );
  pMsg += 8;
  OsalPort_memcpy( pMsg, _NIB.nwkCoordExtAddress, 8 );
  pMsg += 8;
  *pMsg++ = _NIB.nwkLogicalChannel;

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_EXT_NWK_INFO, 24, buf );
}

/***************************************************************************************************
 * @fn          MT_ZdoExtSecApsRemoveReq
 *
 * @brief       Handle the ZDO extension Security Manager APS Remove Request message
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoExtSecApsRemoveReq( uint8_t *pBuf )
{
  ZStatus_t status = 0;
  uint16_t parentAddr;
  uint16_t nwkAddr;
  uint8_t *extAddr;

  pBuf += MT_RPC_FRAME_HDR_SZ;

  if ( ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE )
  {
    nwkAddr = OsalPort_buildUint16( pBuf );
    pBuf += 2;

    extAddr = pBuf;
    pBuf += Z_EXTADDR_LEN;

    parentAddr = OsalPort_buildUint16( pBuf );

    status = ZDSecMgrAPSRemove( nwkAddr, extAddr, parentAddr );
  }
  else
  {
    status = ZUnsupportedMode;
  }

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_EXT_SEC_APS_REMOVE_REQ, 1, &status );
}

/***************************************************************************************************
 * @fn          MT_ZdoExtSetParams
 *
 * @brief       Set Parameters not settable through NV.
 *
 * @param       pBuf - Pointer to the received message data.
 *
 * @return      NULL
 ***************************************************************************************************/
static void MT_ZdoExtSetParams( uint8_t *pBuf )
{
  ZStatus_t status = ZSuccess;
  uint8_t useMultiCast;

  pBuf += MT_RPC_FRAME_HDR_SZ;

  // Is the useMulticast in this message
  useMultiCast = *pBuf++;
  if ( useMultiCast & 0x80 )
  {
    if ( useMultiCast & 0x7F )
    {
      _NIB.nwkUseMultiCast = TRUE;
    }
    else
    {
      _NIB.nwkUseMultiCast = FALSE;
    }
  }

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_ZDO),
                                       MT_ZDO_EXT_SET_PARAMS, 1, &status );
}
#endif // MT_ZDO_EXTENSIONS

#endif   /*ZDO Command Processing in MT*/
/***************************************************************************************************
***************************************************************************************************/
