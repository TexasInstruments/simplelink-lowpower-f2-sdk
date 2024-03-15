/**
 @file  zstacktask.c
 @brief ZStack Thread implementation

 <!--
 Copyright 2014 - 2015 Texas Instruments Incorporated.

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
 -->
 */

#include "ti_zstack_config.h"
#include "rom_jt_154.h"
#include "osal_nv.h"
#include "zglobals.h"
#include "af.h"
#include "zd_config.h"
#include "zd_profile.h"
#include "zd_object.h"
#include "zd_app.h"
#include "zd_sec_mgr.h"
#include "rtg.h"
#include "nwk.h"
#include "nwk_util.h"
#include "aps_groups.h"
#include "zcl.h"

#include "zstack.h"
#include "zstackmsg.h"
#include "zstacktask.h"
#include "zsversion.h"

#include "bdb.h"
#include <driverlib/sys_ctrl.h>
#include "gp_common.h"
#include "cgp_stub.h"
#include "dgp_stub.h"
#include "bdb_reporting.h"

#if defined OTA_SERVER
#include "zcl_ota.h"
#include "mt_ota.h"
#include "ota_common.h"
#endif

#if defined (NPI)
#include "mt.h"
#include "mt_app.h"
#include "mt_rpc.h"
#include "mt_task.h"
#endif

// Added to include TouchLink initiator functionality
#if defined ( BDB_TL_INITIATOR )
  #include "bdb_touchlink_initiator.h"
#endif // BDB_TL_INITIATOR

#if defined ( BDB_TL_TARGET )
  #include "bdb_touchlink_target.h"
#endif // BDB_TL_TARGET

#if defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  #include "bdb_touchlink.h"
#endif

#include "ti_zstack_config.h"

/* ------------------------------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------------------------------
 */

#define ZS_START_EVENT     0x0001

#define ZS_ZDO_SRC_RTG_IND_CBID             0x0001
#define ZS_ZDO_CONCENTRATOR_IND_CBID        0x0002
#define ZS_ZDO_NWK_DISCOVERY_CNF_CBID       0x0004
#define ZS_ZDO_BEACON_NOTIFY_IND_CBID       0x0008
#define ZS_ZDO_JOIN_CNF_CBID                0x0010
#define ZS_ZDO_LEAVE_CNF_CBID               0x0020
#define ZS_ZDO_LEAVE_IND_CBID               0x0040

#define ZS_ZDO_NWK_ADDR_RSP_CDID            0x00000001
#define ZS_ZDO_IEEE_ADDR_RSP_CDID           0x00000002
#define ZS_ZDO_NODE_DESC_RSP_CDID           0x00000004
#define ZS_ZDO_POWER_DESC_RSP_CDID          0x00000008
#define ZS_ZDO_SIMPLE_DESC_RSP_CDID         0x00000010
#define ZS_ZDO_ACTIVE_EP_RSP_CDID           0x00000020
#define ZS_ZDO_MATCH_DESC_RSP_CDID          0x00000040
#define ZS_ZDO_COMPLEX_DESC_RSP_CDID        0x00000080
#define ZS_ZDO_USER_DESC_RSP_CDID           0x00000100
#define ZS_ZDO_DISCOVERY_CACHE_RSP_CDID     0x00000200
#define ZS_ZDO_USER_DESC_CONF_CDID          0x00000400
#define ZS_ZDO_SERVER_DISCOVERY_RSP_CDID    0x00000800
/* can be used 0x00001000 */
#define ZS_ZDO_BIND_RSP_CDID                0x00002000
#define ZS_ZDO_END_DEVICE_BIND_RSP_CDID     0x00004000
#define ZS_ZDO_UNBIND_RSP_CDID              0x00008000
#define ZS_ZDO_MGMT_NWK_DISC_RSP_CDID       0x00010000
#define ZS_ZDO_MGMT_LQI_RSP_CDID            0x00020000
#define ZS_ZDO_MGMT_RTG_RSP_CDID            0x00040000
#define ZS_ZDO_MGMT_BIND_RSP_CDID           0x00080000
#define ZS_ZDO_MGMT_LEAVE_RSP_CDID          0x00100000
#define ZS_ZDO_MGMT_DIRECT_JOIN_RSP_CDID    0x00200000
#define ZS_ZDO_MGMT_PERMIT_JOIN_RSP_CDID    0x00400000
#define ZS_ZDO_MGMT_NWK_UPDATE_NOTIFY_CDID  0x00800000
#define ZS_ZDO_DEVICE_ANNOUNCE_CDID         0x01000000
#define ZS_DEV_STATE_CHANGE_CDID            0x02000000
// 0x04000000 is available for use. Was used for ZS_DEV_JAMMER_IND_CDID
#define ZS_TC_DEVICE_IND_CDID               0x08000000
#define ZS_DEV_PERMIT_JOIN_IND_CDID         0x10000000

/* Capability Information */
#define CAPABLE_PAN_COORD       0x01  /* Device is capable of becoming a PAN
                                        coordinator */
#define CAPABLE_FFD             0x02  /* Device is an FFD */
#define CAPABLE_MAINS_POWER     0x04  /* Device is mains powered rather than
                                        battery powered */
#define CAPABLE_RX_ON_IDLE      0x08  /* Device has its receiver on when idle
                                        */
#define CAPABLE_SECURITY        0x40  /* Device is capable of sending and
                                        receiving secured frames */
#define CAPABLE_ALLOC_ADDR      0x80  /* Request allocation of a short address
                                        in the associate procedure */

/* ------------------------------------------------------------------------------------------------
 * Typedefs
 * ------------------------------------------------------------------------------------------------
 */

typedef struct epItem_s
{
  struct epItem_s *next;     // Next in the link List
  uint8_t connection;        // connection;
  uint16_t zdoCBs;
  uint32_t zdoRsps;
  endPointDesc_t epDesc;
} epItem_t;

// This is also defined in ZDSecMgr.c
typedef struct
{
  uint16_t ami;
  uint16_t keyNvId;   // index to the Link Key table in NV
  ZDSecMgr_Authentication_Option authenticateOption;
} ZDSecMgrEntry_t;

typedef void (*pfnZDOCB)( uint16_t dstID, void *pStr );
typedef void (*pfnZDOMsgCB)( uint16_t dstID, uint16_t srcAddr, void *pStr );

/* ------------------------------------------------------------------------------------------------
 * Global Variables
 * ------------------------------------------------------------------------------------------------
 */
uint8_t maxSupportedEndpoints = MAX_SUPPORTED_ENDPOINTS;

uint8_t zspbPermitJoin = TRUE;
uint8_t nwkUseMultiCast = FALSE;

/* ------------------------------------------------------------------------------------------------
 * Local Variables
 * ------------------------------------------------------------------------------------------------
 */
static uint8_t ZStackServiceTaskId;

static epItem_t *pEpTableHdr = NULL;

#ifdef NPI
static uint8_t mtTaskID;
#endif

#ifndef ZNP_NPI
static devStates_t newDevState = DEV_INIT;
#endif



/* ------------------------------------------------------------------------------------------------
 * External Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */
extern ZStatus_t ZDSecMgrEntryLookupExt( uint8_t *extAddr, ZDSecMgrEntry_t **entry );
extern void ZDApp_NodeProfileSync( uint8_t stackProfile );

uint8_t ZStackTask_getServiceTaskID(void);

/* ------------------------------------------------------------------------------------------------
 * Local Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */
#ifdef NPI
static void StackTask_handleNPIReq( void *pMsg );
static void processNpiIncomingMsgInd( uint8_t *pkt );
static void processSysAppMsgInd( mtSysAppMsg_t *pkt );
#if defined OTA_SERVER
static void processSysOtaMsgInd( OTA_MtMsg_t *pkt );
#endif // OTA_SERVER
#endif // NPI

#ifndef ZNP_NPI // znp does not need any of these APIs since it uses MT
static bool appMsg( uint8_t* pMsg );
static void sendMsgToAllCBs( uint16_t cbMask, void *pBuf, pfnZDOCB pFn );
static void sendMsgToAllCBMsgs( uint32_t cbMask, uint16_t srcAddr, void *pBuf, pfnZDOMsgCB pFn );
static void zsProcessZDOMsgs( zdoIncomingMsg_t *inMsg );

static void processAfDataConfirm( afDataConfirm_t *pkt );
static void processAfIncomingMsgInd( afIncomingMSGPacket_t *pkt );
static void processAfReflectErrorInd( afReflectError_t *pkt );

static uint8_t epTableAddNewEntry( epItem_t *newEntry );
static epItem_t *epTableFindEntryConnection( int connection );
static void epTableRemoveEntry( epItem_t *entry );
static uint8_t epTableNumEntries( void );
static void freeEpItem( epItem_t *pItem );

static void *zdoNwkDiscCnfCB( void *pStr );
static void *zdoBeaconNotifyIndCB( void *pStr );
static void *zdoSrcRtgCB( void *pStr );
static void *zdoConcentratorIndCB( void *pStr );
static void *zdoJoinCnfCB( void *pStr );
static void *zdoLeaveIndCB( void *pStr );
static void *zdoPermitJoinIndCB( void *pStr );
static void *zdoTcDeviceIndCB( void *pStr );

static void sendNwkDiscCnf( uint16_t dstID, void *pStr );
static void sendLeaveInd( uint16_t dstID, void *pStr );
static void sendJoinCnfInd( uint16_t dstID, void *pStr );
static void sendBeaconNotifyInd( uint16_t dstID, void *pStr );
static void sendZdoConcentratorInd( uint16_t dstID, void *pStr );
static void sendZdoSrcRtgInd( uint16_t dstID, void *pStr );
static void sendTcDeviceInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
static void sendDevPermitJoinInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
static void sendDeviceAnnounce( uint16_t srcAddr, void *pMsg );
static void sendDeviceAnnounceInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );

#if defined (ZDO_NWKADDR_REQUEST)
static void sendNwkAddrRsp( uint16_t srcAddr, void *pMsg );
static void sendNwkAddrRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_IEEEADDR_REQUEST)
static void sendIeeeAddrRsp( uint16_t srcAddr, void *pMsg );
static void sendIeeeAddrRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_NODEDESC_REQUEST)
static void sendNodeDescRsp( uint16_t srcAddr, void *pMsg );
static void sendNodeDescRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_POWERDESC_REQUEST)
static void sendPowerDescRsp( uint16_t srcAddr, void *pMsg );
static void sendPowerDescRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_SIMPLEDESC_REQUEST)
static void sendSimpleDescRsp( uint16_t srcAddr, void *pMsg );
static void sendSimpleDescRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_ACTIVEEP_REQUEST)
static void sendActiveEPRsp( uint16_t srcAddr, void *pMsg );
static void sendActiveEPRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_MATCH_REQUEST)
static void sendMatchDescRsp( uint16_t srcAddr, void *pMsg );
static void sendMatchDescRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_USERDESC_REQUEST)
static void sendUserDescRsp( uint16_t srcAddr, void *pMsg );
static void sendUserDescRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_SERVERDISC_REQUEST)
static void sendServerDiscRsp( uint16_t srcAddr, void *pMsg );
static void sendServerDiscRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_BIND_UNBIND_REQUEST)
static void sendBindRsp( uint16_t srcAddr, void *pMsg );
static void sendBindRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_BIND_UNBIND_REQUEST)
static void sendUnbindRsp( uint16_t srcAddr, void *pMsg );
static void sendUnbindRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_ENDDEVICEBIND_REQUEST)
static void sendEndDeviceBindRsp( uint16_t srcAddr, void *pMsg );
static void sendEndDeviceBindRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_MGMT_NWKDISC_REQUEST)
static void sendMgmtNwkDiscRsp( uint16_t srcAddr, void *pMsg );
static void sendMgmtNwkDiscRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_MGMT_LQI_REQUEST)
static void sendMgmtLqiRsp( uint16_t srcAddr, void *pMsg );
static void sendMgmtLqiRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_MGMT_RTG_REQUEST)
static void sendMgmtRtgRsp( uint16_t srcAddr, void *pMsg );
static void sendMgmtRtgRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_MGMT_BIND_REQUEST)
static void sendMgmtBindRsp( uint16_t srcAddr, void *pMsg );
static void sendMgmtBindRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_MGMT_LEAVE_REQUEST)
static void sendMgmtLeaveRsp( uint16_t srcAddr, void *pMsg );
static void sendMgmtLeaveRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_MGMT_JOINDIRECT_REQUEST)
static void sendMgmtDirectJoinRsp( uint16_t srcAddr, void *pMsg );
static void sendMgmtDirectJoinRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_MGMT_PERMIT_JOIN_REQUEST)
static void sendMgmtPermitJoinRsp( uint16_t srcAddr, void *pMsg );
static void sendMgmtPermitJoinRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

#if defined (ZDO_MGMT_NWKUPDATE_REQUEST)
static void sendMgmtNwkUpdateNotify( uint16_t srcAddr, void *pMsg );
static void sendMgmtNwkUpdateNotifyInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );
#endif

static void processDevStateChange( uint16_t srcAddr, void *pMsg );
static void sendDevStateChangeInd( uint16_t dstID, uint16_t srcAddr, void *pMsg );

static bool processSysVersionReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processDevStartReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processSysSetTxPowerReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processDevJoinReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processSysResetReq( uint8_t srcServiceTaskId, void *pMsg );// Hector - sysReset
static bool processDevForceNetworkSettingsReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processDevForceNetworkUpdateReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processDevForceMacParamsReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processDevUpdateNeighborTxCostReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processApsRemoveGroup( uint8_t srcServiceTaskId, void *pMsg );
static bool processApsRemoveAllGroups( uint8_t srcServiceTaskId, void *pMsg );
static bool processApsFindAllGroupsReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processApsFindGroupReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processApsAddGroup( uint8_t srcServiceTaskId, void *pMsg );
static bool processApsCountAllGroups( uint8_t srcServiceTaskId, void *pMsg );
static bool processSecApsRemoveReq( uint8_t srcServiceTaskId, void *pMsg );

static bool processSecNwkKeyUpdateReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processSecNwkKeySwitchReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processSecNwkKeySetReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processSecNwkKeyGetReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processSecApsLinkKeyGetReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processSecApsLinkKeySetReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processSecApsLinkKeyRemoveReq( uint8_t srcServiceTaskId, void *pMsg );

static bool processSecMgrSetupPartner( uint8_t srcServiceTaskId, void *pMsg );
static bool processSecMgrAppKeyTypeSet( uint8_t srcServiceTaskId, void *pMsg );
static bool processSecMgrAppKeyReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processDevNwkManagerSet( uint8_t srcServiceTaskId, void *pMsg );

#if defined (RTR_NWK)
static bool processDevNwkRouteReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processDevNwkCheckRouteReq( uint8_t srcServiceTaskId, void *pMsg );
#endif // RTR_NWK

static bool processSetNwkFrameFwdNoticationReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processDevNwkDiscoveryReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processSysForceLinkStatusReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processSysConfigReadReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processSysConfigWriteReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processSysNwkInfoReadReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processDevZDOCBReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processAfRegisterReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processAfUnRegisterReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processAfConfigGetReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processAfConfigSetReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processAfDataReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processZdoDeviceAnnounceReq( uint8_t srcServiceTaskId, void *pMsg );

#if defined (ZDO_NWKADDR_REQUEST)
static bool processZdoNwkAddrReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_IEEEADDR_REQUEST)
static bool processZdoIeeeAddrReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_NODEDESC_REQUEST)
static bool processZdoNodeDescReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_POWERDESC_REQUEST)
static bool processZdoPowerDescReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_SIMPLEDESC_REQUEST)
static bool processZdoSimpleDescReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_ACTIVEEP_REQUEST)
static bool processZdoActiveEndpointsReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_MATCH_REQUEST)
static bool processZdoMatchDescReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_COMPLEXDESC_REQUEST)
static bool processZdoComplexDescReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_SERVERDISC_REQUEST)
static bool processZdoServerDiscReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_ENDDEVICEBIND_REQUEST)
static bool processZdoEndDeviceBindReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_BIND_UNBIND_REQUEST)
static bool processZdoBindReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_BIND_UNBIND_REQUEST)
static bool processZdoUnbindReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_MGMT_NWKDISC_REQUEST)
static bool processZdoMgmtNwkDiscReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_MGMT_LQI_REQUEST)
static bool processZdoMgmtLqiReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_MGMT_RTG_REQUEST)
static bool processZdoMgmtRtgReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_MGMT_BIND_REQUEST)
static bool processZdoMgmtBindReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_MGMT_LEAVE_REQUEST)
static bool processZdoMgmtLeaveReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_MGMT_JOINDIRECT_REQUEST)
static bool processZdoMgmtDirectJoinReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_MGMT_PERMIT_JOIN_REQUEST)
static bool processZdoMgmtPermitJoinReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_MGMT_NWKUPDATE_REQUEST)
static bool processZdoMgmtNwkUpdateReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_USERDESCSET_REQUEST)
static bool processZdoUserDescSetReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (ZDO_USERDESC_REQUEST)
static bool processZdoUserDescReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

static bool processZdoSetBindUnbindAuthAddrReq( uint8_t srcServiceTaskId, void *pMsg );

#if ( BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE )
static bool processTlScanReqInd( uint8_t srcServiceTaskId, void *pMsg );
static bool processTouchlinkNetworkJoinReqInd( uint8_t srcServiceTaskId, void *pMsg );
static bool processTouchlinkNetworkUpdateReqInd( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if defined (BDB_TL_TARGET)
#if (ZSTACK_ROUTER_BUILD)
static bool processTlTargetNetworkStartReqInd( uint8_t srcServiceTaskId, void *pMsg );
#endif
static bool processTlTargetResetToFactoryNewReqInd( uint8_t srcServiceTaskId );
#endif

#if defined ( BDB_TL_INITIATOR )
static bool processTlGetScanBaseTime( uint8_t srcServiceTaskId, void *pMsg );
static bool processTlInitiatorScanRspInd( uint8_t srcServiceTaskId, void *pMsg );
static bool processTlInitiatorDevInfoRspInd( uint8_t srcServiceTaskId, void *pMsg );
static bool processTlInitiatorNwkStartRspInd( uint8_t srcServiceTaskId, void *pMsg );
static bool processTlInitiatorNwkJoinRspInd( uint8_t srcServiceTaskId, void *pMsg );
#endif

static bool processBdbStartCommissioningReq( uint8_t srcServiceTaskId, void *pMsg );

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
static bool processBdbSetIdentifyActiveEndpointReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processBdbGetIdentifyActiveEndpointReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processBdbStopInitiatorFindingBindingReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

static bool processBdbZclIdentifyCmdIndReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processBdbSetEpDescListToActiveEndpoint( uint8_t srcServiceTaskId, void *pMsg );
static bool processBdbResetLocalActionReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processBdbSetAttributesReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processBdbGetAttributesReq( uint8_t srcServiceTaskId, void *pMsg );

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED == 1)
static bool processBdbGetFBInitiatorStatusReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

static bool processBdbGenerateInstallCodeCRCReq( uint8_t srcServiceTaskId, void *pMsg );

#ifdef BDB_REPORTING
static bool processBdbRepAddAttrCfgRecordDefaultToListReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processBdbRepChangedAttrValueReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processBdbProcessInConfigReportReq(uint8_t srcServiceTaskId, void *pMsg);
static bool processBdbProcessInReadReportCfgReq(uint8_t srcServiceTaskId, void *pMsg);
#endif

static bool processBdbAddInstallCodeReq( uint8_t srcServiceTaskId, void *pMsg );

#if (ZG_BUILD_JOINING_TYPE)
static bool processBdbSetActiveCentralizedLinkKeyReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processBdbCBKETCLinkKeyExchangeAttemptReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processBdbNwkDescFreeReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#ifdef BDB_TL_TARGET
static bool processBdbTouchlinkSetAllowStealingReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processBdbTouchlinkGetAllowStealingReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processBdbTouchLinkTargetEnableCommissioningReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processBdbTouchLinkTargetDisableCommissioningReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processBdbTouchLinkTargetGetTimerReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if (ZG_BUILD_JOINING_TYPE)
static bool processBdbRecoverNwkReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if (ZG_BUILD_JOINING_TYPE)
static bool processBdbFilterNwkDescCompleteReq( uint8_t srcServiceTaskId, void *pMsg );
#endif

#if (ZG_BUILD_RTR_TYPE)
static void sendNwkFrameFwdNotificationCBMsg( nwkFrameFwdNotification_t *nwkFrameFwdNotification );
#endif
static void BDBsendMsgToAllCBs(bdbCommissioningModeMsg_t* bdbCommissioningModeMsg);
#if (ZG_BUILD_COORDINATOR_TYPE)
static void BDBTCLinkKeyExchangeNotificationsendMsgToAllCB(bdb_TCLinkKeyExchProcess_t *bdb_TCLinkKeyExchProcess);
#endif

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
static void gpDataIndMsgSend(gp_DataInd_t* gpDataInd);
static void gpSecRecMsgSend(gp_SecReq_t* gpSecReq);
static bool processGpSecRsp(uint8_t srcServiceTaskId, void *pMsg);
static void gpCheckAnnceMsgSend(uint8_t *ieee, uint16_t nwkAddr);
static bool processGpAddressConfict(uint8_t srcServiceTaskId, void *pMsg);
#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
static bool processGpDecryptDataInd(uint8_t srcServiceTaskId, void *pMsg);
static bool processGpEncryptDecryptCommissioningKey(uint8_t srcServiceTaskId, void *pMsg);
static bool processGpCommissioningSuccess(uint8_t srcServiceTaskId, void *pMsg);
static bool processGpSendDeviceAnnounce(uint8_t srcServiceTaskId, void *pMsg);
#endif // !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
#endif // (ENABLE_GREENPOWER_COMBO_BASIC)(ENABLE_GREENPOWER_COMBO_BASIC)

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED == 1)
static void BDBIdentifyTimesendMsgToAllCBs(uint8_t endpoint);
static void BDBBindNotificationsendMsgToAllCBs(bdbBindNotificationData_t *bindData);
#endif

#if defined ( BDB_TL_TARGET )
static void BDBTouchLinkTargetEnablesendMsgToAllCB(uint8_t enable);
#endif

#if (ZG_BUILD_JOINING_TYPE)
static void BDBCBKETCLinkKeyExchangesendMsgCB(void);
static void BDBFilterNwkDescriptorsendMsgCB(networkDesc_t *pBDBListNwk, uint8_t count);
#endif

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
static void gp_CommissioningModeInd(bool isEntering, bool hasTime, uint16_t time);
static bool processGpAllowChannelChangeReq(uint8_t srcServiceTaskId, void *pMsg);
#endif

static bool isDevicePartOfNetwork( void );
static bool processGetZCLFrameCounterReq( uint8_t srcServiceTaskId, void *pMsg );
static bool processPauseResumeDeviceReq(uint8_t srcServiceTaskId, void *pMsg);

#endif // ZNP_NPI

static epItem_t *epTableFindEntryEP( uint8_t ep );

/**************************************************************************************************
 * @fn          ZStackTaskInit
 *
 * @brief       This function is called when OSAL is initialized.
 *
 * input parameters
 *
 * @param       taskId - OSAL task ID for ZStack Thread
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
void ZStackTaskInit( uint8_t taskId )
{
  ZStackServiceTaskId = taskId;

#ifdef NPI
  // potential race condition: MT Task must be higher OSAL task priority
  // so the MT task is registered before this Init() function is called
  mtTaskID = MTTask_getServiceTaskId();
#endif // NPI

#ifndef ZNP_NPI
  // Register for ZDO Function Callbacks
  ZDO_RegisterForZdoCB( ZDO_SRC_RTG_IND_CBID, zdoSrcRtgCB );
  ZDO_RegisterForZdoCB( ZDO_CONCENTRATOR_IND_CBID, zdoConcentratorIndCB );
  ZDO_RegisterForZdoCB( ZDO_JOIN_CNF_CBID, zdoJoinCnfCB );
  ZDO_RegisterForZdoCB( ZDO_LEAVE_IND_CBID, zdoLeaveIndCB );
  ZDO_RegisterForZdoCB( ZDO_PERMIT_JOIN_CBID, zdoPermitJoinIndCB );
  ZDO_RegisterForZdoCB( ZDO_TC_DEVICE_CBID, zdoTcDeviceIndCB );

  // Register for ZDO Rsp messages
  ZDO_RegisterForZDOMsg( ZStackServiceTaskId, ZDO_ALL_MSGS_CLUSTERID );

  bdb_RegisterCommissioningStatusCB(BDBsendMsgToAllCBs);
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED == 1)
  bdb_RegisterIdentifyTimeChangeCB(BDBIdentifyTimesendMsgToAllCBs);
  bdb_RegisterBindNotificationCB(BDBBindNotificationsendMsgToAllCBs);
#endif

#if (ZG_BUILD_COORDINATOR_TYPE)
  bdb_RegisterTCLinkKeyExchangeProcessCB(BDBTCLinkKeyExchangeNotificationsendMsgToAllCB);
#endif

#if defined ( BDB_TL_TARGET )
  bdb_RegisterTouchlinkTargetEnableCB(BDBTouchLinkTargetEnablesendMsgToAllCB);
#endif

#if (ZG_BUILD_JOINING_TYPE)
  bdb_RegisterCBKETCLinkKeyExchangeCB(BDBCBKETCLinkKeyExchangesendMsgCB);
  bdb_RegisterForFilterNwkDescCB(BDBFilterNwkDescriptorsendMsgCB);
#endif

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
  gp_RegisterCommissioningModeCB(gp_CommissioningModeInd);
#endif

#if (ZG_BUILD_RTR_TYPE)
  nwk_RegisteNwkFrameFwdNotifyCB(sendNwkFrameFwdNotificationCBMsg);
#endif

#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS)) && !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
  bdb_zstackTaskRegisterGPDataIndCB(gpDataIndMsgSend);
  bdb_zstackTaskRegisterGPSecReqCB(gpSecRecMsgSend);
  bdb_zstackTaskRegisterGPCheckDevAnnceCB(gpCheckAnnceMsgSend);
#endif

#endif // ZNP_NPI
}

/**************************************************************************************************
 * @fn          ZStackTaskProcessEvent
 *
 * @brief       This function is the main event handling function of the ZStack Thread executing
 *              in task context.  This function is called by OSAL when an event or message
 *              is pending for the ZStack Thread.
 *
 * input parameters
 *
 * @param       taskId - OSAL task ID of this task.
 * @param       events - OSAL event mask.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
uint32_t ZStackTaskProcessEvent( uint8_t taskId, uint32_t events )
{
  zstackmsg_sysResetReq_t *pMsg;

  (void)taskId;   // Intentionally unreferenced parameter

  // Process system messages
  if ( events & SYS_EVENT_MSG )
  {
    if ( ( pMsg = (zstackmsg_sysResetReq_t *)OsalPort_msgReceive( ZStackServiceTaskId ) ) != NULL )
    {
      bool send = FALSE;

      switch ( pMsg->hdr.event )
      {
#ifdef NPI

        case MT_NPI_GENERIC_MSG:
            processNpiIncomingMsgInd( (uint8_t*)pMsg );
            break;

        case MT_SYS_APP_MSG:
        case MT_SYS_APP_RSP_MSG:
            processSysAppMsgInd( (mtSysAppMsg_t*)pMsg );
            break;

#if defined OTA_SERVER
        case MT_SYS_OTA_MSG:
            processSysOtaMsgInd( (OTA_MtMsg_t*)pMsg );
            break;
#endif // OTA_SERVER
#endif // NPI
#ifndef ZNP_NPI // ZNP does not handle any of these cases in ZStackTask
        case ZDO_CB_MSG:
          zsProcessZDOMsgs( (zdoIncomingMsg_t *)pMsg );
          break;

        case AF_DATA_CONFIRM_CMD:
          processAfDataConfirm( (afDataConfirm_t *)pMsg );
          break;

        case AF_REFLECT_ERROR_CMD:
          processAfReflectErrorInd( (afReflectError_t *)pMsg );
          break;

        case AF_INCOMING_MSG_CMD:
          processAfIncomingMsgInd( (afIncomingMSGPacket_t *)pMsg );
          break;

        case ZDO_STATE_CHANGE:
          {
            if ( newDevState != pMsg->hdr.status )
            {
              newDevState = (devStates_t)pMsg->hdr.status;
              processDevStateChange( 0, &newDevState );

              // Adjust the multicast setting
              _NIB.nwkUseMultiCast = nwkUseMultiCast;
            }
#if defined ( BDB_TL_INITIATOR )
            initiatorProcessStateChange( newDevState );
#elif defined ( BDB_TL_TARGET )
            targetProcessStateChange( newDevState );
#endif
          }
          break;

        //Cases not needed by the app neither by the stack
        case ZDO_NEW_DSTADDR:                    // 0xD0    // ZDO has received a new DstAddr for this app
        case ZDO_MATCH_DESC_RSP_SENT:            // 0xD2    // ZDO match descriptor response was sent
        case ZDO_NETWORK_REPORT:                 // 0xD4    // ZDO received a Network Report message
        case ZDO_NETWORK_UPDATE:                 // 0xD5    // ZDO received a Network Update message
        case ZDO_ADDR_CHANGE_IND:                // 0xD6    // ZDO was informed of device address change
        case NM_CHANNEL_INTERFERE:               // 0x31    // NwkMgr received a Channel Interference message
        case NM_ED_SCAN_CONFIRM:                 // 0x32    // NwkMgr received an ED Scan Confirm message
        case SAPS_CHANNEL_CHANGE:                // 0x33    // Stub APS has changed the device's channel
        case ZCL_INCOMING_MSG:                   // 0x34    // Incoming ZCL foundation message
        case ZCL_KEY_ESTABLISH_IND:              // 0x35    // ZCL Key Establishment Completion Indication
        case ZCL_OTA_CALLBACK_IND:               // 0x36    // ZCL OTA Completion Indication
            break;

        default:
          { // Assume it's a message from another thread
            send = appMsg( (uint8_t*) pMsg );
          }
          break;
#else // ZNP_NPI
        default:
          // discard unhandled messages
            break;
#endif // ZNP_NPI
      }

      if ( send )
      {
        /* Send the message back to the originator so that originator will know
           request has been processed */
        OsalPort_msgSend( pMsg->hdr.srcServiceTask, (uint8_t*) pMsg );
      }
      else
      {
        // Release the memory
        OsalPort_msgDeallocate( (uint8_t *)pMsg );
      }
    }

    // Return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if(events & OSALPORT_CLEAN_UP_TIMERS_EVT)
  {
    OsalPortTimers_cleanUpTimers();
    return (events ^ OSALPORT_CLEAN_UP_TIMERS_EVT);
  }

  // When reaching here, the events are unknown
  // Discard them or make more handlers
  return 0;
}

/**************************************************************************************************
 * @fn          epTableFindEntryEP
 *
 * @brief       Search list for endpoint and return pointer to record
 *
 * @param       ep - endpoint to find
 *
 * @return      pointer to entry record, NULL if not found
 */
static epItem_t *epTableFindEntryEP( uint8_t ep )
{
  epItem_t *srch;

  srch = pEpTableHdr;

  while ( srch )
  {
    if ( srch->epDesc.endPoint == ep )
    {
      return (srch);
    }

    srch = srch->next;
  }

  return (NULL);
}

#ifdef NPI

// -----------------------------------------------------------------------------
//! \brief      Handle the incoming NPI Req. This is a Z-Stack Stack Task and
//!             as a result, the NPI Req is assumed to contain an MT message.
//!
//!             This function copies the MT message into a mtOSALSerialData_t
//!             buffer and passes it along to the MT task.
//!
//! \param[in]  pMsg - pointer to the incoming message
//!
//! \return     void
// -----------------------------------------------------------------------------
static void StackTask_handleNPIReq( void *pMsg )
{
    mtOSALSerialData_t  *pOsalMsg;

    uint8_t *pReq = (uint8_t *)pMsg;

    /* Allocate memory for the MT message */
    pOsalMsg = (mtOSALSerialData_t *)OsalPort_msgAllocate( sizeof ( mtOSALSerialData_t ) );


    if (pOsalMsg)
    {
        /* Fill up what we can */
        pOsalMsg->hdr.event = CMD_SERIAL_MSG;

        pOsalMsg->msg = OsalPort_malloc ( MT_RPC_FRAME_HDR_SZ + pReq[MT_RPC_POS_LEN] );

        if(pOsalMsg->msg) {
          OsalPort_memcpy(pOsalMsg->msg, pReq, (MT_RPC_FRAME_HDR_SZ + pReq[MT_RPC_POS_LEN]) );

          OsalPort_msgSend( mtTaskID, (byte *)pOsalMsg );
        }

    }
}

/**************************************************************************************************
 * @fn      processNpiIncomingMsgInd
 *
 * @brief   Data message processor callback.  This function processes
 *          NPI incomming data.
 *
 * @param   pkt - pointer to incoming packet
 *
 * @return  none
 */
static void processNpiIncomingMsgInd( uint8_t *pkt )
{
    StackTask_handleNPIReq( pkt + 1 );    // To skip the MTRPC_POS_RX_HDR, no need of this on ZNP

    // Release the memory
    OsalPort_msgDeallocate( (uint8_t *)pkt );
}

/**************************************************************************************************
 * @fn      processSysAppMsgInd
 *
 * @brief   process incoming MT Sys App Message
 *
 * @param   pkt - pointer to data confirm message
 *
 * @return  none
 */
static void processSysAppMsgInd( mtSysAppMsg_t *pkt )
{
  zstackmsg_sysAppMsg_t *pMsg;
  epItem_t *pItem;

  pMsg = (zstackmsg_sysAppMsg_t *)OsalPort_msgAllocate( sizeof(zstackmsg_sysAppMsg_t) );
  if ( pMsg == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pMsg, 0, sizeof(zstackmsg_sysAppMsg_t) );

  pMsg->hdr.event = zstackmsg_CmdIDs_SYS_APP_MSG_REQ;
  pMsg->hdr.status = zstack_ZStatusValues_ZSuccess;

  pMsg->Req.dstEndpoint = pkt->endpoint;
  pMsg->Req.appDataLen = pkt->appDataLen;

  pMsg->Req.pAppData = OsalPort_malloc(pkt->appDataLen);

  if(  pMsg->Req.pAppData != NULL )
  {
      OsalPort_memcpy(pMsg->Req.pAppData, pkt->appData, pkt->appDataLen);


      pItem = epTableFindEntryEP( pMsg->Req.dstEndpoint );
      if ( pItem )
      {

        // Send to a subscriber
        OsalPort_msgSend( pItem->connection, (uint8_t*)pMsg );
      }
  }
}



#if defined OTA_SERVER
/**************************************************************************************************
 * @fn      processSysOtaMsgInd
 *
 * @brief   process incoming MT Sys OTA Message
 *
 * @param   pkt - pointer to data confirm message
 *
 * @return  none
 */
static void processSysOtaMsgInd( OTA_MtMsg_t *pkt )
{
  zstackmsg_sysOtaMsg_t *pMsg;
  epItem_t *pItem;

  pMsg = (zstackmsg_sysOtaMsg_t *)OsalPort_msgAllocate( sizeof(zstackmsg_sysOtaMsg_t) );
  if ( pMsg == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pMsg, 0, sizeof(zstackmsg_sysOtaMsg_t) );

  pMsg->hdr.event = zstackmsg_CmdIDs_SYS_OTA_MSG_REQ;
  pMsg->hdr.status = zstack_ZStatusValues_ZSuccess;

  pMsg->Req.cmd = pkt->cmd;

  if(pkt->cmd == MT_OTA_NEXT_IMG_RSP)
  {
      pMsg->Req.pData = OsalPort_malloc(pkt->len + sizeof(OTA_MtMsg_t) - 1);  //osal header and payload, minus one as the payload is counted by PAYLOAD_MAX_LEN_QUERY_NEXT_IMAGE_RSP

      if(  pMsg->Req.pData != NULL )
      {
          ((OTA_MtMsg_t*)(pMsg->Req.pData))->cmd = pkt->cmd;

          OsalPort_memcpy(((OTA_MtMsg_t*)(pMsg->Req.pData))->data, pkt->data, pkt->len);

          pItem = epTableFindEntryEP( ZCL_OTA_ENDPOINT );
          if ( pItem )
          {
            // Send to a subscriber
            OsalPort_msgSend( pItem->connection, (uint8_t*)pMsg );
          }
      }
  }
  else if (pkt->cmd == MT_OTA_FILE_READ_RSP)
  {
      pMsg->Req.pData = OsalPort_malloc(pkt->len + sizeof(OTA_MtMsg_t) - 1); //osal header and payload, minus one as the payload is counted by PAYLOAD_MAX_LEN_QUERY_NEXT_IMAGE_RSP

      if(  pMsg->Req.pData != NULL )
      {
          ((OTA_MtMsg_t*)(pMsg->Req.pData))->cmd = pkt->cmd;

          OsalPort_memcpy(((OTA_MtMsg_t*)(pMsg->Req.pData))->data, pkt->data, pkt->len);

          pItem = epTableFindEntryEP( ZCL_OTA_ENDPOINT );
          if ( pItem )
          {
            // Send to a subscriber
            OsalPort_msgSend( pItem->connection, (uint8_t*)pMsg );
          }
      }
  }
  else
  {
    //This should not happen.
  }
}

#endif // OTA SERVER
#endif // NPI

#ifndef ZNP_NPI
/**************************************************************************************************
 * @fn          appMsg
 *
 * @brief       Process an incoming Application task message.
 *
 * @param       srcServiceTaskId - source thread ID
 * @param       pMsg - pointer to the incoming message
 *
 * @return      TRUE to send the message back to the sender, FALSE if not
 */
static bool appMsg( uint8_t* pMsg )
{
  bool resend = TRUE; // default to resend to app task
  // Temp convert to get the event
  zstackmsg_sysResetReq_t *pReq = (zstackmsg_sysResetReq_t *)pMsg;
  uint8_t srcServiceTaskId = pReq->hdr.srcServiceTask;

  switch ( pReq->hdr.event )
  {
    case zstackmsg_CmdIDs_DEV_REJOIN_REQ:
    case zstackmsg_CmdIDs_AF_INTERPAN_CTL_REQ:
      // Not supported yet

      pReq->hdr.status = zstack_ZStatusValues_ZUnsupportedMode;
      break;
    case zstackmsg_CmdIDs_SYS_RESET_REQ:
      resend = processSysResetReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_DEV_FORCE_NETWORK_SETTINGS_REQ:
      resend = processDevForceNetworkSettingsReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_DEV_FORCE_NETWORK_UPDATE_REQ:
      resend = processDevForceNetworkUpdateReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_DEV_FORCE_MAC_PARAMS_REQ:
      resend = processDevForceMacParamsReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_DEV_UPDATE_NEIGHBOR_TXCOST_REQ:
      resend = processDevUpdateNeighborTxCostReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_DEV_JOIN_REQ:
      resend = processDevJoinReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SYS_NWK_FRAME_FWD_NOTIFICATION_REQ:
      resend = processSetNwkFrameFwdNoticationReq(srcServiceTaskId, pMsg);
      break;

    case zstackmsg_CmdIDs_DEV_NWK_DISCOVERY_REQ:
      resend = processDevNwkDiscoveryReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SYS_FORCE_LINK_STATUS_REQ:
      resend = processSysForceLinkStatusReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SYS_VERSION_REQ:
      resend = processSysVersionReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_DEV_START_REQ:
      resend = processDevStartReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SYS_SET_TX_POWER_REQ:
      resend = processSysSetTxPowerReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_APS_REMOVE_GROUP:
      resend = processApsRemoveGroup( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_APS_REMOVE_ALL_GROUPS:
      resend = processApsRemoveAllGroups( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_APS_FIND_ALL_GROUPS_REQ:
      resend = processApsFindAllGroupsReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_APS_FIND_GROUP_REQ:
      resend = processApsFindGroupReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_APS_ADD_GROUP:
      resend = processApsAddGroup( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_APS_COUNT_ALL_GROUPS:
      resend = processApsCountAllGroups( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SEC_APS_REMOVE_REQ:
      resend = processSecApsRemoveReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SEC_NWK_KEY_UPDATE_REQ:
      resend = processSecNwkKeyUpdateReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SEC_NWK_KEY_SWITCH_REQ:
      resend = processSecNwkKeySwitchReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SEC_NWK_KEY_SET_REQ:
      resend = processSecNwkKeySetReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SEC_NWK_KEY_GET_REQ:
      resend = processSecNwkKeyGetReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SEC_APS_LINKKEY_GET_REQ:
      resend = processSecApsLinkKeyGetReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SEC_APS_LINKKEY_SET_REQ:
      resend = processSecApsLinkKeySetReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SEC_APS_LINKKEY_REMOVE_REQ:
      resend = processSecApsLinkKeyRemoveReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SEC_MGR_SETUP_PARTNER_REQ:
      resend = processSecMgrSetupPartner( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SEC_MGR_APP_KEY_TYPE_SET_REQ:
      resend = processSecMgrAppKeyTypeSet( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SEC_MGR_APP_KEY_REQ:
      resend = processSecMgrAppKeyReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_DEV_NWK_MANAGER_SET_REQ:
      resend = processDevNwkManagerSet( srcServiceTaskId, pMsg );
      break;

#if defined (RTR_NWK)
    case zstackmsg_CmdIDs_DEV_NWK_ROUTE_REQ:
      resend = processDevNwkRouteReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_DEV_NWK_CHECK_ROUTE_REQ:
      resend = processDevNwkCheckRouteReq( srcServiceTaskId, pMsg );
      break;
#endif // RTR_NWK

    case zstackmsg_CmdIDs_SYS_CONFIG_READ_REQ:
      resend = processSysConfigReadReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SYS_CONFIG_WRITE_REQ:
      resend = processSysConfigWriteReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_SYS_NWK_INFO_READ_REQ:
      resend = processSysNwkInfoReadReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_DEV_ZDO_CBS_REQ:
      resend = processDevZDOCBReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_AF_REGISTER_REQ:
      resend = processAfRegisterReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_AF_UNREGISTER_REQ:
      resend = processAfUnRegisterReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_AF_CONFIG_GET_REQ:
      resend = processAfConfigGetReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_AF_CONFIG_SET_REQ:
      resend = processAfConfigSetReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_AF_DATA_REQ:
      resend = processAfDataReq( srcServiceTaskId, pMsg );
      break;

#if defined (ZDO_NWKADDR_REQUEST)
    case zstackmsg_CmdIDs_ZDO_NWK_ADDR_REQ:
      resend = processZdoNwkAddrReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_IEEEADDR_REQUEST)
    case zstackmsg_CmdIDs_ZDO_IEEE_ADDR_REQ:
      resend = processZdoIeeeAddrReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_NODEDESC_REQUEST)
    case zstackmsg_CmdIDs_ZDO_NODE_DESC_REQ:
      resend = processZdoNodeDescReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_MATCH_REQUEST)
    case zstackmsg_CmdIDs_ZDO_MATCH_DESC_REQ:
      resend = processZdoMatchDescReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_POWERDESC_REQUEST)
    case zstackmsg_CmdIDs_ZDO_POWER_DESC_REQ:
      resend = processZdoPowerDescReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_SIMPLEDESC_REQUEST)
    case zstackmsg_CmdIDs_ZDO_SIMPLE_DESC_REQ:
      resend = processZdoSimpleDescReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_ACTIVEEP_REQUEST)
    case zstackmsg_CmdIDs_ZDO_ACTIVE_ENDPOINT_REQ:
      resend = processZdoActiveEndpointsReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_COMPLEXDESC_REQUEST)
    case zstackmsg_CmdIDs_ZDO_COMPLEX_DESC_REQ:
      resend = processZdoComplexDescReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_SERVERDISC_REQUEST)
    case zstackmsg_CmdIDs_ZDO_SERVER_DISC_REQ:
      resend = processZdoServerDiscReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_ENDDEVICEBIND_REQUEST)
    case zstackmsg_CmdIDs_ZDO_END_DEVICE_BIND_REQ:
      resend = processZdoEndDeviceBindReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_BIND_UNBIND_REQUEST)
    case zstackmsg_CmdIDs_ZDO_BIND_REQ:
      resend = processZdoBindReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_BIND_UNBIND_REQUEST)
    case zstackmsg_CmdIDs_ZDO_UNBIND_REQ:
      resend = processZdoUnbindReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_MGMT_NWKDISC_REQUEST)
    case zstackmsg_CmdIDs_ZDO_MGMT_NWK_DISC_REQ:
      resend = processZdoMgmtNwkDiscReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_MGMT_LQI_REQUEST)
    case zstackmsg_CmdIDs_ZDO_MGMT_LQI_REQ:
      resend = processZdoMgmtLqiReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_MGMT_RTG_REQUEST)
    case zstackmsg_CmdIDs_ZDO_MGMT_RTG_REQ:
      resend = processZdoMgmtRtgReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_MGMT_BIND_REQUEST)
    case zstackmsg_CmdIDs_ZDO_MGMT_BIND_REQ:
      resend = processZdoMgmtBindReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_MGMT_LEAVE_REQUEST)
    case zstackmsg_CmdIDs_ZDO_MGMT_LEAVE_REQ:
      resend = processZdoMgmtLeaveReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_MGMT_JOINDIRECT_REQUEST)
    case zstackmsg_CmdIDs_ZDO_MGMT_DIRECT_JOIN_REQ:
      resend = processZdoMgmtDirectJoinReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_MGMT_PERMIT_JOIN_REQUEST)
    case zstackmsg_CmdIDs_ZDO_MGMT_PERMIT_JOIN_REQ:
      resend = processZdoMgmtPermitJoinReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_MGMT_NWKUPDATE_REQUEST)
    case zstackmsg_CmdIDs_ZDO_MGMT_NWK_UPDATE_REQ:
      resend = processZdoMgmtNwkUpdateReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_ENDDEVICE_ANNCE)
    case zstackmsg_CmdIDs_ZDO_DEVICE_ANNOUNCE_REQ:
      resend = processZdoDeviceAnnounceReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_USERDESCSET_REQUEST)
    case zstackmsg_CmdIDs_ZDO_USER_DESCR_SET_REQ:
      resend = processZdoUserDescSetReq( srcServiceTaskId, pMsg );
      break;
#endif

#if defined (ZDO_USERDESC_REQUEST)
    case zstackmsg_CmdIDs_ZDO_USER_DESC_REQ:
      resend = processZdoUserDescReq( srcServiceTaskId, pMsg );
      break;
#endif
    case zstackmsg_CmdIDs_ZDO_SET_BIND_UNBIND_AUTH_ADDR_REQ:
        resend = processZdoSetBindUnbindAuthAddrReq( srcServiceTaskId, pMsg );
        break;

#if ( BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE )
    case zstackmsg_CmdIDs_TL_SCAN_REC_IND:
      resend = processTlScanReqInd( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_TOUCHLINK_NWK_JOIN_REC_IND:
      resend = processTouchlinkNetworkJoinReqInd( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_TOUCHLINK_NWK_UPDATE_REC_IND:
        resend = processTouchlinkNetworkUpdateReqInd( srcServiceTaskId, pMsg );
      break;
#endif

#if ( defined ( BDB_TL_TARGET ) && (BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE) )
#if (ZSTACK_ROUTER_BUILD)
    case zstackmsg_CmdIDs_TL_TARGET_NWK_START_REC_IND:
      resend = processTlTargetNetworkStartReqInd( srcServiceTaskId, pMsg );
      break;
#endif

    case zstackmsg_CmdIDs_TL_TARGET_RESET_TO_FN_REC_IND:
      resend = processTlTargetResetToFactoryNewReqInd( srcServiceTaskId );
      break;
#endif

#if ( defined ( BDB_TL_INITIATOR ) && (BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE) )
    case zstackmsg_CmdIDs_TL_GET_SCAN_BASE_TIME:
        resend = processTlGetScanBaseTime( srcServiceTaskId, pMsg );
    break;

    case zstackmsg_CmdIDs_TL_INITIATOR_SCAN_RSP_IND:
        resend = processTlInitiatorScanRspInd( srcServiceTaskId, pMsg );
    break;

    case zstackmsg_CmdIDs_TOUCHLINK_DEV_INFO_RSP_IND:
        resend = processTlInitiatorDevInfoRspInd( srcServiceTaskId, pMsg );
    break;

    case zstackmsg_CmdIDs_TL_INITIATOR_NWK_START_RSP_IND:
        resend = processTlInitiatorNwkStartRspInd( srcServiceTaskId, pMsg );
    break;

    case zstackmsg_CmdIDs_TL_INITIATOR_NWK_JOIN_RSP_IND:
        resend = processTlInitiatorNwkJoinRspInd( srcServiceTaskId, pMsg );
    break;
#endif

    case zstackmsg_CmdIDs_BDB_START_COMMISSIONING_REQ:
      resend = processBdbStartCommissioningReq( srcServiceTaskId, pMsg );
      break;

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
    case zstackmsg_CmdIDs_BDB_SET_IDENTIFY_ACTIVE_ENDPOINT_REQ:
      resend = processBdbSetIdentifyActiveEndpointReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_BDB_GET_IDENTIFY_ACTIVE_ENDPOINT_REQ:
      resend = processBdbGetIdentifyActiveEndpointReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_BDB_STOP_INITIATOR_FINDING_BINDING_REQ:
      resend = processBdbStopInitiatorFindingBindingReq( srcServiceTaskId, pMsg );
      break;
#endif
    case zstackmsg_CmdIDs_BDB_ZCL_IDENTIFY_CMD_IND_REQ:
      resend = processBdbZclIdentifyCmdIndReq( srcServiceTaskId, pMsg );
    break;

    case zstackmsg_CmdIDs_GET_ZCL_FRAME_COUNTER_REQ:
      resend = processGetZCLFrameCounterReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_BDB_SET_EP_DESC_LIST_ACTIVE_EP:
        resend = processBdbSetEpDescListToActiveEndpoint(srcServiceTaskId, pMsg );
        break;
    case zstackmsg_CmdIDs_BDB_RESET_LOCAL_ACTION_REQ:
      resend = processBdbResetLocalActionReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_BDB_SET_ATTRIBUTES_REQ:
      resend = processBdbSetAttributesReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_BDB_GET_ATTRIBUTES_REQ:
      resend = processBdbGetAttributesReq( srcServiceTaskId, pMsg );
      break;

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED == 1)
    case zstackmsg_CmdIDs_BDB_GET_FB_INITIATOR_STATUS_REQ:
      resend = processBdbGetFBInitiatorStatusReq( srcServiceTaskId, pMsg );
      break;
#endif

    case zstackmsg_CmdIDs_BDB_GENERATE_INSTALL_CODE_CRC_REQ:
      resend = processBdbGenerateInstallCodeCRCReq( srcServiceTaskId, pMsg );
      break;
#ifdef BDB_REPORTING
    case zstackmsg_CmdIDs_BDB_REP_ADD_ATTR_CFG_RECORD_DEFAULT_TO_LIST_REQ:
      resend = processBdbRepAddAttrCfgRecordDefaultToListReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_BDB_REP_CHANGED_ATTR_VALUE_REQ:
      resend = processBdbRepChangedAttrValueReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_BDB_PROCESS_IN_CONFIG_REPORT_REQ:
      resend = processBdbProcessInConfigReportReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_BDB_PROCESS_IN_READ_REPORT_CFG_REQ:
      resend = processBdbProcessInReadReportCfgReq( srcServiceTaskId, pMsg );
      break;

#endif

    case zstackmsg_CmdIDs_BDB_ADD_INSTALL_CODE_REQ:
      resend = processBdbAddInstallCodeReq( srcServiceTaskId, pMsg );
      break;

#if (ZG_BUILD_JOINING_TYPE)
    case zstackmsg_CmdIDs_BDB_SET_ACTIVE_CENTRALIZED_LINK_KEY_REQ:
      resend = processBdbSetActiveCentralizedLinkKeyReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_BDB_CBKE_TC_LINK_KEY_EXCHANGE_ATTEMPT_REQ:
      resend = processBdbCBKETCLinkKeyExchangeAttemptReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_BDB_NWK_DESC_FREE_REQ:
      resend = processBdbNwkDescFreeReq( srcServiceTaskId, pMsg );
      break;
#endif

#ifdef BDB_TL_TARGET
    case zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_ENABLE_COMMISSIONING_REQ:
      resend = processBdbTouchLinkTargetEnableCommissioningReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_DISABLE_COMMISSIONING_REQ:
      resend = processBdbTouchLinkTargetDisableCommissioningReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_GETT_IMER_REQ:
      resend = processBdbTouchLinkTargetGetTimerReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_BDB_TOUCHLINK_SET_ALLOW_STEALING_REQ:
      resend = processBdbTouchlinkSetAllowStealingReq( srcServiceTaskId, pMsg );
      break;

    case zstackmsg_CmdIDs_BDB_TOUCHLINK_GET_ALLOW_STEALING_REQ:
      resend = processBdbTouchlinkGetAllowStealingReq( srcServiceTaskId, pMsg );
      break;
#endif

#if (ZG_BUILD_JOINING_TYPE)
    case zstackmsg_CmdIDs_BDB_ZED_ATTEMPT_RECOVER_NWK_REQ:
      resend = processBdbRecoverNwkReq( srcServiceTaskId, pMsg );
      break;
#endif

#if (ZG_BUILD_JOINING_TYPE)
    case zstackmsg_CmdIDs_BDB_FILTER_NWK_DESC_COMPLETE_REQ:
        resend = processBdbFilterNwkDescCompleteReq(srcServiceTaskId, pMsg);
    break;
#endif

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
    case zstackmsg_CmdIDs_GP_ALLOW_CHANNEL_CHANGE_REQ:
        resend = processGpAllowChannelChangeReq(srcServiceTaskId, pMsg);
    break;

    case zstackmsg_CmdIDs_GP_SECURITY_RSP:
        resend = processGpSecRsp(srcServiceTaskId, pMsg);
    break;

    case zstackmsg_CmdIDs_GP_ADDRESS_CONFLICT:
        resend = processGpAddressConfict(srcServiceTaskId, pMsg);
    break;

#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
    case zstackmsg_CmdIDs_GP_CCM_STAR_DATA_IND:
        resend =  processGpDecryptDataInd(srcServiceTaskId, pMsg);
    break;

    case zstackmsg_CmdIDs_GP_CCM_STAR_COMMISSIONING_KEY:
        resend = processGpEncryptDecryptCommissioningKey(srcServiceTaskId, pMsg);
    break;

    case zstackmsg_CmdIDs_GP_COMMISSIONING_SUCCESS:
        resend = processGpCommissioningSuccess(srcServiceTaskId, pMsg);
    break;

    case zstackmsg_CmdIDs_GP_SEND_DEV_ANNOUNCE:
        resend = processGpSendDeviceAnnounce(srcServiceTaskId, pMsg);
    break;
#endif
#endif
    case zstackmsg_CmdIDs_PAUSE_DEVICE_REQ:
      resend = processPauseResumeDeviceReq( srcServiceTaskId, pMsg );
    break;
    default:
      pReq->hdr.status = zstack_ZStatusValues_ZUnsupportedMode;
      break;
  }

  return (resend);
}

/**************************************************************************************************
 * @fn      processAfDataConfirm
 *
 * @brief   process incoming AF Data Confirm
 *
 * @param   pkt - pointer to data confirm message
 *
 * @return  none
 */
static void processAfDataConfirm( afDataConfirm_t *pkt )
{
  zstackmsg_afDataConfirmInd_t *pReq;
  epItem_t *pItem;

  pReq = (zstackmsg_afDataConfirmInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_afDataConfirmInd_t) );
  if ( pReq == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pReq, 0, sizeof(zstackmsg_afDataConfirmInd_t) );

  pReq->hdr.event = zstackmsg_CmdIDs_AF_DATA_CONFIRM_IND;
  pReq->hdr.status = zstack_ZStatusValues_ZSuccess;

  pReq->req.endpoint = pkt->endpoint;
  pReq->req.status = (zstack_ZStatusValues)pkt->hdr.status;
  pReq->req.transID = pkt->transID;
  pReq->req.clusterID = pkt->clusterID;

  pItem = epTableFindEntryEP( pkt->endpoint );
  if ( pItem )
  {
    // Send to a subscriber
    OsalPort_msgSend( pItem->connection, (uint8_t*)pReq );
  }
  else
  {
      OsalPort_msgDeallocate((uint8_t*)pReq);
  }
}

/**************************************************************************************************
 * @fn      processAfReflectErrorInd
 *
 * @brief   process incoming AF Reflect Error Indication
 *
 * @param   pkt - pointer to AF Reflect Error message
 *
 * @return  none
 */
static void processAfReflectErrorInd( afReflectError_t *pkt )
{
  zstackmsg_afReflectErrorInd_t *pReq;
  epItem_t *pItem;

  pReq = (zstackmsg_afReflectErrorInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_afReflectErrorInd_t) );
  if ( pReq == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pReq, 0, sizeof(zstackmsg_afReflectErrorInd_t) );

  pReq->hdr.event = zstackmsg_CmdIDs_AF_REFLECT_ERROR_IND;
  pReq->hdr.status = zstack_ZStatusValues_ZSuccess;

  pReq->req.status = (zstack_ZStatusValues)pkt->hdr.status;

  pReq->req.dstAddr.endpoint = pkt->endpoint;
  if ( pkt->dstAddrMode == zstack_AFAddrMode_GROUP )
  {
    pReq->req.dstAddr.addrMode = zstack_AFAddrMode_GROUP;
  }
  else
  {
    pReq->req.dstAddr.addrMode = zstack_AFAddrMode_SHORT;
  }
  pReq->req.dstAddr.addr.shortAddr = pkt->dstAddr;

  pReq->req.transID = pkt->transID;

  pItem = epTableFindEntryEP( pkt->endpoint );
  if ( pItem )
  {
    // Send to a subscriber
    OsalPort_msgSend( pItem->connection, (uint8_t*)pReq );
  }
  else
  {
      OsalPort_msgDeallocate((uint8_t*)pReq);
  }
}

/**************************************************************************************************
 * @fn      processAfIncomingMsgInd
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   pkt - pointer to incoming packet
 *
 * @return  none
 */
static void processAfIncomingMsgInd( afIncomingMSGPacket_t *pkt )
{
  zstackmsg_afIncomingMsgInd_t *pReq;
  epItem_t *pItem;

  pReq = (zstackmsg_afIncomingMsgInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_afIncomingMsgInd_t) );
  if ( pReq == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pReq, 0, sizeof(zstackmsg_afIncomingMsgInd_t) );

  pReq->hdr.event = zstackmsg_CmdIDs_AF_INCOMING_MSG_IND;
  pReq->hdr.status = 0;

  pReq->req.srcAddr.addrMode = (zstack_AFAddrMode)pkt->srcAddr.addrMode;

  if ( pReq->req.srcAddr.addrMode == zstack_AFAddrMode_EXT )
  {
    OsalPort_memcpy( &(pReq->req.srcAddr.addr.extAddr),
          pkt->srcAddr.addr.extAddr, Z_EXTADDR_LEN );
  }
  else
  {
    pReq->req.srcAddr.addr.shortAddr = pkt->srcAddr.addr.shortAddr;
  }

  pReq->req.srcAddr.endpoint = pkt->srcAddr.endPoint;
  pReq->req.srcAddr.panID = pkt->srcAddr.panId;

  pReq->req.groupID = pkt->groupId;
  pReq->req.clusterId = pkt->clusterId;
  pReq->req.macDestAddr = pkt->macDestAddr;
  pReq->req.endpoint = pkt->endPoint;
  pReq->req.wasBroadcast = pkt->wasBroadcast;
  pReq->req.securityUse = pkt->SecurityUse;
  pReq->req.linkQuality = pkt->LinkQuality;
  pReq->req.correlation = pkt->correlation;
  pReq->req.rssi = pkt->rssi;
  pReq->req.timestamp = pkt->timestamp;
  pReq->req.nwkSeqNum = pkt->nwkSeqNum;
  pReq->req.macSrcAddr = pkt->macSrcAddr;
  pReq->req.radius = pkt->radius;
  pReq->req.n_payload = pkt->cmd.DataLength;
  pReq->req.pPayload = OsalPort_malloc( pkt->cmd.DataLength );
  if ( pReq->req.pPayload != NULL)
  {
    OsalPort_memcpy( pReq->req.pPayload, pkt->cmd.Data, pkt->cmd.DataLength );

    pItem = epTableFindEntryEP( pkt->endPoint );
    if ( pItem )
    {
      // Send to a subscriber
      OsalPort_msgSend( pItem->connection,
            (uint8_t*)pReq );
    }
    else
    {
      OsalPort_free(pReq->req.pPayload);
      OsalPort_msgDeallocate((uint8_t*)pReq);
    }
  }
  else
  {
    // TODO: handle malloc error
  }
}


/**************************************************************************************************
 * @fn      zsProcessZDOMsgs()
 *
 * @brief   Process response messages
 *
 * @param   inMsg - incoming ZDO message
 *
 * @return  none
 */
static void zsProcessZDOMsgs( zdoIncomingMsg_t *inMsg )
{
  switch ( inMsg->clusterID )
  {
    case Device_annce:
      {
        ZDO_DeviceAnnce_t devAnn;

        memset( &devAnn, 0, sizeof(ZDO_DeviceAnnce_t) );

        ZDO_ParseDeviceAnnce( inMsg, &devAnn );
        sendDeviceAnnounce( inMsg->srcAddr.addr.shortAddr, &devAnn );
      }
      break;

#if defined (ZDO_NWKADDR_REQUEST)
    case NWK_addr_rsp:
      {
        ZDO_NwkIEEEAddrResp_t *pAddrRsp = ZDO_ParseAddrRsp( inMsg );
        if ( pAddrRsp )
        {
          sendNwkAddrRsp( inMsg->srcAddr.addr.shortAddr, pAddrRsp );

          OsalPort_free( pAddrRsp );
        }
      }
      break;
#endif

#if defined (ZDO_IEEEADDR_REQUEST)
    case IEEE_addr_rsp:
      {
        ZDO_NwkIEEEAddrResp_t *pAddrRsp = ZDO_ParseAddrRsp( inMsg );
        if ( pAddrRsp )
        {
          sendIeeeAddrRsp( inMsg->srcAddr.addr.shortAddr, pAddrRsp );

          OsalPort_free( pAddrRsp );
        }
      }
      break;
#endif

#if defined (ZDO_NODEDESC_REQUEST)
    case Node_Desc_rsp:
      {
        ZDO_NodeDescRsp_t ndRsp;

        memset( &ndRsp, 0, sizeof(ZDO_NodeDescRsp_t) );

        ZDO_ParseNodeDescRsp( inMsg, &ndRsp );
        sendNodeDescRsp( inMsg->srcAddr.addr.shortAddr, &ndRsp );
      }
      break;
#endif

#if defined (ZDO_POWERDESC_REQUEST)
    case Power_Desc_rsp:
      {
        ZDO_PowerRsp_t powerRsp;

        memset( &powerRsp, 0, sizeof(ZDO_PowerRsp_t) );

        ZDO_ParsePowerDescRsp( inMsg, &powerRsp );
        sendPowerDescRsp( inMsg->srcAddr.addr.shortAddr, &powerRsp );
      }
      break;
#endif

#if defined (ZDO_SIMPLEDESC_REQUEST)
    case Simple_Desc_rsp:
      {
        ZDO_SimpleDescRsp_t simpleRsp;

        memset( &simpleRsp, 0, sizeof(ZDO_SimpleDescRsp_t) );

        ZDO_ParseSimpleDescRsp( inMsg, &simpleRsp );
        sendSimpleDescRsp( inMsg->srcAddr.addr.shortAddr, &simpleRsp );

        if(simpleRsp.simpleDesc.pAppInClusterList != NULL)
        {
          OsalPort_free( simpleRsp.simpleDesc.pAppInClusterList );
          simpleRsp.simpleDesc.pAppInClusterList = ( cId_t* )NULL;
        }
        if(simpleRsp.simpleDesc.pAppOutClusterList != NULL)
        {
          OsalPort_free( simpleRsp.simpleDesc.pAppOutClusterList );
          simpleRsp.simpleDesc.pAppOutClusterList = ( cId_t* )NULL;
        }
      }
      break;
#endif

#if defined (ZDO_ACTIVEEP_REQUEST)
    case Active_EP_rsp:
      {
        ZDO_ActiveEndpointRsp_t *epRsp = ZDO_ParseEPListRsp( inMsg );
        if ( epRsp )
        {
          sendActiveEPRsp( inMsg->srcAddr.addr.shortAddr, epRsp );

          OsalPort_free( epRsp );
        }
      }
      break;
#endif

#if defined (ZDO_MATCH_REQUEST)
    case Match_Desc_rsp:
      {
        ZDO_ActiveEndpointRsp_t *epRsp = ZDO_ParseEPListRsp( inMsg );
        if ( epRsp )
        {
          sendMatchDescRsp( inMsg->srcAddr.addr.shortAddr, epRsp );

          OsalPort_free( epRsp );
        }
      }
      break;
#endif

#if defined (ZDO_USERDESC_REQUEST)
    case User_Desc_rsp:
      {
        ZDO_UserDescRsp_t *udRsp = ZDO_ParseUserDescRsp( inMsg );
        if ( udRsp )
        {
          sendUserDescRsp( inMsg->srcAddr.addr.shortAddr, udRsp );

          OsalPort_free( udRsp );
        }
      }
      break;
#endif

#if defined (ZDO_SERVERDISC_REQUEST)
    case Server_Discovery_rsp:
      {
        ZDO_ServerDiscRsp_t sdRsp;

        memset( &sdRsp, 0, sizeof(ZDO_ServerDiscRsp_t) );

        ZDO_ParseServerDiscRsp( inMsg, &sdRsp );
        sendServerDiscRsp( inMsg->srcAddr.addr.shortAddr, &sdRsp );
      }
      break;
#endif

#if defined (ZDO_BIND_UNBIND_REQUEST)
    case Bind_rsp:
      {
        uint8_t result = ZDO_ParseBindRsp( inMsg );
        sendBindRsp( inMsg->srcAddr.addr.shortAddr, &result );
      }
      break;
#endif

#if defined (ZDO_ENDDEVICEBIND_REQUEST)
    case End_Device_Bind_rsp:
      {
        uint8_t result = ZDO_ParseBindRsp( inMsg );
        sendEndDeviceBindRsp( inMsg->srcAddr.addr.shortAddr, &result );
      }
      break;
#endif

#if defined (ZDO_BIND_UNBIND_REQUEST)
    case Unbind_rsp:
      {
        uint8_t result = ZDO_ParseBindRsp( inMsg );
        sendUnbindRsp( inMsg->srcAddr.addr.shortAddr, &result );
      }
      break;
#endif

#if defined (ZDO_MGMT_NWKDISC_REQUEST)
    case Mgmt_NWK_Disc_rsp:
      {
        ZDO_MgmNwkDiscRsp_t *pNwkDiscRsp = ZDO_ParseMgmNwkDiscRsp( inMsg );
        if ( pNwkDiscRsp )
        {
          sendMgmtNwkDiscRsp( inMsg->srcAddr.addr.shortAddr, pNwkDiscRsp );

          OsalPort_free( pNwkDiscRsp );
        }
      }
      break;
#endif

#if defined (ZDO_MGMT_LQI_REQUEST)
    case Mgmt_Lqi_rsp:
      {
        ZDO_MgmtLqiRsp_t *pLqiRsp = ZDO_ParseMgmtLqiRsp( inMsg );
        if ( pLqiRsp )
        {
          sendMgmtLqiRsp( inMsg->srcAddr.addr.shortAddr, pLqiRsp );

          OsalPort_free( pLqiRsp );
        }
      }
      break;
#endif

#if defined (ZDO_MGMT_RTG_REQUEST)
    case Mgmt_Rtg_rsp:
      {
        ZDO_MgmtRtgRsp_t *pRtgRsp = ZDO_ParseMgmtRtgRsp( inMsg );
        if ( pRtgRsp )
        {
          sendMgmtRtgRsp( inMsg->srcAddr.addr.shortAddr, pRtgRsp );

          OsalPort_free( pRtgRsp );
        }
      }
      break;
#endif

#if defined (ZDO_MGMT_BIND_REQUEST)
    case Mgmt_Bind_rsp:
      {
        ZDO_MgmtBindRsp_t *pBindRsp = ZDO_ParseMgmtBindRsp( inMsg );
        if ( pBindRsp )
        {
          sendMgmtBindRsp( inMsg->srcAddr.addr.shortAddr, pBindRsp );

          OsalPort_free( pBindRsp );
        }
      }
      break;
#endif

#if defined (ZDO_MGMT_LEAVE_REQUEST)
    case Mgmt_Leave_rsp:
      {
        uint8_t result = ZDO_ParseMgmtLeaveRsp( inMsg );
        sendMgmtLeaveRsp( inMsg->srcAddr.addr.shortAddr, &result );
      }
      break;
#endif

#if defined (ZDO_MGMT_JOINDIRECT_REQUEST)
    case Mgmt_Direct_Join_rsp:
      {
        uint8_t result = ZDO_ParseMgmtDirectJoinRsp( inMsg );
        sendMgmtDirectJoinRsp( inMsg->srcAddr.addr.shortAddr, &result );
      }
      break;
#endif

#if defined (ZDO_MGMT_PERMIT_JOIN_REQUEST)
    case Mgmt_Permit_Join_rsp:
      {
        uint8_t result = ZDO_ParseMgmtPermitJoinRsp( inMsg );
        sendMgmtPermitJoinRsp( inMsg->srcAddr.addr.shortAddr, &result );
      }
      break;
#endif

#if defined (ZDO_MGMT_NWKUPDATE_REQUEST)
    case Mgmt_NWK_Update_notify:
      {
        ZDO_MgmtNwkUpdateNotify_t *pNwkUpdateNotifyRsp =
            ZDO_ParseMgmtNwkUpdateNotify( inMsg );
        if ( pNwkUpdateNotifyRsp )
        {
          sendMgmtNwkUpdateNotify( inMsg->srcAddr.addr.shortAddr,
              pNwkUpdateNotifyRsp );

          OsalPort_free( pNwkUpdateNotifyRsp );
        }
      }
      break;
#endif

    // TBD: Not implemented yet
    case Complex_Desc_rsp:
    case Discovery_Cache_rsp:
    default:
      break;
  }
}

/**************************************************************************************************
 * @fn          epTableAddNewEntry
 *
 * @brief       Add entry to the list.  It will add to the end of the list.
 *
 * @param       newEntry - pointer to the entry
 *
 * @return      TRUE if added, FALSE if MAX reached
 */
static uint8_t epTableAddNewEntry( epItem_t *newEntry )
{
  epItem_t *srch;

  if ( epTableNumEntries( ) > maxSupportedEndpoints )
  {
    return (FALSE);
  }

  srch = pEpTableHdr;

  while ( srch && srch->next )
  {
    srch = srch->next;
  }

  newEntry->next = (epItem_t *)NULL;

  if ( srch )
  {
    srch->next = newEntry;
  }
  else
  {
    pEpTableHdr = newEntry;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          epTableFindEntryConnection
 *
 * @brief       Search list for connection and return pointer to record
 *
 * @param       connection - connection to find
 *
 * @return      pointer to entry record, NULL if not found
 */
static epItem_t *epTableFindEntryConnection( int connection )
{
  epItem_t *srch;

  srch = pEpTableHdr;

  while ( srch )
  {
    if ( srch->connection == connection )
    {
      return (srch);
    }

    srch = srch->next;
  }

  return (NULL);
}

/**************************************************************************************************
 * @fn          epTableRemoveEntry
 *
 * @brief       Remove an entry from the list and release its memory.
 *
 * @param       entry - pointer to entry
 *
 * @return      none
 */
static void epTableRemoveEntry( epItem_t *entry )
{
  epItem_t *srch;
  epItem_t *prev;

  // Is this a real entry and is there anything in the list
  if ( entry && pEpTableHdr )
  {
    srch = pEpTableHdr;
    prev = (epItem_t *)NULL;

    while ( srch && srch != entry )
    {
      prev = srch;
      srch = srch->next;
    }

    // Found?
    if ( srch == entry )
    {
      // Update the list
      if ( entry == pEpTableHdr )
      {
        // First entry
        pEpTableHdr = entry->next;
      }
      else
      {
        prev->next = entry->next;
      }

      // Release the entry's memory
      freeEpItem( entry );
    }
  }
}

/**************************************************************************************************
 * @fn          epTableNumEntries
 *
 * @brief       Get the number of entries currently in the list.
 *
 * @param       none
 *
 * @return      number of entries
 */
static uint8_t epTableNumEntries( void )
{
  epItem_t *srch;
  uint8_t cnt = 0;

  srch = pEpTableHdr;

  while ( srch )
  {
    cnt++;
    srch = srch->next;
  }

  return (cnt);
}

/**************************************************************************************************
 * @fn          freeEpItem
 *
 * @brief       Free the endpoint item
 *
 * @param       none
 *
 * @return      number of entries
 */
static void freeEpItem( epItem_t *pItem )
{
  if ( pItem )
  {
    OsalPort_free( pItem );
  }
}

/***************************************************************************************************
 * @fn          buildMsgCapInfo
 *
 * @brief       Convert from bitmask byte to ZStack Msg capInfo
 *
 * @param       cInfo - source
 * @param       pPBcapInfo - destination
 *
 * @return      None
 */
static void buildMsgCapInfo( uint8_t cInfo, zstack_CapabilityInfo_t *pPBcapInfo )
{
  if ( cInfo & CAPABLE_PAN_COORD )
  {
    pPBcapInfo->panCoord = 1;
  }

  if ( cInfo & CAPABLE_FFD )
  {
    pPBcapInfo->ffd = 1;
  }

  if ( cInfo & CAPABLE_MAINS_POWER )
  {
    pPBcapInfo->mainsPower = 1;
  }

  if ( cInfo & CAPABLE_RX_ON_IDLE )
  {
    pPBcapInfo->rxOnWhenIdle = 1;
  }

  if ( cInfo & CAPABLE_SECURITY )
  {
    pPBcapInfo->security = 1;
  }
}

#if defined (ZDO_SERVERDISC_REQUEST) || defined (ZDO_NODEDESC_REQUEST)
/***************************************************************************************************
 * @fn          buildMsgServerCap
 *
 * @brief       Convert from bitmask byte to ZStack Message ServerCapabilities
 *
 * @param       cInfo - source
 * @param       pPBsrvCap - destination
 *
 * @return      None
 */
static void buildMsgServerCap( uint16_t sInfo, zstack_ServerCapabilities_t *pPBsrvCap )
{
  if ( sInfo & PRIM_TRUST_CENTER )
  {
    pPBsrvCap->primaryTrustCenter = 1;
  }

  if ( sInfo & BKUP_TRUST_CENTER )
  {
    pPBsrvCap->backupTrustCenter = 1;
  }

  if ( sInfo & PRIM_BIND_TABLE )
  {
    pPBsrvCap->primaryBindingTableCache = 1;
  }

  if ( sInfo & BKUP_BIND_TABLE )
  {
    pPBsrvCap->backupBindingTableCache = 1;
  }

  if ( sInfo & PRIM_DISC_TABLE )
  {
    pPBsrvCap->primaryDiscoveryCache = 1;
  }

  if ( sInfo & BKUP_DISC_TABLE )
  {
    pPBsrvCap->backupDiscoveryCache = 1;
  }

  if ( sInfo & NETWORK_MANAGER )
  {
    pPBsrvCap->networkManager = 1;
  }
}
#endif // ZDO_SERVERDISC_REQUEST || ZDO_NODEDESC_REQUEST

#if defined (ZDO_POWERDESC_REQUEST)
/***************************************************************************************************
 * @fn          buildMsgPowerSource
 *
 * @brief       Convert from bitmask byte to ZStack Message PowerSource
 *
 * @param       cInfo - source
 * @param       pPBsrvCap - destination
 *
 * @return      None
 */
static void buildMsgPowerSource( uint8_t pInfo, zstack_PowerSource_t *pPBpwrSrc )
{
  if ( pInfo & NODEAVAILPWR_MAINS )
  {
    pPBpwrSrc->mains = 1;
  }

  if ( pInfo & NODEAVAILPWR_RECHARGE )
  {
    pPBpwrSrc->recharge = 1;
  }

  if ( pInfo & NODEAVAILPWR_DISPOSE )
  {
    pPBpwrSrc->dispose = 1;
  }
}
#endif // ZDO_POWERDESC_REQUEST

/***************************************************************************************************
 * @fn          convertMsgTransOptions
 *
 * @brief       Convert PB TransOptions data type to uint8_t txOptions
 *
 * @param       pOptions - TransOptions pointer
 *
 * @return      txOptions
 */
static uint8_t convertMsgTransOptions( zstack_TransOptions_t *pOptions )
{
  uint8_t options = 0;

  if ( pOptions->wildcardProfileID )
  {
    options |= AF_WILDCARD_PROFILEID;
  }

  if ( pOptions->ackRequest )
  {
    options |= AF_ACK_REQUEST;
  }

  if ( pOptions->limitConcentrator )
  {
    options |= AF_LIMIT_CONCENTRATOR;
  }

  if ( pOptions->suppressRouteDisc )
  {
    options |= AF_SUPRESS_ROUTE_DISC_NETWORK;
  }

  if ( pOptions->apsSecurity )
  {
    options |= AF_EN_SECURITY;
  }

  if ( pOptions->skipRouting )
  {
    options |= AF_SKIP_ROUTING;
  }

  return (options);
}

#if defined (ZDO_SERVERDISC_REQUEST)
/***************************************************************************************************
 * @fn          convertMsgServerCapabilities
 *
 * @brief       Convert Zigbee API Message ServerCapabilities data type to uint16_t server
 *              capabilities mask
 *
 * @param       pSrvCap - Server capabilities
 *
 * @return      Server Capabilities mask
 */
static uint16_t convertMsgServerCapabilities( zstack_ServerCapabilities_t *pSrvCap )
{
  uint16_t mask = 0;

  if ( pSrvCap->primaryTrustCenter )
  {
    mask |= PRIM_TRUST_CENTER;
  }

  if ( pSrvCap->backupTrustCenter )
  {
    mask |= BKUP_TRUST_CENTER;
  }

  if ( pSrvCap->primaryBindingTableCache )
  {
    mask |= PRIM_BIND_TABLE;
  }

  if ( pSrvCap->backupBindingTableCache )
  {
    mask |= BKUP_BIND_TABLE;
  }

  if ( pSrvCap->primaryDiscoveryCache )
  {
    mask |= PRIM_DISC_TABLE;
  }

  if ( pSrvCap->backupDiscoveryCache )
  {
    mask |= BKUP_DISC_TABLE;
  }

  if ( pSrvCap->networkManager )
  {
    mask |= NETWORK_MANAGER;
  }

  return (mask);
}
#endif // ZDO_SERVERDISC_REQUEST

/***************************************************************************************************
 * @fn          convertCapabilityInfo
 *
 * @brief       Convert Zigbee API Message zstack_CapabilityInfo_t data type to uint8_t capInfo
 *
 * @param       pMsgcapInfo - CapabilityInfo pointer
 *
 * @return      capInfo
 */
static uint8_t convertCapabilityInfo( zstack_CapabilityInfo_t *pMsgcapInfo )
{
  uint8_t capInfo = 0;

  if ( pMsgcapInfo->panCoord )
  {
    capInfo |= CAPABLE_PAN_COORD;
  }

  if ( pMsgcapInfo->ffd )
  {
    capInfo |= CAPABLE_FFD;
  }

  if ( pMsgcapInfo->mainsPower )
  {
    capInfo |= CAPABLE_MAINS_POWER;
  }

  if ( pMsgcapInfo->rxOnWhenIdle )
  {
    capInfo |= CAPABLE_RX_ON_IDLE;
  }

  if ( pMsgcapInfo->security )
  {
    capInfo |= CAPABLE_SECURITY;
  }

  return (capInfo);
}

/**************************************************************************************************
 * @fn          sendMsgToAllCBs
 *
 * @brief       Send a message to all that is subscribed to ZDO CB functions
 *
 * @param       cbMask - callback mask
 * @param       pFn - function pointer to call
 *
 * @return      none
 */
static void sendMsgToAllCBs( uint16_t cbMask, void *pBuf, pfnZDOCB pFn )
{
  epItem_t *srch;

  srch = pEpTableHdr;

  while ( srch )
  {
    if ( (srch->zdoCBs & cbMask) == cbMask )
    {
      if ( pFn )
      {
        // Send the a subscriber
        pFn( srch->connection, pBuf );
      }
    }

    srch = srch->next;
  }
}

/**************************************************************************************************
 * @fn          sendMsgToAllCBMsgs
 *
 * @brief       Send a message to all that is subscribed to ZDO CB Messages
 *
 * @param       cbMask - callback mask
 * @param       srcAddr - source address of the message
 * @param       pBuf - pointer to incoming message
 *
 * @return      none
 */
static void sendMsgToAllCBMsgs( uint32_t cbMask, uint16_t srcAddr, void *pBuf, pfnZDOMsgCB pFn )
{
  epItem_t *srch;

  srch = pEpTableHdr;

  while ( srch )
  {
    if ( (srch->zdoRsps & cbMask) == cbMask )
    {
      // Send the a subscriber
      pFn( srch->connection, srcAddr, pBuf );
    }

    srch = srch->next;
  }
}


#if (ZG_BUILD_RTR_TYPE)
/**************************************************************************************************
 * @fn          NwkFrameFwdNotification
 *
 * @brief       Function to send a notification to the application that a frame is being queued to
 *              be forwarded
 *
 * @param       pStr - pointer to beacon notification information
 *
 * @return      NULL
 */
static void sendNwkFrameFwdNotificationCBMsg( nwkFrameFwdNotification_t *nwkFrameFwdNotification )
{
    epItem_t *srch;
    zstackmsg_nwkFrameFwdNotification_t *pReq;

    pReq = (zstackmsg_nwkFrameFwdNotification_t *)OsalPort_msgAllocate( sizeof(zstackmsg_nwkFrameFwdNotification_t) );
    if ( pReq == NULL )
    {
      // Ignore the message
      return;
    }

    memset( pReq, 0, sizeof(zstackmsg_nwkFrameFwdNotification_t) );

    srch = pEpTableHdr;

    pReq->hdr.event = zstackmsg_CmdIDs_SYS_NWK_FRAME_FWD_NOTIFICATION_IND;
    pReq->hdr.status = zstack_ZStatusValues_ZSuccess;
    pReq->req.nwkFrameFwdNotification.dstAddr = nwkFrameFwdNotification->dstAddr;
    pReq->req.nwkFrameFwdNotification.srcAddr = nwkFrameFwdNotification->srcAddr;
    pReq->req.nwkFrameFwdNotification.handle = nwkFrameFwdNotification->handle;
    pReq->req.nwkFrameFwdNotification.status = nwkFrameFwdNotification->status;
    pReq->req.nwkFrameFwdNotification.frameState = nwkFrameFwdNotification->frameState;

    if ( srch )
    {
      //Only the first application connection will receive this notification
      // Send to a subscriber
	  OsalPort_msgSend( srch->connection, (uint8_t*)pReq );
    }
}
#endif

/**************************************************************************************************
 * @fn          zdoNwkDiscCnfCB
 *
 * @brief       callback function to handle Network Discovery Confirmation
 *
 * @param       pStr - pointer to beacon notification information
 *
 * @return      NULL
 */
static void *zdoNwkDiscCnfCB( void *pStr )
{
  epItem_t *pItem;

  sendMsgToAllCBs( ZS_ZDO_NWK_DISCOVERY_CNF_CBID, pStr, sendNwkDiscCnf );

  // Clear the scan information
  NLME_NwkDiscTerm( );

  // Remove ZDO callbacks for network discovery and beacon notification
  ZDO_DeregisterForZdoCB( ZDO_NWK_DISCOVERY_CNF_CBID );
  ZDO_DeregisterForZdoCB( ZDO_BEACON_NOTIFY_IND_CBID );

  // Remove all callback messages for network discovery and beacon notification
  pItem = pEpTableHdr;

  while ( pItem )
  {
    pItem->zdoCBs &= ~ZS_ZDO_BEACON_NOTIFY_IND_CBID;
    pItem->zdoCBs &= ~ZS_ZDO_NWK_DISCOVERY_CNF_CBID;

    pItem = pItem->next;
  }

  return (NULL);
}

/**************************************************************************************************
 * @fn          sendNwkDiscCnf
 *
 * @brief       Function to send Network Discovery Confirmation
 *
 * @param       dstID - Destination Task ID
 * @param       pStr - pointer to etwork Discovery Confirmation information
 *
 * @return      none
 */
static void sendNwkDiscCnf( uint16_t dstID, void *pStr )
{
  zstackmsg_zdoNwkDiscCnf_t *pNwkDiscCnf;
  uint8_t nwkDiscStatus = *(uint8_t *)pStr;

  pNwkDiscCnf = (zstackmsg_zdoNwkDiscCnf_t *)OsalPort_msgAllocate( sizeof(zstackmsg_zdoNwkDiscCnf_t) );
  if ( pNwkDiscCnf == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pNwkDiscCnf, 0, sizeof(zstackmsg_zdoNwkDiscCnf_t) );

  pNwkDiscCnf->hdr.event = zstackmsg_CmdIDs_ZDO_NWK_DISC_CNF;
  pNwkDiscCnf->hdr.status = zstack_ZStatusValues_ZSuccess;

  pNwkDiscCnf->req.status = nwkDiscStatus;

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pNwkDiscCnf );
}

/**************************************************************************************************
 * @fn          zdoBeaconNotifyIndCB
 *
 * @brief       callback function to handle Beacon Notification Indications
 *
 * @param       pStr - pointer to beacon notification information
 *
 * @return      NULL
 */
static void *zdoBeaconNotifyIndCB( void *pStr )
{
  sendMsgToAllCBs( ZS_ZDO_BEACON_NOTIFY_IND_CBID, pStr, sendBeaconNotifyInd );

  return (NULL);
}

/**************************************************************************************************
 * @fn          sendBeaconNotifyInd
 *
 * @brief       Function to send Beacon Notification Indication
 *
 * @param       dstID - Destination Task ID
 * @param       pStr - pointer to Beacon Notification information
 *
 * @return      none
 */
static void sendBeaconNotifyInd( uint16_t dstID, void *pStr )
{
  zstackmsg_zdoBeaconNotifyInd_t *pBeaconInd;
  zdoBeaconInd_t *pBeacon = (zdoBeaconInd_t *)pStr;

  pBeaconInd =
        (zstackmsg_zdoBeaconNotifyInd_t *)OsalPort_msgAllocate( sizeof(
          zstackmsg_zdoBeaconNotifyInd_t) );
  if ( pBeaconInd == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pBeaconInd, 0, sizeof(zstackmsg_zdoBeaconNotifyInd_t) );

  pBeaconInd->hdr.event = zstackmsg_CmdIDs_ZDO_BEACON_NOTIFY_IND;
  pBeaconInd->hdr.status = zstack_ZStatusValues_ZSuccess;

  pBeaconInd->req.sourceAddr = pBeacon->sourceAddr;
  pBeaconInd->req.panID = pBeacon->panID;
  pBeaconInd->req.logicalChannel = pBeacon->logicalChannel;
  pBeaconInd->req.permitJoining = pBeacon->permitJoining;
  pBeaconInd->req.routerCapacity = pBeacon->routerCapacity;
  pBeaconInd->req.deviceCapacity = pBeacon->deviceCapacity;
  pBeaconInd->req.protocolVersion = pBeacon->protocolVersion;
  pBeaconInd->req.stackProfile = pBeacon->stackProfile;
  pBeaconInd->req.lqi = pBeacon->LQI;
  pBeaconInd->req.depth = pBeacon->depth;
  pBeaconInd->req.updateID = pBeacon->updateID;
  OsalPort_memcpy( &(pBeaconInd->req.extendedPANID), pBeacon->extendedPanID, Z_EXTADDR_LEN );

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pBeaconInd );
}

/**************************************************************************************************
 * @fn          zdoSrcRtgCB
 *
 * @brief       callback function to handle Source Route indication
 *
 * @param       pStr - pointer to source route information
 *
 * @return      NULL
 */
static void *zdoSrcRtgCB( void *pStr )
{
  sendMsgToAllCBs( ZS_ZDO_SRC_RTG_IND_CBID, pStr, sendZdoSrcRtgInd );

  return (NULL);
}


/**************************************************************************************************
 * @fn          sendZdoSrcRtgInd
 *
 * @brief       Function to send a ZDO Source Route indication
 *
 * @param       dstID - Destination Task ID
 * @param       pStr - pointer to source route information
 *
 * @return      none
 */
static void sendZdoSrcRtgInd( uint16_t dstID, void *pStr )
{
  zstackmsg_zdoSrcRtgInd_t *pSrcRtgInd;
  zdoSrcRtg_t *pSrcRtg = (zdoSrcRtg_t *)pStr;

  pSrcRtgInd = (zstackmsg_zdoSrcRtgInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_zdoSrcRtgInd_t) );
  if ( pSrcRtgInd == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pSrcRtgInd, 0, sizeof(zstackmsg_zdoSrcRtgInd_t) );

  pSrcRtgInd->hdr.event = zstackmsg_CmdIDs_ZDO_SRC_RTG_IND;
  pSrcRtgInd->hdr.status = 0;

  pSrcRtgInd->req.pRelay = OsalPort_malloc( sizeof(uint16_t) * pSrcRtg->relayCnt );
  if ( pSrcRtgInd->req.pRelay == NULL )
  {
    OsalPort_msgDeallocate( (uint8_t*) pSrcRtgInd );
    return;
  }

  pSrcRtgInd->req.srcAddr = pSrcRtg->srcAddr;
  pSrcRtgInd->req.n_relay = pSrcRtg->relayCnt;
  if ( pSrcRtg->relayCnt )
  {
    int i;
    for ( i = 0; i < pSrcRtg->relayCnt; i++ )
    {
      pSrcRtgInd->req.pRelay[i] = pSrcRtg->pRelayList[i];
    }
  }

  // Send the a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pSrcRtgInd );
}



/**************************************************************************************************
 * @fn          zdoConcentratorIndCB
 *
 * @brief       callback function to handle Concentrator indication
 *
 * @param       pStr - pointer to concentrator information
 *
 * @return      NULL
 */
static void *zdoConcentratorIndCB( void *pStr )
{
  sendMsgToAllCBs( ZS_ZDO_CONCENTRATOR_IND_CBID, pStr, sendZdoConcentratorInd );

  return (NULL);
}


/**************************************************************************************************
 * @fn          sendZdoConcentratorInd
 *
 * @brief       function to send ZDO Concentrator indication
 *
 * @param       dstID - Destination Task ID
 * @param       pStr - pointer to concentrator information
 *
 * @return      none
 */
static void sendZdoConcentratorInd( uint16_t dstID, void *pStr )
{
  zstackmsg_zdoCncntrtrInd_t *pConInd;
  zdoConcentratorInd_t *pCon = (zdoConcentratorInd_t *)pStr;

  pConInd = (zstackmsg_zdoCncntrtrInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_zdoCncntrtrInd_t) );
  if ( pConInd == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pConInd, 0, sizeof(zstackmsg_zdoCncntrtrInd_t) );

  pConInd->hdr.event = zstackmsg_CmdIDs_ZDO_CONCENTRATOR_IND;
  pConInd->hdr.status = 0;

  pConInd->req.nwkAddr = pCon->nwkAddr;
  pConInd->req.pktCost = pCon->pktCost;
  OsalPort_memcpy( &(pConInd->req.ieeeAddr), pCon->extAddr, Z_EXTADDR_LEN );

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pConInd );
}


/**************************************************************************************************
 * @fn          zdoJoinCnfCB
 *
 * @brief       callback function to handle Join Confirm
 *
 * @param       pStr - pointer to join confirm information
 *
 * @return      NULL
 */
static void *zdoJoinCnfCB( void *pStr )
{
  sendMsgToAllCBs( ZS_ZDO_JOIN_CNF_CBID, pStr, sendJoinCnfInd );

  return (NULL);
}


/**************************************************************************************************
 * @fn          sendJoinCnfInd
 *
 * @brief       Function to send Join Confirm Indication
 *
 * @param       dstID - Destination Task ID
 * @param       pStr - pointer to join confirm information
 *
 * @return      none
 */
static void sendJoinCnfInd( uint16_t dstID, void *pStr )
{
  zstackmsg_zdoJoinConf_t *pJoinCnfInd;
  zdoJoinCnf_t *pJoinCnf = (zdoJoinCnf_t *)pStr;

  pJoinCnfInd = (zstackmsg_zdoJoinConf_t *)OsalPort_msgAllocate( sizeof(zstackmsg_zdoJoinConf_t) );
  if ( pJoinCnfInd == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pJoinCnfInd, 0, sizeof(zstackmsg_zdoJoinConf_t) );

  pJoinCnfInd->hdr.event = zstackmsg_CmdIDs_ZDO_JOIN_CNF;
  pJoinCnfInd->hdr.status = pJoinCnf->status;

  pJoinCnfInd->req.devAddr = pJoinCnf->deviceAddr;
  pJoinCnfInd->req.parentAddr = pJoinCnf->parentAddr;

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pJoinCnfInd );
}


/**************************************************************************************************
 * @fn          zdoLeaveIndCB
 *
 * @brief       callback function to handle Leave indication
 *
 * @param       pStr - pointer to leave information
 *
 * @return      NULL
 */
static void *zdoLeaveIndCB( void *pStr )
{
  sendMsgToAllCBs( ZS_ZDO_LEAVE_IND_CBID, pStr, sendLeaveInd );
#if defined ( BDB_TL_TARGET )
  targetZdoLeaveCnfCB(NULL);
#endif

  return (NULL);
}


/**************************************************************************************************
 * @fn          sendLeaveInd
 *
 * @brief       Function to send Leave Indication
 *
 * @param       dstID - Destination Task ID
 * @param       pStr - pointer to leave information
 *
 * @return      none
 */
static void sendLeaveInd( uint16_t dstID, void *pStr )
{
  zstackmsg_zdoLeaveInd_t *pLeave;
  NLME_LeaveInd_t *pLeaveInd = (NLME_LeaveInd_t *)pStr;

  pLeave = (zstackmsg_zdoLeaveInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_zdoLeaveInd_t) );
  if ( pLeave == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pLeave, 0, sizeof(zstackmsg_zdoLeaveInd_t) );

  pLeave->hdr.event = zstackmsg_CmdIDs_ZDO_LEAVE_IND;
  pLeave->hdr.status = 0;

  pLeave->req.srcAddr = pLeaveInd->srcAddr;
  pLeave->req.request = pLeaveInd->request;
  pLeave->req.rejoin = pLeaveInd->rejoin;
  pLeave->req.removeChildren = pLeaveInd->removeChildren;
  OsalPort_memcpy( &pLeave->req.extendedAddr, pLeaveInd->extAddr, Z_EXTADDR_LEN );

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pLeave );
}


/**************************************************************************************************
 * @fn          zdoPermitJoinIndCB
 *
 * @brief       callback function to handle Permit Join CB indication
 *
 * @param       pStr - pointer to permit join information
 *
 * @return      NULL
 */
static void *zdoPermitJoinIndCB( void *pStr )
{
  sendMsgToAllCBMsgs( ZS_DEV_PERMIT_JOIN_IND_CDID, 0, pStr, sendDevPermitJoinInd );

  return (NULL);
}

/**************************************************************************************************
 * @fn          sendDevPermitJoinInd
 *
 * @brief       Send a Device Permit Join Indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pStr - pointer to permit join information
 *
 * @return      none
 */
static void sendDevPermitJoinInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  uint8_t duration = *( (uint8_t *)pMsg );
  zstackmsg_devPermitJoinInd_t *pDevInd;

  pDevInd =
        (zstackmsg_devPermitJoinInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_devPermitJoinInd_t) );
  if ( pDevInd == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pDevInd, 0, sizeof(zstackmsg_devPermitJoinInd_t) );

  pDevInd->hdr.event = zstackmsg_CmdIDs_DEV_PERMIT_JOIN_IND;
  pDevInd->hdr.status = 0;
  pDevInd->Req.duration = duration;

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pDevInd );
}

/**************************************************************************************************
 * @fn          zdoTcDeviceIndCB
 *
 * @brief       callback function to handle Trust Center Device CB indication
 *
 * @param       pStr - pointer to Device information
 *
 * @return      NULL
 */
static void *zdoTcDeviceIndCB( void *pStr )
{
  sendMsgToAllCBMsgs( ZS_TC_DEVICE_IND_CDID, 0, pStr, sendTcDeviceInd );

  return (NULL);
}

/**************************************************************************************************
 * @fn          sendTcDeviceInd
 *
 * @brief       Send a ZDO TC Device indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to incoming message
 *
 * @return      none
 */
static void sendTcDeviceInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  ZDO_TC_Device_t *pDev = (ZDO_TC_Device_t *)pMsg;
  zstackmsg_zdoTcDeviceInd_t *pDevInd;

  pDevInd = (zstackmsg_zdoTcDeviceInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_zdoTcDeviceInd_t) );
  if ( pDevInd == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pDevInd, 0, sizeof(zstackmsg_zdoTcDeviceInd_t) );

  pDevInd->hdr.event = zstackmsg_CmdIDs_ZDO_TC_DEVICE_IND;
  pDevInd->hdr.status = 0;

  pDevInd->req.nwkAddr = pDev->nwkAddr;
  OsalPort_memcpy( &pDevInd->req.extendedAddr, pDev->extAddr, Z_EXTADDR_LEN );
  pDevInd->req.parentAddr = pDev->parentAddr;

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pDevInd );
}

/**************************************************************************************************
 * @fn          sendDeviceAnnounce
 *
 * @brief       function to handle Device Announce
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendDeviceAnnounce( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_DEVICE_ANNOUNCE_CDID, srcAddr, pMsg, sendDeviceAnnounceInd );
}

/**************************************************************************************************
 * @fn          sendDeviceAnnounceInd
 *
 * @brief       Send a ZDO Device Announce indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendDeviceAnnounceInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  ZDO_DeviceAnnce_t *pDevAnn = (ZDO_DeviceAnnce_t *)pMsg;
  zstackmsg_zdoDeviceAnnounceInd_t *pDevInd;

  pDevInd =
        (zstackmsg_zdoDeviceAnnounceInd_t *)OsalPort_msgAllocate( sizeof(
          zstackmsg_zdoDeviceAnnounceInd_t) );
  if ( pDevInd == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pDevInd, 0, sizeof(zstackmsg_zdoDeviceAnnounceInd_t) );

  pDevInd->hdr.event = zstackmsg_CmdIDs_ZDO_DEVICE_ANNOUNCE;
  pDevInd->hdr.status = 0;

  pDevInd->req.srcAddr = srcAddr;
  pDevInd->req.devAddr = pDevAnn->nwkAddr;

  OsalPort_memcpy( &pDevInd->req.devExtAddr, pDevAnn->extAddr, Z_EXTADDR_LEN );

  buildMsgCapInfo( pDevAnn->capabilities, &pDevInd->req.capInfo );

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pDevInd );
}

#if defined (ZDO_NWKADDR_REQUEST)
/**************************************************************************************************
 * @fn          sendNwkAddrRsp
 *
 * @brief       function to handle a ZDO Network Address Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendNwkAddrRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_NWK_ADDR_RSP_CDID, srcAddr, pMsg, sendNwkAddrRspInd );
}

/**************************************************************************************************
 * @fn          sendNwkAddrRspInd
 *
 * @brief       Send a ZDO Network Address Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendNwkAddrRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  ZDO_NwkIEEEAddrResp_t *pAddrRsp = (ZDO_NwkIEEEAddrResp_t *)pMsg;
  zstackmsg_zdoNwkAddrRspInd_t *pNwkAddrRsp;

  pNwkAddrRsp =
        (zstackmsg_zdoNwkAddrRspInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_zdoNwkAddrRspInd_t) );
  if ( pNwkAddrRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pNwkAddrRsp, 0, sizeof(zstackmsg_zdoNwkAddrRspInd_t) );

  pNwkAddrRsp->hdr.event = zstackmsg_CmdIDs_ZDO_NWK_ADDR_RSP;
  pNwkAddrRsp->hdr.status = 0;

  pNwkAddrRsp->rsp.status = (zstack_ZdpStatus)pAddrRsp->status;
  pNwkAddrRsp->rsp.nwkAddr = pAddrRsp->nwkAddr;
  OsalPort_memcpy( &pNwkAddrRsp->rsp.ieeeAddr, pAddrRsp->extAddr, Z_EXTADDR_LEN );

  pNwkAddrRsp->rsp.n_assocDevList = pAddrRsp->numAssocDevs;
  if ( pNwkAddrRsp->rsp.n_assocDevList )
  {
    pNwkAddrRsp->rsp.pAssocDevList =
          (uint16_t *)OsalPort_malloc( (sizeof(uint16_t) * pAddrRsp->numAssocDevs) );
    if ( pNwkAddrRsp->rsp.pAssocDevList )
    {
      int i;
      for ( i = 0; i < pNwkAddrRsp->rsp.n_assocDevList; i++ )
      {
        pNwkAddrRsp->rsp.pAssocDevList[i] = pAddrRsp->devList[i];
      }
    }
  }

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pNwkAddrRsp );
}
#endif // ZDO_NWKADDR_REQUEST

#if defined (ZDO_IEEEADDR_REQUEST)
/**************************************************************************************************
 * @fn          sendIeeeAddrRsp
 *
 * @brief       function to handle a ZDO IEEE Address Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendIeeeAddrRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_IEEE_ADDR_RSP_CDID, srcAddr, pMsg, sendIeeeAddrRspInd );
}

/**************************************************************************************************
 * @fn          sendIeeeAddrRspInd
 *
 * @brief       Send a ZDO IEEE Address Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendIeeeAddrRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  ZDO_NwkIEEEAddrResp_t *pAddrRsp = (ZDO_NwkIEEEAddrResp_t *)pMsg;
  zstackmsg_zdoIeeeAddrRspInd_t *pIeeeAddrRsp;

  pIeeeAddrRsp =
        (zstackmsg_zdoIeeeAddrRspInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_zdoIeeeAddrRspInd_t) );
  if ( pIeeeAddrRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pIeeeAddrRsp, 0, sizeof(zstackmsg_zdoIeeeAddrRspInd_t) );

  pIeeeAddrRsp->hdr.event = zstackmsg_CmdIDs_ZDO_IEEE_ADDR_RSP;
  pIeeeAddrRsp->hdr.status = 0;

  pIeeeAddrRsp->rsp.status = (zstack_ZdpStatus)pAddrRsp->status;
  pIeeeAddrRsp->rsp.nwkAddr = pAddrRsp->nwkAddr;
  OsalPort_memcpy( &pIeeeAddrRsp->rsp.ieeeAddr, pAddrRsp->extAddr, Z_EXTADDR_LEN );

  pIeeeAddrRsp->rsp.n_assocDevList = pAddrRsp->numAssocDevs;
  if ( pIeeeAddrRsp->rsp.n_assocDevList )
  {
    pIeeeAddrRsp->rsp.pAssocDevList =
          (uint16_t *)OsalPort_malloc( (sizeof(uint16_t) * pAddrRsp->numAssocDevs) );
    if ( pIeeeAddrRsp->rsp.pAssocDevList )
    {
      int i;
      for ( i = 0; i < pIeeeAddrRsp->rsp.n_assocDevList; i++ )
      {
        pIeeeAddrRsp->rsp.pAssocDevList[i] = pAddrRsp->devList[i];
      }
    }
  }

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pIeeeAddrRsp );
}
#endif // ZDO_IEEEADDR_REQUEST

#if defined (ZDO_NODEDESC_REQUEST)
/**************************************************************************************************
 * @fn          sendNodeDescRsp
 *
 * @brief       function to handle a ZDO Node Descriptor Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendNodeDescRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_NODE_DESC_RSP_CDID, srcAddr, pMsg, sendNodeDescRspInd );
}

/**************************************************************************************************
 * @fn          sendNodeDescRspInd
 *
 * @brief       Send a ZDO Node Descriptor Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendNodeDescRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  ZDO_NodeDescRsp_t *pNdRsp = (ZDO_NodeDescRsp_t *)pMsg;
  zstackmsg_zdoNodeDescRspInd_t *pRsp;

  pRsp = (zstackmsg_zdoNodeDescRspInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_zdoNodeDescRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoNodeDescRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_NODE_DESC_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)pNdRsp->status;
  pRsp->rsp.nwkAddrOfInterest = pNdRsp->nwkAddr;

  buildMsgCapInfo( pNdRsp->nodeDesc.CapabilityFlags, &pRsp->rsp.nodeDesc.capInfo );
  buildMsgServerCap( pNdRsp->nodeDesc.ServerMask, &pRsp->rsp.nodeDesc.serverMask );
  pRsp->rsp.nodeDesc.logicalType = (zstack_LogicalTypes)pNdRsp->nodeDesc.LogicalType;
  pRsp->rsp.nodeDesc.complexDescAvail = pNdRsp->nodeDesc.ComplexDescAvail;
  pRsp->rsp.nodeDesc.userDescAvail = pNdRsp->nodeDesc.UserDescAvail;
  pRsp->rsp.nodeDesc.apsFlags = pNdRsp->nodeDesc.APSFlags;
  pRsp->rsp.nodeDesc.freqBand = pNdRsp->nodeDesc.FrequencyBand;
  pRsp->rsp.nodeDesc.manufacturerCode =
        BUILD_UINT16( pNdRsp->nodeDesc.ManufacturerCode[0],
        pNdRsp->nodeDesc.ManufacturerCode[1] );
  pRsp->rsp.nodeDesc.maxBufferSize = pNdRsp->nodeDesc.MaxBufferSize;
  pRsp->rsp.nodeDesc.maxInTransferSize =
        BUILD_UINT16( pNdRsp->nodeDesc.MaxInTransferSize[0],
        pNdRsp->nodeDesc.MaxInTransferSize[1] );
  pRsp->rsp.nodeDesc.maxOutTransferSize =
        BUILD_UINT16( pNdRsp->nodeDesc.MaxOutTransferSize[0],
        pNdRsp->nodeDesc.MaxOutTransferSize[1] );
  pRsp->rsp.nodeDesc.descCap = pNdRsp->nodeDesc.DescriptorCapability;

  // Send to a subscriber
  OsalPort_msgSend( dstID,
        (uint8_t*)pRsp );
}
#endif

#if defined (ZDO_POWERDESC_REQUEST)
/**************************************************************************************************
 * @fn          sendPowerDescRsp
 *
 * @brief       function to handle a ZDO Power Descriptor Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendPowerDescRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_POWER_DESC_RSP_CDID, srcAddr, pMsg, sendPowerDescRspInd );
}

/**************************************************************************************************
 * @fn          sendPowerDescRspInd
 *
 * @brief       Send a ZDO Power Descriptor Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendPowerDescRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  ZDO_PowerRsp_t *pPowerRsp = (ZDO_PowerRsp_t *)pMsg;
  zstackmsg_zdoPowerDescRspInd_t *pRsp;

  pRsp =
        (zstackmsg_zdoPowerDescRspInd_t *)OsalPort_msgAllocate( sizeof(
          zstackmsg_zdoPowerDescRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoPowerDescRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_POWER_DESC_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)pPowerRsp->status;
  pRsp->rsp.nwkAddrOfInterest = pPowerRsp->nwkAddr;

  buildMsgPowerSource( pPowerRsp->pwrDesc.AvailablePowerSources,
        &pRsp->rsp.powerDesc.availPowerSource );
  buildMsgPowerSource( pPowerRsp->pwrDesc.CurrentPowerSource,
        &pRsp->rsp.powerDesc.currentPowerSource );

  pRsp->rsp.powerDesc.currentPowerLevel
    = (zstack_PowerLevel)pPowerRsp->pwrDesc.CurrentPowerSourceLevel;
  pRsp->rsp.powerDesc.powerMode = (zstack_PowerModes)pPowerRsp->pwrDesc.PowerMode;

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif

#if defined (ZDO_SIMPLEDESC_REQUEST)
/**************************************************************************************************
 * @fn          sendSimpleDescRsp
 *
 * @brief       function to handle a ZDO Simple Descriptor Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendSimpleDescRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_SIMPLE_DESC_RSP_CDID, srcAddr, pMsg, sendSimpleDescRspInd );
}

/**************************************************************************************************
 * @fn          sendSimpleDescRspInd
 *
 * @brief       Send a ZDO Simple Descriptor Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendSimpleDescRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  int i;
  ZDO_SimpleDescRsp_t *pSimpleRsp = (ZDO_SimpleDescRsp_t *)pMsg;
  zstackmsg_zdoSimpleDescRspInd_t *pRsp;

  pRsp =
        (zstackmsg_zdoSimpleDescRspInd_t *)OsalPort_msgAllocate( sizeof(
          zstackmsg_zdoSimpleDescRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoSimpleDescRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_SIMPLE_DESC_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)pSimpleRsp->status;
  pRsp->rsp.nwkAddrOfInterest = pSimpleRsp->nwkAddr;

  pRsp->rsp.simpleDesc.deviceID = pSimpleRsp->simpleDesc.AppDeviceId;
  pRsp->rsp.simpleDesc.deviceVer = pSimpleRsp->simpleDesc.AppDevVer;
  pRsp->rsp.simpleDesc.endpoint = pSimpleRsp->simpleDesc.EndPoint;
  pRsp->rsp.simpleDesc.profileID = pSimpleRsp->simpleDesc.AppProfId;
  pRsp->rsp.simpleDesc.n_inputClusters = pSimpleRsp->simpleDesc.AppNumInClusters;
  pRsp->rsp.simpleDesc.n_outputClusters = pSimpleRsp->simpleDesc.AppNumOutClusters;

  pRsp->rsp.simpleDesc.pInputClusters =
        (uint16_t *)OsalPort_malloc( (sizeof(uint16_t) * pSimpleRsp->simpleDesc.AppNumInClusters) );
  if ( pRsp->rsp.simpleDesc.pInputClusters )
  {
    for ( i = 0; i < pRsp->rsp.simpleDesc.n_inputClusters; i++ )
    {
      pRsp->rsp.simpleDesc.pInputClusters[i]
        = pSimpleRsp->simpleDesc.pAppInClusterList[i];
    }
  }

  pRsp->rsp.simpleDesc.pOutputClusters =
        (uint16_t *)OsalPort_malloc( (sizeof(uint16_t) * pSimpleRsp->simpleDesc.AppNumOutClusters) );
  if ( pRsp->rsp.simpleDesc.pOutputClusters )
  {
    for ( i = 0; i < pRsp->rsp.simpleDesc.n_outputClusters; i++ )
    {
      pRsp->rsp.simpleDesc.pOutputClusters[i] =
            pSimpleRsp->simpleDesc.pAppOutClusterList[i];
    }
  }

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif

#if defined (ZDO_ACTIVEEP_REQUEST)
/**************************************************************************************************
 * @fn          sendActiveEPRsp
 *
 * @brief       function to handle a ZDO Active Endpoint Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendActiveEPRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_ACTIVE_EP_RSP_CDID, srcAddr, pMsg, sendActiveEPRspInd );
}

/**************************************************************************************************
 * @fn          sendActiveEPRspInd
 *
 * @brief       Send a ZDO Active Endpoint Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendActiveEPRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  ZDO_ActiveEndpointRsp_t *pActiveEPRsp = (ZDO_ActiveEndpointRsp_t *)pMsg;
  zstackmsg_zdoActiveEndpointsRspInd_t *pRsp;

  pRsp =
        (zstackmsg_zdoActiveEndpointsRspInd_t *)OsalPort_msgAllocate( sizeof(
          zstackmsg_zdoActiveEndpointsRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoActiveEndpointsRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_ACTIVE_EP_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)pActiveEPRsp->status;
  pRsp->rsp.nwkAddrOfInterest = pActiveEPRsp->nwkAddr;

  pRsp->rsp.n_activeEPList = pActiveEPRsp->cnt;

  pRsp->rsp.pActiveEPList = (uint8_t *)OsalPort_malloc( (sizeof(uint8_t) * pActiveEPRsp->cnt) );
  if ( pRsp->rsp.pActiveEPList )
  {
    int i;
    for ( i = 0; i < pRsp->rsp.n_activeEPList; i++ )
    {
      pRsp->rsp.pActiveEPList[i] = pActiveEPRsp->epList[i];
    }
  }

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif

#if defined (ZDO_MATCH_REQUEST)
/**************************************************************************************************
 * @fn          sendMatchDescRsp
 *
 * @brief       function to handle a ZDO Match Descriptor Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMatchDescRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_MATCH_DESC_RSP_CDID, srcAddr, pMsg, sendMatchDescRspInd );
}

/**************************************************************************************************
 * @fn          sendMatchDescRspInd
 *
 * @brief       Send a ZDO Match Descriptor Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMatchDescRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  ZDO_ActiveEndpointRsp_t *pActiveEPRsp = (ZDO_ActiveEndpointRsp_t *)pMsg;
  zstackmsg_zdoMatchDescRspInd_t *pRsp;

  pRsp =
        (zstackmsg_zdoMatchDescRspInd_t *)OsalPort_msgAllocate( sizeof(
          zstackmsg_zdoMatchDescRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoMatchDescRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_MATCH_DESC_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)pActiveEPRsp->status;
  pRsp->rsp.nwkAddrOfInterest = pActiveEPRsp->nwkAddr;

  pRsp->rsp.n_matchList = pActiveEPRsp->cnt;

  pRsp->rsp.pMatchList = (uint8_t *)OsalPort_malloc( (sizeof(uint8_t) * pActiveEPRsp->cnt) );
  if ( pRsp->rsp.pMatchList )
  {
    int i;
    for ( i = 0; i < pRsp->rsp.n_matchList; i++ )
    {
      pRsp->rsp.pMatchList[i] = pActiveEPRsp->epList[i];
    }
  }

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif

#if defined (ZDO_USERDESC_REQUEST)
/**************************************************************************************************
 * @fn          sendUserDescRsp
 *
 * @brief       function to handle a ZDO User Descriptor Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendUserDescRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_USER_DESC_RSP_CDID, srcAddr, pMsg, sendUserDescRspInd );
}

/**************************************************************************************************
 * @fn          sendUserDescRspInd
 *
 * @brief       Send a ZDO User Descriptor Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendUserDescRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  ZDO_UserDescRsp_t *pUdRsp = (ZDO_UserDescRsp_t *)pMsg;
  zstackmsg_zdoUserDescRspInd_t *pRsp;

  pRsp = (zstackmsg_zdoUserDescRspInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_zdoUserDescRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoUserDescRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_USER_DESC_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)pUdRsp->status;
  pRsp->rsp.nwkAddrOfInterest = pUdRsp->nwkAddr;

  pRsp->rsp.n_desc = pUdRsp->length;

  pRsp->rsp.pDesc = OsalPort_malloc( pUdRsp->length );
  if ( pRsp->rsp.pDesc )
  {
    OsalPort_memcpy( pRsp->rsp.pDesc, pUdRsp->desc, pUdRsp->length );
  }

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif

#if defined (ZDO_SERVERDISC_REQUEST)
/**************************************************************************************************
 * @fn          sendServerDiscRsp
 *
 * @brief       function to handle a ZDO Server Descriptor Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendServerDiscRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_SERVER_DISCOVERY_RSP_CDID, srcAddr,
        pMsg, sendServerDiscRspInd );
}

/**************************************************************************************************
 * @fn          sendServerDiscRspInd
 *
 * @brief       Send a ZDO Server Descriptor Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendServerDiscRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  ZDO_ServerDiscRsp_t *pSdRsp = (ZDO_ServerDiscRsp_t *)pMsg;
  zstackmsg_zdoServerDiscoveryRspInd_t *pRsp;

  pRsp =
        (zstackmsg_zdoServerDiscoveryRspInd_t *)OsalPort_msgAllocate( sizeof(
          zstackmsg_zdoServerDiscoveryRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoServerDiscoveryRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_SERVER_DISC_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)pSdRsp->status;
  //pRsp->rsp.nwkAddrOfInterest = pSdRsp->nwkAddr;

  buildMsgServerCap( pSdRsp->serverMask, &pRsp->rsp.serverCap );

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif

#if defined (ZDO_BIND_UNBIND_REQUEST)
/**************************************************************************************************
 * @fn          sendBindRsp
 *
 * @brief       function to handle a ZDO Bind Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendBindRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_BIND_RSP_CDID, srcAddr, pMsg, sendBindRspInd );
}

/**************************************************************************************************
 * @fn          sendBindRspInd
 *
 * @brief       Send a ZDO Bind Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendBindRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  uint8_t result = *( (uint8_t *)pMsg );
  zstackmsg_zdoBindRspInd_t *pRsp;

  pRsp = (zstackmsg_zdoBindRspInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_zdoBindRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoBindRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_BIND_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)result;

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif

#if defined (ZDO_ENDDEVICEBIND_REQUEST)
/**************************************************************************************************
 * @fn          sendEndDeviceBindRsp
 *
 * @brief       function to handle a ZDO End Device Bind Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendEndDeviceBindRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_END_DEVICE_BIND_RSP_CDID, srcAddr,
        pMsg, sendEndDeviceBindRspInd );
}

/**************************************************************************************************
 * @fn          sendEndDeviceBindRspInd
 *
 * @brief       Send a ZDO End Device Bind Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendEndDeviceBindRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  uint8_t result = *( (uint8_t *)pMsg );
  zstackmsg_zdoEndDeviceBindRspInd_t *pRsp;

  pRsp =
        (zstackmsg_zdoEndDeviceBindRspInd_t *)OsalPort_msgAllocate( sizeof(
          zstackmsg_zdoEndDeviceBindRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoEndDeviceBindRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_END_DEVICE_BIND_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)result;

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif

#if defined (ZDO_BIND_UNBIND_REQUEST)
/**************************************************************************************************
 * @fn          sendUnbindRsp
 *
 * @brief       function to handle a ZDO Unbind Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendUnbindRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_UNBIND_RSP_CDID, srcAddr, pMsg, sendUnbindRspInd );
}

/**************************************************************************************************
 * @fn          sendUnbindRspInd
 *
 * @brief       Send a ZDO Unbind Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendUnbindRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  uint8_t result = *( (uint8_t *)pMsg );
  zstackmsg_zdoUnbindRspInd_t *pRsp;

  pRsp = (zstackmsg_zdoUnbindRspInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_zdoUnbindRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoUnbindRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_UNBIND_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)result;

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif

#if defined (ZDO_MGMT_NWKDISC_REQUEST)
/**************************************************************************************************
 * @fn          sendMgmtNwkDiscRsp
 *
 * @brief       function to handle a ZDO Management Network Discovery Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMgmtNwkDiscRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_MGMT_NWK_DISC_RSP_CDID, srcAddr,
        pMsg, sendMgmtNwkDiscRspInd );
}

/**************************************************************************************************
 * @fn          sendMgmtNwkDiscRspInd
 *
 * @brief       Send a ZDO Management Network Discovery Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMgmtNwkDiscRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  ZDO_MgmNwkDiscRsp_t *pNwkDiscRsp = (ZDO_MgmNwkDiscRsp_t *)pMsg;
  zstackmsg_zdoMgmtNwkDiscRspInd_t *pRsp;

  pRsp =
        (zstackmsg_zdoMgmtNwkDiscRspInd_t *)OsalPort_msgAllocate( sizeof(
          zstackmsg_zdoMgmtNwkDiscRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoMgmtNwkDiscRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_MGMT_NWK_DISC_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)pNwkDiscRsp->status;

  pRsp->rsp.networkCount = pNwkDiscRsp->networkCount;
  pRsp->rsp.startIndex = pNwkDiscRsp->startIndex;
  pRsp->rsp.n_netList = pNwkDiscRsp->networkListCount;

  pRsp->rsp.pNetList =
        (zstack_nwkDiscItem_t *)OsalPort_malloc( (sizeof(zstack_nwkDiscItem_t) *
          pRsp->rsp.n_netList) );
  if ( pRsp->rsp.pNetList )
  {
    int x;
    zstack_nwkDiscItem_t *pDisc = pRsp->rsp.pNetList;
    for ( x = 0; x < pRsp->rsp.n_netList; x++, pDisc++ )
    {
      memset( pDisc, 0, sizeof(zstack_nwkDiscItem_t) );

      OsalPort_memcpy( &(pDisc->extendedPANID), pNwkDiscRsp->list[x].extendedPANID, Z_EXTADDR_LEN );
      pDisc->logicalChan = pNwkDiscRsp->list[x].logicalChannel;
      pDisc->stackProfile = pNwkDiscRsp->list[x].stackProfile;
      pDisc->version = pNwkDiscRsp->list[x].version;
      pDisc->beaconOrder = pNwkDiscRsp->list[x].beaconOrder;
      pDisc->superFrameOrder = pNwkDiscRsp->list[x].superFrameOrder;
      pDisc->permitJoin = pNwkDiscRsp->list[x].permitJoining;
    }
  }

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif // ZDO_MGMT_NWKDISC_REQUEST

#if defined (ZDO_MGMT_LQI_REQUEST)
/**************************************************************************************************
 * @fn          sendMgmtLqiRsp
 *
 * @brief       function to handle a ZDO Management LQI Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMgmtLqiRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_MGMT_LQI_RSP_CDID, srcAddr, pMsg, sendMgmtLqiRspInd );
}

/**************************************************************************************************
 * @fn          sendMgmtLqiRspInd
 *
 * @brief       Send a ZDO Management LQI Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMgmtLqiRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  ZDO_MgmtLqiRsp_t *pLqiRsp = (ZDO_MgmtLqiRsp_t *)pMsg;
  zstackmsg_zdoMgmtLqiRspInd_t *pRsp;

  pRsp = (zstackmsg_zdoMgmtLqiRspInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_zdoMgmtLqiRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoMgmtLqiRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_MGMT_LQI_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)pLqiRsp->status;

  pRsp->rsp.neighborLqiEntries = pLqiRsp->neighborLqiEntries;
  pRsp->rsp.startIndex = pLqiRsp->startIndex;

  pRsp->rsp.n_lqiList = pLqiRsp->neighborLqiCount;
  pRsp->rsp.pLqiList =
        (zstack_nwkLqiItem_t *)OsalPort_malloc( (sizeof(zstack_nwkLqiItem_t) *
          pRsp->rsp.n_lqiList) );
  if ( pRsp->rsp.pLqiList )
  {
    zstack_nwkLqiItem_t *pItem = pRsp->rsp.pLqiList;
    ZDP_MgmtLqiItem_t *pList = (ZDP_MgmtLqiItem_t *)pLqiRsp->list;
    int x;

    memset( pRsp->rsp.pLqiList, 0,
          (sizeof(zstack_nwkLqiItem_t) * pRsp->rsp.n_lqiList) );

    for ( x = 0; x < pRsp->rsp.n_lqiList; x++, pItem++, pList++ )
    {
      OsalPort_memcpy( &(pItem->extendedPANID), pList->extPanID, Z_EXTADDR_LEN );
      OsalPort_memcpy( &(pItem->extendedAddr), pList->extAddr, Z_EXTADDR_LEN );
      pItem->panID = pList->panID;
      pItem->nwkAddr = pList->nwkAddr;
      pItem->deviceType = (zstack_LogicalTypes)pList->devType;
      pItem->rxOnWhenIdle = (zstack_RxOnWhenIdleTypes)pList->rxOnIdle;
      pItem->relationship = (zstack_RelationTypes)pList->relation;
      pItem->permit = (zstack_PermitJoinTypes)pList->permit;
      pItem->depth = pList->depth;
      pItem->rxLqi = pList->lqi;
    }
  }

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif // ZDO_MGMT_LQI_REQUEST

#if defined (ZDO_MGMT_RTG_REQUEST)
/**************************************************************************************************
 * @fn          sendMgmtRtgRsp
 *
 * @brief       function to handle a ZDO Management Routing Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMgmtRtgRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_MGMT_RTG_RSP_CDID, srcAddr, pMsg, sendMgmtRtgRspInd );
}

/**************************************************************************************************
 * @fn          sendMgmtRtgRspInd
 *
 * @brief       Send a ZDO Management Routing Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMgmtRtgRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  ZDO_MgmtRtgRsp_t *pRtgRsp = (ZDO_MgmtRtgRsp_t *)pMsg;
  zstackmsg_zdoMgmtRtgRspInd_t *pRsp;

  pRsp = (zstackmsg_zdoMgmtRtgRspInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_zdoMgmtRtgRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoMgmtRtgRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_MGMT_RTG_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)pRtgRsp->status;

  pRsp->rsp.rtgEntries = pRtgRsp->rtgCount;
  pRsp->rsp.startIndex = pRtgRsp->startIndex;
  pRsp->rsp.n_rtgList = pRtgRsp->rtgListCount;

  pRsp->rsp.pRtgList =
        (zstack_routeItem_t *)OsalPort_malloc( (sizeof(zstack_routeItem_t) * pRsp->rsp.n_rtgList) );
  if ( pRsp->rsp.pRtgList )
  {
    zstack_routeItem_t *pItem = (zstack_routeItem_t *)pRsp->rsp.pRtgList;
    int x;

    memset( pRsp->rsp.pRtgList, 0,
          (sizeof(zstack_routeItem_t) * pRsp->rsp.n_rtgList) );

    for ( x = 0; x < pRsp->rsp.n_rtgList; x++, pItem++ )
    {
      pItem->dstAddr = pRtgRsp->list[x].dstAddress;
      pItem->nextHop = pRtgRsp->list[x].nextHopAddress;
      pItem->status = (zstack_RouteStatus)pRtgRsp->list[x].status;

      if ( pRtgRsp->list[x].options & ZDO_MGMT_RTG_ENTRY_MANYTOONE )
      {
        pItem->manyToOne = TRUE;
      }

      if ( pRtgRsp->list[x].options & ZDO_MGMT_RTG_ENTRY_ROUTE_RECORD_REQUIRED )
      {
        pItem->routeRecordRequired = TRUE;
      }

      if ( pRtgRsp->list[x].options & ZDO_MGMT_RTG_ENTRY_MEMORY_CONSTRAINED )
      {
        pItem->memoryConstrained = TRUE;
      }
    }
  }

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif // ZDO_MGMT_RTG_REQUEST

#if defined (ZDO_MGMT_BIND_REQUEST)
/**************************************************************************************************
 * @fn          sendMgmtBindRsp
 *
 * @brief       function to handle a ZDO Management Bind Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMgmtBindRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_MGMT_BIND_RSP_CDID, srcAddr, pMsg, sendMgmtBindRspInd );
}

/**************************************************************************************************
 * @fn          sendMgmtBindRspInd
 *
 * @brief       Send a ZDO Management Bind Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMgmtBindRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  ZDO_MgmtBindRsp_t *pBindRsp = (ZDO_MgmtBindRsp_t *)pMsg;
  zstackmsg_zdoMgmtBindRspInd_t *pRsp;

  pRsp = (zstackmsg_zdoMgmtBindRspInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_zdoMgmtBindRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoMgmtBindRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_MGMT_BIND_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)pBindRsp->status;

  pRsp->rsp.bindEntries = pBindRsp->bindingCount;
  pRsp->rsp.startIndex = pBindRsp->startIndex;
  pRsp->rsp.n_bindList = pBindRsp->bindingListCount;

  pRsp->rsp.pBindList =
        (zstack_bindItem_t *)OsalPort_malloc( (sizeof(zstack_bindItem_t) * pRsp->rsp.n_bindList) );
  if ( pRsp->rsp.pBindList )
  {
    zstack_bindItem_t *pItem = (zstack_bindItem_t *)pRsp->rsp.pBindList;
    int x;

    memset( pRsp->rsp.pBindList, 0,
          (sizeof(zstack_bindItem_t) * pRsp->rsp.n_bindList) );

    for ( x = 0; x < pRsp->rsp.n_bindList; x++, pItem++ )
    {
      OsalPort_memcpy( &(pItem->srcAddr), pBindRsp->list[x].srcAddr, Z_EXTADDR_LEN );
      pItem->srcEndpoint = pBindRsp->list[x].srcEP;
      pItem->clustedID = pBindRsp->list[x].clusterID;

      pItem->dstAddr.addrMode
        = (zstack_AFAddrMode)pBindRsp->list[x].dstAddr.addrMode;

      if ( pItem->dstAddr.addrMode == zstack_AFAddrMode_EXT )
      {
        OsalPort_memcpy( &(pItem->dstAddr.addr.extAddr),
              pBindRsp->list[x].dstAddr.addr.extAddr, Z_EXTADDR_LEN );
      }
      else
      {
        pItem->dstAddr.addr.shortAddr =
              pBindRsp->list[x].dstAddr.addr.shortAddr;
      }

      pItem->dstAddr.endpoint = pBindRsp->list[x].dstEP;
    }
  }

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif // ZDO_MGMT_BIND_REQUEST

#if defined (ZDO_MGMT_LEAVE_REQUEST)
/**************************************************************************************************
 * @fn          sendMgmtLeaveRsp
 *
 * @brief       function to handle a ZDO Management Leave Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMgmtLeaveRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_MGMT_LEAVE_RSP_CDID, srcAddr, pMsg, sendMgmtLeaveRspInd );
}

/**************************************************************************************************
 * @fn          sendMgmtLeaveRspInd
 *
 * @brief       Send a ZDO Management Leave Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMgmtLeaveRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  uint8_t result = *(uint8_t *)pMsg;
  zstackmsg_zdoMgmtLeaveRspInd_t *pRsp;

  pRsp =
        (zstackmsg_zdoMgmtLeaveRspInd_t *)OsalPort_msgAllocate( sizeof(
          zstackmsg_zdoMgmtLeaveRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoMgmtLeaveRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_MGMT_LEAVE_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)result;

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif // ZDO_MGMT_LEAVE_REQUEST

#if defined (ZDO_MGMT_JOINDIRECT_REQUEST)
/**************************************************************************************************
 * @fn          sendMgmtDirectJoinRsp
 *
 * @brief       function to handle a ZDO Management Direct Join Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMgmtDirectJoinRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_MGMT_DIRECT_JOIN_RSP_CDID, srcAddr,
        pMsg, sendMgmtDirectJoinRspInd );
}

/**************************************************************************************************
 * @fn          sendMgmtDirectJoinRspInd
 *
 * @brief       Send a ZDO Management Direct Join Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMgmtDirectJoinRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  uint8_t result = *(uint8_t *)pMsg;
  zstackmsg_zdoMgmtDirectJoinRspInd_t *pRsp;

  pRsp =
        (zstackmsg_zdoMgmtDirectJoinRspInd_t *)OsalPort_msgAllocate( sizeof(
          zstackmsg_zdoMgmtDirectJoinRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoMgmtDirectJoinRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_MGMT_DIRECT_JOIN_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)result;

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif // ZDO_MGMT_JOINDIRECT_REQUEST

#if defined (ZDO_MGMT_PERMIT_JOIN_REQUEST)
/**************************************************************************************************
 * @fn          sendMgmtPermitJoinRsp
 *
 * @brief       function to handle a ZDO Management Permit Join Response
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMgmtPermitJoinRsp( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_MGMT_PERMIT_JOIN_RSP_CDID, srcAddr,
        pMsg, sendMgmtPermitJoinRspInd );
}

/**************************************************************************************************
 * @fn          sendMgmtPermitJoinRspInd
 *
 * @brief       Send a ZDO Management Permit Join Response indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMgmtPermitJoinRspInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  uint8_t result = *(uint8_t *)pMsg;
  zstackmsg_zdoMgmtPermitJoinRspInd_t *pRsp;

  pRsp =
        (zstackmsg_zdoMgmtPermitJoinRspInd_t *)OsalPort_msgAllocate( sizeof(
          zstackmsg_zdoMgmtPermitJoinRspInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoMgmtPermitJoinRspInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_MGMT_PERMIT_JOIN_RSP;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)result;

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif // ZDO_MGMT_PERMIT_JOIN_REQUEST

#if defined (ZDO_MGMT_NWKUPDATE_REQUEST)
/**************************************************************************************************
 * @fn          sendMgmtNwkUpdateNotify
 *
 * @brief       function to handle a ZDO Management Network Update Notify
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMgmtNwkUpdateNotify( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_ZDO_MGMT_NWK_UPDATE_NOTIFY_CDID, srcAddr,
        pMsg, sendMgmtNwkUpdateNotifyInd );
}

/**************************************************************************************************
 * @fn          sendMgmtNwkUpdateNotifyInd
 *
 * @brief       Send a ZDO Management Network Update Notify indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendMgmtNwkUpdateNotifyInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  ZDO_MgmtNwkUpdateNotify_t *pNotify = (ZDO_MgmtNwkUpdateNotify_t *)pMsg;
  zstackmsg_zdoMgmtNwkUpdateNotifyInd_t *pRsp;

  pRsp =
        (zstackmsg_zdoMgmtNwkUpdateNotifyInd_t *)OsalPort_msgAllocate( sizeof(
          zstackmsg_zdoMgmtNwkUpdateNotifyInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_zdoMgmtNwkUpdateNotifyInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_ZDO_MGMT_NWK_UPDATE_NOTIFY;
  pRsp->hdr.status = 0;

  pRsp->rsp.srcAddr = srcAddr;
  pRsp->rsp.status = (zstack_ZdpStatus)pNotify->status;

  pRsp->rsp.scannedChannels = pNotify->scannedChannels;
  pRsp->rsp.totalTrans = pNotify->totalTransmissions;
  pRsp->rsp.transFails = pNotify->transmissionFailures;
  pRsp->rsp.n_energyValuesList = pNotify->listCount;

  pRsp->rsp.pEnergyValuesList = OsalPort_malloc( pRsp->rsp.n_energyValuesList );

  if ( pRsp->rsp.pEnergyValuesList )
  {
    OsalPort_memcpy( pRsp->rsp.pEnergyValuesList, pNotify->energyValues,
          pRsp->rsp.n_energyValuesList );
  }

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}
#endif // ZDO_MGMT_NWKUPDATE_REQUEST

/**************************************************************************************************
 * @fn          processDevStateChange
 *
 * @brief       Process a Device State Change
 *
 * @param       srcAddr - Network address of source device
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void processDevStateChange( uint16_t srcAddr, void *pMsg )
{
  sendMsgToAllCBMsgs( ZS_DEV_STATE_CHANGE_CDID, srcAddr, pMsg,
        sendDevStateChangeInd );
}

/**************************************************************************************************
 * @fn          sendDevStateChangeInd
 *
 * @brief       Send a Device State Change indication
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      none
 */
static void sendDevStateChangeInd( uint16_t dstID, uint16_t srcAddr, void *pMsg )
{
  uint8_t state = *( (uint8_t *)pMsg );
  zstackmsg_devStateChangeInd_t *pRsp;

  pRsp = (zstackmsg_devStateChangeInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_devStateChangeInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_devStateChangeInd_t) );

  pRsp->hdr.event = zstackmsg_CmdIDs_DEV_STATE_CHANGE_IND;
  pRsp->req.state = (zstack_DevState)state;

  // Send to a subscriber
  OsalPort_msgSend( dstID, (uint8_t*)pRsp );
}

/**************************************************************************************************
 * @fn          processSysVersionReq
 *
 * @brief       Process the System Version Request by filling out
 *              the response section of the message
 *
 * @param       dstID - Destination Task ID
 * @param       srcAddr - source address of message
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSysVersionReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_sysVersionReq_t *pReq = (zstackmsg_sysVersionReq_t *)pMsg;

  if ( pReq->pRsp )
  {
    pReq->hdr.status = zstack_ZStatusValues_ZSuccess;

    pReq->pRsp->transportRev = 1;

    if ( ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE )
    {
      pReq->pRsp->product = zstack_BuildTypes_COORDINATOR;
    }
    else if ( ZG_BUILD_RTR_TYPE )
    {
      pReq->pRsp->product = zstack_BuildTypes_ROUTER;
    }
    else
    {
      pReq->pRsp->product = zstack_BuildTypes_ENDDEVICE;
    }

    pReq->pRsp->majorRel = ZSTACK_SERVER_VERSION_MAJOR;
    pReq->pRsp->minorRel = ZSTACK_SERVER_VERSION_MINOR;
    pReq->pRsp->maintRel = ZSTACK_SERVER_VERSION_MAINTENANCE;
  }
  else
  {
    pReq->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}


/**************************************************************************************************
 * @fn          processDevStartReq
 *
 * @brief       Process the Device Start Request by filling out
 *              the response section of the message
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processDevStartReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_devStartReq_t *pReq = (zstackmsg_devStartReq_t *)pMsg;

  if ( pReq->pReq )
  {
    ZDOInitDevice( pReq->pReq->startDelay );
  }
  else
  {
    pReq->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processSysSetTxPowerReq
 *
 * @brief       Process the System Tx Power Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSysSetTxPowerReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_sysSetTxPowerReq_t *pReq = (zstackmsg_sysSetTxPowerReq_t *)pMsg;

  if ( pReq->pReq )
  {
    int8_t txPower = pReq->pReq->requestedTxPower;

    (void)ZMacSetTransmitPower( (ZMacTransmitPower_t)txPower );

    if ( pReq->pRsp )
    {
      // We don't have a corrected value, so use the requested
      pReq->pRsp->txPower = txPower;
    }
    pReq->hdr.status = zstack_ZStatusValues_ZSuccess;
  }
  else
  {
    pReq->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processDevJoinReq
 *
 * @brief       Process the Device Join Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processDevJoinReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_devJoinReq_t *pReq = (zstackmsg_devJoinReq_t *)pMsg;

#if !defined (ZDO_COORDINATOR)
  if ( pReq->pReq )
  {
    uint8_t extendedPANID[Z_EXTADDR_LEN];

    OsalPort_memcpy( extendedPANID, &(pReq->pReq->extendedPANID), Z_EXTADDR_LEN );

    // Set the ZDOApp state variable to finish the Device start
    ZDApp_ChangeState( DEV_NWK_JOINING );

    ZDApp_NodeProfileSync( pReq->pReq->stackProfile );

    pReq->hdr.status = NLME_JoinRequest( extendedPANID, pReq->pReq->panID,
          pReq->pReq->logicalChannel,
          ZDO_Config_Node_Descriptor.CapabilityFlags,
          pReq->pReq->chosenParent,
          pReq->pReq->parentDepth );
  }
  else
  {
    pReq->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }
#else
    pReq->hdr.status = zstack_ZStatusValues_ZUnsupportedMode;
#endif

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processSysResetReq
 *
 * @brief       Process the Device Force Network Settings Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSysResetReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_sysResetReq_t *pReq = (zstackmsg_sysResetReq_t *)pMsg;

  if ( pReq->pReq )
  {
    if(pReq->pReq->newNwkState)
    {
      zgWriteStartupOptions( ZG_STARTUP_SET,
                         ZCD_STARTOPT_DEFAULT_NETWORK_STATE | ZCD_STARTOPT_DEFAULT_CONFIG_STATE);

    }
    SysCtrlSystemReset();
    pReq->hdr.status = zstack_ZStatusValues_ZSuccess;
  }
  else
  {
    pReq->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processDevForceNetworkSettingsReq
 *
 * @brief       Process the Device Force Network Settings Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processDevForceNetworkSettingsReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_devForceNetworkSettingsReq_t *pReq =
        (zstackmsg_devForceNetworkSettingsReq_t *)pMsg;

  if ( pReq->pReq )
  {
    // Copy the new network parameters to NIB
    _NIB.nwkState = (nwk_states_t)pReq->pReq->state;
    _NIB.nwkDevAddress = pReq->pReq->nwkAddr;
    _NIB.nwkLogicalChannel = pReq->pReq->logicalChannel;
    _NIB.nwkCoordAddress = pReq->pReq->parentNwkAddr;
    _NIB.channelList = pReq->pReq->channelList;
    _NIB.nwkPanId = pReq->pReq->panID;
    _NIB.nodeDepth = pReq->pReq->depth;
    _NIB.MaxRouters = pReq->pReq->maxRouters;
    _NIB.MaxChildren = pReq->pReq->maxChildren;
    _NIB.allocatedRouterAddresses = pReq->pReq->allocatedRouterAddresses;
    _NIB.allocatedEndDeviceAddresses = pReq->pReq->allocatedEndDeviceAddresses;

    if ( _NIB.nwkUpdateId != pReq->pReq->updateID )
    {
      NLME_SetUpdateID( pReq->pReq->updateID );
    }

    osal_cpyExtAddr( _NIB.extendedPANID, &(pReq->pReq->extendedPANID) );

    // Save the NIB
    if ( _NIB.nwkState == NWK_ROUTER )
    {
      // Update NIB in NV
      osal_nv_write( ZCD_NV_NIB, sizeof(nwkIB_t), &_NIB );

      // Reset the NV startup option to resume from NV by clearing
      // the "New" join option.
      zgWriteStartupOptions( ZG_STARTUP_CLEAR,
            ZCD_STARTOPT_DEFAULT_NETWORK_STATE );
    }

    pReq->hdr.status = zstack_ZStatusValues_ZSuccess;
  }
  else
  {
    pReq->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processDevForceNetworkUpdateReq
 *
 * @brief       Process the Device Force Network Update Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processDevForceNetworkUpdateReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_devForceNetworkUpdateReq_t *pReq =
        (zstackmsg_devForceNetworkUpdateReq_t *)pMsg;

  if ( pReq->pReq )
  {
    // Update the network update id
    NLME_SetUpdateID( pReq->pReq->updateID );

    // Switch channel
    if ( _NIB.nwkLogicalChannel != pReq->pReq->logicalChannel )
    {
      uint8_t curChannel;

      _NIB.nwkLogicalChannel = pReq->pReq->logicalChannel;

      // Try to change channel
      ZMacGetReq( ZMacChannel, &curChannel );

      if ( curChannel != pReq->pReq->logicalChannel )
      {
        curChannel = pReq->pReq->logicalChannel;
        // Set the new channel
        ZMacSetReq( ZMacChannel, &curChannel );
      }
    }

    // Update channel list
    _NIB.channelList = (uint32_t)1 << pReq->pReq->logicalChannel;

    // Our Channel has been changed -- notify to save info into NV
    ZDApp_NwkStateUpdateCB( );

    // Reset the total transmit count and the transmit failure counters
    _NIB.nwkTotalTransmissions = 0;
    nwkTransmissionFailures( TRUE );

    pReq->hdr.status = zstack_ZStatusValues_ZSuccess;
  }
  else
  {
    pReq->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processDevForceMacParamsReq
 *
 * @brief       Process the Device Force MAC Parameters Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processDevForceMacParamsReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_devForceMacParamsReq_t *pReq =
        (zstackmsg_devForceMacParamsReq_t *)pMsg;

  if ( pReq->pReq )
  {
    uint8_t curChannel;
    uint16_t nwkAddr = pReq->pReq->nwkAddr;
    uint16_t panId = pReq->pReq->panID;

    // Set our short address
    ZMacSetReq( ZMacShortAddress, (uint8_t *)&nwkAddr );

    // Set our PAN ID
    ZMacSetReq( ZMacPanId, (uint8_t *)&panId );

    // Try to change channel
    ZMacGetReq( ZMacChannel, &curChannel );

    if ( curChannel != pReq->pReq->logicalChannel )
    {
      curChannel = pReq->pReq->logicalChannel;
      // Set the new channel
      ZMacSetReq( ZMacChannel, &curChannel );
    }

    pReq->hdr.status = zstack_ZStatusValues_ZSuccess;
  }
  else
  {
    pReq->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processDevUpdateNeighborTxCostReq
 *
 * @brief       Process the Device Update Neighbor TxCost Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processDevUpdateNeighborTxCostReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_devUpdateNeighborTxCostReq_t *pReq =
        (zstackmsg_devUpdateNeighborTxCostReq_t *)pMsg;

#if defined (RTR_NWK)
  if ( pReq->pReq )
  {
    linkInfo_t *linkInfo;

    // check if entry exists
    linkInfo = nwkNeighborGetLinkInfo( pReq->pReq->nwkAddr, pReq->pReq->panID );

    // if not, look for a vacant entry to add this node...
    if ( linkInfo == NULL )
    {
      nwkNeighborAdd( pReq->pReq->nwkAddr, pReq->pReq->panID, 1 );

      linkInfo = nwkNeighborGetLinkInfo( pReq->pReq->nwkAddr, pReq->pReq->panID );
    }

    if ( linkInfo )
    {
      linkInfo->txCost = pReq->pReq->txCost;
      pReq->hdr.status = zstack_ZStatusValues_ZSuccess;
    }
    else
    {
      pReq->hdr.status = zstack_ZStatusValues_ZNwkUnknownDevice;
    }
  }
  else
  {
    pReq->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }
#else
  pReq->hdr.status = zstack_ZStatusValues_ZUnsupportedMode;
#endif

  return (TRUE);
}



/**************************************************************************************************
 * @fn          processNwkFrameFwdNoticationReq
 *
 * @brief       Process the request from application to enabled/disable notifications on frames
 *              being forwared by routing device
 *
 * @param       srcEntityID - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSetNwkFrameFwdNoticationReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_setNwkFrameFwdNotificationReq_t *pReq = (zstackmsg_setNwkFrameFwdNotificationReq_t *)pMsg;
  if(pReq->pReq == NULL)
  {
    pReq->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }
  else
  {
  #if (ZSTACK_ROUTER_BUILD)
    setNwkFrameFwdNotification(pReq->pReq->Enabled);
    pReq->hdr.status = zstack_ZStatusValues_ZSuccess;
  #else
    pReq->hdr.status = zstack_ZStatusValues_ZNwkInvalidRequest;
  #endif
  }

  return (TRUE);
}



/**************************************************************************************************
 * @fn          processDevNwkDiscoveryReq
 *
 * @brief       Process the Device Network Discovery Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processDevNwkDiscoveryReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_devNwkDiscReq_t *pReq = (zstackmsg_devNwkDiscReq_t *)pMsg;

  // Do checks to see if you can do this?
  if ( pReq->pReq == NULL )
  {
    pReq->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }
  else
  {
    epItem_t *pItem;

    // Find the endpoint connection relationship
    pItem = epTableFindEntryConnection( srcServiceTaskId );
    if ( pItem == NULL )
    {
      // EP entry wasn't found, so create one with empty EP descriptor
      pItem = (epItem_t *)OsalPort_malloc( sizeof(epItem_t) );
      if ( pItem )
      {
        memset( pItem, 0, sizeof(epItem_t) );
        pItem->connection = srcServiceTaskId;
        if ( epTableAddNewEntry( pItem ) == FALSE )
        {
          OsalPort_free( pItem );
          pItem = NULL;
          pReq->hdr.status = zstack_ZStatusValues_ZAfEndpointMax;
        }
      }
    }

    if ( pItem != NULL )
    {
      NLME_ScanFields_t scanFields;

      pItem->zdoCBs |= ZS_ZDO_BEACON_NOTIFY_IND_CBID;
      pItem->zdoCBs |= ZS_ZDO_NWK_DISCOVERY_CNF_CBID;

      ZDO_RegisterForZdoCB( ZDO_NWK_DISCOVERY_CNF_CBID, zdoNwkDiscCnfCB );
      ZDO_RegisterForZdoCB( ZDO_BEACON_NOTIFY_IND_CBID, zdoBeaconNotifyIndCB );

      scanFields.channels = pReq->pReq->scanChannels;
      scanFields.duration = pReq->pReq->scanDuration;
      scanFields.scanType = ZMAC_ACTIVE_SCAN;
      scanFields.scanApp = NLME_DISC_SCAN;

      pReq->hdr.status = NLME_NwkDiscReq2( &scanFields );
    }
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processSysForceLinkStatusReq
 *
 * @brief       Process the System Force Link Status Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSysForceLinkStatusReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_sysForceLinkStatusReq_t *pReq = (zstackmsg_sysForceLinkStatusReq_t *)pMsg;

#if defined (RTR_NWK)
  OsalPort_setEvent( NWK_TaskID, NWK_LINK_STATUS_EVT );

  pReq->hdr.status = zstack_ZStatusValues_ZSuccess;
#else
  pReq->hdr.status = zstack_ZStatusValues_ZUnsupportedMode;
#endif

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processApsRemoveGroup
 *
 * @brief       Process the APS Remove Group Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processApsRemoveGroup( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_apsRemoveGroup_t *pReq = (zstackmsg_apsRemoveGroup_t *)pMsg;

  pReq->hdr.status = zstack_ZStatusValues_ZInvalidParameter;

  if ( pReq->pReq )
  {
    if ( aps_RemoveGroup( pReq->pReq->endpoint, pReq->pReq->groupID ) )
    {
      pReq->hdr.status = zstack_ZStatusValues_ZSuccess;
    }
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processApsRemoveAllGroups
 *
 * @brief       Process the APS Remove All Groups Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processApsRemoveAllGroups( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_apsRemoveAllGroups_t *pReq = (zstackmsg_apsRemoveAllGroups_t *)pMsg;

  if ( pReq->pReq )
  {
    pReq->hdr.status = zstack_ZStatusValues_ZSuccess;
    aps_RemoveAllGroup( pReq->pReq->endpoint );
  }
  else
  {
    pReq->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processApsFindAllGroupsReq
 *
 * @brief       Process the APS Find All Groups Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processApsFindAllGroupsReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_apsFindAllGroupsReq_t *pReq =
        (zstackmsg_apsFindAllGroupsReq_t *)pMsg;

  if ( pReq->pReq && pReq->pRsp )
  {
    pReq->hdr.status = zstack_ZStatusValues_ZSuccess;

    pReq->pRsp->numGroups = aps_CountGroups( pReq->pReq->endpoint );

    pReq->pRsp->pGroupList = (uint16_t *)OsalPort_malloc(
          (sizeof(uint16_t) * pReq->pRsp->numGroups) );

    (void)aps_FindAllGroupsForEndpoint( pReq->pReq->endpoint,
          pReq->pRsp->pGroupList );
  }
  else
  {
    pReq->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processApsFindAllGroupsReq
 *
 * @brief       Process the APS Find a Group Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processApsFindGroupReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_apsFindGroupReq_t *pReq = (zstackmsg_apsFindGroupReq_t *)pMsg;

  if ( pReq->pReq && pReq->pRsp )
  {
    aps_Group_t *pGroup;

    memset( pReq->pRsp, 0, sizeof(zstack_apsFindGroupRsp_t) );

    pGroup = aps_FindGroup( pReq->pReq->endpoint, pReq->pReq->groupID );
    if ( pGroup )
    {
      pReq->hdr.status = zstack_ZStatusValues_ZSuccess;

      pReq->pRsp->groupID = pGroup->ID;
      pReq->pRsp->n_name = pGroup->name[0];
      if ( pReq->pRsp->n_name )
      {
        pReq->pRsp->pName = (uint8_t *)OsalPort_malloc( pReq->pRsp->n_name );
        if ( pReq->pRsp->pName )
        {
          OsalPort_memcpy( pReq->pRsp->pName, &(pGroup->name[1]),
                pReq->pRsp->n_name );
        }
      }
    }
    else
    {
      pReq->hdr.status = zstack_ZStatusValues_ZApsFail;
    }

  }
  else
  {
    pReq->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processApsAddGroup
 *
 * @brief       Process the APS Add a Group Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processApsAddGroup( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_apsAddGroup_t *pReq = (zstackmsg_apsAddGroup_t *)pMsg;

  if ( pReq->pReq )
  {
    aps_Group_t group = {0};

    // Build local group structure
    group.ID = pReq->pReq->groupID;
    if ( pReq->pReq->n_name && pReq->pReq->pName )
    {
      uint8_t len = pReq->pReq->n_name;
      if ( len > (APS_GROUP_NAME_LEN - 1) )
      {
        len = (APS_GROUP_NAME_LEN - 1);
      }
      group.name[0] = len;
      OsalPort_memcpy( &group.name[1], pReq->pReq->pName, len );
    }

    pReq->hdr.status = (zstack_ZStatusValues)aps_AddGroup(
          pReq->pReq->endpoint, &group, true );
  }
  else
  {
    pReq->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processApsCountAllGroups
 *
 * @brief       Process the APS Count all Groups Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processApsCountAllGroups( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_apsCountAllGroups_t *pReq = (zstackmsg_apsCountAllGroups_t *)pMsg;

  // The number of groups is returned in the status field
  pReq->hdr.status = (zstack_ZStatusValues)aps_CountAllGroups( );

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processSecApsRemoveReq
 *
 * @brief       Process the Security APS Remove Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSecApsRemoveReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_secApsRemoveReq_t *pPtr = (zstackmsg_secApsRemoveReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    if ( ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE )
    {
      pPtr->hdr.status = (zstack_ZStatusValues)ZDSecMgrAPSRemove(
            pPtr->pReq->nwkAddr, pPtr->pReq->extAddr, pPtr->pReq->parentAddr );
    }
    else
    {
      pPtr->hdr.status = zstack_ZStatusValues_ZUnsupportedMode;
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processSecNwkKeyUpdateReq
 *
 * @brief       Process the Security Network Key Update Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSecNwkKeyUpdateReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_secNwkKeyUpdateReq_t *pPtr = (zstackmsg_secNwkKeyUpdateReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    if ( ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE )
    {
      nwkKeyDesc nwkKey = {0};
      uint16_t nvID = 0;
      uint8_t done = FALSE;

      pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
      // Key provided in the request, don't look for it.
      if(pPtr->pReq->key)
      {
          pPtr->hdr.status = (zstack_ZStatusValues)ZDSecMgrUpdateNwkKey(
                  pPtr->pReq->key, pPtr->pReq->seqNum, pPtr->pReq->dstAddr );
      }
      else
      {
        while ( !done )
        {
          if ( nvID == 0 )
          {
            nvID = ZCD_NV_NWK_ALTERN_KEY_INFO;
          }
          else
          {
            nvID = ZCD_NV_NWK_ACTIVE_KEY_INFO;
            done = TRUE;
          }

          osal_nv_read( nvID, 0, sizeof(nwkKeyDesc), &nwkKey );

          if ( nwkKey.keySeqNum == pPtr->pReq->seqNum )
          {
            done = TRUE;
            pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
          }
        }
        if ( pPtr->hdr.status == zstack_ZStatusValues_ZSuccess )
        {
          pPtr->hdr.status = (zstack_ZStatusValues)ZDSecMgrUpdateNwkKey(
                  nwkKey.key, pPtr->pReq->seqNum, pPtr->pReq->dstAddr );
        }
      }
    }
    else
    {
      pPtr->hdr.status = zstack_ZStatusValues_ZUnsupportedMode;
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processSecMgrSetupPartner
 *
 * @brief       Process the Security APS Remove Request
 *
 * @param       srcEntityID - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSecMgrSetupPartner( uint8_t srcServiceTaskId, void *pMsg )
{
    zstackmsg_secMgrSetupPartnerReq_t *pPtr = (zstackmsg_secMgrSetupPartnerReq_t *)pMsg;

  if ( pPtr->pReq )
  {
#if ( ZG_BUILD_JOINING_TYPE )
    if ( ZG_BUILD_RTR_TYPE && ZG_DEVICE_RTR_TYPE )
    {
      pPtr->hdr.status = (zstack_ZStatusValues)ZDSecMgrSetupPartner(pPtr->pReq->partNwkAddr, pPtr->pReq->partExtAddr);
    }
    else
#endif
    {
      pPtr->hdr.status = zstack_ZStatusValues_ZUnsupportedMode;
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processSecMgrAppKeyReq
 *
 * @brief       Process the Security APS Key Request
 *
 * @param       srcEntityID - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSecMgrAppKeyReq( uint8_t srcServiceTaskId, void *pMsg )
{
    zstackmsg_secMgrAppKeyReq_t *pPtr = (zstackmsg_secMgrAppKeyReq_t *)pMsg;

  if ( pPtr->pReq )
  {
#if ( ZG_BUILD_JOINING_TYPE )
    if ( ZG_BUILD_RTR_TYPE && ZG_DEVICE_RTR_TYPE )
    {
      pPtr->hdr.status = (zstack_ZStatusValues)ZDSecMgrRequestAppKey(pPtr->pReq->partExtAddr);
    }
    else
#endif
    {
      pPtr->hdr.status = zstack_ZStatusValues_ZUnsupportedMode;
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}


/**************************************************************************************************
 * @fn          processSecMgrAppKeyTypeSet
 *
 * @brief       Process the Security APS Key Type Set Request
 *
 * @param       srcEntityID - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSecMgrAppKeyTypeSet( uint8_t srcServiceTaskId, void *pMsg )
{
    zstackmsg_secMgrAppKeyTypeSetReq_t *pPtr = (zstackmsg_secMgrAppKeyTypeSetReq_t *)pMsg;

  if ( pPtr->pReq )
  {
#if ( ZG_BUILD_COORDINATOR_TYPE )
    if ( ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE )
    {
      pPtr->hdr.status = (zstack_ZStatusValues)ZDSecMgrAppKeyTypeSet(pPtr->pReq->keytype);
    }
    else
#endif
    {
      pPtr->hdr.status = zstack_ZStatusValues_ZUnsupportedMode;
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processDevNwkManagerSet
 *
 * @brief       set nwk manager
 *
 * @param       srcEntityID - Source Task ID
 * @param       pMsg - pointer to message empty request
 *
 * @return      TRUE to send the response back
 */
static bool processDevNwkManagerSet( uint8_t srcServiceTaskId, void *pMsg )
{
    zstackmsg_devNwkManagerSetReq_t *pPtr = (zstackmsg_devNwkManagerSetReq_t *)pMsg;

#if ( ZG_BUILD_COORDINATOR_TYPE )
    if ( ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE )
    {
#ifdef NWK_MANAGER
      NwkMgr_SetNwkManager();
#endif
      pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    }
    else
#endif
    {
      pPtr->hdr.status = zstack_ZStatusValues_ZUnsupportedMode;
    }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processSecNwkKeySwitchReq
 *
 * @brief       Process the Security Network Key Switch Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSecNwkKeySwitchReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_secNwkKeySwitchReq_t *pPtr = (zstackmsg_secNwkKeySwitchReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    if ( ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE )
    {
      pPtr->hdr.status = (zstack_ZStatusValues)ZDSecMgrSwitchNwkKey(
            pPtr->pReq->seqNum, pPtr->pReq->dstAddr );
    }
    else
    {
      pPtr->hdr.status = zstack_ZStatusValues_ZUnsupportedMode;
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processSecNwkKeySetReq
 *
 * @brief       Process the Security Network Key Set Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSecNwkKeySetReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_secNwkKeySetReq_t *pPtr = (zstackmsg_secNwkKeySetReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    nwkKeyDesc nwkKey;
    uint16_t nvID;

    if ( pPtr->pReq->activeKey )
    {
      nvID = ZCD_NV_NWK_ACTIVE_KEY_INFO;
    }
    else
    {
      nvID = ZCD_NV_NWK_ALTERN_KEY_INFO;
    }

    nwkKey.keySeqNum = pPtr->pReq->seqNum;
    if ( pPtr->pReq->has_key )
    {
      OsalPort_memcpy( nwkKey.key, pPtr->pReq->key, 16 );
    }
    else
    {
      ZDSecMgrGenerateRndKey(&nwkKey.key[0]);
    }

    pPtr->hdr.status = (zstack_ZStatusValues)osal_nv_write( nvID,
          sizeof(nwkKeyDesc), &nwkKey );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processSecNwkKeyGetReq
 *
 * @brief       Process the Security Network Key Get Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSecNwkKeyGetReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_secNwkKeyGetReq_t *pPtr = (zstackmsg_secNwkKeyGetReq_t *)pMsg;

  if ( pPtr->pReq && pPtr->pRsp )
  {
    nwkKeyDesc nwkKey;
    uint16_t nvID;

    if ( pPtr->pReq->activeKey )
    {
      nvID = ZCD_NV_NWK_ACTIVE_KEY_INFO;
    }
    else
    {
      nvID = ZCD_NV_NWK_ALTERN_KEY_INFO;
    }

    pPtr->hdr.status = (zstack_ZStatusValues)osal_nv_read( nvID, 0,
          sizeof(nwkKeyDesc), &nwkKey );

    if ( pPtr->hdr.status == zstack_ZStatusValues_ZSuccess )
    {
      pPtr->pRsp->activeKey = pPtr->pReq->activeKey;
      pPtr->pRsp->seqNum = nwkKey.keySeqNum;
      OsalPort_memcpy( pPtr->pRsp->key, nwkKey.key, 16 );
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processSecApsLinkKeyGetReq
 *
 * @brief       Process the Security APS Link Key Get Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSecApsLinkKeyGetReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_secApsLinkKeyGetReq_t *pPtr =
        (zstackmsg_secApsLinkKeyGetReq_t *)pMsg;

  if ( pPtr->pReq && pPtr->pRsp )
  {
    pPtr->pRsp->tcLinkKey = pPtr->pReq->tcLinkKey;
    OsalPort_memcpy( pPtr->pRsp->ieeeAddr, pPtr->pReq->ieeeAddr, Z_EXTADDR_LEN );

    if ( pPtr->pReq->tcLinkKey )
    {
      int x;
      APSME_TCLinkKeyNVEntry_t tcLinkKey;

      for ( x = 0; x < gZDSECMGR_TC_DEVICE_MAX; x++ )
      {
        pPtr->hdr.status = osal_nv_read_ex( ZCD_NV_EX_TCLK_TABLE, x, 0,
                                            sizeof(APSME_TCLinkKeyNVEntry_t),
                                            &tcLinkKey );

        if ( pPtr->hdr.status == zstack_ZStatusValues_ZSuccess )
        {
          if ( OsalPort_memcmp( pPtr->pReq->ieeeAddr, tcLinkKey.extAddr, Z_EXTADDR_LEN ) == 0 )
          {
            pPtr->pRsp->rxFrmCntr = tcLinkKey.rxFrmCntr;
            pPtr->pRsp->txFrmCntr = tcLinkKey.txFrmCntr;

            break;
          }
        }
      }

      if ( x == gZDSECMGR_TC_DEVICE_MAX )
      {
        pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
      }
    }
    else
    {
      ZDSecMgrEntry_t *pEntry;

      pPtr->hdr.status = (zstack_ZStatusValues)ZDSecMgrEntryLookupExt(
            pPtr->pReq->ieeeAddr, &pEntry );
      if ( pPtr->hdr.status == zstack_ZStatusValues_ZSuccess )
      {
        APSME_ApsLinkKeyNVEntry_t linkKey;

        pPtr->hdr.status = (zstack_ZStatusValues)osal_nv_read( pEntry->keyNvId,
              0, sizeof(APSME_ApsLinkKeyNVEntry_t),
              &linkKey );
        if ( pPtr->hdr.status == zstack_ZStatusValues_ZSuccess )
        {
          pPtr->pRsp->rxFrmCntr = linkKey.rxFrmCntr;
          pPtr->pRsp->txFrmCntr = linkKey.txFrmCntr;
          OsalPort_memcpy( pPtr->pRsp->key, linkKey.key, 16 );
        }
      }
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processSecApsLinkKeySetReq
 *
 * @brief       Process the Security APS Link Key Set Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSecApsLinkKeySetReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_secApsLinkKeySetReq_t *pPtr =
        (zstackmsg_secApsLinkKeySetReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    if ( pPtr->pReq->tcLinkKey )
    {
      int y = 0;
      int x;
      APSME_TCLinkKeyNVEntry_t tcLinkKey;
      uint8_t dummyKey[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

      for ( x = 0; x < gZDSECMGR_TC_DEVICE_MAX; x++ )
      {
        pPtr->hdr.status = osal_nv_read_ex( ZCD_NV_EX_TCLK_TABLE, x, 0,
                                            sizeof(APSME_TCLinkKeyNVEntry_t),
                                            &tcLinkKey );

        if ( pPtr->hdr.status == zstack_ZStatusValues_ZSuccess )
        {
          if ( OsalPort_memcmp( pPtr->pReq->ieeeAddr, tcLinkKey.extAddr, Z_EXTADDR_LEN ) == 0 )
          {
            y = x;
            break;
          }
          else if ( (y == 0)
                && (OsalPort_memcmp( pPtr->pReq->ieeeAddr, dummyKey, Z_EXTADDR_LEN ) == 0) )
          {
            // First empty spot
            y = x;
          }
        }
      }

      if ( y > 0 )
      {
        OsalPort_memcpy( tcLinkKey.extAddr, pPtr->pReq->ieeeAddr, Z_EXTADDR_LEN );
        tcLinkKey.txFrmCntr = pPtr->pReq->txFrmCntr;
        tcLinkKey.rxFrmCntr = pPtr->pReq->rxFrmCntr;

        pPtr->hdr.status = osal_nv_write_ex( ZCD_NV_EX_TCLK_TABLE, x,
                                             sizeof(APSME_TCLinkKeyNVEntry_t),
                                             &tcLinkKey );
      }
    }
    else
    {
      ZDSecMgrEntry_t *pEntry;
      APSME_ApsLinkKeyNVEntry_t linkKey;

      OsalPort_memcpy( linkKey.key, pPtr->pReq->key, SEC_KEY_LEN );
      linkKey.txFrmCntr = pPtr->pReq->txFrmCntr;
      linkKey.rxFrmCntr = pPtr->pReq->rxFrmCntr;

      pPtr->hdr.status = (zstack_ZStatusValues)ZDSecMgrEntryLookupExt(
            pPtr->pReq->ieeeAddr, &pEntry );
      if ( pPtr->hdr.status == zstack_ZStatusValues_ZSuccess )
      {
        pPtr->hdr.status = (zstack_ZStatusValues)osal_nv_write(
              pEntry->keyNvId, sizeof(APSME_ApsLinkKeyNVEntry_t), &linkKey );
      }
      else
      {
        pPtr->hdr.status = (zstack_ZStatusValues)ZDSecMgrAddLinkKey(
              pPtr->pReq->shortAddr, pPtr->pReq->ieeeAddr, linkKey.key );
      }
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processSecApsLinkKeyRemoveReq
 *
 * @brief       Process the Security APS Link Key Remove Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSecApsLinkKeyRemoveReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_secApsLinkKeyRemoveReq_t *pPtr =
        (zstackmsg_secApsLinkKeyRemoveReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    if ( pPtr->pReq->tcLinkKey )
    {
      int x;
      APSME_TCLinkKeyNVEntry_t tcLinkKey;

      for ( x = 0; x < gZDSECMGR_TC_DEVICE_MAX; x++ )
      {
        pPtr->hdr.status = osal_nv_read_ex( ZCD_NV_EX_TCLK_TABLE, x, 0,
                                            sizeof(APSME_TCLinkKeyNVEntry_t),
                                            &tcLinkKey );

        if ( pPtr->hdr.status == zstack_ZStatusValues_ZSuccess )
        {
          if ( OsalPort_memcmp( pPtr->pReq->ieeeAddr, tcLinkKey.extAddr, Z_EXTADDR_LEN ) == 0 )
          {
            memset( &tcLinkKey, 0, sizeof(APSME_TCLinkKeyNVEntry_t) );

            pPtr->hdr.status = osal_nv_write_ex( ZCD_NV_EX_TCLK_TABLE, x,
                                                 sizeof(APSME_TCLinkKeyNVEntry_t),
                                                 &tcLinkKey );
            break;
          }
        }
      }
    }
    else
    {
      pPtr->hdr.status = (zstack_ZStatusValues)ZDSecMgrDeviceRemoveByExtAddr(
            pPtr->pReq->ieeeAddr );
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

#if defined (RTR_NWK)
/**************************************************************************************************
 * @fn          processDevNwkRouteReq
 *
 * @brief       Process the Device Network Route Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processDevNwkRouteReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_devNwkRouteReq_t *pPtr = (zstackmsg_devNwkRouteReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    uint8_t options = 0;

    if ( pPtr->pReq->mtoRoute )
    {
      options |= MTO_ROUTE;

      if ( pPtr->pReq->mtoNoCache )
      {
        options |= NO_ROUTE_CACHE;
      }
    }

    if ( pPtr->pReq->multicast )
    {
      options |= MULTICAST_ROUTE;
    }

    pPtr->hdr.status = (zstack_ZStatusValues)NLME_RouteDiscoveryRequest(
          pPtr->pReq->dstAddr, options, pPtr->pReq->radius );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // RTR_NWK

#if defined (RTR_NWK)
/**************************************************************************************************
 * @fn          processDevNwkCheckRouteReq
 *
 * @brief       Process the Device Network Check Route Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processDevNwkCheckRouteReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_devNwkCheckRouteReq_t *pPtr = (zstackmsg_devNwkCheckRouteReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    uint8_t options = 0;

    if ( pPtr->pReq->mtoRoute )
    {
      options |= MTO_ROUTE;
    }

    pPtr->hdr.status = (zstack_ZStatusValues)RTG_CheckRtStatus(
          pPtr->pReq->dstAddr, RT_ACTIVE, options );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // RTR_NWK

/**************************************************************************************************
 * @fn          processSysConfigReadReq
 *
 * @brief       Process the System Config Read Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSysConfigReadReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_sysConfigReadReq_t *pPtr = (zstackmsg_sysConfigReadReq_t *)pMsg;

  if ( pPtr->pReq && pPtr->pRsp )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;

    if ( pPtr->pReq->preConfigKeyEnable )
    {
      pPtr->pRsp->has_preConfigKeyEnable = TRUE;
      pPtr->pRsp->preConfigKeyEnable = zgPreConfigKeys;
    }

    if ( pPtr->pReq->useDefaultTCLK )
    {
      pPtr->pRsp->has_useDefaultTCLK = TRUE;
      pPtr->pRsp->useDefaultTCLK = zgUseDefaultTCLK;
    }

    if ( pPtr->pReq->disabledPollRate )
    {
      uint16_t pollRateType;
      pollRateType = nwk_GetCurrentPollRateType(NULL);
      pPtr->pRsp->has_disabledPollRateState = TRUE;

      if(pollRateType & POLL_RATE_DISABLED)
      {
        pPtr->pRsp->disabledPollRate = TRUE;
      }
      else
      {
        pPtr->pRsp->disabledPollRate = FALSE;
      }
    }

    if ( pPtr->pReq->pollRate )
    {
      pPtr->pRsp->has_pollRate = TRUE;

      pPtr->pRsp->CurrentPollRateTypesEnabled = nwk_GetCurrentPollRateType((uint32_t*)&(pPtr->pRsp->CurrentPollRate));

      if( ZG_DEVICE_ENDDEVICE_TYPE && zgRxAlwaysOn == FALSE )
      {
          pPtr->pRsp->PollRateDefault    = nwk_GetConfigPollRate(POLL_RATE_TYPE_DEFAULT);
          pPtr->pRsp->PollRateApp1       = nwk_GetConfigPollRate(POLL_RATE_TYPE_APP_1);
          pPtr->pRsp->PollRateApp2       = nwk_GetConfigPollRate(POLL_RATE_TYPE_APP_2);
          pPtr->pRsp->PollRateJoinRejoin = nwk_GetConfigPollRate(POLL_RATE_TYPE_JOIN_REJOIN);
          pPtr->pRsp->PollRateQueue      = nwk_GetConfigPollRate(POLL_RATE_TYPE_QUEUED);
          pPtr->pRsp->PollRateResponse   = nwk_GetConfigPollRate(POLL_RATE_TYPE_RESPONSE);
      }
    }
    if ( pPtr->pReq->apsAckWaitDuration )
    {
      pPtr->pRsp->has_apsAckWaitDuration = TRUE;
      pPtr->pRsp->apsAckWaitDuration = zgApscAckWaitDurationPolled;
    }

    if ( pPtr->pReq->bindingTime )
    {
      pPtr->pRsp->has_bindingTime = TRUE;
      pPtr->pRsp->bindingTime = zgApsDefaultMaxBindingTime;
    }

    if ( pPtr->pReq->panID )
    {
      pPtr->pRsp->has_panID = TRUE;
      pPtr->pRsp->panID = zgConfigPANID;
    }

    if ( pPtr->pReq->maxMissingMacAckLinkFailure )
    {
      pPtr->pRsp->has_maxMissingMacAckLinkFailure = TRUE;
      pPtr->pRsp->maxMissingMacAckLinkFailure = zgMaxMissingMacAckLinkFailure;
    }

    if ( pPtr->pReq->indirectMsgTimeout )
    {
      pPtr->pRsp->has_indirectMsgTimeout = TRUE;
      pPtr->pRsp->indirectMsgTimeout = zgIndirectMsgTimeout;
    }

    if ( pPtr->pReq->apsFrameRetries )
    {
      pPtr->pRsp->has_apsFrameRetries = TRUE;
      pPtr->pRsp->apsFrameRetries = zgApscMaxFrameRetries;
    }

    if ( pPtr->pReq->bcastRetries )
    {
      pPtr->pRsp->has_bcastRetries = TRUE;
      pPtr->pRsp->bcastRetries = zgMaxBcastRetires;
    }

    if ( pPtr->pReq->passiveAckTimeout )
    {
      pPtr->pRsp->has_passiveAckTimeout = TRUE;
      pPtr->pRsp->passiveAckTimeout = zgPassiveAckTimeout;
    }

    if ( pPtr->pReq->bcastDeliveryTime )
    {
      pPtr->pRsp->has_bcastDeliveryTime = TRUE;
      pPtr->pRsp->bcastDeliveryTime = zgBcastDeliveryTime;
    }

    if ( pPtr->pReq->routeExpiryTime )
    {
      pPtr->pRsp->has_routeExpiryTime = TRUE;
      pPtr->pRsp->routeExpiryTime = zgRouteExpiryTime;
    }

    if ( pPtr->pReq->userDesc )
    {
      UserDescriptorFormat_t userDesc;

      pPtr->pRsp->has_userDesc = TRUE;

      osal_nv_read( ZCD_NV_USERDESC, 0, sizeof(UserDescriptorFormat_t),
            &userDesc );

      pPtr->pRsp->n_userDesc = userDesc.len;
      if ( pPtr->pRsp->n_userDesc )
      {
        pPtr->pRsp->pUserDesc = OsalPort_malloc( userDesc.len );
        if ( pPtr->pRsp->pUserDesc )
        {
          OsalPort_memcpy( pPtr->pRsp->pUserDesc, userDesc.desc, userDesc.len );
        }
        else
        {
          pPtr->pRsp->n_userDesc = 0;
          pPtr->pRsp->pUserDesc = NULL;
        }
      }
      else
      {
        pPtr->pRsp->pUserDesc = NULL;
      }
    }

    if ( pPtr->pReq->preConfigKey )
    {
      pPtr->pRsp->pPreConfigKey = OsalPort_malloc( 16 );
      if ( pPtr->pRsp->pPreConfigKey )
      {
        pPtr->pRsp->has_preConfigKey = TRUE;

        osal_nv_read( ZCD_NV_PRECFGKEY, 0, 16, pPtr->pRsp->pPreConfigKey );
      }
    }

    if ( pPtr->pReq->chanList )
    {
      pPtr->pRsp->has_chanList = TRUE;
      pPtr->pRsp->chanList = zgDefaultChannelList;
    }

    if ( pPtr->pReq->multicastRadius )
    {
      pPtr->pRsp->has_multicastRadius = TRUE;
      pPtr->pRsp->multicastRadius = zgApsNonMemberRadius;
    }

    if ( pPtr->pReq->extendedPANID )
    {
      pPtr->pRsp->has_extendedPANID = TRUE;
      OsalPort_memcpy( &pPtr->pRsp->extendedPANID, _NIB.extendedPANID, Z_EXTADDR_LEN );
    }

    if ( pPtr->pReq->ieeeAddr )
    {
      pPtr->pRsp->has_ieeeAddr = TRUE;
      OsalPort_memcpy( &pPtr->pRsp->ieeeAddr, NLME_GetExtAddr( ), Z_EXTADDR_LEN );
    }

    if ( pPtr->pReq->macRxOnIdle )
    {
      uint8_t x;
      ZMacGetReq( ZMacRxOnIdle, &x );

      pPtr->pRsp->has_macRxOnIdle = TRUE;
      if ( x )
      {
        pPtr->pRsp->macRxOnIdle = TRUE;
      }
      else
      {
        pPtr->pRsp->macRxOnIdle = FALSE;
      }
    }

    if ( pPtr->pReq->snifferFeature )
    {
      pPtr->pRsp->has_snifferFeature = TRUE;
      // TBD
      pPtr->pRsp->snifferFeature = FALSE;
    }

    if ( pPtr->pReq->concentratorEnable )
    {
      pPtr->pRsp->has_concentratorEnable = TRUE;
      pPtr->pRsp->concentratorEnable = zgConcentratorEnable;
    }

    if ( pPtr->pReq->concentratorDiscTime )
    {
      pPtr->pRsp->has_concentratorDiscTime = TRUE;
      pPtr->pRsp->concentratorDiscTime = zgConcentratorDiscoveryTime;
    }

    if ( pPtr->pReq->nwkUseMultiCast )
    {
      pPtr->pRsp->has_nwkUseMultiCast = TRUE;
      pPtr->pRsp->nwkUseMultiCast = nwkUseMultiCast;
    }

    if ( pPtr->pReq->devPartOfNetwork )
    {
      pPtr->pRsp->has_devPartOfNetwork = TRUE;
      pPtr->pRsp->devPartOfNetwork = isDevicePartOfNetwork( );
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processSysConfigWriteReq
 *
 * @brief       Process the System Config Write Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSysConfigWriteReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_sysConfigWriteReq_t *pPtr = (zstackmsg_sysConfigWriteReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;

    if ( pPtr->pReq->has_preConfigKeyEnable )
    {
      if ( pPtr->pReq->preConfigKeyEnable )
      {
        zgPreConfigKeys = TRUE;
      }
      else
      {
        zgPreConfigKeys = FALSE;
      }
      osal_nv_write( ZCD_NV_PRECFGKEYS_ENABLE, sizeof(zgPreConfigKeys),
            &zgPreConfigKeys );
    }
    if ( pPtr->pReq->has_useDefaultTCLK )
    {
      if ( pPtr->pReq->useDefaultTCLK )
      {
        zgUseDefaultTCLK = TRUE;
      }
      else
      {
        zgUseDefaultTCLK = FALSE;
      }
      osal_nv_write( ZCD_NV_USE_DEFAULT_TCLK, sizeof(zgUseDefaultTCLK),
            &zgUseDefaultTCLK );
    }
    if ( pPtr->pReq->has_disablePollRate )
    {
      nwk_SetCurrentPollRateType(POLL_RATE_DISABLED,pPtr->pReq->disablePollRate);
    }
    //Only process if has_disablePollRate is not requested.
    else if ( pPtr->pReq->has_pollRate )
    {
      //Process all poll rate request from App if those are not application pollrates (POLL_RATE_TYPE_DEFAULT | POLL_RATE_TYPE_APP_1 | POLL_RATE_TYPE_APP_2)
      //If those are application poll rates, then make sure are bigger than MINIMUM_APP_POLL_RATE.
      if(! ((uint16_t)pPtr->pReq->pollRateType & (POLL_RATE_TYPE_DEFAULT | POLL_RATE_TYPE_APP_1 | POLL_RATE_TYPE_APP_2))  ||
            ((uint32_t)pPtr->pReq->pollRate >= MINIMUM_APP_POLL_RATE) )
        {
          //Configure the pollrates enabled
          nwk_SetConfigPollRate((uint16_t)pPtr->pReq->pollRateType,(uint32_t)pPtr->pReq->pollRate );

          //Force the new configuration to take place if it applies.
          nwk_SetCurrentPollRateType(0,TRUE);

          //Update the Nv Items
          osal_nv_write(ZCD_NV_POLL_RATE, sizeof(nwk_pollRateTable), &nwk_pollRateTable);
        }
    }

    if ( pPtr->pReq->has_apsAckWaitDuration )
    {
      zgApscAckWaitDurationPolled = (uint16_t)pPtr->pReq->apsAckWaitDuration;
      osal_nv_write( ZCD_NV_APS_ACK_WAIT_DURATION,
            sizeof(zgApscAckWaitDurationPolled), &zgApscAckWaitDurationPolled );
    }

    if ( pPtr->pReq->has_bindingTime )
    {
      zgApsDefaultMaxBindingTime = (uint16_t)pPtr->pReq->bindingTime;
      osal_nv_write( ZCD_NV_BINDING_TIME, sizeof(zgApsDefaultMaxBindingTime),
            &zgApsDefaultMaxBindingTime );
    }

    if ( pPtr->pReq->has_panID )
    {
      zgConfigPANID = (uint16_t)pPtr->pReq->panID;
      osal_nv_write( ZCD_NV_PANID, sizeof(zgConfigPANID), &zgConfigPANID );
    }

    if ( pPtr->pReq->has_maxMissingMacAckLinkFailure )
    {
        zgMaxMissingMacAckLinkFailure = (uint8_t)pPtr->pReq->maxMissingMacAckLinkFailure;
      osal_nv_write( ZCD_NV_POLL_FAILURE_RETRIES,
            sizeof(zgMaxMissingMacAckLinkFailure), &zgMaxMissingMacAckLinkFailure );
    }

    if ( pPtr->pReq->has_indirectMsgTimeout )
    {
      zgIndirectMsgTimeout = (uint8_t)pPtr->pReq->indirectMsgTimeout;
      osal_nv_write( ZCD_NV_INDIRECT_MSG_TIMEOUT,
            sizeof(zgIndirectMsgTimeout), &zgIndirectMsgTimeout );
    }

    if ( pPtr->pReq->has_apsFrameRetries )
    {
      zgApscMaxFrameRetries = (uint8_t)pPtr->pReq->apsFrameRetries;
      osal_nv_write( ZCD_NV_APS_FRAME_RETRIES, sizeof(zgApscMaxFrameRetries),
            &zgApscMaxFrameRetries );
    }

    if ( pPtr->pReq->has_bcastRetries )
    {
      zgMaxBcastRetires = (uint8_t)pPtr->pReq->bcastRetries;
      osal_nv_write( ZCD_NV_BCAST_RETRIES, sizeof(zgMaxBcastRetires),
            &zgMaxBcastRetires );
    }

    if ( pPtr->pReq->has_passiveAckTimeout )
    {
      zgPassiveAckTimeout = (uint8_t)pPtr->pReq->passiveAckTimeout;
      osal_nv_write( ZCD_NV_PASSIVE_ACK_TIMEOUT, sizeof(zgPassiveAckTimeout),
            &zgPassiveAckTimeout );
    }

    if ( pPtr->pReq->has_bcastDeliveryTime )
    {
      zgBcastDeliveryTime = (uint8_t)pPtr->pReq->bcastDeliveryTime;
      osal_nv_write( ZCD_NV_BCAST_DELIVERY_TIME, sizeof(zgBcastDeliveryTime),
            &zgBcastDeliveryTime );
    }

    if ( pPtr->pReq->has_routeExpiryTime )
    {
      zgRouteExpiryTime = (uint8_t)pPtr->pReq->routeExpiryTime;
      osal_nv_write( ZCD_NV_ROUTE_EXPIRY_TIME, sizeof(zgRouteExpiryTime),
            &zgRouteExpiryTime );
    }

    if ( pPtr->pReq->has_userDesc )
    {
      UserDescriptorFormat_t userDesc;

      userDesc.len = pPtr->pReq->n_userDesc;
      if ( userDesc.len > AF_MAX_USER_DESCRIPTOR_LEN )
      {
        userDesc.len = AF_MAX_USER_DESCRIPTOR_LEN;
      }

      OsalPort_memcpy( userDesc.desc, pPtr->pReq->pUserDesc, userDesc.len );
      osal_nv_write( ZCD_NV_USERDESC, sizeof(UserDescriptorFormat_t),
            &userDesc );
    }

    if ( pPtr->pReq->has_preConfigKey )
    {
      int len;

      len = pPtr->pReq->n_preConfigKey;
      if ( len > 16 )
      {
        len = 16;
      }

      osal_nv_write( ZCD_NV_PRECFGKEY, len, pPtr->pReq->pPreConfigKey );
    }

    if ( pPtr->pReq->has_chanList )
    {
      zgDefaultChannelList = (uint32_t)pPtr->pReq->chanList;
      osal_nv_write( ZCD_NV_CHANLIST, sizeof(zgDefaultChannelList),
            &zgDefaultChannelList );
    }

    if ( pPtr->pReq->has_multicastRadius )
    {
      zgApsNonMemberRadius = (uint8_t)pPtr->pReq->multicastRadius;
      osal_nv_write( ZCD_NV_APS_NONMEMBER_RADIUS,
            sizeof(zgApsNonMemberRadius), &zgApsNonMemberRadius );
    }

    if ( pPtr->pReq->has_extendedPANID )
    {
      OsalPort_memcpy( zgApsUseExtendedPANID, &(pPtr->pReq->extendedPANID), Z_EXTADDR_LEN );
      osal_nv_write( ZCD_NV_APS_USE_EXT_PANID, Z_EXTADDR_LEN, zgApsUseExtendedPANID );
    }

    if ( pPtr->pReq->has_ieeeAddr )
    {
      osal_nv_write( ZCD_NV_EXTADDR, Z_EXTADDR_LEN, pPtr->pReq->ieeeAddr );
    }

    if ( pPtr->pReq->has_macRxOnIdle )
    {
      uint8_t x = pPtr->pReq->macRxOnIdle;
      ZMacSetReq( ZMacRxOnIdle, &x );
    }

    if ( pPtr->pReq->has_snifferFeature )
    {
      // TBD - Added enable sniffer
    }

    if ( pPtr->pReq->has_concentratorDiscTime )
    {
      zgConcentratorDiscoveryTime = (uint8_t)pPtr->pReq->concentratorDiscTime;
      osal_nv_write( ZCD_NV_CONCENTRATOR_DISCOVERY,
            sizeof(zgConcentratorDiscoveryTime), &zgConcentratorDiscoveryTime );
    }

    if ( pPtr->pReq->has_concentratorEnable )
    {
      zgConcentratorEnable = (uint8_t)pPtr->pReq->concentratorEnable;
      osal_nv_write( ZCD_NV_CONCENTRATOR_ENABLE,
            sizeof(zgConcentratorEnable), &zgConcentratorEnable );

      ZDApp_ForceConcentratorChange( );
    }

    if ( pPtr->pReq->has_nwkUseMultiCast )
    {
      nwkUseMultiCast = pPtr->pReq->nwkUseMultiCast;
      _NIB.nwkUseMultiCast = nwkUseMultiCast;
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processSysNwkInfoReadReq
 *
 * @brief       Process the System Network Info Read Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processSysNwkInfoReadReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_sysNwkInfoReadReq_t *pPtr = (zstackmsg_sysNwkInfoReadReq_t *)pMsg;

  if ( pPtr->pRsp )
  {
    uint8_t deviceType = ZSTACK_DEVICE_BUILD;

    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;

    memset( pPtr->pRsp, 0, sizeof(zstack_sysNwkInfoReadRsp_t) );

    pPtr->pRsp->nwkAddr = _NIB.nwkDevAddress;
    pPtr->pRsp->devState = (zstack_DevState)devState;
    pPtr->pRsp->panId = _NIB.nwkPanId;
    pPtr->pRsp->parentNwkAddr = _NIB.nwkCoordAddress;
    OsalPort_memcpy( pPtr->pRsp->extendedPanId, _NIB.extendedPANID, Z_EXTADDR_LEN );
    OsalPort_memcpy( pPtr->pRsp->ieeeAddr, NLME_GetExtAddr( ), Z_EXTADDR_LEN );
    OsalPort_memcpy( pPtr->pRsp->parentExtAddr, _NIB.nwkCoordExtAddress, Z_EXTADDR_LEN );
    pPtr->pRsp->logicalChannel = _NIB.nwkLogicalChannel;

    if ( deviceType & DEVICE_BUILD_COORDINATOR )
    {
      pPtr->pRsp->devTypes.coodinator = TRUE;
    }
    if ( deviceType & DEVICE_BUILD_ROUTER )
    {
      pPtr->pRsp->devTypes.router = TRUE;
    }
    if ( deviceType & DEVICE_BUILD_ENDDEVICE )
    {
      pPtr->pRsp->devTypes.enddevice = TRUE;
    }

    // capability info field
    if( ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_ALTPANCOORD )
    {
      pPtr->pRsp->capInfo.panCoord = TRUE;
    }
    if( ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_DEVICETYPE_FFD )
    {
      pPtr->pRsp->capInfo.ffd = TRUE;
    }
    if( ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_POWER_AC )
    {
      pPtr->pRsp->capInfo.mainsPower = TRUE;
    }
    if( ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_RCVR_ON_IDLE )
    {
      pPtr->pRsp->capInfo.rxOnWhenIdle = TRUE;
    }
    if( ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_SECURITY_CAPABLE )
    {
      pPtr->pRsp->capInfo.security = TRUE;
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processDevZDOCBReq
 *
 * @brief       Process the Device ZDO Callback Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processDevZDOCBReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_devZDOCBReq_t *pPtr = (zstackmsg_devZDOCBReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    epItem_t *pItem;

    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;

    pItem = epTableFindEntryConnection( srcServiceTaskId );
    if ( pItem == NULL )
    {
      // EP entry wasn't found, so create one with empty EP descriptor
      pItem = (epItem_t *)OsalPort_malloc( sizeof(epItem_t) );
      if ( pItem )
      {
        memset( pItem, 0, sizeof(epItem_t) );
        pItem->connection = srcServiceTaskId;
        if ( epTableAddNewEntry( pItem ) == FALSE )
        {
          OsalPort_free( pItem );
          pItem = NULL;
          pPtr->hdr.status = zstack_ZStatusValues_ZAfEndpointMax;
        }
      }
    }

    if ( pItem != NULL )
    {
      if ( pPtr->pReq->has_srcRtgIndCB )
      {
        if ( pPtr->pReq->srcRtgIndCB )
        {
          pItem->zdoCBs |= ZS_ZDO_SRC_RTG_IND_CBID;
        }
        else
        {
          pItem->zdoCBs &= ~ZS_ZDO_SRC_RTG_IND_CBID;
        }
      }

      if ( pPtr->pReq->has_concentratorIndCb )
      {
        if ( pPtr->pReq->concentratorIndCb )
        {
          pItem->zdoCBs |= ZS_ZDO_CONCENTRATOR_IND_CBID;
        }
        else
        {
          pItem->zdoCBs &= ~ZS_ZDO_CONCENTRATOR_IND_CBID;
        }
      }

      if ( pPtr->pReq->has_nwkDiscCnfCB )
      {
        if ( pPtr->pReq->nwkDiscCnfCB )
        {
          pItem->zdoCBs |= ZS_ZDO_NWK_DISCOVERY_CNF_CBID;
        }
        else
        {
          pItem->zdoCBs &= ~ZS_ZDO_NWK_DISCOVERY_CNF_CBID;
        }
      }

      if ( pPtr->pReq->has_beaconNotIndCB )
      {
        if ( pPtr->pReq->beaconNotIndCB )
        {
          pItem->zdoCBs |= ZS_ZDO_BEACON_NOTIFY_IND_CBID;
        }
        else
        {
          pItem->zdoCBs &= ~ZS_ZDO_BEACON_NOTIFY_IND_CBID;
        }
      }

      if ( pPtr->pReq->has_joinCnfCB )
      {
        if ( pPtr->pReq->joinCnfCB )
        {
          pItem->zdoCBs |= ZS_ZDO_JOIN_CNF_CBID;
        }
        else
        {
          pItem->zdoCBs &= ~ZS_ZDO_JOIN_CNF_CBID;
        }
      }

      if ( pPtr->pReq->has_leaveCnfCB )
      {
        if ( pPtr->pReq->leaveCnfCB )
        {
          pItem->zdoCBs |= ZS_ZDO_LEAVE_CNF_CBID;
        }
        else
        {
          pItem->zdoCBs &= ~ZS_ZDO_LEAVE_CNF_CBID;
        }
      }

      if ( pPtr->pReq->has_leaveIndCB )
      {
        if ( pPtr->pReq->leaveIndCB )
        {
          pItem->zdoCBs |= ZS_ZDO_LEAVE_IND_CBID;
        }
        else
        {
          pItem->zdoCBs &= ~ZS_ZDO_LEAVE_IND_CBID;
        }
      }

#if defined (ZDO_NWKADDR_REQUEST)
      if ( pPtr->pReq->has_nwkAddrRsp )
      {
        if ( pPtr->pReq->nwkAddrRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_NWK_ADDR_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_NWK_ADDR_RSP_CDID;
        }
      }
#endif // ZDO_NWKADDR_REQUEST

#if defined (ZDO_IEEEADDR_REQUEST)
      if ( pPtr->pReq->has_ieeeAddrRsp )
      {
        if ( pPtr->pReq->ieeeAddrRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_IEEE_ADDR_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_IEEE_ADDR_RSP_CDID;
        }
      }
#endif // ZDO_IEEEADDR_REQUEST

#if defined (ZDO_NODEDESC_REQUEST)
      if ( pPtr->pReq->has_nodeDescRsp )
      {
        if ( pPtr->pReq->nodeDescRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_NODE_DESC_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_NODE_DESC_RSP_CDID;
        }
      }
#endif // ZDO_NODEDESC_REQUEST

#if defined (ZDO_POWERDESC_REQUEST)
      if ( pPtr->pReq->has_powerDescRsp )
      {
        if ( pPtr->pReq->powerDescRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_POWER_DESC_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_POWER_DESC_RSP_CDID;
        }
      }
#endif // ZDO_POWERDESC_REQUEST

#if defined (ZDO_SIMPLEDESC_REQUEST)
      if ( pPtr->pReq->has_simpleDescRsp )
      {
        if ( pPtr->pReq->simpleDescRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_SIMPLE_DESC_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_SIMPLE_DESC_RSP_CDID;
        }
      }
#endif // ZDO_SIMPLEDESC_REQUEST

#if defined (ZDO_ACTIVEEP_REQUEST)
      if ( pPtr->pReq->has_activeEndpointRsp )
      {
        if ( pPtr->pReq->activeEndpointRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_ACTIVE_EP_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_ACTIVE_EP_RSP_CDID;
        }
      }
#endif // ZDO_ACTIVEEP_REQUEST

#if defined (ZDO_MATCH_REQUEST)
      if ( pPtr->pReq->has_matchDescRsp )
      {
        if ( pPtr->pReq->matchDescRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_MATCH_DESC_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_MATCH_DESC_RSP_CDID;
        }
      }
#endif // ZDO_MATCH_REQUEST

#if defined (ZDO_COMPLEXDESC_REQUEST)
      if ( pPtr->pReq->has_complexDescRsp )
      {
        if ( pPtr->pReq->complexDescRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_COMPLEX_DESC_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_COMPLEX_DESC_RSP_CDID;
        }
      }
#endif // ZDO_COMPLEXDESC_REQUEST

#if defined (ZDO_USERDESC_REQUEST)
      if ( pPtr->pReq->has_userDescRsp )
      {
        if ( pPtr->pReq->userDescRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_USER_DESC_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_USER_DESC_RSP_CDID;
        }
      }
#endif // ZDO_USERDESC_REQUEST

      if ( pPtr->pReq->has_userDescCnf )
      {
        if ( pPtr->pReq->userDescCnf )
        {
          pItem->zdoRsps |= ZS_ZDO_USER_DESC_CONF_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_USER_DESC_CONF_CDID;
        }
      }

#if defined (ZDO_SERVERDISC_REQUEST)
      if ( pPtr->pReq->has_serverDiscoveryRsp )
      {
        if ( pPtr->pReq->serverDiscoveryRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_SERVER_DISCOVERY_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_SERVER_DISCOVERY_RSP_CDID;
        }
      }
#endif // ZDO_SERVERDISC_REQUEST

#if defined (ZDO_BIND_UNBIND_REQUEST)
      if ( pPtr->pReq->has_bindRsp )
      {
        if ( pPtr->pReq->bindRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_BIND_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_BIND_RSP_CDID;
        }
      }
#endif // ZDO_BIND_UNBIND_REQUEST

#if defined (ZDO_ENDDEVICEBIND_REQUEST)
      if ( pPtr->pReq->has_endDeviceBindRsp )
      {
        if ( pPtr->pReq->endDeviceBindRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_END_DEVICE_BIND_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_END_DEVICE_BIND_RSP_CDID;
        }
      }
#endif // ZDO_ENDDEVICEBIND_REQUEST

#if defined (ZDO_BIND_UNBIND_REQUEST)
      if ( pPtr->pReq->has_unbindRsp )
      {
        if ( pPtr->pReq->unbindRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_UNBIND_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_UNBIND_RSP_CDID;
        }
      }
#endif // ZDO_BIND_UNBIND_REQUEST

#if defined (ZDO_MGMT_NWKDISC_REQUEST)
      if ( pPtr->pReq->has_mgmtNwkDiscRsp )
      {
        if ( pPtr->pReq->mgmtNwkDiscRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_MGMT_NWK_DISC_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_MGMT_NWK_DISC_RSP_CDID;
        }
      }
#endif // ZDO_MGMT_NWKDISC_REQUEST

#if defined (ZDO_MGMT_LQI_REQUEST)
      if ( pPtr->pReq->has_mgmtLqiRsp )
      {
        if ( pPtr->pReq->mgmtLqiRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_MGMT_LQI_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_MGMT_LQI_RSP_CDID;
        }
      }
#endif // ZDO_MGMT_LQI_REQUEST

#if defined (ZDO_MGMT_RTG_REQUEST)
      if ( pPtr->pReq->has_mgmtRtgRsp )
      {
        if ( pPtr->pReq->mgmtRtgRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_MGMT_RTG_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_MGMT_RTG_RSP_CDID;
        }
      }
#endif // ZDO_MGMT_RTG_REQUEST

#if defined (ZDO_MGMT_BIND_REQUEST)
      if ( pPtr->pReq->has_mgmtBindRsp )
      {
        if ( pPtr->pReq->mgmtBindRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_MGMT_BIND_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_MGMT_BIND_RSP_CDID;
        }
      }
#endif // ZDO_MGMT_BIND_REQUEST

#if defined (ZDO_MGMT_LEAVE_REQUEST)
      if ( pPtr->pReq->has_mgmtLeaveRsp )
      {
        if ( pPtr->pReq->mgmtLeaveRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_MGMT_LEAVE_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_MGMT_LEAVE_RSP_CDID;
        }
      }
#endif // ZDO_MGMT_LEAVE_REQUEST

#if defined (ZDO_MGMT_JOINDIRECT_REQUEST)
      if ( pPtr->pReq->has_mgmtDirectJoinRsp )
      {
        if ( pPtr->pReq->mgmtDirectJoinRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_MGMT_DIRECT_JOIN_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_MGMT_DIRECT_JOIN_RSP_CDID;
        }
      }
#endif // ZDO_MGMT_JOINDIRECT_REQUEST

#if defined (ZDO_MGMT_PERMIT_JOIN_REQUEST)
      if ( pPtr->pReq->has_mgmtPermitJoinRsp )
      {
        if ( pPtr->pReq->mgmtPermitJoinRsp )
        {
          pItem->zdoRsps |= ZS_ZDO_MGMT_PERMIT_JOIN_RSP_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_MGMT_PERMIT_JOIN_RSP_CDID;
        }
      }
#endif // ZDO_MGMT_PERMIT_JOIN_REQUEST

#if defined (ZDO_MGMT_NWKUPDATE_REQUEST)
      if ( pPtr->pReq->has_mgmtNwkUpdateNotify )
      {
        if ( pPtr->pReq->mgmtNwkUpdateNotify )
        {
          pItem->zdoRsps |= ZS_ZDO_MGMT_NWK_UPDATE_NOTIFY_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_MGMT_NWK_UPDATE_NOTIFY_CDID;
        }
      }
#endif // ZDO_MGMT_NWKUPDATE_REQUEST

      if ( pPtr->pReq->has_deviceAnnounce )
      {
        if ( pPtr->pReq->deviceAnnounce )
        {
          pItem->zdoRsps |= ZS_ZDO_DEVICE_ANNOUNCE_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_ZDO_DEVICE_ANNOUNCE_CDID;
        }
      }

      if ( pPtr->pReq->has_devStateChange )
      {
        if ( pPtr->pReq->devStateChange )
        {
          pItem->zdoRsps |= ZS_DEV_STATE_CHANGE_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_DEV_STATE_CHANGE_CDID;
        }
      }

      if ( pPtr->pReq->has_tcDeviceInd )
      {
        if ( pPtr->pReq->tcDeviceInd )
        {
          pItem->zdoRsps |= ZS_TC_DEVICE_IND_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_TC_DEVICE_IND_CDID;
        }
      }

      if ( pPtr->pReq->has_devPermitJoinInd )
      {
        if ( pPtr->pReq->devPermitJoinInd )
        {
          pItem->zdoRsps |= ZS_DEV_PERMIT_JOIN_IND_CDID;
        }
        else
        {
          pItem->zdoRsps &= ~ZS_DEV_PERMIT_JOIN_IND_CDID;
        }
      }
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processAfRegisterReq
 *
 * @brief       Process the AF Register Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processAfRegisterReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_afRegisterReq_t *pPtr = (zstackmsg_afRegisterReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    epItem_t *pItem;

    if ( epTableFindEntryEP( (uint8_t)pPtr->pReq->endpoint ) )
    {
      pPtr->hdr.status = zstack_ZStatusValues_ZAfDuplicateEndpoint;
    }
    else if ( epTableNumEntries( ) >= maxSupportedEndpoints )
    {
      pPtr->hdr.status = zstack_ZStatusValues_ZAfEndpointMax;
    }
    else
    {
      uint8_t allocated = FALSE;

      pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;

      // See if there is an EP item that hasn't defined an endpointl
      pItem = epTableFindEntryConnection( srcServiceTaskId );
      if ( (pItem == NULL) || (pItem->epDesc.endPoint != 0) )
      {
        // Allocate a new epItem_t if one doesn't existed or an endpoint has
        // already been
        // defined.
        pItem = (epItem_t *)OsalPort_malloc( sizeof(epItem_t) );
        if ( pItem )
        {
          allocated = TRUE;
          memset( pItem, 0, sizeof(epItem_t) );
          pItem->connection = srcServiceTaskId;
        }
      }

      if ( pItem )
      {
        uint8_t status;

        pItem->epDesc.endPoint = pPtr->pReq->endpoint;
        pItem->epDesc.latencyReq =
              (afNetworkLatencyReq_t)pPtr->pReq->latencyReq;
        pItem->epDesc.task_id = &ZStackServiceTaskId;

        pItem->epDesc.simpleDesc = (SimpleDescriptionFormat_t *)OsalPort_malloc(
              sizeof(SimpleDescriptionFormat_t) );
        if ( pItem->epDesc.simpleDesc )
        {
          int i;

          memset( pItem->epDesc.simpleDesc, 0,
                sizeof(SimpleDescriptionFormat_t) );

          pItem->epDesc.simpleDesc->AppDevVer =
                pPtr->pReq->pSimpleDesc->deviceVer;
          pItem->epDesc.simpleDesc->AppDeviceId =
                pPtr->pReq->pSimpleDesc->deviceID;
          pItem->epDesc.simpleDesc->EndPoint =
                pPtr->pReq->pSimpleDesc->endpoint;
          pItem->epDesc.simpleDesc->AppProfId =
                pPtr->pReq->pSimpleDesc->profileID;
          pItem->epDesc.simpleDesc->AppNumInClusters =
                pPtr->pReq->pSimpleDesc->n_inputClusters;
          pItem->epDesc.simpleDesc->AppNumOutClusters =
                pPtr->pReq->pSimpleDesc->n_outputClusters;

          pItem->epDesc.simpleDesc->pAppInClusterList =
                (cId_t *)OsalPort_malloc(
                sizeof(cId_t) * pItem->epDesc.simpleDesc->AppNumInClusters );
          if ( pItem->epDesc.simpleDesc->pAppInClusterList )
          {
            for ( i = 0; i < pItem->epDesc.simpleDesc->AppNumInClusters; i++ )
            {
              pItem->epDesc.simpleDesc->pAppInClusterList[i] =
                    pPtr->pReq->pSimpleDesc->pInputClusters[i];
            }
          }

          pItem->epDesc.simpleDesc->pAppOutClusterList =
                (cId_t *)OsalPort_malloc(
                sizeof(cId_t) * pItem->epDesc.simpleDesc->AppNumOutClusters );
          if ( pItem->epDesc.simpleDesc->pAppOutClusterList )
          {
            for ( i = 0; i < pItem->epDesc.simpleDesc->AppNumOutClusters; i++ )
            {
              pItem->epDesc.simpleDesc->pAppOutClusterList[i] =
                    pPtr->pReq->pSimpleDesc->pOutputClusters[i];
            }
          }
        }

        status = afRegister( &(pItem->epDesc) );

        if ( (status == afStatus_INVALID_PARAMETER)
              || (status == ZApsDuplicateEntry) )
        {
          afDelete( pItem->epDesc.endPoint );
          status = (zstack_ZStatusValues)afRegister( &(pItem->epDesc) );
        }

        if ( status == ZSuccess )
        {
          if ( allocated == TRUE )
          {
            if ( epTableAddNewEntry( pItem ) == FALSE )
            {
              freeEpItem( pItem );
            }
          }
        }
        else
        {
          if ( allocated == TRUE )
          {
            freeEpItem( pItem );
          }
          else
          {
            // AF registration failed clear the end point descriptor
            if ( pItem->epDesc.simpleDesc )
            {
              if ( pItem->epDesc.simpleDesc->pAppInClusterList )
              {
                OsalPort_free( pItem->epDesc.simpleDesc->pAppInClusterList );
              }
              if ( pItem->epDesc.simpleDesc->pAppOutClusterList )
              {
                OsalPort_free( pItem->epDesc.simpleDesc->pAppOutClusterList );
              }
              OsalPort_free( pItem->epDesc.simpleDesc );
            }

            memset( &(pItem->epDesc), 0, sizeof(endPointDesc_t) );
          }
        }

        // If status is still bad
        if ( (status == afStatus_INVALID_PARAMETER)
              || (status == ZApsDuplicateEntry) )
        {
          pPtr->hdr.status = zstack_ZStatusValues_ZAfDuplicateEndpoint;
        }
      }
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processAfUnRegisterReq
 *
 * @brief       Process the AF Unregister Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processAfUnRegisterReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_afUnRegisterReq_t *pPtr = (zstackmsg_afUnRegisterReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    epItem_t *pItem;

    pItem = epTableFindEntryEP( (uint8_t)pPtr->pReq->endpoint );
    if ( pItem == NULL )
    {
      pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
    }
    else
    {
      pPtr->hdr.status = (zstack_ZStatusValues)afDelete(
            pItem->epDesc.endPoint );

      if ( pPtr->hdr.status == zstack_ZStatusValues_ZSuccess )
      {
        epTableRemoveEntry( pItem );
      }
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processAfConfigGetReq
 *
 * @brief       Process the AF Config Get Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processAfConfigGetReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_afConfigGetReq_t *pPtr = (zstackmsg_afConfigGetReq_t *)pMsg;

  if ( pPtr->pReq && pPtr->pRsp )
  {
    afAPSF_Config_t config;

    afAPSF_ConfigGet( pPtr->pReq->endpoint, &config );

    pPtr->pRsp->endpoint = pPtr->pReq->endpoint;
    pPtr->pRsp->frameDelay = config.frameDelay;
    pPtr->pRsp->windowSize = config.windowSize;

    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processAfConfigSetReq
 *
 * @brief       Process the AF Config Set Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processAfConfigSetReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_afConfigSetReq_t *pPtr = (zstackmsg_afConfigSetReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    afAPSF_Config_t config;

    config.frameDelay = (uint8_t)pPtr->pReq->frameDelay;
    config.windowSize = (uint8_t)pPtr->pReq->windowSize;

    pPtr->hdr.status = (zstack_ZStatusValues)afAPSF_ConfigSet(
          pPtr->pReq->endpoint, &config );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processAfDataReq
 *
 * @brief       Process the AF Data Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processAfDataReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_afDataReq_t *pPtr = (zstackmsg_afDataReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    if ( epTableFindEntryEP( pPtr->pReq->srcEndpoint ) == NULL )
    {
      pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
    }
    else
    {
      endPointDesc_t *pEPDesc = NULL;
      afAddrType_t dstAddr;
      uint8_t *transId = pPtr->pReq->transID;
      uint8_t txOptions = convertMsgTransOptions( &pPtr->pReq->options );

      pEPDesc = afFindEndPointDesc( pPtr->pReq->srcEndpoint );
      if ( pEPDesc )
      {
        dstAddr.endPoint = pPtr->pReq->dstAddr.endpoint;
        dstAddr.panId = pPtr->pReq->dstAddr.panID;
        dstAddr.addrMode = (afAddrMode_t)pPtr->pReq->dstAddr.addrMode;
        if ( (dstAddr.addrMode == afAddr16Bit)
              || (dstAddr.addrMode == afAddrGroup)
              || (dstAddr.addrMode == afAddrBroadcast) )
        {
          dstAddr.addr.shortAddr = pPtr->pReq->dstAddr.addr.shortAddr;
        }
        else if ( dstAddr.addrMode == afAddr64Bit )
        {
          OsalPort_memcpy( dstAddr.addr.extAddr,
                &(pPtr->pReq->dstAddr.addr.extAddr), Z_EXTADDR_LEN );
        }

        if ( pPtr->pReq->n_relayList )
        {
          uint16_t *pList = OsalPort_malloc(
                sizeof(uint16_t) * pPtr->pReq->n_relayList );
          if ( pList )
          {
            int i;
            for ( i = 0; i < pPtr->pReq->n_relayList; i++ )
            {
              pList[i] = (uint16_t)pPtr->pReq->pRelayList[i];
            }

            pPtr->hdr.status = (zstack_ZStatusValues)AF_DataRequestSrcRtg(
                  &dstAddr, pEPDesc, pPtr->pReq->clusterID, pPtr->pReq->n_payload,
                  pPtr->pReq->pPayload, transId, txOptions, pPtr->pReq->radius,
                  pPtr->pReq->n_relayList, pList );

            OsalPort_free( pList );
          }
          else
          {
            pPtr->hdr.status = zstack_ZStatusValues_ZMemError;
          }
        }
        else
        {
          pPtr->hdr.status = (zstack_ZStatusValues)AF_DataRequest( &dstAddr,
                pEPDesc, pPtr->pReq->clusterID,
                pPtr->pReq->n_payload,
                pPtr->pReq->pPayload, transId,
                txOptions,
                pPtr->pReq->radius );
        }
      }
      else
      {
        pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
      }
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

#if defined (ZDO_NWKADDR_REQUEST)
/**************************************************************************************************
 * @fn          processZdoNwkAddrReq
 *
 * @brief       Process the ZDO Network Address Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoNwkAddrReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoNwkAddrReq_t *pPtr = (zstackmsg_zdoNwkAddrReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_NwkAddrReq(
          pPtr->pReq->ieeeAddr, pPtr->pReq->type,
          pPtr->pReq->startIndex, FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_NWKADDR_REQUEST

#if defined (ZDO_IEEEADDR_REQUEST)
/**************************************************************************************************
 * @fn          processZdoIeeeAddrReq
 *
 * @brief       Process the ZDO IEEE Address Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoIeeeAddrReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoIeeeAddrReq_t *pPtr = (zstackmsg_zdoIeeeAddrReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_IEEEAddrReq(
          pPtr->pReq->nwkAddr, pPtr->pReq->type,
          pPtr->pReq->startIndex, FALSE );

  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_IEEEADDR_REQUEST

#if defined (ZDO_NODEDESC_REQUEST)
/**************************************************************************************************
 * @fn          processZdoNodeDescReq
 *
 * @brief       Process the ZDO Node Descriptor Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoNodeDescReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoNodeDescReq_t *pPtr = (zstackmsg_zdoNodeDescReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;

    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = pPtr->pReq->dstAddr;

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_NodeDescReq( &dstAddr,
          pPtr->pReq->nwkAddrOfInterest, FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif

#if defined (ZDO_POWERDESC_REQUEST)
/**************************************************************************************************
 * @fn          processZdoPowerDescReq
 *
 * @brief       Process the ZDO Power Descriptor Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoPowerDescReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoPowerDescReq_t *pPtr = (zstackmsg_zdoPowerDescReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;

    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = pPtr->pReq->dstAddr;

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_PowerDescReq( &dstAddr,
          pPtr->pReq->nwkAddrOfInterest, FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif

#if defined (ZDO_SIMPLEDESC_REQUEST)
/**************************************************************************************************
 * @fn          processZdoSimpleDescReq
 *
 * @brief       Process the ZDO Simple Descriptor Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoSimpleDescReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoSimpleDescReq_t *pPtr = (zstackmsg_zdoSimpleDescReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;

    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = pPtr->pReq->dstAddr;

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_SimpleDescReq( &dstAddr,
          pPtr->pReq->nwkAddrOfInterest,
          pPtr->pReq->endpoint, FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif

#if defined (ZDO_ACTIVEEP_REQUEST)
/**************************************************************************************************
 * @fn          processZdoActiveEndpointsReq
 *
 * @brief       Process the ZDO Active Endpoints Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoActiveEndpointsReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoActiveEndpointReq_t *pPtr = (zstackmsg_zdoActiveEndpointReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;

    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = pPtr->pReq->dstAddr;

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_ActiveEPReq( &dstAddr,
          pPtr->pReq->nwkAddrOfInterest, FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_ACTIVEEP_REQUEST

#if defined (ZDO_MATCH_REQUEST)
/**************************************************************************************************
 * @fn          processZdoMatchDescReq
 *
 * @brief       Process the ZDO Match Descriptor Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoMatchDescReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoMatchDescReq_t *pPtr = (zstackmsg_zdoMatchDescReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;

    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = pPtr->pReq->dstAddr;

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_MatchDescReq( &dstAddr,
          pPtr->pReq->nwkAddrOfInterest,
          pPtr->pReq->profileID,
          pPtr->pReq->n_inputClusters,
          pPtr->pReq->pInputClusters,
          pPtr->pReq->n_outputClusters,
          pPtr->pReq->pOutputClusters,
          FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_MATCH_REQUEST

#if defined (ZDO_COMPLEXDESC_REQUEST)
/**************************************************************************************************
 * @fn          processZdoComplexDescReq
 *
 * @brief       Process the ZDO Complex Descriptor Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoComplexDescReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoComplexDescReq_t *pPtr = (zstackmsg_zdoComplexDescReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;

    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = pPtr->pReq->dstAddr;

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_ComplexDescReq( &dstAddr,
          pPtr->pReq->nwkAddrOfInterest,
          FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_COMPLEXDESC_REQUEST

#if defined (ZDO_SERVERDISC_REQUEST)
/**************************************************************************************************
 * @fn          processZdoServerDiscReq
 *
 * @brief       Process the ZDO Server Discovery Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoServerDiscReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoServerDiscReq_t *pPtr = (zstackmsg_zdoServerDiscReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    uint16_t serverMask;

    serverMask = convertMsgServerCapabilities( &pPtr->pReq->serverMask );

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_ServerDiscReq( serverMask, FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_SERVERDISC_REQUEST

#if defined (ZDO_ENDDEVICEBIND_REQUEST)
/**************************************************************************************************
 * @fn          processZdoEndDeviceBindReq
 *
 * @brief       Process the ZDO End Device Bind Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoEndDeviceBindReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoEndDeviceBindReq_t *pPtr = (zstackmsg_zdoEndDeviceBindReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;

    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = pPtr->pReq->dstAddr;

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_EndDeviceBindReq( &dstAddr,
          pPtr->pReq->bindingTarget,
          pPtr->pReq->endpoint,
          pPtr->pReq->profileID,
          pPtr->pReq->n_inputClusters,
          pPtr->pReq->pInputClusters,
          pPtr->pReq->n_outputClusters,
          pPtr->pReq->pOutputClusters,
          FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_ENDDEVICEBIND_REQUEST

#if defined (ZDO_BIND_UNBIND_REQUEST)
/**************************************************************************************************
 * @fn          processZdoBindReq
 *
 * @brief       Process the ZDO Bind Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoBindReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoBindReq_t *pPtr = (zstackmsg_zdoBindReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t srcBindAddr;
    zAddrType_t remoteDevAddr;

    srcBindAddr.addrMode = Addr16Bit;
    srcBindAddr.addr.shortAddr = pPtr->pReq->nwkAddr;

    remoteDevAddr.addrMode = pPtr->pReq->bindInfo.dstAddr.addrMode;
    if ( (remoteDevAddr.addrMode == afAddr16Bit)
          || (remoteDevAddr.addrMode == afAddrGroup)
          || (remoteDevAddr.addrMode == afAddrBroadcast) )
    {
      remoteDevAddr.addr.shortAddr = pPtr->pReq->bindInfo.dstAddr.addr.shortAddr;
    }
    else if ( remoteDevAddr.addrMode == afAddr64Bit )
    {
      OsalPort_memcpy( remoteDevAddr.addr.extAddr,
            pPtr->pReq->bindInfo.dstAddr.addr.extAddr, Z_EXTADDR_LEN );
    }

    if ( srcBindAddr.addr.shortAddr == _NIB.nwkDevAddress )
    {
      if ( bindAddEntry( pPtr->pReq->bindInfo.srcEndpoint,
              &remoteDevAddr,
              pPtr->pReq->bindInfo.dstAddr.endpoint,
              1,
              (uint16_t *)&(pPtr->pReq->bindInfo.clusterID) ) )
      {
        pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
      }
      else
      {
        pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
      }
    }
    else
    {
      pPtr->hdr.status = (zstack_ZStatusValues)ZDP_BindUnbindReq(
            Bind_req,
            &srcBindAddr,
            pPtr->pReq->bindInfo.srcAddr,
            pPtr->pReq->bindInfo.srcEndpoint,
            (cId_t)pPtr->pReq->bindInfo.
            clusterID,
            &remoteDevAddr,
            pPtr->pReq->bindInfo.dstAddr.
            endpoint,
            FALSE );
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_BIND_UNBIND_REQUEST

#if defined (ZDO_BIND_UNBIND_REQUEST)
/**************************************************************************************************
 * @fn          processZdoUnbindReq
 *
 * @brief       Process the ZDO Unbind Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoUnbindReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoUnbindReq_t *pPtr = (zstackmsg_zdoUnbindReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t srcBindAddr;
    zAddrType_t remoteDevAddr;

    srcBindAddr.addrMode = Addr16Bit;
    srcBindAddr.addr.shortAddr = pPtr->pReq->nwkAddr;

    remoteDevAddr.addrMode = pPtr->pReq->bindInfo.dstAddr.addrMode;
    if ( (remoteDevAddr.addrMode == afAddr16Bit)
          || (remoteDevAddr.addrMode == afAddrGroup)
          || (remoteDevAddr.addrMode == afAddrBroadcast) )
    {
      remoteDevAddr.addr.shortAddr = pPtr->pReq->bindInfo.dstAddr.addr.shortAddr;
    }
    else if ( remoteDevAddr.addrMode == afAddr64Bit )
    {
      OsalPort_memcpy( remoteDevAddr.addr.extAddr,
            pPtr->pReq->bindInfo.dstAddr.addr.extAddr, Z_EXTADDR_LEN );
    }

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_BindUnbindReq(
          Unbind_req,
          &srcBindAddr,
          pPtr->pReq->bindInfo.srcAddr,
          pPtr->pReq->bindInfo.srcEndpoint,
          (cId_t)pPtr->pReq->bindInfo.clusterID,
          &remoteDevAddr,
          pPtr->pReq->bindInfo.dstAddr.endpoint, FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_BIND_UNBIND_REQUEST

#if defined (ZDO_MGMT_NWKDISC_REQUEST)
/**************************************************************************************************
 * @fn          processZdoMgmtNwkDiscReq
 *
 * @brief       Process the ZDO Management Network Discovery Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoMgmtNwkDiscReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoMgmtNwkDiscReq_t *pPtr = (zstackmsg_zdoMgmtNwkDiscReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;

    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = pPtr->pReq->nwkAddr;

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_MgmtNwkDiscReq(
          &dstAddr, pPtr->pReq->scanChannels,
          pPtr->pReq->scanDuration, pPtr->pReq->startIndex, FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_MGMT_NWKDISC_REQUEST

#if defined (ZDO_MGMT_LQI_REQUEST)
/**************************************************************************************************
 * @fn          processZdoMgmtLqiReq
 *
 * @brief       Process the ZDO Management LQI Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoMgmtLqiReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoMgmtLqiReq_t *pPtr = (zstackmsg_zdoMgmtLqiReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;

    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = pPtr->pReq->nwkAddr;

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_MgmtLqiReq(
          &dstAddr, pPtr->pReq->startIndex, FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_MGMT_LQI_REQUEST

#if defined (ZDO_MGMT_RTG_REQUEST)
/**************************************************************************************************
 * @fn          processZdoMgmtRtgReq
 *
 * @brief       Process the ZDO Management Routing Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoMgmtRtgReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoMgmtRtgReq_t *pPtr = (zstackmsg_zdoMgmtRtgReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;

    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = pPtr->pReq->nwkAddr;

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_MgmtRtgReq(
          &dstAddr, pPtr->pReq->startIndex, FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_MGMT_RTG_REQUEST

#if defined (ZDO_MGMT_BIND_REQUEST)
/**************************************************************************************************
 * @fn          processZdoMgmtBindReq
 *
 * @brief       Process the ZDO Management Binding Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoMgmtBindReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoMgmtBindReq_t *pPtr = (zstackmsg_zdoMgmtBindReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;

    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = pPtr->pReq->nwkAddr;

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_MgmtBindReq(
          &dstAddr, pPtr->pReq->startIndex, FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_MGMT_BIND_REQUEST

#if defined (ZDO_MGMT_LEAVE_REQUEST)
/**************************************************************************************************
 * @fn          processZdoMgmtLeaveReq
 *
 * @brief       Process the ZDO Management Leave Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoMgmtLeaveReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoMgmtLeaveReq_t *pPtr = (zstackmsg_zdoMgmtLeaveReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;

    dstAddr.addrMode = Addr16Bit;

    // Check for broadcast message
    if ( (pPtr->pReq->nwkAddr == 0xFFFF) || (pPtr->pReq->nwkAddr == 0xFFFD)
          || (pPtr->pReq->nwkAddr == 0xFFFC) )
    {
      // Send to self first
      dstAddr.addr.shortAddr = NLME_GetShortAddr( );
      pPtr->hdr.status = (zstack_ZStatusValues)ZDP_MgmtLeaveReq( &dstAddr,
            pPtr->pReq->deviceAddress,
            pPtr->pReq->options.removeChildren,
            pPtr->pReq->options.rejoin,
            FALSE );
    }

    dstAddr.addr.shortAddr = pPtr->pReq->nwkAddr;

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_MgmtLeaveReq( &dstAddr,
          pPtr->pReq->deviceAddress,
          pPtr->pReq->options.removeChildren,
          pPtr->pReq->options.rejoin,
          FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_MGMT_LEAVE_REQUEST

#if defined (ZDO_MGMT_JOINDIRECT_REQUEST)
/**************************************************************************************************
 * @fn          processZdoMgmtDirectJoinReq
 *
 * @brief       Process the ZDO Management Direct Join Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoMgmtDirectJoinReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoMgmtDirectJoinReq_t *pPtr = (zstackmsg_zdoMgmtDirectJoinReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;
    uint8_t cInfo;

    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = pPtr->pReq->nwkAddr;

    cInfo = convertCapabilityInfo( &pPtr->pReq->capInfo );

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_MgmtDirectJoinReq( &dstAddr,
          pPtr->pReq->deviceAddress,
          cInfo,
          FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_MGMT_JOINDIRECT_REQUEST

#if defined (ZDO_MGMT_PERMIT_JOIN_REQUEST)
/**************************************************************************************************
 * @fn          processZdoMgmtPermitJoinReq
 *
 * @brief       Process the ZDO Management Permit Join Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoMgmtPermitJoinReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoMgmtPermitJoinReq_t *pPtr = (zstackmsg_zdoMgmtPermitJoinReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;

    dstAddr.addrMode = Addr16Bit;

    dstAddr.addr.shortAddr = pPtr->pReq->nwkAddr;

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_MgmtPermitJoinReq( &dstAddr,
          pPtr->pReq->duration,
          pPtr->pReq->tcSignificance,
          FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

#endif // ZDO_MGMT_PERMIT_JOIN_REQUEST

#if defined (ZDO_MGMT_NWKUPDATE_REQUEST)
/**************************************************************************************************
 * @fn          processZdoMgmtNwkUpdateReq
 *
 * @brief       Process the ZDO Management Network Update Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoMgmtNwkUpdateReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoMgmtNwkUpdateReq_t *pPtr = (zstackmsg_zdoMgmtNwkUpdateReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;

    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = pPtr->pReq->dstAddr;

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_MgmtNwkUpdateReq( &dstAddr,
          pPtr->pReq->channelMask,
          pPtr->pReq->scanDuration,
          pPtr->pReq->scanCount,
          pPtr->pReq->nwkUpdateId,
          pPtr->pReq->nwkMgrAddr );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_MGMT_NWKUPDATE_REQUEST

/**************************************************************************************************
 * @fn          processZdoDeviceAnnounceReq
 *
 * @brief       Process the ZDO Device Announce Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoDeviceAnnounceReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoDeviceAnnounceReq_t *pPtr =
        (zstackmsg_zdoDeviceAnnounceReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    uint8_t cInfo;

    cInfo = convertCapabilityInfo( &pPtr->pReq->capabilities );

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_DeviceAnnce(
          pPtr->pReq->nwkAddr, pPtr->pReq->ieeeAddr, cInfo, FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

#if defined (ZDO_USERDESCSET_REQUEST)
/**************************************************************************************************
 * @fn          processZdoUserDescSetReq
 *
 * @brief       Process the ZDO Management User Descriptor Set Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoUserDescSetReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoUserDescSetReq_t *pPtr = (zstackmsg_zdoUserDescSetReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;
    UserDescriptorFormat_t userDesc;

    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = pPtr->pReq->dstAddr;

    userDesc.len = pPtr->pReq->n_userDescriptor;
    if ( userDesc.len > AF_MAX_USER_DESCRIPTOR_LEN )
    {
      userDesc.len = AF_MAX_USER_DESCRIPTOR_LEN;
    }
    OsalPort_memcpy( userDesc.desc, pPtr->pReq->pUserDescriptor, userDesc.len );

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_UserDescSet( &dstAddr,
          pPtr->pReq->nwkAddrOfInterest,
          &userDesc,
          FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_USERDESCSET_REQUEST

#if defined (ZDO_USERDESC_REQUEST)
/**************************************************************************************************
 * @fn          processZdoUserDescReq
 *
 * @brief       Process the ZDO User Descriptor Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoUserDescReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_zdoUserDescReq_t *pPtr = (zstackmsg_zdoUserDescReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    zAddrType_t dstAddr;

    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = pPtr->pReq->dstAddr;

    pPtr->hdr.status = (zstack_ZStatusValues)ZDP_UserDescReq( &dstAddr,
          pPtr->pReq->nwkAddrOfInterest, FALSE );
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif // ZDO_USERDESC_REQUEST




/**************************************************************************************************
 * @fn          processZdoSetBindUnbindAuthAddrReq
 *
 * @brief       Process the request from the application to handle bind request and autorize
 *              if those take place or not. If set, application has to use Zstack API to add
 *              binds to itself using Zstackapi_ZdoBindReq, the notifications of a bind/unbind will
 *              come in zstackmsg_CmdIDs_ZDO_BIND_UNBIND_APP_AUTH_IND
 *
 * @param       srcEntityID - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processZdoSetBindUnbindAuthAddrReq( uint8_t srcServiceTaskId, void *pMsg )
{
    zstackmsg_zdoSetBindUnbindAuthAddrReq_t *pPtr = (zstackmsg_zdoSetBindUnbindAuthAddrReq_t *)pMsg;

  if ( pPtr->pReq )
  {
      OsalPort_memcpy(zdoBindUnbindAuthAddr, pPtr->pReq->AuthAddress, Z_EXTADDR_LEN);
      zdoBindUnbindAuthClusterId = pPtr->pReq->ClusterId;
      zdoBindUnbindAuthEndpoint  = pPtr->pReq->Endpoint;

      pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}



#if ( BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE )
/**************************************************************************************************
 * @fn          processTlScanReqInd
 *
 * @brief       Process touchlink scan request indication
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processTlScanReqInd( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_tlScanReq_t *pPtr = (zstackmsg_tlScanReq_t *)pMsg;

  if ( pPtr != NULL )
  {
      pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
#if defined ( BDB_TL_TARGET )
      touchLink_TargetSendScanRsp( TOUCHLINK_INTERNAL_ENDPOINT, &(pPtr->pReq->addr), pPtr->pReq->transID, pPtr->pReq->seqNum );
#elif defined ( BDB_TL_INITIATOR )
      touchLink_InitiatorSendScanRsp( TOUCHLINK_INTERNAL_ENDPOINT, &(pPtr->pReq->addr), pPtr->pReq->transID, pPtr->pReq->seqNum );
#endif
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processTouchlinkNetworkJoinReqInd
 *
 * @brief       Process touchlink target network join request indication
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processTouchlinkNetworkJoinReqInd( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_touchlinkNwkJointReq_t *pPtr = (zstackmsg_touchlinkNwkJointReq_t *)pMsg;

  if ( pPtr != NULL )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
#if defined ( BDB_TL_TARGET )
    targetProcessNwkJoinEvt(&(pPtr->pReq->addr), &(pPtr->pReq->nwkJoinReq), pPtr->pReq->seqNum , pPtr->pReq->allowStealing);
#elif defined ( BDB_TL_INITIATOR )
    initiatorProcessNwkJoinEvt(&(pPtr->pReq->addr), &(pPtr->pReq->nwkJoinReq), pPtr->pReq->seqNum);
#endif
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processTouchlinkNetworkUpdateReqInd
 *
 * @brief       Process touchlink target network update request indication
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processTouchlinkNetworkUpdateReqInd( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_touchlinkNwkUpdateReq_t *pPtr = (zstackmsg_touchlinkNwkUpdateReq_t *)pMsg;
  bdbTLNwkUpdateReq_t updateReq;
  if ( pPtr != NULL )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    OsalPort_memcpy(&updateReq, pPtr->pReq, sizeof(bdbTLNwkUpdateReq_t));
#if defined ( BDB_TL_TARGET )
    targetProcessNwkUpdateEvt(&updateReq);
#elif defined ( BDB_TL_INITIATOR )
    initiatorProcessNwkUpdateEvt(&updateReq);
#endif
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

#endif

#if defined ( BDB_TL_TARGET )
#if (ZSTACK_ROUTER_BUILD)
/**************************************************************************************************
 * @fn          processTlTargetNetworkStartReqInd
 *
 * @brief       Process touchlink target network start request indication
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processTlTargetNetworkStartReqInd( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_tlTargetNwkStartReq_t *pPtr = (zstackmsg_tlTargetNwkStartReq_t *)pMsg;

  if ( pPtr != NULL )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    targetProcessNwkStartEvt(&(pPtr->pReq->addr), &(pPtr->pReq->nwkStartReq), pPtr->pReq->seqNum, pPtr->pReq->allowStealing);
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif

/**************************************************************************************************
 * @fn          processTlTargetResetToFactoryNewReqInd
 *
 * @brief       Process touchlink target reset to factory new request indication
 *
 * @param       srcServiceTaskId - Source Task ID
 *
 * @return      TRUE to send the response back
 */
static bool processTlTargetResetToFactoryNewReqInd( uint8_t srcServiceTaskId )
{
  targetProcessResetToFNEvt( );
  return (TRUE);
}
#endif


#if defined ( BDB_TL_INITIATOR )
/**************************************************************************************************
 * @fn          processTlGetScanBaseTime
 *
 * @brief       Process touchlink get scan base time
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processTlGetScanBaseTime( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_touchlinkGetScanBaseTime_t *pPtr = (zstackmsg_touchlinkGetScanBaseTime_t *)pMsg;

  if ( pPtr != NULL )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;

    pPtr->pReq->time = initiatorProcessGetScanBaseTime(pPtr->pReq->stopTimer);
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processTlInitiatorScanRspInd
 *
 * @brief       Process touchlink scan response indication
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processTlInitiatorScanRspInd( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_touchlinkScanRsp_t *pPtr = (zstackmsg_touchlinkScanRsp_t *)pMsg;

  if ( pPtr != NULL )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    initiatorProcessScanRsp(&(pPtr->pReq->addr), &(pPtr->pReq->scanRsp));
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processTlInitiatorDevInfoRspInd
 *
 * @brief       Process touchlink device information response indication
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processTlInitiatorDevInfoRspInd( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_touchlinkDevInfoRsp_t *pPtr = (zstackmsg_touchlinkDevInfoRsp_t *)pMsg;

  if ( pPtr != NULL )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    initiatorProcessDevInfoRsp(&(pPtr->pReq->addr), &(pPtr->pReq->devInfoRsp));
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processTlInitiatorNwkStartRspInd
 *
 * @brief       Process touchlink network start response indication
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processTlInitiatorNwkStartRspInd( uint8_t srcServiceTaskId, void *pMsg )
{
    zstackmsg_touchlinkNwkStartRsp_t *pPtr = (zstackmsg_touchlinkNwkStartRsp_t *)pMsg;

  if ( pPtr != NULL )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    initiatorProcessNwkStartRsp(&(pPtr->pReq->nwkStartRsp));
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processTlInitiatorNwkJoinRspInd
 *
 * @brief       Process touchlink network start response indication
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processTlInitiatorNwkJoinRspInd( uint8_t srcServiceTaskId, void *pMsg )
{
    zstackmsg_touchlinkNwkJoinRsp_t *pPtr = (zstackmsg_touchlinkNwkJoinRsp_t *)pMsg;

  if ( pPtr != NULL )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    initiatorProcessNwkJointRsp(&(pPtr->pReq->addr), &(pPtr->pReq->nwkJoinRsp));
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif

/**************************************************************************************************
 * @fn          processBdbStartCommissioningReq
 *
 * @brief       Process BDB Start Commissioning Request based on the modes requested
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbStartCommissioningReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbStartCommissioningReq_t *pPtr = (zstackmsg_bdbStartCommissioningReq_t *)pMsg;

  if ( pPtr->pReq )
  {
      pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
      bdb_StartCommissioning(pPtr->pReq->commissioning_mode);
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
/**************************************************************************************************
 * @fn          processBdbSetIdentifyActiveEndpointReq
 *
 * @brief       Process BDB Set Identify Active Endpoint Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbSetIdentifyActiveEndpointReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbSetIdentifyActiveEndpointReq_t *pPtr = (zstackmsg_bdbSetIdentifyActiveEndpointReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    pPtr->hdr.status = bdb_SetIdentifyActiveEndpoint(pPtr->pReq->activeEndpoint);
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processBdbGetIdentifyActiveEndpointReq
 *
 * @brief       Process BDB Get Identify Active Endpoint Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbGetIdentifyActiveEndpointReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbGetIdentifyActiveEndpointReq_t *pPtr = (zstackmsg_bdbGetIdentifyActiveEndpointReq_t *)pMsg;

  if ( pPtr->pRsp )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    pPtr->pRsp->activeEndpoint = bdb_GetIdentifyActiveEndpoint();
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processBdbStopInitiatorFindingBindingReq
 *
 * @brief       Process BDB Stop Initiator Finding and Binding Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbStopInitiatorFindingBindingReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_genericReq_t *pPtr = (zstackmsg_genericReq_t *)pMsg;

  pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;

  bdb_StopInitiatorFindingBinding();

  return (TRUE);
}
#endif

/**************************************************************************************************
 * @fn          processBdbZclIdentifyCmdIndReq
 *
 * @brief       Process BDB Zcl Identify Cmd Ind Request.
 *
 * @param       srcEntityID - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbZclIdentifyCmdIndReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbZCLIdentifyCmdIndReq_t *pPtr = (zstackmsg_bdbZCLIdentifyCmdIndReq_t *)pMsg;
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
  if ( pPtr->pReq )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    bdb_ZclIdentifyCmdInd(pPtr->pReq->identifyTime,pPtr->pReq->endpoint);
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }
#else
  pPtr->hdr.status = zstack_ZStatusValues_ZFailure;
#endif

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processGetZCLFrameCounterReq
 *
 * @brief       Process Get ZCL Frame Counter Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processGetZCLFrameCounterReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_getZCLFrameCounterReq_t *pPtr = (zstackmsg_getZCLFrameCounterReq_t *)pMsg;

  if ( pPtr->pRsp )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    pPtr->pRsp->zclFrameCounter = zcl_getFrameCounter();
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processBdbSetEpDescListToActiveEndpoint
 *
 * @brief       Process BDB Set Endpoint Descriptor List To Active Endpoint
 *
 * @param       srcEntityID - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbSetEpDescListToActiveEndpoint( uint8_t srcServiceTaskId, void *pMsg )
{
    zstackmsg_bdbSetEpDescListToActiveEndpointReq_t *pPtr = (zstackmsg_bdbSetEpDescListToActiveEndpointReq_t *)pMsg;
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
  if ( pPtr->pRsp )
  {
    endPointDesc_t *bdb_CurrEpDescriptor;
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;

    bdb_CurrEpDescriptor = bdb_setEpDescListToActiveEndpoint();

    if(bdb_CurrEpDescriptor != NULL)
    {
      pPtr->pRsp->EndPoint = bdb_CurrEpDescriptor->endPoint;
    }
    else
    {
      pPtr->hdr.status = zstack_ZStatusValues_ZFailure;
    }
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }
#else
  pPtr->hdr.status = zstack_ZStatusValues_ZFailure;
#endif
  return (TRUE);
}




/**************************************************************************************************
 * @fn          processBdbResetLocalActionReq
 *
 * @brief       Process BDB Reset Local Action Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbResetLocalActionReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_genericReq_t *pPtr = (zstackmsg_genericReq_t *)pMsg;

  pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;

  bdb_resetLocalAction();

  (void)pMsg;

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processBdbSetAttributesReq
 *
 * @brief       Process BDB Set attributes Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbSetAttributesReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbSetAttributesReq_t *pPtr = (zstackmsg_bdbSetAttributesReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;

#if (ZG_BUILD_JOINING_TYPE)
    if ( pPtr->pReq->has_bdbTCLinkKeyExchangeAttemptsMax )
    {
    	bdbAttributes.bdbTCLinkKeyExchangeAttemptsMax = pPtr->pReq->bdbTCLinkKeyExchangeAttemptsMax;
    }

    if ( pPtr->pReq->has_bdbTCLinkKeyExchangeMethod )
    {
    	bdbAttributes.bdbTCLinkKeyExchangeMethod = pPtr->pReq->bdbTCLinkKeyExchangeMethod;
    }
#endif

    if ( pPtr->pReq->has_bdbCommissioningGroupID )
    {
    	bdbAttributes.bdbCommissioningGroupID = pPtr->pReq->bdbCommissioningGroupID;
    }

    if ( pPtr->pReq->has_bdbPrimaryChannelSet )
    {
    	bdbAttributes.bdbPrimaryChannelSet = pPtr->pReq->bdbPrimaryChannelSet;
    }

    if ( pPtr->pReq->has_bdbScanDuration )
    {
    	bdbAttributes.bdbScanDuration = pPtr->pReq->bdbScanDuration;
    }

    if ( pPtr->pReq->has_bdbSecondaryChannelSet )
    {
    	bdbAttributes.bdbSecondaryChannelSet = pPtr->pReq->bdbSecondaryChannelSet;
    }

#if (ZG_BUILD_COORDINATOR_TYPE)
    if ( pPtr->pReq->has_bdbJoinUsesInstallCodeKey )
    {
    	bdbAttributes.bdbJoinUsesInstallCodeKey = pPtr->pReq->bdbJoinUsesInstallCodeKey;
    }

    if ( pPtr->pReq->has_bdbTrustCenterNodeJoinTimeout )
    {
    	bdbAttributes.bdbTrustCenterNodeJoinTimeout = pPtr->pReq->bdbTrustCenterNodeJoinTimeout;
    }

    if ( pPtr->pReq->has_bdbTrustCenterRequireKeyExchange )
    {
    	bdbAttributes.bdbTrustCenterRequireKeyExchange = pPtr->pReq->bdbTrustCenterRequireKeyExchange;
    }
#endif
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processBdbGetAttributesReq
 *
 * @brief       Process BDB Get Attributes Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbGetAttributesReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbGetAttributesReq_t *pPtr = (zstackmsg_bdbGetAttributesReq_t *)pMsg;

  if ( pPtr->pRsp )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;

    pPtr->pRsp->bdbCommissioningGroupID = bdbAttributes.bdbCommissioningGroupID;
    pPtr->pRsp->bdbCommissioningMode = bdbAttributes.bdbCommissioningMode;
    pPtr->pRsp->bdbCommissioningStatus = bdbAttributes.bdbCommissioningStatus;
    pPtr->pRsp->bdbNodeCommissioningCapability = bdbAttributes.bdbNodeCommissioningCapability;
    pPtr->pRsp->bdbNodeIsOnANetwork = bdbAttributes.bdbNodeIsOnANetwork;
    pPtr->pRsp->bdbPrimaryChannelSet = bdbAttributes.bdbPrimaryChannelSet;
    pPtr->pRsp->bdbScanDuration = bdbAttributes.bdbScanDuration;
    pPtr->pRsp->bdbSecondaryChannelSet = bdbAttributes.bdbSecondaryChannelSet;
#if (ZG_BUILD_COORDINATOR_TYPE)
    pPtr->pRsp->bdbJoinUsesInstallCodeKey = bdbAttributes.bdbJoinUsesInstallCodeKey;
    pPtr->pRsp->bdbTrustCenterNodeJoinTimeout = bdbAttributes.bdbTrustCenterNodeJoinTimeout;
    pPtr->pRsp->bdbTrustCenterRequireKeyExchange = bdbAttributes.bdbTrustCenterRequireKeyExchange;
#endif
#if (ZG_BUILD_JOINING_TYPE)
    pPtr->pRsp->bdbNodeJoinLinkKeyType = bdbAttributes.bdbNodeJoinLinkKeyType;
    pPtr->pRsp->bdbTCLinkKeyExchangeAttempts = bdbAttributes.bdbTCLinkKeyExchangeAttempts;
    pPtr->pRsp->bdbTCLinkKeyExchangeAttemptsMax = bdbAttributes.bdbTCLinkKeyExchangeAttemptsMax;
    pPtr->pRsp->bdbTCLinkKeyExchangeMethod = bdbAttributes.bdbTCLinkKeyExchangeMethod;
#endif
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED == 1)
/**************************************************************************************************
 * @fn          processBdbGetFBInitiatorStatusReq
 *
 * @brief       Process BDB Get Finding and Binding Initiator Status Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbGetFBInitiatorStatusReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbGetFBInitiatorStatusReq_t *pPtr = (zstackmsg_bdbGetFBInitiatorStatusReq_t *)pMsg;

  if ( pPtr->pRsp )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    bdb_GetFBInitiatorStatus(&pPtr->pRsp->RemainingTime, &pPtr->pRsp->AttemptsLeft);
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif

/**************************************************************************************************
 * @fn          processBdbGenerateInstallCodeCRCReq
 *
 * @brief       Process BDB Generate Install Code CRC Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbGenerateInstallCodeCRCReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbGenerateInstallCodeCRCReq_t *pPtr = (zstackmsg_bdbGenerateInstallCodeCRCReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    pPtr->pRsp->CRC = bdb_GenerateInstallCodeCRC(pPtr->pReq->installCode);
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

#ifdef BDB_REPORTING
/**************************************************************************************************
 * @fn          processBdbRepAddAttrCfgRecordDefaultToListReq
 *
 * @brief       Process BDB Reporting Add Attribute Configuration Record Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbRepAddAttrCfgRecordDefaultToListReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbRepAddAttrCfgRecordDefaultToListReq_t *pPtr = (zstackmsg_bdbRepAddAttrCfgRecordDefaultToListReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    pPtr->hdr.status = bdb_RepAddAttrCfgRecordDefaultToList(pPtr->pReq->endpoint,
                                         pPtr->pReq->cluster,
                                         pPtr->pReq->attrID,
                                         pPtr->pReq->minReportInt,
                                         pPtr->pReq->maxReportInt,
                                         pPtr->pReq->reportableChange);


  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processBdbRepChangedAttrValueReq
 *
 * @brief       Process BDB Report Changed Attribute Value Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbRepChangedAttrValueReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbRepChangedAttrValueReq_t *pPtr = (zstackmsg_bdbRepChangedAttrValueReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    pPtr->hdr.status = bdb_RepChangedAttrValue(pPtr->pReq->endpoint,pPtr->pReq->cluster, pPtr->pReq->attrID);
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processBdbProcessInReadReportCfgReq
 *
 * @brief       Process BDB an Incomming Configure Report Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */

static bool processBdbProcessInReadReportCfgReq(uint8_t srcServiceTaskId, void *pMsg)
{
    zstackmsg_bdbProcessInReadReportCfgReq_t *pPtr = (zstackmsg_bdbProcessInReadReportCfgReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    pPtr->hdr.status = bdb_ProcessInReadReportCfgCmd(pPtr->pReq->pZclIncommingMsg);
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processBdbProcessInConfigReportReq
 *
 * @brief       Process BDB an Incomming Configure Report Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */

static bool processBdbProcessInConfigReportReq(uint8_t srcServiceTaskId, void *pMsg)
{
    zstackmsg_bdbProcessInConfigReportReq_t *pPtr = (zstackmsg_bdbProcessInConfigReportReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    pPtr->hdr.status = bdb_ProcessInConfigReportCmd(pPtr->pReq->pZclIncommingMsg);
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif



/**************************************************************************************************
 * @fn          processBdbAddInstallCodeReq
 *
 * @brief       Process BDB Add Install Code Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbAddInstallCodeReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbAddInstallCodeReq_t *pPtr = (zstackmsg_bdbAddInstallCodeReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    pPtr->hdr.status = bdb_addInstallCode(pPtr->pReq->pInstallCode, pPtr->pReq->pExt);

  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

#if (ZG_BUILD_JOINING_TYPE)
/**************************************************************************************************
 * @fn          processBdbSetActiveCentralizedLinkKeyReq
 *
 * @brief       Process BDB Set Active Centralized Link Key
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbSetActiveCentralizedLinkKeyReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbSetActiveCentralizedLinkKeyReq_t *pPtr = (zstackmsg_bdbSetActiveCentralizedLinkKeyReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    pPtr->hdr.status = bdb_setActiveCentralizedLinkKey(pPtr->pReq->zstack_CentralizedLinkKeyModes,
                                    pPtr->pReq->pKey);
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processBdbCBKETCLinkKeyExchangeAttemptReq
 *
 * @brief       Process BDB Certificate Base Key Exchange (CBKE) Trust Center (TC) Link Key exchange
 *              Attempt Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbCBKETCLinkKeyExchangeAttemptReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbCBKETCLinkKeyExchangeAttemptReq_t *pPtr = (zstackmsg_bdbCBKETCLinkKeyExchangeAttemptReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    bdb_CBKETCLinkKeyExchangeAttempt(pPtr->pReq->didSuccess);
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processBdbNwkDescFreeReq
 *
 * @brief       Process BDB Network Descriptor Free Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbNwkDescFreeReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbNwkDescFreeReq_t *pPtr = (zstackmsg_bdbNwkDescFreeReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    pPtr->hdr.status = bdb_nwkDescFree(pPtr->pReq->nodeDescToRemove);
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}


/**************************************************************************************************
 * @fn          processBdbFilterNwkDescCompleteReq
 *
 * @brief       Process BDB Filter Nwk Descriptor Complete Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbFilterNwkDescCompleteReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_genericReq_t *pPtr = (zstackmsg_genericReq_t *)pMsg;

  pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;

  bdb_tryNwkAssoc();

  return (TRUE);
}

#endif

#ifdef BDB_TL_TARGET
/**************************************************************************************************
 * @fn          processBdbTouchLinkTargetEnableCommissioningReq
 *
 * @brief       Process BDB Touchlink Target Enable Commissioning Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbTouchLinkTargetEnableCommissioningReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbTouchLinkTargetEnableCommissioningReq_t *pPtr = (zstackmsg_bdbTouchLinkTargetEnableCommissioningReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    touchLinkTarget_EnableCommissioning(pPtr->pReq->timeoutTime);
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processBdbTouchLinkTargetDisableCommissioningReq
 *
 * @brief       Process BDB TouchlLink Target Disable Commissioning Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbTouchLinkTargetDisableCommissioningReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_genericReq_t *pPtr = (zstackmsg_genericReq_t *)pMsg;

  pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;

  touchLinkTarget_DisableCommissioning();

  (void)pMsg;
  return (TRUE);
}

/**************************************************************************************************
 * @fn          processBdbTouchLinkTargetGetTimerReq
 *
 * @brief       Process BDB Touchlink Target Get Timer Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbTouchLinkTargetGetTimerReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbTouchLinkTargetGetTimerReq_t *pPtr = (zstackmsg_bdbTouchLinkTargetGetTimerReq_t *)pMsg;

  if ( pPtr->pRsp )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    pPtr->pRsp->Time = touchLinkTarget_GetTimer();
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}



/**************************************************************************************************
 * @fn          processBdbTouchlinkSetAllowStealingReq
 *
 * @brief       Process BDB Touchlink Set Allow Stealing Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbTouchlinkSetAllowStealingReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbTouchlinkSetAllowStealingReq_t *pPtr = (zstackmsg_bdbTouchlinkSetAllowStealingReq_t *)pMsg;

  if ( pPtr->pReq )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    bdb_TouchlinkSetAllowStealing(pPtr->pReq->allowStealing);
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processBdbTouchlinkGetAllowStealingReq
 *
 * @brief       Process BDB Touchlink Get Allow Stealing Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbTouchlinkGetAllowStealingReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbTouchlinkGetAllowStealingReq_t *pPtr = (zstackmsg_bdbTouchlinkGetAllowStealingReq_t *)pMsg;

  if ( pPtr->pRsp )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    pPtr->pRsp->allowStealing = bdb_TouchlinkGetAllowStealing();
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif

#if (ZG_BUILD_JOINING_TYPE)
/**************************************************************************************************
 * @fn          processBdbRecoverNwkReq
 *
 * @brief       Process BDB Recover Network Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processBdbRecoverNwkReq( uint8_t srcServiceTaskId, void *pMsg )
{
  zstackmsg_bdbRecoverNwkReq_t *pPtr = (zstackmsg_bdbRecoverNwkReq_t *)pMsg;

  if ( pPtr->pRsp )
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
    pPtr->pRsp->status = bdb_recoverNwk();
  }
  else
  {
    pPtr->hdr.status = zstack_ZStatusValues_ZInvalidParameter;
  }

  return (TRUE);
}
#endif


#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
/**************************************************************************************************
 * @fn          processGpAllowChannelChangeReq
 *
 * @brief       Process GreenPower Allow Channel Change Request
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processGpAllowChannelChangeReq(uint8_t srcServiceTaskId, void *pMsg)
{
  zstackmsg_gpAllowChangeChannelReq_t *pPtr = (zstackmsg_gpAllowChangeChannelReq_t *)pMsg;

  pPtr->hdr.status = zstack_ZStatusValues_ZSuccess;
  gpApplicationAllowChannelChange = pPtr->AllowChannelChange;

  return (TRUE);
}
#endif

/**************************************************************************************************
 * @fn          isDevicePartOfNetwork
 *
 * @brief       Checks to see if the device has already joined a network
 *              by looking at _NIB.nwkState === NWK_INIT, then checks
 *              what is stored in NV's NIB.
 *
 * @param       none
 *
 * @return      TRUE if the device is already part of a network, FALSE if not
 */
static bool isDevicePartOfNetwork( void )
{
  bool ret = FALSE;
  if ( (_NIB.nwkState == NWK_ENDDEVICE) || (_NIB.nwkState == NWK_ROUTER) )
  {
    ret = TRUE;
  }
  else if ( _NIB.nwkState == NWK_INIT )
  {
    // Could be that the device hasn't started yet, so check in NV for a NIB
    if ( osal_nv_item_len( ZCD_NV_NIB ) == sizeof(nwkIB_t) )
    {
      nwkIB_t tempNIB;
      if ( osal_nv_read( ZCD_NV_NIB, 0, sizeof(nwkIB_t), &tempNIB )
            == ZSUCCESS )
      {
        if ( (tempNIB.nwkState == NWK_ENDDEVICE)
              || (tempNIB.nwkState == NWK_ROUTER) )
        {
          ret = TRUE;
        }
      }
    }
  }

  return (ret);
}

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
/**************************************************************************************************
 * @fn          gp_CommissioningModeInd
 *
 * @brief       Send a message to the Green Power Endpoint to notify that the device has enter/exit
 *              into Green Power commissioning mode
 *
 * @param       isEntering - TRUE if entering commissioning, FALSE otherwise
 *
 * @return      none
 */
static void gp_CommissioningModeInd(bool isEntering, bool hasTime, uint16_t time)
{
    zstackmsg_gpCommissioningModeInd_t *pCommissioningMode;
    epItem_t *pItem;

    pCommissioningMode = (zstackmsg_gpCommissioningModeInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_gpCommissioningModeInd_t) );
    if ( pCommissioningMode == NULL )
    {
      // Ignore the message
      return;
    }

    memset( pCommissioningMode, 0, sizeof(zstackmsg_gpCommissioningModeInd_t) );

    pCommissioningMode->hdr.event = zstackmsg_CmdIDs_GP_COMMISSIONING_MODE_IND;
    pCommissioningMode->hdr.status = zstack_ZStatusValues_ZSuccess;

    pCommissioningMode->Req.isEntering = isEntering;
    pCommissioningMode->Req.hasTime = hasTime;
    pCommissioningMode->Req.time = time;

    pItem = epTableFindEntryEP( GREEN_POWER_INTERNAL_ENDPOINT );

    if(pItem)
    {
	    // Send to a subscriber
	    OsalPort_msgSend( pItem->connection, (uint8_t*)pCommissioningMode );
    }
    else
    {
        OsalPort_msgDeallocate((uint8_t*)pCommissioningMode);
    }

}
#endif


#if (ZG_BUILD_JOINING_TYPE)
#ifndef ZNP_NPI
/**************************************************************************************************
 * @fn          BDBFilterNwkDescriptorsendMsgCB
 *
 * @brief       Send a message to the first Endpoint registered with the networks discovered to be
 *              filtered by the application
 *
 * @param       pBDBListNwk - List of network descriptors of the networks discovered
 * @param       count - Number of network descriptors
 *
 * @return      none
 */
static void BDBFilterNwkDescriptorsendMsgCB(networkDesc_t *pBDBListNwk, uint8_t count)
{
  epItem_t *srch;
  zstackmsg_bdbFilterNwkDescriptorInd_t *pRsp;

  pRsp = (zstackmsg_bdbFilterNwkDescriptorInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_bdbFilterNwkDescriptorInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_bdbFilterNwkDescriptorInd_t) );

  srch = pEpTableHdr;

  pRsp->hdr.event = zstackmsg_CmdIDs_BDB_FILTER_NWK_DESCRIPTOR_IND;
  pRsp->hdr.status = zstack_ZStatusValues_ZSuccess;

  pRsp->bdbFilterNetworkDesc.count = count;
  pRsp->bdbFilterNetworkDesc.pBDBListNwk = pBDBListNwk;

  if ( srch )
  {
    //Only the first application connection will receive this notification
    // Send to a subscriber
    OsalPort_msgSend( srch->connection, (uint8_t*)pRsp );
  }
  else
  {
      OsalPort_msgDeallocate((uint8_t *)pRsp);
  }

}

/**************************************************************************************************
 * @fn          BDBCBKETCLinkKeyExchangesendMsgCB
 *
 * @brief       Send a message to the first Endpoint registered notifying that the TCLK exchange
 *              is in progress.
 *              Application must choose between its own implementation of TCLK exchange (need
 *              to be implemented by application developer) or use Z3.0 TCLK exchange procedure by calling
 *              Zstackapi_bdbCBKETCLinkKeyExchangeAttemptReq with status failure.
 *
 * @param       none
 *
 * @return      none
 */
static void BDBCBKETCLinkKeyExchangesendMsgCB(void)
{
  epItem_t *srch;
  zstackmsg_bdbCBKETCLinkKeyExchangeInd_t *pRsp;

  pRsp = (zstackmsg_bdbCBKETCLinkKeyExchangeInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_bdbCBKETCLinkKeyExchangeInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_bdbCBKETCLinkKeyExchangeInd_t) );

  srch = pEpTableHdr;

  pRsp->hdr.event = zstackmsg_CmdIDs_BDB_CBKE_TC_LINK_KEY_EXCHANGE_IND;
  pRsp->hdr.status = zstack_ZStatusValues_ZSuccess;

  if ( srch )
  {
    //Only the first application connection will receive this notification
    // Send to a subscriber
    OsalPort_msgSend( srch->connection, (uint8_t*)pRsp );
  }
  else
  {
      OsalPort_msgDeallocate((uint8_t *)pRsp);
  }
}
#endif //#ifndef ZNP_NPI
#endif

#if defined ( BDB_TL_TARGET )
static void BDBTouchLinkTargetEnablesendMsgToAllCB(uint8_t enable)
{
  epItem_t *srch;
  zstackmsg_bdbTouchLinkTargetEnableInd_t *pRsp;

  pRsp = (zstackmsg_bdbTouchLinkTargetEnableInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_bdbTouchLinkTargetEnableInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_bdbTouchLinkTargetEnableInd_t) );

  srch = pEpTableHdr;

  pRsp->hdr.event = zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_ENABLE_IND;
  pRsp->hdr.status = zstack_ZStatusValues_ZSuccess;

  pRsp->Enable = enable;

  if ( srch )
  {
    //Only the first application connection will receive this notification
    // Send to a subscriber
    OsalPort_msgSend( srch->connection, (uint8_t*)pRsp );
  }
  else
  {
      OsalPort_msgDeallocate((uint8_t *)pRsp);
  }
}
#endif



#if (ZG_BUILD_COORDINATOR_TYPE)
static void BDBTCLinkKeyExchangeNotificationsendMsgToAllCB(bdb_TCLinkKeyExchProcess_t *bdb_TCLinkKeyExchProcess)
{
  epItem_t *srch;
  zstackmsg_bdbTCLinkKeyExchangeInd_t *pRsp;

  pRsp = (zstackmsg_bdbTCLinkKeyExchangeInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_bdbTCLinkKeyExchangeInd_t) );
  if ( pRsp == NULL )
  {
    // Ignore the message
    return;
  }

  memset( pRsp, 0, sizeof(zstackmsg_bdbTCLinkKeyExchangeInd_t) );

  srch = pEpTableHdr;

  pRsp->hdr.event = zstackmsg_CmdIDs_BDB_TC_LINK_KEY_EXCHANGE_NOTIFICATION_IND;
  pRsp->hdr.status = zstack_ZStatusValues_ZSuccess;

  OsalPort_memcpy(pRsp->Req.extAddr, bdb_TCLinkKeyExchProcess->extAddr, Z_EXTADDR_LEN);
  pRsp->Req.status = bdb_TCLinkKeyExchProcess->status;
  pRsp->Req.nwkAddr = bdb_TCLinkKeyExchProcess->nwkAddr;

  if ( srch )
  {
    //Only the first application connection will receive this notification
    // Send to a subscriber
    OsalPort_msgSend( srch->connection, (uint8_t*)pRsp );
  }
  else
  {
      OsalPort_msgDeallocate((uint8_t *)pRsp);
  }
}
#endif

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED == 1)
static void BDBBindNotificationsendMsgToAllCBs(bdbBindNotificationData_t *bindData)
{
    epItem_t *srch;
    zstackmsg_bdbBindNotificationInd_t *pRsp;

    pRsp = (zstackmsg_bdbBindNotificationInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_bdbBindNotificationInd_t) );
    if ( pRsp == NULL )
    {
      // Ignore the message
      return;
    }

    memset( pRsp, 0, sizeof(zstackmsg_bdbBindNotificationInd_t) );

    srch = pEpTableHdr;

    pRsp->hdr.event = zstackmsg_CmdIDs_BDB_BIND_NOTIFICATION_CB;
    pRsp->hdr.status = zstack_ZStatusValues_ZSuccess;
    pRsp->Req.clusterId = bindData->clusterId;
    pRsp->Req.ep = bindData->ep;

    OsalPort_memcpy(&pRsp->Req.dstAddr,&bindData->dstAddr,sizeof(zAddrType_t));

    if ( srch )
    {
        //Only the first application connection will receive this notification
        // Send to a subscriber
        OsalPort_msgSend( srch->connection, (uint8_t*)pRsp );
    }
    else
    {
        OsalPort_msgDeallocate((uint8_t *)pRsp);
    }
}

static void BDBIdentifyTimesendMsgToAllCBs(uint8_t endpoint)
{
    epItem_t *srch;
    zstackmsg_bdbIdentifyTimeoutInd_t *pRsp;

    pRsp = (zstackmsg_bdbIdentifyTimeoutInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_bdbIdentifyTimeoutInd_t) );
    if ( pRsp == NULL )
    {
      // Ignore the message
      return;
    }

    memset( pRsp, 0, sizeof(zstackmsg_bdbIdentifyTimeoutInd_t) );

    srch = pEpTableHdr;

    pRsp->hdr.event = zstackmsg_CmdIDs_BDB_IDENTIFY_TIME_CB;
    pRsp->hdr.status = zstack_ZStatusValues_ZSuccess;
    pRsp->EndPoint = endpoint;

    if ( srch )
    {
        //Only the first application connection will receive this notification
        // Send to a subscriber
        OsalPort_msgSend( srch->connection, (uint8_t*)pRsp );
    }
    else
    {
        OsalPort_msgDeallocate((uint8_t *)pRsp);
    }
}
#endif

static void BDBsendMsgToAllCBs(bdbCommissioningModeMsg_t* bdbCommissioningModeMsg)
{
    epItem_t *srch;
    zstackmsg_bdbNotificationInd_t *pRsp;

    if(bdbCommissioningModeMsg == NULL)
    {
        return;
    }
    pRsp = (zstackmsg_bdbNotificationInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_bdbNotificationInd_t) );
    if ( pRsp == NULL )
    {
      // Ignore the message
      return;
    }

    memset( pRsp, 0, sizeof(zstackmsg_bdbNotificationInd_t) );

    srch = pEpTableHdr;

    pRsp->hdr.event = zstackmsg_CmdIDs_BDB_NOTIFICATION;
    pRsp->hdr.status = zstack_ZStatusValues_ZSuccess;
    pRsp->Req.bdbCommissioningStatus = bdbCommissioningModeMsg->bdbCommissioningStatus;
    pRsp->Req.bdbCommissioningMode = bdbCommissioningModeMsg->bdbCommissioningMode;
    pRsp->Req.bdbRemainingCommissioningModes = bdbCommissioningModeMsg->bdbRemainingCommissioningModes;

    if ( srch )
    {
      //Only the first application connection will receive this notification
      // Send to a subscriber
      OsalPort_msgSend( srch->connection, (uint8_t*)pRsp );
    }
    else
    {
        OsalPort_msgDeallocate((uint8_t *)pRsp);
    }
}
#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)

/**************************************************************************************************
 * @fn          gpDataIndMsgSend
 *
 * @brief       Send data indication from Green Power Stub to application
 *
 * @param       gpDataInd - Data Indication struct
 *
 * @return
 */
static void gpDataIndMsgSend(gp_DataInd_t* gpDataInd)
{
    zstackmsg_gpDataInd_t *pMsg;
    epItem_t *pItem;

    if(gpDataInd == NULL)
    {
        return;
    }

    pMsg = (zstackmsg_gpDataInd_t *)OsalPort_msgAllocate( sizeof(zstackmsg_gpDataInd_t) + gpDataInd->GPDasduLength - 1 );
    if ( pMsg != NULL )
    {
        memset( pMsg, 0, sizeof(zstackmsg_gpDataInd_t) + gpDataInd->GPDasduLength - 1 );

        pMsg->hdr.event = zstackmsg_CmdIDs_GP_DATA_IND;
        pMsg->hdr.status = zstack_ZStatusValues_ZSuccess;
        pMsg->Req.SecReqHandling = gpDataInd->SecReqHandling;
        pMsg->Req.timestamp = gpDataInd->timestamp;
        pMsg->Req.status = gpDataInd->status;
        pMsg->Req.LinkQuality = gpDataInd->LinkQuality;
        pMsg->Req.SeqNumber = gpDataInd->SeqNumber;
        pMsg->Req.srcAddr = gpDataInd->srcAddr;
        pMsg->Req.srcPanID = gpDataInd->srcPanID;
        pMsg->Req.appID = gpDataInd->appID;
        pMsg->Req.GPDFSecLvl = gpDataInd->GPDFSecLvl;
        pMsg->Req.GPDFKeyType = gpDataInd->GPDFKeyType;
        pMsg->Req.AutoCommissioning = gpDataInd->AutoCommissioning;
        pMsg->Req.RxAfterTx = gpDataInd->RxAfterTx;
        pMsg->Req.SrcId = gpDataInd->SrcId;
        pMsg->Req.EndPoint = gpDataInd->EndPoint;
        pMsg->Req.GPDSecFrameCounter = gpDataInd->GPDSecFrameCounter;
        pMsg->Req.GPDCmmdID = gpDataInd->GPDCmmdID;
        pMsg->Req.MIC = gpDataInd->MIC;
        pMsg->Req.GPDasduLength = gpDataInd->GPDasduLength;
        OsalPort_memcpy(pMsg->Req.GPDasdu, gpDataInd->GPDasdu, gpDataInd->GPDasduLength);

        pItem = epTableFindEntryEP( GREEN_POWER_INTERNAL_ENDPOINT );
        if ( pItem )
        {
            // Send to a subscriber
            OsalPort_msgSend( pItem->connection, (uint8_t*)pMsg );
        }
        else
        {
            OsalPort_msgDeallocate((uint8_t*)pMsg);
        }
    }
}

/**************************************************************************************************
 * @fn          gpSecRecMsgSend
 *
 * @brief       Send Security Request from Green Power Stub to application
 *
 * @param       gpSecReq - Security Request struct
 *
 * @return
 */
static void gpSecRecMsgSend(gp_SecReq_t* gpSecReq)
{
    zstackmsg_gpSecReq_t *pMsg;
    epItem_t *pItem;

    if(gpSecReq == NULL)
    {
        return;
    }

    pMsg = (zstackmsg_gpSecReq_t *)OsalPort_msgAllocate( sizeof(zstackmsg_gpSecReq_t) );
    if ( pMsg != NULL )
    {
        memset( pMsg, 0, sizeof(zstackmsg_gpSecReq_t) );

        pMsg->hdr.event = zstackmsg_CmdIDs_GP_SECURITY_REQ;
        pMsg->hdr.status = zstack_ZStatusValues_ZSuccess;
        pMsg->Req.gpdID = gpSecReq->gpdID;
        pMsg->Req.EndPoint = gpSecReq->EndPoint;
        pMsg->Req.gp_SecData = gpSecReq->gp_SecData;
        pMsg->Req.dGPStubHandle = gpSecReq->dGPStubHandle;

        pItem = epTableFindEntryEP( GREEN_POWER_INTERNAL_ENDPOINT );
        if ( pItem )
        {
            // Send to a subscriber
            OsalPort_msgSend( pItem->connection, (uint8_t*)pMsg );
        }
        else
        {
            OsalPort_msgDeallocate((uint8_t*)pMsg);
        }
    }
}

/**************************************************************************************************
 * @fn          processGpSecRsp
 *
 * @brief       Process GreenPower Security Response
 *
 * @param       srcServiceTaskId - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processGpSecRsp(uint8_t srcServiceTaskId, void *pMsg)
{
  zstackmsg_gpSecRsp_t *pPtr = (zstackmsg_gpSecRsp_t *)pMsg;
  gp_SecRsp_t rsp;

  if(pPtr->pReq != NULL)
  {
      rsp.hdr.event = GP_SEC_RSP;
      rsp.hdr.status = ZSuccess;
      rsp.Status = pPtr->pReq->Status;
      rsp.dGPStubHandle = pPtr->pReq->dGPStubHandle;
      rsp.gpdID = pPtr->pReq->gpdID;
      rsp.EndPoint = pPtr->pReq->EndPoint;
      rsp.gp_SecData = pPtr->pReq->gp_SecData;
      OsalPort_memcpy(rsp.GPDKey, pPtr->pReq->GPDKey, SEC_KEY_LEN);

      sendGpSecRsp(&rsp);
  }
  return (TRUE);
}

/**************************************************************************************************
 * @fn          gpCheckAnnceMsgSend
 *
 * @brief       Send message to check device announce with proxy table for address conflict resolution
 *
 * @param       ieee - IEEE address of anounced device
 * @param       nwkAddr - Network address of announced device
 *
 * @return
 */
static void gpCheckAnnceMsgSend(uint8_t *ieee, uint16_t nwkAddr)
{
    zstackmsg_gpCheckAnnounce_t *pMsg;
    epItem_t *pItem;

    pMsg = (zstackmsg_gpCheckAnnounce_t *)OsalPort_msgAllocate( sizeof(zstackmsg_gpCheckAnnounce_t) );
    if ( pMsg != NULL )
    {
        memset( pMsg, 0, sizeof(zstackmsg_gpCheckAnnounce_t) );

        pMsg->hdr.event = zstackmsg_CmdIDs_GP_CHECK_ANNCE;
        pMsg->hdr.status = zstack_ZStatusValues_ZSuccess;
        pMsg->Req.nwkAddr = nwkAddr;
        OsalPort_memcpy(pMsg->Req.extAddr, ieee, Z_EXTADDR_LEN);

        pItem = epTableFindEntryEP( 0xF2 );
        if ( pItem )
        {
            // Send to a subscriber
            OsalPort_msgSend( pItem->connection, (uint8_t*)pMsg );
        }
        else
        {
            OsalPort_msgDeallocate((uint8_t*)pMsg);
        }
    }
}

/**************************************************************************************************
 *
 * @fn          processGpAddressConfict
 *
 * @brief       Process GreenPower Address Conflict with proxy table
 *
 * @param       srcServiceTaskId - Source Task ID
 *
 * @return      TRUE to send the response back
 */
static bool processGpAddressConfict(uint8_t srcServiceTaskId, void *pMsg)
{
  zstackmsg_gpAliasConflict_t *pPtr = (zstackmsg_gpAliasConflict_t *)pMsg;

  if(pPtr->pReq != NULL)
  {
     GpResolveAddressConflict( pPtr->pReq->nwkAddr, pPtr->pReq->conflictResolution );
  }

  return (TRUE);
}

#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
/**************************************************************************************************
 * @fn          processGpDecryptDataInd
 *
 * @brief       Process GreenPower Decrypt data indication payload
 *
 * @param       srcEntityID - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processGpDecryptDataInd(uint8_t srcServiceTaskId, void *pMsg)
{
    zstackmsg_gpDecryptDataInd_t *pPtr = (zstackmsg_gpDecryptDataInd_t *)pMsg;

    if( (pPtr->pReq != NULL) && (pPtr->pRsp != NULL) )
    {
        if(gp_ccmStar(pPtr->pReq->pGpDataInd, pPtr->pReq->key) == SUCCESS)
        {
            pPtr->pRsp->pGpDataInd = pPtr->pReq->pGpDataInd;
            pPtr->pRsp->status = SUCCESS;
        }
        else
        {
            pPtr->pRsp->status = FAILURE;
        }
    }

    return (TRUE);
}

/**************************************************************************************************
 * @fn          processGpEncryptDecryptCommissioningKey
 *
 * @brief       Process GreenPower Decrypt Commissioning key
 *
 * @param       srcEntityID - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processGpEncryptDecryptCommissioningKey(uint8_t srcServiceTaskId, void *pMsg)
{
    zstackmsg_gpEncryptDecryptCommissioningKey_t *pPtr = (zstackmsg_gpEncryptDecryptCommissioningKey_t *)pMsg;

    if( (pPtr->pReq != NULL) && (pPtr->pRsp != NULL) )
    {
        pPtr->pRsp->status = gp_ccmStarKey(&pPtr->pReq->gpdId, pPtr->pReq->keyType, pPtr->pReq->key,
                                          (uint8_t*)&pPtr->pReq->keyMic, pPtr->pReq->secKey,
                                          pPtr->pReq->secFrameCounter, pPtr->pReq->encrypt);
        OsalPort_memcpy(pPtr->pRsp->key, pPtr->pReq->key, SEC_KEY_LEN);
        pPtr->pRsp->secFrameCounter = pPtr->pReq->secFrameCounter + 1;
    }

    return (TRUE);
}

/**************************************************************************************************
 * @fn          processGpCommissioningSuccess
 *
 * @brief       Process GreenPower Commissioning success
 *
 * @param       srcEntityID - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processGpCommissioningSuccess(uint8_t srcServiceTaskId, void *pMsg)
{
    zstackmsg_gpCommissioningSuccess_t *pPtr = (zstackmsg_gpCommissioningSuccess_t *)pMsg;
    epItem_t *pItem;


    if( pPtr->pReq != NULL )
    {
        uint8_t ieeeAddr[Z_EXTADDR_LEN];

        //Send device announce
        memset(ieeeAddr, 0xFF, Z_EXTADDR_LEN);
        ZDP_DeviceAnnce( pPtr->pReq->alias, ieeeAddr, 0x00, FALSE );

        pItem = epTableFindEntryEP( 0xF2 );
        if ( pItem )
        {
            // Send to a subscriber
            (void)OsalPort_msgSend( pItem->connection, (uint8_t *)pMsg );
        }
    }

  return (TRUE);
}

/**************************************************************************************************
 * @fn          processGpSendDeviceAnnounce
 *
 * @brief       Process GreenPower Commissioning success
 *
 * @param       srcEntityID - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return      TRUE to send the response back
 */
static bool processGpSendDeviceAnnounce(uint8_t srcEntityID, void *pMsg)
{
    zstackmsg_gpSendDeviceAnnounce_t *pPtr = (zstackmsg_gpSendDeviceAnnounce_t *)pMsg;

    if( pPtr->pReq != NULL )
    {
        uint8_t ieeeAddr[Z_EXTADDR_LEN];

        //Send device announce
        memset(ieeeAddr, 0xFF, Z_EXTADDR_LEN);
        ZDP_DeviceAnnce( pPtr->pReq->alias, ieeeAddr, 0x00, FALSE );
    }

    return (TRUE);
}
#endif  // #if defined (ENABLE_GREENPOWER_COMBO_BASIC)

#endif // #if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)

/**************************************************************************************************
 * @fn          processPauseResumeDeviceReq
 *
 * @brief       Send Pause/Resume Device Req
 *
 * @param       srcEntityID - Source Task ID
 * @param       pMsg - pointer to message
 *
 * @return
 */
static bool processPauseResumeDeviceReq(uint8_t srcServiceTaskId, void *pMsg)
{
    zstackmsg_pauseResumeDeviceReq_t *pReq = (zstackmsg_pauseResumeDeviceReq_t*) pMsg;

    if (pReq->pReq->pause) {
        ZDApp_PauseNwk();
    } else {
        ZDApp_ResumeNwk();
    }
    return (TRUE);
}

#endif // ZNP_NPI

uint8_t ZStackTask_getServiceTaskID(void)
{
    return ZStackServiceTaskId;
}
