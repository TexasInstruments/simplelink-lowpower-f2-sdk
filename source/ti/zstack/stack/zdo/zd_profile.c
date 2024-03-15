/**************************************************************************************************
  Filename:       ZDProfile.c
  Revised:        $Date: 2015-10-14 11:48:06 -0700 (Wed, 14 Oct 2015) $
  Revision:       $Revision: 44530 $

  Description:    This is the Zigbee Device Profile.


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

/*********************************************************************
 * INCLUDES
 */
#include "zcomdef.h"
#include "rom_jt_154.h"
#include "af.h"
#include "nl_mede.h"
#include "nwk_util.h"
#include "aps.h"

#include "addr_mgr.h"
#include "zd_config.h"
#include "zd_profile.h"
#include "zd_object.h"
#include "zd_nwk_mgr.h"

#if defined( LCD_SUPPORTED )

#endif

#include "nwk_util.h"

#if defined( MT_ZDO_FUNC )
  #include "mt_zdo.h"
#endif

/*********************************************************************
 * MACROS
 */

#define ZADDR_TO_AFADDR( pZADDR, AFADDR ) {                            \
  (AFADDR).endPoint = ZDP_AF_ENDPOINT;                                 \
  (AFADDR).addrMode = (afAddrMode_t)(pZADDR)->addrMode;                \
  (AFADDR).addr.shortAddr = (pZADDR)->addr.shortAddr;                  \
}

#define FillAndSendBuffer( TRANSSEQ, ADDR, ID, LEN, BUF ) {     \
  afStatus_t stat;                                    \
  ZDP_TmpBuf = (BUF)+1;                               \
  stat = fillAndSend( (TRANSSEQ), (ADDR), (ID), (LEN) );          \
  OsalPort_free( (BUF) );                             \
  ZDP_TmpBuf = ZDP_Buf+1;                             \
  return stat;                                        \
}

#define FillAndSendTxOptions( TRANSSEQ, ADDR, ID, LEN, TxO ) {  \
  afStatus_t stat;                                    \
  ZDP_TxOptions = (TxO);                              \
  stat = fillAndSend( (TRANSSEQ), (ADDR), (ID), (LEN) );          \
  ZDP_TxOptions = AF_TX_OPTIONS_NONE;                 \
  return stat;                                        \
}

#define FillAndSendBufferTxOptions( TRANSSEQ, ADDR, ID, LEN, BUF, TxO ) { \
  afStatus_t stat;                                    \
  ZDP_TmpBuf = (BUF)+1;                               \
  ZDP_TxOptions = (TxO);                              \
  stat = fillAndSend( (TRANSSEQ), (ADDR), (ID), (LEN) );          \
  OsalPort_free( (BUF) );                             \
  ZDP_TmpBuf = ZDP_Buf+1;                             \
  ZDP_TxOptions = AF_TX_OPTIONS_NONE;                 \
  return stat;                                        \
}

/*********************************************************************
 * CONSTANTS
 */

#define ZDP_BUF_SZ          82

CONST byte ZDP_AF_ENDPOINT = 0;

// Routing table options
#define ZP_RTG_RECORD       0x04       // Routing table indication that a route record is needed
#define ZP_MTO_ROUTE_RC     0x08       // Concentrator with routing cache
#define ZP_MTO_ROUTE_NRC    0x10       // Concentrator with limited cache


/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
  void *next;
  uint8_t taskID;
  uint16_t clusterID;
} ZDO_MsgCB_t;


/*********************************************************************
 * GLOBAL VARIABLES
 */

byte ZDP_SeqNum = 0;
uint8_t childIndex = 0;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

extern endPointDesc_t ZDApp_epDesc;

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
extern void ZDApp_SetParentAnnceTimer( void );

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static afStatus_t fillAndSend( uint8_t *transSeq, zAddrType_t *addr, cId_t clusterID, byte len );
uint8_t ZDO_SendMsgCBs( zdoIncomingMsg_t *inMsg );
void zdpProcessAddrReq( zdoIncomingMsg_t *inMsg );

/*********************************************************************
 * LOCAL VARIABLES
 */

static uint8_t  ZDP_Buf[ ZDP_BUF_SZ ];
static uint8_t *ZDP_TmpBuf = ZDP_Buf+1;
static uint8_t ZDP_TransID = 0;

byte ZDP_TxOptions = AF_TX_OPTIONS_NONE;
ZDO_MsgCB_t *zdoMsgCBs = (ZDO_MsgCB_t *)NULL;

/*********************************************************************
 * ZDO Message Processing table
 */

typedef void (*pfnZDPMsgProcessor)( zdoIncomingMsg_t *inMsg );

typedef struct
{
  uint16_t                clusterID;
  pfnZDPMsgProcessor    pFn;
} zdpMsgProcItem_t;

CONST zdpMsgProcItem_t zdpMsgProcs[] =
{
#if ( RFD_RX_ALWAYS_ON_CAPABLE == TRUE ) || ( ZG_BUILD_RTR_TYPE )
  // These aren't processed by sleeping end devices.
  { Device_annce,           ZDO_ProcessDeviceAnnce },
#endif
#if ( ZG_BUILD_RTR_TYPE )
  // These aren't processed by end devices.
  { Parent_annce,           ZDO_ProcessParentAnnce },
  { Parent_annce_rsp,       ZDO_ProcessParentAnnceRsp },
#endif
  { NWK_addr_req,           zdpProcessAddrReq },
  { IEEE_addr_req,          zdpProcessAddrReq },
  { Node_Desc_req,          ZDO_ProcessNodeDescReq },
  { Node_Desc_rsp,          ZDO_ProcessNodeDescRsp },
  { Power_Desc_req,         ZDO_ProcessPowerDescReq },
  { Simple_Desc_req,        ZDO_ProcessSimpleDescReq },
  { Simple_Desc_rsp,        ZDO_ProcessSimpleDescRsp },
  { Active_EP_req,          ZDO_ProcessActiveEPReq },
  { Match_Desc_req,         ZDO_ProcessMatchDescReq },
#if defined ( ZDO_MGMT_NWKDISC_RESPONSE )
  { Mgmt_NWK_Disc_req,      ZDO_ProcessMgmtNwkDiscReq },
#endif
#if defined ( ZDO_MGMT_LQI_RESPONSE ) && ( ZG_BUILD_RTR_TYPE || ZG_BUILD_ENDDEVICE_TYPE )
  { Mgmt_Lqi_req,           ZDO_ProcessMgmtLqiReq },
#endif
#if defined ( ZDO_MGMT_RTG_RESPONSE ) && ( ZG_BUILD_RTR_TYPE )
  { Mgmt_Rtg_req,           ZDO_ProcessMgmtRtgReq },
#endif
#if defined ( ZDO_MGMT_BIND_RESPONSE )
  { Mgmt_Bind_req,          ZDO_ProcessMgmtBindReq },
#endif
#if defined ( ZDO_MGMT_JOINDIRECT_RESPONSE ) && ( ZG_BUILD_RTR_TYPE )
#endif
#if defined ( ZDO_MGMT_LEAVE_RESPONSE )
  { Mgmt_Leave_req,         ZDO_ProcessMgmtLeaveReq },
#endif
#if defined ( ZDO_MGMT_PERMIT_JOIN_RESPONSE )  && ( ZG_BUILD_RTR_TYPE )
  { Mgmt_Permit_Join_req,   ZDO_ProcessMgmtPermitJoinReq },
#endif
#if defined ( ZDO_USERDESC_RESPONSE )
  { User_Desc_req,          ZDO_ProcessUserDescReq },
#endif
#if defined ( ZDO_USERDESCSET_RESPONSE )
  { User_Desc_set,          ZDO_ProcessUserDescSet },
#endif
#if defined ( ZDO_SERVERDISC_RESPONSE )
  { Server_Discovery_req,   ZDO_ProcessServerDiscReq },
#endif
  {0xFFFF, NULL} // Last
};

/*********************************************************************
 * @fn          fillAndSend
 *
 * @brief       Combined to reduce space
 *
 * @param
 * @param
 *
 * @return      afStatus_t
 */
static afStatus_t fillAndSend( uint8_t *transSeq, zAddrType_t *addr, cId_t clusterID, byte len )
{
  afAddrType_t afAddr;
  afStatus_t status = afStatus_FAILED;

  memset( &afAddr, 0, sizeof(afAddrType_t) );
  ZADDR_TO_AFADDR( addr, afAddr );

  *(ZDP_TmpBuf-1) = *transSeq;

  status = AF_DataRequest( &afAddr, &ZDApp_epDesc, clusterID,
                           (uint16_t)(len+1), (uint8_t*)(ZDP_TmpBuf-1),
                           &ZDP_TransID, ZDP_TxOptions,  AF_DEFAULT_RADIUS );

  if ( status == afStatus_SUCCESS )
  {
    (*transSeq)++;
  }

  return status;
}

/*********************************************************************
 * @fn          ZDP_SendData
 *
 * @brief       This builds and send a request message that has
 *              NWKAddrOfInterest as its only parameter.
 *
 * @param       dstAddr - destination address
 * @param       cmd - clusterID
 * @param       dataLen - number of bytes of data
 * @param       data - pointer to the data
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_SendData( uint8_t *TransSeq, zAddrType_t *dstAddr, uint16_t cmd,
                        byte len, uint8_t *buf, byte SecurityEnable )
{
  uint8_t *pBuf = ZDP_TmpBuf;
  byte cnt = len;

  while ( cnt-- )
  {
    *pBuf++ = *buf++;
  }

  FillAndSendTxOptions( TransSeq, dstAddr, cmd, len, ((SecurityEnable) ? AF_EN_SECURITY : 0) );
}

/*********************************************************************
 * @fn          ZDP_NWKAddrOfInterestReq
 *
 * @brief       This builds and send a request message that has
 *              NWKAddrOfInterest as its only parameter.
 *
 * @param       dstAddr - destination address
 * @param       nwkAddr - 16 bit address
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_NWKAddrOfInterestReq( zAddrType_t *dstAddr, uint16_t nwkAddr,
                                     byte cmd, byte SecurityEnable )
{
  (void)SecurityEnable;  // Intentionally unreferenced parameter

  ZDP_TmpBuf[0] = LO_UINT16( nwkAddr );
  ZDP_TmpBuf[1] = HI_UINT16( nwkAddr );

  return fillAndSend( &ZDP_SeqNum, dstAddr, cmd, 2 );
}

/*********************************************************************
 * Address Requests
 */

/*********************************************************************
 * @fn          ZDP_NwkAddrReq
 *
 * @brief       This builds and send a NWK_addr_req message.  This
 *              function sends a broadcast message looking for a 16
 *              bit address with a 64 bit address as bait.
 *
 * @param       IEEEAddress - looking for this device
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_NwkAddrReq( uint8_t *IEEEAddress, byte ReqType,
                           byte StartIndex, byte SecurityEnable )
{
  uint8_t *pBuf = ZDP_TmpBuf;
  byte len = Z_EXTADDR_LEN + 1 + 1;  // IEEEAddress + ReqType + StartIndex.
  zAddrType_t dstAddr;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  if ( osal_ExtAddrEqual( saveExtAddr, IEEEAddress ) == FALSE )
  {
    dstAddr.addrMode = AddrBroadcast;
    dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVRXON;
  }
  else
  {
    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = ZDAppNwkAddr.addr.shortAddr;
  }

  pBuf = osal_cpyExtAddr( pBuf, IEEEAddress );

  *pBuf++ = ReqType;
  *pBuf++ = StartIndex;

  return fillAndSend( &ZDP_SeqNum, &dstAddr, NWK_addr_req, len );
}

/*********************************************************************
 * @fn          ZDP_IEEEAddrReq
 *
 * @brief       This builds and send a IEEE_addr_req message.  This
 *              function sends a unicast message looking for a 64
 *              bit IEEE address with a 16 bit address as bait.
 *
 * @param       ReqType - ZDP_IEEEADDR_REQTYPE_SINGLE or
 *                        ZDP_IEEEADDR_REQTYPE_EXTENDED
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_IEEEAddrReq( uint16_t shortAddr, byte ReqType,
                            byte StartIndex, byte SecurityEnable )
{
  uint8_t *pBuf = ZDP_TmpBuf;
  byte len = 2 + 1 + 1;  // shortAddr + ReqType + StartIndex.
  zAddrType_t dstAddr;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  dstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  dstAddr.addr.shortAddr = shortAddr;

  *pBuf++ = LO_UINT16( shortAddr );
  *pBuf++ = HI_UINT16( shortAddr );

  *pBuf++ = ReqType;
  *pBuf++ = StartIndex;

  return fillAndSend( &ZDP_SeqNum, &dstAddr, IEEE_addr_req, len );
}

/*********************************************************************
 * @fn          ZDP_MatchDescReq
 *
 * @brief       This builds and send a Match_Desc_req message.  This
 *              function sends a broadcast or unicast message
 *              requesting the list of endpoint/interfaces that
 *              match profile ID and cluster IDs.
 *
 * @param       dstAddr - destination address
 * @param       nwkAddr - network address of interest
 * @param       ProfileID - Profile ID
 * @param       NumInClusters - number of input clusters
 * @param       InClusterList - input cluster ID list
 * @param       NumOutClusters - number of output clusters
 * @param       OutClusterList - output cluster ID list
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_MatchDescReq( zAddrType_t *dstAddr, uint16_t nwkAddr,
                                uint16_t ProfileID,
                                byte NumInClusters, cId_t *InClusterList,
                                byte NumOutClusters, cId_t *OutClusterList,
                                byte SecurityEnable )
{
  uint8_t *pBuf = ZDP_TmpBuf;
  // nwkAddr+ProfileID+NumInClusters+NumOutClusters.
  byte i, len = 2 + 2 + 1 + 1;  // nwkAddr+ProfileID+NumInClusters+NumOutClusters.

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  len += (NumInClusters + NumOutClusters) * sizeof(uint16_t);

  if ( len >= ZDP_BUF_SZ-1 )
  {
    return afStatus_MEM_FAIL;
  }

  // The spec changed in Zigbee 2007 (2.4.3.1.7.1) to not allow sending
  // this command to 0xFFFF.  So, here we will filter this and replace
  // with 0xFFFD to only send to devices with RX ON.  This includes the
  // network address of interest.
  if ( ((dstAddr->addrMode == AddrBroadcast) || (dstAddr->addrMode == Addr16Bit))
      && (dstAddr->addr.shortAddr == NWK_BROADCAST_SHORTADDR_DEVALL) )
  {
    dstAddr->addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVRXON;
  }
  if ( nwkAddr == NWK_BROADCAST_SHORTADDR_DEVALL )
  {
    nwkAddr = NWK_BROADCAST_SHORTADDR_DEVRXON;
  }

  *pBuf++ = LO_UINT16( nwkAddr );   // NWKAddrOfInterest
  *pBuf++ = HI_UINT16( nwkAddr );

  *pBuf++ = LO_UINT16( ProfileID );   // Profile ID
  *pBuf++ = HI_UINT16( ProfileID );

  *pBuf++ = NumInClusters; // Input cluster list
  if ( NumInClusters )
  {
    for (i=0; i<NumInClusters; ++i)  {
      *pBuf++ = LO_UINT16( InClusterList[i] );
      *pBuf++ = HI_UINT16( InClusterList[i] );
    }
  }

  *pBuf++ = NumOutClusters; // Output cluster list
  if ( NumOutClusters )
  {
    for (i=0; i<NumOutClusters; ++i)  {
      *pBuf++ = LO_UINT16( OutClusterList[i] );
      *pBuf++ = HI_UINT16( OutClusterList[i] );
    }
  }

  return fillAndSend( &ZDP_SeqNum, dstAddr, Match_Desc_req, len );
}

/*********************************************************************
 * @fn          ZDP_SimpleDescReq
 *
 * @brief       This builds and send a NWK_Simple_Desc_req
 *              message.  This function sends unicast message to the
 *              destination device.
 *
 * @param       dstAddr - destination address
 * @param       nwkAddr - 16 bit address
 * @param       epIntf - endpoint/interface
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_SimpleDescReq( zAddrType_t *dstAddr, uint16_t nwkAddr,
                                    byte endPoint, byte SecurityEnable )

{
  (void)SecurityEnable;  // Intentionally unreferenced parameter

  ZDP_TmpBuf[0] = LO_UINT16( nwkAddr );
  ZDP_TmpBuf[1] = HI_UINT16( nwkAddr );
  ZDP_TmpBuf[2] = endPoint;

  return fillAndSend( &ZDP_SeqNum, dstAddr, Simple_Desc_req, 3 );
}

/*********************************************************************
 * @fn          ZDP_UserDescSet
 *
 * @brief       This builds and send a User_Desc_set message to set
 *              the user descriptor.  This function sends unicast
 *              message to the destination device.
 *
 * @param       dstAddr - destination address
 * @param       nwkAddr - 16 bit address
 * @param       UserDescriptor - user descriptor
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_UserDescSet( zAddrType_t *dstAddr, uint16_t nwkAddr,
                          UserDescriptorFormat_t *UserDescriptor,
                          byte SecurityEnable )
{
  uint8_t *pBuf = ZDP_TmpBuf;
  byte len = (UserDescriptor->len < AF_MAX_USER_DESCRIPTOR_LEN) ?
              UserDescriptor->len : AF_MAX_USER_DESCRIPTOR_LEN;
  byte addrLen = 2;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  *pBuf++ = LO_UINT16( nwkAddr );
  *pBuf++ = HI_UINT16( nwkAddr );

  *pBuf++ = len;
  addrLen = 3;

  memset( pBuf, AF_USER_DESCRIPTOR_FILL, AF_MAX_USER_DESCRIPTOR_LEN);

  OsalPort_memcpy( pBuf, UserDescriptor->desc, len );

  return fillAndSend( &ZDP_SeqNum, dstAddr, User_Desc_set, (AF_MAX_USER_DESCRIPTOR_LEN + addrLen) );
}

/*********************************************************************
 * @fn          ZDP_ServerDiscReq
 *
 * @brief       Build and send a Server_Discovery_req request message.
 *
 * @param       serverMask - 16-bit bit-mask of server services being sought.
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_ServerDiscReq( uint16_t serverMask, byte SecurityEnable )
{
  uint8_t *pBuf = ZDP_TmpBuf;
  zAddrType_t dstAddr;

  dstAddr.addrMode = AddrBroadcast;
  dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVRXON;

  *pBuf++ = LO_UINT16( serverMask );
  *pBuf = HI_UINT16( serverMask );

  FillAndSendTxOptions( &ZDP_SeqNum, &dstAddr, Server_Discovery_req, 2,
             ((SecurityEnable) ? AF_EN_SECURITY : AF_TX_OPTIONS_NONE) );
}

/*********************************************************************
 * @fn          ZDP_DeviceAnnce
 *
 * @brief       This builds and send a Device_Annce message.  This
 *              function sends a broadcast message.
 *
 * @param       nwkAddr - 16 bit address of the device
 * @param       IEEEAddr - 64 bit address of the device
 * @param       capabilities - device capabilities.  This field is only
 *                 sent for v1.1 networks.
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_DeviceAnnce( uint16_t nwkAddr, uint8_t *IEEEAddr,
                              byte capabilities, byte SecurityEnable )
{
  zAddrType_t dstAddr;
  uint8_t len;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  dstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
  dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVRXON;

  ZDP_TmpBuf[0] = LO_UINT16( nwkAddr );
  ZDP_TmpBuf[1] = HI_UINT16( nwkAddr );
  osal_cpyExtAddr( &ZDP_TmpBuf[2], IEEEAddr );
  len = 2 + Z_EXTADDR_LEN;

  ZDP_TmpBuf[10] = capabilities;
  len++;

  return fillAndSend( &ZDP_SeqNum, &dstAddr, Device_annce, len );
}

/*********************************************************************
 * @fn          ZDP_ParentAnnce
 *
 * @brief       This builds and send a Parent_Annce and Parent_Annce_Rsp
 *              messages, it will depend on the clusterID parameter.
 *
 * @param       TransSeq - ZDP Transaction Sequence Number
 * @param       dstAddr - destination address
 * @param       numberOfChildren - 8 bit number of children
 * @param       childInfo - list of children information (ExtAddr and Age)
 * @param       clusterID - Parent_annce or Parent_annce_rsp
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_ParentAnnce( uint8_t *TransSeq,
                            zAddrType_t *dstAddr,
                            uint8_t numberOfChildren,
                            uint8_t *childInfo,
                            cId_t clusterID,
                            uint8_t SecurityEnable )
{
  uint8_t *pBuf = ZDP_TmpBuf;
  ZDO_ChildInfo_t *pChildInfo;
  uint8_t i, len;
  uint8_t *numOfChild;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  pChildInfo = (ZDO_ChildInfo_t *)childInfo;

  if ( dstAddr->addrMode == AddrBroadcast )
  {
    // Make sure is sent to 0xFFFC
    dstAddr->addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVZCZR;
  }
  len = 1;
  if ( clusterID == Parent_annce_rsp )
  {
    // + Status Byte
    len += 1;
    // Set the status bit to success
    *pBuf++ = 0;
  }

  numOfChild = pBuf;
  *pBuf++ = numberOfChildren;

  for ( i = 0; i < MAX_PARENT_ANNCE_CHILD; i++ )
  {
    pBuf = osal_cpyExtAddr( pBuf, pChildInfo[childIndex].extAddr );
    childIndex++;

    len += Z_EXTADDR_LEN;

    if ( childIndex == numberOfChildren )
    {
      pBuf = numOfChild;
      *pBuf = i + 1;
      // All childs are taken, restart index and go out
      childIndex = 0;
      return fillAndSend( TransSeq, dstAddr, clusterID, len );
    }
  }

  pBuf = numOfChild;
  *pBuf = MAX_PARENT_ANNCE_CHILD;
  if ( childIndex < numberOfChildren )
  {
    if ( clusterID == Parent_annce )
    {
      ZDApp_SetParentAnnceTimer();
    }
    if ( clusterID == Parent_annce_rsp )
    {
      OsalPortTimers_startTimer( ZDAppTaskID, ZDO_PARENT_ANNCE_EVT, 10 );
    }
  }

  return fillAndSend( TransSeq, dstAddr, clusterID, len );
}

/*********************************************************************
 * Address Responses
 */

/*********************************************************************
 * @fn      zdpProcessAddrReq
 *
 * @brief   Process an incoming NWK_addr_req or IEEE_addr_req message and then
 *          build and send a corresponding NWK_addr_rsp or IEEE_addr_rsp msg.
 *
 * @param   inMsg - incoming message
 *
 * @return  none
 */
void zdpProcessAddrReq( zdoIncomingMsg_t *inMsg )
{
  AddrMgrEntry_t addrEntry;
  associated_devices_t *pAssoc;
  uint8_t reqType;
  uint16_t aoi = INVALID_NODE_ADDR;
  uint8_t invalidIEEEAddr[Z_EXTADDR_LEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  uint8_t *ieee = NULL;

  reqType = inMsg->asdu[(inMsg->clusterID == NWK_addr_req) ? Z_EXTADDR_LEN : sizeof( uint16_t ) ];

  if ( inMsg->clusterID == NWK_addr_req )

  {
    ieee = inMsg->asdu;

    if ( osal_ExtAddrEqual( saveExtAddr, ieee ) )
    {
      aoi = ZDAppNwkAddr.addr.shortAddr;
    }
    // Handle response for sleeping end devices
    else if ( (ZSTACK_ROUTER_BUILD)
      && (((pAssoc = AssocGetWithExt( ieee )) != NULL)
             && ((pAssoc->nodeRelation == CHILD_RFD) || (pAssoc->nodeRelation == CHILD_RFD_RX_IDLE)) ) )
    {
      aoi = pAssoc->shortAddr;
      if ( reqType != ZDP_ADDR_REQTYPE_SINGLE )
        reqType = 0xFF; // Force Invalid
    }
  }
  else  // if ( inMsg->clusterID == IEEE_addr_req )
  {
    aoi = BUILD_UINT16( inMsg->asdu[0], inMsg->asdu[1] );

    if ( aoi == ZDAppNwkAddr.addr.shortAddr )
    {
      ieee = saveExtAddr;
    }
    else if ( (ZSTACK_ROUTER_BUILD)
      && (((pAssoc = AssocGetWithShort( aoi )) != NULL)
             && (pAssoc->nodeRelation == CHILD_RFD)) )
    {
      addrEntry.user = ADDRMGR_USER_DEFAULT;
      addrEntry.index = pAssoc->addrIdx;
      if ( AddrMgrEntryGet( &addrEntry ) )
      {
        ieee = addrEntry.extAddr;
      }

      if ( reqType != ZDP_ADDR_REQTYPE_SINGLE )
        reqType = 0xFF; // Force Invalid
    }
  }

  if ( ((aoi != INVALID_NODE_ADDR) && (ieee != NULL)) || (inMsg->wasBroadcast == FALSE) )
  {
    uint8_t stat;
    uint8_t *pBuf = ZDP_TmpBuf;
    // Status + IEEE-Addr + Nwk-Addr.
    uint8_t len = 1 + Z_EXTADDR_LEN + 2;

    // If aoi and iee are both setup, we found results
    if ( (aoi != INVALID_NODE_ADDR) && (ieee != NULL) )
    {
      stat = ((reqType == ZDP_ADDR_REQTYPE_SINGLE) || (reqType == ZDP_ADDR_REQTYPE_EXTENDED))
                    ? ZDP_SUCCESS : ZDP_INVALID_REQTYPE;

      if(stat == ZDP_INVALID_REQTYPE)
      {
        //R21 Errata update CCB 2111
        if(inMsg->wasBroadcast == TRUE)
        {
          return;
        }

        stat = ZDP_INVALID_REQTYPE;
      }
    }
    else
    {
      // not found and the req was unicast to this device
      stat = ZDP_DEVICE_NOT_FOUND;

      // Fill in the missing field with this device's address
      if ( inMsg->clusterID == NWK_addr_req )
      {
        //CCB 2112 Zigbee Core spec
        aoi = 0xFFFF;
      }
      else
      {
        //CCB 2113 Zigbee Core spec
        ieee = invalidIEEEAddr;
      }
    }

    *pBuf++ = stat;

    if(ieee != NULL)
    {
      pBuf = osal_cpyExtAddr( pBuf, ieee );
    }
    else
    {
      pBuf = osal_cpyExtAddr( pBuf, invalidIEEEAddr );
    }

    *pBuf++ = LO_UINT16( aoi );
    *pBuf++ = HI_UINT16( aoi );

    if ( ZSTACK_ROUTER_BUILD )
    {
      if ( (reqType == ZDP_ADDR_REQTYPE_EXTENDED) && (aoi == ZDAppNwkAddr.addr.shortAddr)
           && (stat == ZDP_SUCCESS) )
      {
        uint8_t  cnt = 0;

        //Updated to only search for ZED devices as per R21 spec (2.4.3.1.1.2)
        uint16_t *list = AssocMakeList( &cnt );

        if ( list != NULL )
        {
          byte idx = inMsg->asdu[(((inMsg->clusterID == NWK_addr_req) ? Z_EXTADDR_LEN : sizeof( uint16_t )) + 1)];
          uint16_t *pList = list + idx;

          // NumAssocDev field is only present on success.
          if ( cnt > idx )
          {
            cnt -= idx;
            len += (cnt * sizeof( uint16_t ));
          }
          else
          {
            cnt = 0;
          }
          *pBuf++ = cnt;
          len++;

          // StartIndex field is only present if NumAssocDev field is non-zero.
          *pBuf++ = idx;
          len++;

          while ( cnt != 0 )
          {
            *pBuf++ = LO_UINT16( *pList );
            *pBuf++ = HI_UINT16( *pList );
            pList++;
            cnt--;
          }

          OsalPort_free( (uint8_t *)list );
        }
        else
        {
          // NumAssocDev field is only present on success.
          *pBuf++ = 0;
          len++;
        }
      }
    }

    ZDP_TxOptions = AF_MSG_ACK_REQUEST;
    fillAndSend( &(inMsg->TransSeq), &(inMsg->srcAddr), (cId_t)(inMsg->clusterID | ZDO_RESPONSE_BIT), len );
    ZDP_TxOptions = AF_TX_OPTIONS_NONE;
  }
}

/*********************************************************************
 * @fn          ZDP_NodeDescMsg
 *
 * @brief       Builds and sends a Node Descriptor message, unicast to the
 *              specified device.
 *
 * @param       inMsg - incoming message
 * @param       nwkAddr - 16 bit network address for device
 * @param       pNodeDesc - pointer to the node descriptor
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_NodeDescMsg( zdoIncomingMsg_t *inMsg,
                           uint16_t nwkAddr, NodeDescriptorFormat_t *pNodeDesc )
{
  uint8_t *pBuf = ZDP_TmpBuf;
  byte len;

  len = 1 + 2 + 13;  // Status + nwkAddr + Node descriptor

  *pBuf++ = ZDP_SUCCESS;

  *pBuf++ = LO_UINT16( nwkAddr );
  *pBuf++ = HI_UINT16( nwkAddr );

  *pBuf++ = (byte)((pNodeDesc->ComplexDescAvail << 3) |
                     (pNodeDesc->UserDescAvail << 4) |
                     (pNodeDesc->LogicalType & 0x07));

  *pBuf++ = (byte)((pNodeDesc->FrequencyBand << 3) | (pNodeDesc->APSFlags & 0x07));
  *pBuf++ = pNodeDesc->CapabilityFlags;
  *pBuf++ = pNodeDesc->ManufacturerCode[0];
  *pBuf++ = pNodeDesc->ManufacturerCode[1];
  *pBuf++ = pNodeDesc->MaxBufferSize;
  *pBuf++ = pNodeDesc->MaxInTransferSize[0];
  *pBuf++ = pNodeDesc->MaxInTransferSize[1];

  *pBuf++ = LO_UINT16( pNodeDesc->ServerMask );
  *pBuf++ = HI_UINT16( pNodeDesc->ServerMask );
  *pBuf++ = pNodeDesc->MaxOutTransferSize[0];
  *pBuf++ = pNodeDesc->MaxOutTransferSize[1];
  *pBuf = pNodeDesc->DescriptorCapability;

  return fillAndSend( &(inMsg->TransSeq), &(inMsg->srcAddr), Node_Desc_rsp, len );
}

/*********************************************************************
 * @fn          ZDP_PowerDescMsg
 *
 * @brief       Builds and sends a Power Descriptor message, unicast to the
 *              specified device.
 *
 * @param       inMsg - incoming message (request)
 * @param       nwkAddr - 16 bit network address for device
 * @param       pPowerDesc - pointer to the node descriptor
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_PowerDescMsg( zdoIncomingMsg_t *inMsg,
                     uint16_t nwkAddr, NodePowerDescriptorFormat_t *pPowerDesc )
{
  uint8_t *pBuf = ZDP_TmpBuf;
  byte len = 1 + 2 + 2;  // Status + nwkAddr + Node Power descriptor.

  *pBuf++ = ZDP_SUCCESS;

  *pBuf++ = LO_UINT16( nwkAddr );
  *pBuf++ = HI_UINT16( nwkAddr );

  *pBuf++ = (byte)((pPowerDesc->AvailablePowerSources << 4)
                    | (pPowerDesc->PowerMode & 0x0F));
  *pBuf++ = (byte)((pPowerDesc->CurrentPowerSourceLevel << 4)
                    | (pPowerDesc->CurrentPowerSource & 0x0F));

  return fillAndSend( &(inMsg->TransSeq), &(inMsg->srcAddr), Power_Desc_rsp, len );
}

/*********************************************************************
 * @fn          ZDP_SimpleDescMsg
 *
 * @brief       Builds and sends a Simple Descriptor message, unicast to the
 *              specified device.
 *
 * @param       inMsg - incoming message (request)
 * @param       Status - message status (ZDP_SUCCESS or other)
 * @param       pSimpleDesc - pointer to the node descriptor
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_SimpleDescMsg( zdoIncomingMsg_t *inMsg, byte Status,
                              SimpleDescriptionFormat_t *pSimpleDesc )
{
  uint8_t *pBuf = ZDP_TmpBuf;
  uint8_t i, len;

  if ( Status == ZDP_SUCCESS && pSimpleDesc )
  {
    // Status + NWKAddrOfInterest + desc length + empty simple descriptor.
    len = 1 + 2 + 1 + 8;
    len += (pSimpleDesc->AppNumInClusters + pSimpleDesc->AppNumOutClusters) * sizeof ( uint16_t );
  }
  else
  {
    len = 1 + 2 + 1; // Status + desc length
  }
  if ( len >= ZDP_BUF_SZ-1 )
  {
    return afStatus_MEM_FAIL;
  }

  *pBuf++ = Status;

  //From spec 2.4.3.1.5 The NWKAddrOfInterest field shall match
  //that specified in the original Simple_Desc_req command
  *pBuf++ = inMsg->asdu[0];
  *pBuf++ = inMsg->asdu[1];

  if ( len > 4 )
  {
    *pBuf++ = len - 4;   // Simple descriptor length

    *pBuf++ = pSimpleDesc->EndPoint;
    *pBuf++ = LO_UINT16( pSimpleDesc->AppProfId );
    *pBuf++ = HI_UINT16( pSimpleDesc->AppProfId );
    *pBuf++ = LO_UINT16( pSimpleDesc->AppDeviceId );
    *pBuf++ = HI_UINT16( pSimpleDesc->AppDeviceId );

    *pBuf++ = (byte)(pSimpleDesc->AppDevVer & 0x0F);

    *pBuf++ = pSimpleDesc->AppNumInClusters;
    if ( pSimpleDesc->AppNumInClusters )
    {
      for (i=0; i<pSimpleDesc->AppNumInClusters; ++i)
      {
        *pBuf++ = LO_UINT16( pSimpleDesc->pAppInClusterList[i] );
        *pBuf++ = HI_UINT16( pSimpleDesc->pAppInClusterList[i] );
      }
    }

    *pBuf++ = pSimpleDesc->AppNumOutClusters;
    if ( pSimpleDesc->AppNumOutClusters )
    {
      for (i=0; i<pSimpleDesc->AppNumOutClusters; ++i)
      {
        *pBuf++ = LO_UINT16( pSimpleDesc->pAppOutClusterList[i] );
        *pBuf++ = HI_UINT16( pSimpleDesc->pAppOutClusterList[i] );
      }
    }
  }

  else
  {
    *pBuf = 0; // Description Length = 0;
  }

  return fillAndSend( &(inMsg->TransSeq), &(inMsg->srcAddr), Simple_Desc_rsp, len );
}

/*********************************************************************
 * @fn          ZDP_EPRsp
 *
 * @brief       This builds and send an endpoint list. Used in
 *              Active_EP_rsp and Match_Desc_Rsp
 *              message.  This function sends unicast message to the
 *              requesting device.
 *
 * @param       MsgType - either Active_EP_rsp or Match_Desc_Rsp
 * @param       dstAddr - destination address
 * @param       Status - message status (ZDP_SUCCESS or other)
 * @param       nwkAddr - Device's short address that this response describes
 * @param       Count - number of endpoint/interfaces in list
 * @param       pEPIntfList - Array of Endpoint/Interfaces
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_EPRsp( uint16_t MsgType, byte TransSeq, zAddrType_t *dstAddr,
                        byte Status, uint16_t nwkAddr, byte Count,
                        uint8_t *pEPList,
                        byte SecurityEnable )
{
  uint8_t *pBuf = ZDP_TmpBuf;
  byte len = 1 + 2 + 1;  // Status + nwkAddr + endpoint/interface count.
  byte txOptions;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  if ( MsgType == Match_Desc_rsp )
    txOptions = AF_MSG_ACK_REQUEST;
  else
    txOptions = 0;

    *pBuf++ = Status;
  *pBuf++ = LO_UINT16( nwkAddr );
  *pBuf++ = HI_UINT16( nwkAddr );

  *pBuf++ = Count;   // Endpoint/Interface count

  if ( Count )
  {
    len += Count;
    OsalPort_memcpy( pBuf, pEPList, Count );
  }

  FillAndSendTxOptions( &TransSeq, dstAddr, MsgType, len, txOptions );
}

/*********************************************************************
 * @fn          ZDP_UserDescRsp
 *
 * @brief       Build and send the User Decriptor Response.
 *
 *
 * @param       dstAddr - destination address
 * @param       nwkAddrOfInterest -
 * @param       userDesc -
 * @param       SecurityEnable - Security Options
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDP_UserDescRsp( byte TransSeq, zAddrType_t *dstAddr,
                uint16_t nwkAddrOfInterest, UserDescriptorFormat_t *userDesc,
                byte SecurityEnable )
{
  uint8_t *pBuf = ZDP_TmpBuf;
  byte len = 1 + 2 + 1;  // Status + nwkAddr + descriptor length.

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  len += userDesc->len;

  *pBuf++ = ZSUCCESS;

  *pBuf++ = LO_UINT16( nwkAddrOfInterest );
  *pBuf++ = HI_UINT16( nwkAddrOfInterest );

  *pBuf++ = userDesc->len;
  OsalPort_memcpy( pBuf, userDesc->desc, userDesc->len );

  return (ZStatus_t)fillAndSend( &TransSeq, dstAddr, User_Desc_rsp, len );
}

/*********************************************************************
 * @fn          ZDP_ServerDiscRsp
 *
 * @brief       Build and send the Server_Discovery_rsp response.
 *
 * @param       transID - Transaction sequence number of request.
 * @param       dstAddr - Network Destination Address.
 * @param       status - Status of response to request.
 * @param       aoi - Network Address of Interest of request.
 * @param       serverMask - Bit map of service(s) being sought.
 * @param       SecurityEnable - Security Options
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDP_ServerDiscRsp( byte transID, zAddrType_t *dstAddr, byte status,
                           uint16_t aoi, uint16_t serverMask, byte SecurityEnable )
{
  const byte len = 1  + 2;  // status + aoi + mask.
  uint8_t *pBuf = ZDP_TmpBuf;
  ZStatus_t stat;

  // Intentionally unreferenced parameters
  (void)aoi;
  (void)SecurityEnable;

  *pBuf++ = status;

  *pBuf++ = LO_UINT16( serverMask );
  *pBuf++ = HI_UINT16( serverMask );

  ZDP_TxOptions = AF_MSG_ACK_REQUEST;
  stat = fillAndSend( &transID, dstAddr, Server_Discovery_rsp, len );
  ZDP_TxOptions = AF_TX_OPTIONS_NONE;

  return ( stat );
}

/*********************************************************************
 * @fn          ZDP_GenericRsp
 *
 * @brief       Sends a response message with only the parameter status
 *              byte and the addr of interest for data.
 *              This function sends unicast message to the
 *              requesting device.
 *
 * @param       dstAddr - destination address
 * @param       status  - generic status for response
 * @param       aoi     - address of interest
 * @param       dstAddr - destination address
 * @param       rspId   - response cluster ID
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_GenericRsp( byte TransSeq, zAddrType_t *dstAddr,
                     byte status, uint16_t aoi, uint16_t rspID, byte SecurityEnable )
{
  uint8_t len;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  ZDP_TmpBuf[0] = status;
  ZDP_TmpBuf[1] = LO_UINT16( aoi );
  ZDP_TmpBuf[2] = HI_UINT16( aoi );

  // Length byte
  ZDP_TmpBuf[3] = 0;
  len = 4;

  return fillAndSend( &TransSeq, dstAddr, rspID, len );
}

/*********************************************************************
 * Binding
 */
/*********************************************************************
 * @fn          ZDP_EndDeviceBindReq
 *
 * @brief       This builds and sends a End_Device_Bind_req message.
 *              This function sends a unicast message.
 *
 * @param       dstAddr - destination address
 * @param       LocalCoordinator - short address of local coordinator
 * @param       epIntf - Endpoint/Interface of Simple Desc
 * @param       ProfileID - Profile ID
 *
 *   The Input cluster list is the opposite of what you would think.
 *   This is the output cluster list of this device
 * @param       NumInClusters - number of input clusters
 * @param       InClusterList - input cluster ID list
 *
 *   The Output cluster list is the opposite of what you would think.
 *   This is the input cluster list of this device
 * @param       NumOutClusters - number of output clusters
 * @param       OutClusterList - output cluster ID list
 *
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_EndDeviceBindReq( zAddrType_t *dstAddr,
                                 uint16_t LocalCoordinator,
                                 byte endPoint,
                                 uint16_t ProfileID,
                                 byte NumInClusters, cId_t *InClusterList,
                                 byte NumOutClusters, cId_t *OutClusterList,
                                 byte SecurityEnable )
{
  uint8_t *pBuf = ZDP_TmpBuf;
  uint8_t i, len;
  uint8_t *ieeeAddr;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  // LocalCoordinator + SrcExtAddr + ep + ProfileID +  NumInClusters + NumOutClusters.
  len = 2 + Z_EXTADDR_LEN + 1 + 2 + 1 + 1;
  len += (NumInClusters + NumOutClusters) * sizeof ( uint16_t );

  if ( len >= ZDP_BUF_SZ-1 )
  {
    return afStatus_MEM_FAIL;
  }

  if ( LocalCoordinator != NLME_GetShortAddr() )
  {
    return afStatus_INVALID_PARAMETER;
  }

  *pBuf++ = LO_UINT16( LocalCoordinator );
  *pBuf++ = HI_UINT16( LocalCoordinator );

  ieeeAddr = NLME_GetExtAddr();
  pBuf = osal_cpyExtAddr( pBuf, ieeeAddr );

  *pBuf++ = endPoint;

  *pBuf++ = LO_UINT16( ProfileID );   // Profile ID
  *pBuf++ = HI_UINT16( ProfileID );

  *pBuf++ = NumInClusters; // Input cluster list
  for ( i = 0; i < NumInClusters; ++i )
  {
    *pBuf++ = LO_UINT16(InClusterList[i]);
    *pBuf++ = HI_UINT16(InClusterList[i]);
  }

  *pBuf++ = NumOutClusters; // Output cluster list
  for ( i = 0; i < NumOutClusters; ++i )
  {
    *pBuf++ = LO_UINT16(OutClusterList[i]);
    *pBuf++ = HI_UINT16(OutClusterList[i]);
  }

  return fillAndSend( &ZDP_SeqNum, dstAddr, End_Device_Bind_req, len );
}

/*********************************************************************
 * @fn          ZDP_BindUnbindReq
 *
 * @brief       This builds and send a Bind_req or Unbind_req message
 *              Depending on the ClusterID. This function
 *              sends a unicast message to the local coordinator.
 *
 * @param       BindOrUnbind - either Bind_req or Unbind_req
 * @param       dstAddr - destination address of the message
 * @param       SourceAddr - source 64 bit address of the binding
 * @param       SrcEPIntf - Source endpoint/interface
 * @param       ClusterID - Binding cluster ID
 * @param       DestinationAddr - destination 64 bit addr of binding
 * @param       DstEPIntf - destination endpoint/interface
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_BindUnbindReq( uint16_t BindOrUnbind, zAddrType_t *dstAddr,
                              uint8_t *SourceAddr, byte SrcEndPoint,
                              cId_t ClusterID,
                              zAddrType_t *destinationAddr, byte DstEndPoint,
                              byte SecurityEnable )
{
  uint8_t *pBuf = ZDP_TmpBuf;
  byte len;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  // SourceAddr + SrcEPIntf + ClusterID +  addrMode.
  len = Z_EXTADDR_LEN + 1 + sizeof( cId_t ) + sizeof( uint8_t );
  if ( destinationAddr->addrMode == Addr64Bit )
    len += Z_EXTADDR_LEN + 1;     // +1 for DstEPIntf
  else if ( destinationAddr->addrMode == AddrGroup )
    len += sizeof ( uint16_t );

  pBuf = osal_cpyExtAddr( pBuf, SourceAddr );
  *pBuf++ = SrcEndPoint;

  *pBuf++ = LO_UINT16( ClusterID );

  *pBuf++ = HI_UINT16( ClusterID );
  *pBuf++ = destinationAddr->addrMode;
  if ( destinationAddr->addrMode == Addr64Bit )
  {
    pBuf = osal_cpyExtAddr( pBuf, destinationAddr->addr.extAddr );
    *pBuf = DstEndPoint;
  }
  else if ( destinationAddr->addrMode == AddrGroup )
  {
    *pBuf++ = LO_UINT16( destinationAddr->addr.shortAddr );
    *pBuf++ = HI_UINT16( destinationAddr->addr.shortAddr );
  }

  FillAndSendTxOptions( &ZDP_SeqNum, dstAddr, BindOrUnbind, len, AF_MSG_ACK_REQUEST );
}

/*********************************************************************
 * Network Management
 */

/*********************************************************************
 * @fn          ZDP_MgmtNwkDiscReq
 *
 * @brief       This builds and send a Mgmt_NWK_Disc_req message. This
 *              function sends a unicast message.
 *
 * @param       dstAddr - destination address of the message
 * @param       ScanChannels - 32 bit address bit map
 * @param       StartIndex - Starting index within the reporting network
 *                           list
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_MgmtNwkDiscReq( zAddrType_t *dstAddr,
                               uint32_t ScanChannels,
                               byte ScanDuration,
                               byte StartIndex,
                               byte SecurityEnable )
{
  uint8_t *pBuf = ZDP_TmpBuf;
  byte len = sizeof( uint32_t )+1+1;  // ScanChannels + ScanDuration + StartIndex.

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  pBuf = OsalPort_bufferUint32( pBuf, ScanChannels );

  *pBuf++ = ScanDuration;
  *pBuf = StartIndex;

  return fillAndSend( &ZDP_SeqNum, dstAddr, Mgmt_NWK_Disc_req, len );
}

/*********************************************************************
 * @fn          ZDP_MgmtDirectJoinReq
 *
 * @brief       This builds and send a Mgmt_Direct_Join_req message. This
 *              function sends a unicast message.
 *
 * @param       dstAddr - destination address of the message
 * @param       deviceAddr - 64 bit IEEE Address
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_MgmtDirectJoinReq( zAddrType_t *dstAddr,
                               uint8_t *deviceAddr,
                               byte capInfo,
                               byte SecurityEnable )
{
  (void)SecurityEnable;  // Intentionally unreferenced parameter

  osal_cpyExtAddr( ZDP_TmpBuf, deviceAddr );
  ZDP_TmpBuf[Z_EXTADDR_LEN] = capInfo;

  return fillAndSend( &ZDP_SeqNum, dstAddr, Mgmt_Direct_Join_req, (Z_EXTADDR_LEN + 1) );
}

/*********************************************************************
 * @fn          ZDP_MgmtPermitJoinReq
 *
 * @brief       This builds and send a Mgmt_Permit_Join_req message.
 *
 * @param       dstAddr - destination address of the message
 * @param       duration - Permit duration
 * @param       TcSignificance - Trust Center Significance
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_MgmtPermitJoinReq( zAddrType_t *dstAddr, byte duration,
                                  byte TcSignificance, byte SecurityEnable )
{
  (void)SecurityEnable;  // Intentionally unreferenced parameter

  // Build buffer
  ZDP_TmpBuf[ZDP_MGMT_PERMIT_JOIN_REQ_DURATION] = duration;
  ZDP_TmpBuf[ZDP_MGMT_PERMIT_JOIN_REQ_TC_SIG]   = TcSignificance;

  // Check of this is a broadcast message
  if ( ((dstAddr->addrMode == Addr16Bit) || (dstAddr->addrMode == AddrBroadcast))
      && ((dstAddr->addr.shortAddr == NWK_BROADCAST_SHORTADDR_DEVALL)
          || (dstAddr->addr.shortAddr == NWK_BROADCAST_SHORTADDR_DEVZCZR)
          || (dstAddr->addr.shortAddr == NWK_BROADCAST_SHORTADDR_DEVRXON)) )
  {
    // Send this to our self as well as broadcast to network
    zAddrType_t tmpAddr;

    tmpAddr.addrMode = Addr16Bit;
    tmpAddr.addr.shortAddr = NLME_GetShortAddr();

    fillAndSend( &ZDP_SeqNum, &tmpAddr, Mgmt_Permit_Join_req,
                      ZDP_MGMT_PERMIT_JOIN_REQ_SIZE );
  }

  // Send the message
  return fillAndSend( &ZDP_SeqNum, dstAddr, Mgmt_Permit_Join_req,
                      ZDP_MGMT_PERMIT_JOIN_REQ_SIZE );
}

/*********************************************************************
 * @fn          ZDP_MgmtLeaveReq
 *
 * @brief       This builds and send a Mgmt_Leave_req message.
 *
 * @param       dstAddr - destination address of the message
 *              IEEEAddr - IEEE adddress of device that is removed
 *              RemoveChildren - set to 1 to remove the children of the
 *                                device as well. 0 otherwise.
 *              Rejoin - set to 1 if the removed device should rejoin
                         afterwards. 0 otherwise.
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_MgmtLeaveReq( zAddrType_t *dstAddr, uint8_t *IEEEAddr, uint8_t RemoveChildren,
                 uint8_t Rejoin, uint8_t SecurityEnable )

{
  (void)SecurityEnable;  // Intentionally unreferenced parameter

  osal_cpyExtAddr( ZDP_TmpBuf, IEEEAddr );
  ZDP_TmpBuf[Z_EXTADDR_LEN] = 0;

  if ( RemoveChildren == TRUE )
  {
    ZDP_TmpBuf[Z_EXTADDR_LEN] |= ZDP_MGMT_LEAVE_REQ_RC;
  }
  if ( Rejoin == TRUE )
  {
    ZDP_TmpBuf[Z_EXTADDR_LEN] |= ZDP_MGMT_LEAVE_REQ_REJOIN;
  }

  return fillAndSend( &ZDP_SeqNum, dstAddr, Mgmt_Leave_req, (Z_EXTADDR_LEN + 1) );
}

/*********************************************************************
 * @fn          ZDP_MgmtNwkUpdateReq
 *
 * @brief       This builds and send a Mgmt_NWK_Update_req message. This
 *              function sends a unicast or broadcast message.
 *
 * @param       dstAddr - destination address of the message
 * @param       ChannelMask - 32 bit address bit map
 * @param       ScanDuration - length of time to spend scanning each channel
 * @param       ScanCount - number of energy scans to be conducted
 * @param       NwkUpdateId - NWk Update Id value
 * @param       NwkManagerAddr - NWK address for device with Network Manager
 *                               bit set in its Node Descriptor
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_MgmtNwkUpdateReq( zAddrType_t *dstAddr,
                                 uint32_t ChannelMask,
                                 uint8_t ScanDuration,
                                 uint8_t ScanCount,
                                 uint8_t NwkUpdateId,
                                 uint16_t NwkManagerAddr )
{
  uint8_t *pBuf = ZDP_TmpBuf;
  byte len = sizeof( uint32_t ) + 1;  // ChannelMask + ScanDuration

  pBuf = OsalPort_bufferUint32( pBuf, ChannelMask );

  *pBuf++ = ScanDuration;

  if ( ScanDuration <= 0x05 )
  {
    // Request is to scan over channelMask
    len += sizeof( uint8_t );

    *pBuf++ = ScanCount;
  }
  else if ( ( ScanDuration == 0xFE ) || ( ScanDuration == 0xFF ) )
  {
    // Request is to change Channel (0xFE) or apsChannelMask and NwkManagerAddr (0xFF)
    len += sizeof( uint8_t );

    *pBuf++ = NwkUpdateId;

    if ( ScanDuration == 0xFF )
    {
      len += sizeof( uint16_t );

      *pBuf++  = LO_UINT16( NwkManagerAddr );
      *pBuf++  = HI_UINT16( NwkManagerAddr );
    }
  }

  return fillAndSend( &ZDP_SeqNum, dstAddr, Mgmt_NWK_Update_req, len );
}


/*********************************************************************
 * Network Management Responses
 */

/*********************************************************************
 * @fn          ZDP_MgmtNwkDiscRsp
 *
 * @brief       This builds and send a Mgmt_NWK_Disc_rsp message. This
 *              function sends a unicast message.
 *
 * @param       dstAddr - destination address of the message
 * @param       Status - message status (ZDP_SUCCESS or other)
 * @param       NetworkCount - Total number of networks found
 * @param       StartIndex - Starting index within the reporting network
 *                           list
 * @param       NetworkListCount - number of network lists included
 *                                 in this message
 * @param       NetworkList - List of network descriptors
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_MgmtNwkDiscRsp( byte TransSeq, zAddrType_t *dstAddr,
                            byte Status,
                            byte NetworkCount,
                            byte StartIndex,
                            byte NetworkListCount,
                            networkDesc_t *NetworkList,
                            byte SecurityEnable )
{
  uint8_t *buf;
  uint8_t *pBuf;
  byte len = 1+1+1+1;  // Status + NetworkCount + StartIndex + NetworkCountList.
  byte idx;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  len += (NetworkListCount * ( ZDP_NETWORK_EXTENDED_DISCRIPTOR_SIZE - 2 ));

  buf = OsalPort_malloc( len+1 );
  if ( buf == NULL )
  {
    return afStatus_MEM_FAIL;
  }

  pBuf = buf+1;

  *pBuf++ = Status;
  *pBuf++ = NetworkCount;
  *pBuf++ = StartIndex;
  *pBuf++ = NetworkListCount;

  for ( idx = 0; idx < NetworkListCount; idx++ )
  {
    osal_cpyExtAddr( pBuf, NetworkList->extendedPANID);
    pBuf += Z_EXTADDR_LEN;

    *pBuf++  = NetworkList->logicalChannel;                // LogicalChannel
    *pBuf    = NetworkList->stackProfile;                  // Stack profile
    *pBuf++ |= (byte)(NetworkList->version << 4);          // ZigBee Version
    *pBuf    = BEACON_ORDER_NO_BEACONS;                    // Beacon Order
    *pBuf++ |= (uint8_t)(BEACON_ORDER_NO_BEACONS << 4);      // Superframe Order

    if ( NetworkList->chosenRouter != INVALID_NODE_ADDR )
    {
      *pBuf++ = TRUE;                         // Permit Joining
    }
    else
    {
      *pBuf++ = FALSE;
    }

    NetworkList = NetworkList->nextDesc;    // Move to next list entry
  }

  FillAndSendBuffer( &TransSeq, dstAddr, Mgmt_NWK_Disc_rsp, len, buf );
}

/*********************************************************************
 * @fn          ZDP_MgmtLqiRsp
 *
 * @brief       This builds and send a Mgmt_Lqi_rsp message. This
 *              function sends a unicast message.
 *
 * @param       dstAddr - destination address of the message
 * @param       Status - message status (ZDP_SUCCESS or other)
 * @param       NeighborLqiEntries - Total number of entries found
 * @param       StartIndex - Starting index within the reporting list
 * @param       NeighborLqiCount - number of lists included
 *                                 in this message
 * @param       NeighborLqiList - List of NeighborLqiItems.  This list
 *                is the list to be sent, not the entire list
 * @param       SecurityEnable - true if secure
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDP_MgmtLqiRsp( byte TransSeq, zAddrType_t *dstAddr,
                          byte Status,
                          byte NeighborLqiEntries,
                          byte StartIndex,
                          byte NeighborLqiCount,
                          ZDP_MgmtLqiItem_t* NeighborList,
                          byte SecurityEnable )
{
  ZDP_MgmtLqiItem_t* list = NeighborList;
  uint8_t *buf, *pBuf;
  byte len, x;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  if ( ZSuccess != Status )
  {
    ZDP_TmpBuf[0] = Status;
    return fillAndSend( &TransSeq, dstAddr, Mgmt_Lqi_rsp, 1 );
  }

  // (Status + NeighborLqiEntries + StartIndex + NeighborLqiCount) +
  //  neighbor LQI data.
  len = (1 + 1 + 1 + 1) + (NeighborLqiCount * ZDP_MGMTLQI_EXTENDED_SIZE);

  buf = OsalPort_malloc( len+1 );
  if ( buf == NULL )
  {
    return afStatus_MEM_FAIL;
  }

  pBuf = buf+1;

  *pBuf++ = Status;
  *pBuf++ = NeighborLqiEntries;
  *pBuf++ = StartIndex;
  *pBuf++ = NeighborLqiCount;

  for ( x = 0; x < NeighborLqiCount; x++ )
  {
    osal_cpyExtAddr( pBuf, list->extPanID);         // Extended PanID
    pBuf += Z_EXTADDR_LEN;

    // EXTADDR
    pBuf = osal_cpyExtAddr( pBuf, list->extAddr );

    // NWKADDR
    *pBuf++ = LO_UINT16( list->nwkAddr );
    *pBuf++ = HI_UINT16( list->nwkAddr );

    // DEVICETYPE
    *pBuf = list->devType;

    // RXONIDLE
    *pBuf |= (uint8_t)(list->rxOnIdle << 2);

    // RELATIONSHIP
    *pBuf++ |= (uint8_t)(list->relation << 4);

    // PERMITJOINING
    *pBuf++ = (uint8_t)(list->permit);

    // DEPTH
    *pBuf++ = list->depth;

    // LQI
    *pBuf++ = list->lqi;

    list++; // next list entry
  }

  FillAndSendBuffer( &TransSeq, dstAddr, Mgmt_Lqi_rsp, len, buf );
}

/*********************************************************************
 * @fn          ZDP_MgmtRtgRsp
 *
 * @brief       This builds and send a Mgmt_Rtg_rsp message. This
 *              function sends a unicast message.
 *
 * @param       dstAddr - destination address of the message
 * @param       Status - message status (ZDP_SUCCESS or other)
 * @param       RoutingTableEntries - Total number of entries
 * @param       StartIndex - Starting index within the reporting list
 * @param       RoutingTableListCount - number of entries included
 *                                      in this message
 * @param       RoutingTableList - List of Routing Table entries
 * @param       SecurityEnable - true to enable security for this message
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDP_MgmtRtgRsp( byte TransSeq, zAddrType_t *dstAddr,
                            byte Status,
                            byte RoutingTableEntries,
                            byte StartIndex,
                            byte RoutingListCount,
                            rtgItem_t *RoutingTableList,
                            byte SecurityEnable )
{
  uint8_t *buf;
  uint8_t *pBuf;
  // Status + RoutingTableEntries + StartIndex + RoutingListCount.
  byte len = 1 + 1 + 1 + 1;
  byte x;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  // Add an array for Routing List data
  len += (RoutingListCount * ZDP_ROUTINGENTRY_SIZE);

  buf = OsalPort_malloc( (short)(len+1) );
  if ( buf == NULL )
  {
    return afStatus_MEM_FAIL;
  }

  pBuf = buf+1;

  *pBuf++ = Status;
  *pBuf++ = RoutingTableEntries;
  *pBuf++ = StartIndex;
  *pBuf++ = RoutingListCount;

  for ( x = 0; x < RoutingListCount; x++ )
  {
    *pBuf++ = LO_UINT16( RoutingTableList->dstAddress );  // Destination Address
    *pBuf++ = HI_UINT16( RoutingTableList->dstAddress );

    *pBuf = (RoutingTableList->status & 0x07);
    if ( RoutingTableList->options & (ZP_MTO_ROUTE_RC | ZP_MTO_ROUTE_NRC) )
    {
      uint8_t options = 0;
      options |= ZDO_MGMT_RTG_ENTRY_MANYTOONE;

      if ( RoutingTableList->options & ZP_RTG_RECORD )
      {
        options |= ZDO_MGMT_RTG_ENTRY_ROUTE_RECORD_REQUIRED;
      }

      if ( RoutingTableList->options & ZP_MTO_ROUTE_NRC )
      {
        options |= ZDO_MGMT_RTG_ENTRY_MEMORY_CONSTRAINED;
      }

      *pBuf |= (options << 3);
    }
    pBuf++;

    *pBuf++ = LO_UINT16( RoutingTableList->nextHopAddress );  // Next hop
    *pBuf++ = HI_UINT16( RoutingTableList->nextHopAddress );
    RoutingTableList++;    // Move to next list entry
  }

  FillAndSendBuffer( &TransSeq, dstAddr, Mgmt_Rtg_rsp, len, buf );
}

/*********************************************************************
 * @fn          ZDP_MgmtBindRsp
 *
 * @brief       This builds and send a Mgmt_Bind_rsp message. This
 *              function sends a unicast message.
 *
 * @param       dstAddr - destination address of the message
 * @param       Status - message status (ZDP_SUCCESS or other)
 * @param       BindingTableEntries - Total number of entries
 * @param       StartIndex - Starting index within the reporting list
 * @param       BindingTableListCount - number of entries included
 *                                 in this message
 * @param       BindingTableList - List of Binding Table entries
 * @param       SecurityEnable - Security Options
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDP_MgmtBindRsp( byte TransSeq, zAddrType_t *dstAddr,
                            byte Status,
                            byte BindingTableEntries,
                            byte StartIndex,
                            byte BindingTableListCount,
                            apsBindingItem_t *BindingTableList,
                            byte SecurityEnable )
{
  uint8_t *buf;
  uint8_t *pBuf;
  uint8_t maxLen; // maxLen is the maximum packet length to allocate enough memory space
  uint8_t len;    // Actual length varies due to different addrMode
  uint8_t x;
  byte extZdpBindEntrySize = ZDP_BINDINGENTRY_SIZE + 1 + 1; // One more byte for cluserID and DstAddrMode
  byte shortZdpBindEntrySize = ZDP_BINDINGENTRY_SIZE + 1 + 1 + 2 - 8 - 1; // clusterID + DstAddrMode + shortAddr - ExtAddr - DstEndpoint

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  // Status + BindingTableEntries + StartIndex + BindingTableListCount.
  maxLen = 1 + 1 + 1 + 1;
  maxLen += (BindingTableListCount * extZdpBindEntrySize );  //max length
  buf = OsalPort_malloc( maxLen + 1 );  // +1 for transaction ID

  if ( buf == NULL )
  {
    return afStatus_MEM_FAIL;
  }

  pBuf = buf+1;

  *pBuf++ = Status;
  *pBuf++ = BindingTableEntries;
  *pBuf++ = StartIndex;
  *pBuf++ = BindingTableListCount;

  // Initial length = Status + BindingTableEntries + StartIndex + BindingTableListCount.
  // length += ZDP_BINDINGENTRY_SIZE   -- Version 1.0
  //           extZdpBindEntrySize     -- Version 1.1 extended address mode
  //           shortZdpBindEntrySize   -- Version 1.1 group address mode

  len = 1 + 1 + 1 + 1;
  for ( x = 0; x < BindingTableListCount; x++ )
  {
    pBuf = osal_cpyExtAddr( pBuf, BindingTableList->srcAddr );
    *pBuf++ = BindingTableList->srcEP;

    // Cluster ID
    *pBuf++ = LO_UINT16( BindingTableList->clusterID );
    *pBuf++ = HI_UINT16( BindingTableList->clusterID );

    *pBuf++ = BindingTableList->dstAddr.addrMode;
    if ( BindingTableList->dstAddr.addrMode == Addr64Bit )
    {
      len += extZdpBindEntrySize;
      pBuf = osal_cpyExtAddr( pBuf, BindingTableList->dstAddr.addr.extAddr );
      *pBuf++ = BindingTableList->dstEP;
    }
    else
    {
      len += shortZdpBindEntrySize;
      *pBuf++ = LO_UINT16( BindingTableList->dstAddr.addr.shortAddr );
      *pBuf++ = HI_UINT16( BindingTableList->dstAddr.addr.shortAddr );
    }
    BindingTableList++;    // Move to next list entry
  }

  FillAndSendBuffer( &TransSeq, dstAddr, Mgmt_Bind_rsp, len, buf );
}

/*********************************************************************
 * @fn          ZDP_MgmtNwkUpdateNotify
 *
 * @brief       This builds and send a Mgmt_NWK_Update_notify message. This
 *              function sends a unicast message.
 *
 * @param       dstAddr - destination address of the message
 * @param       status - message status (ZDP_SUCCESS or other)
 * @param       scannedChannels - List of channels scanned by the request
 * @param       totalTransmissions - Total transmissions
 * @param       transmissionFailures - Sum of transmission failures
 * @param       listCount - Number of records contained in the energyValues list
 * @param       energyValues - List of descriptors, one for each of ListCount,
 *                             of the enegry detect descriptors
 * @param       txOptions - Transmit options
 * @param       securityEnable - Security options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_MgmtNwkUpdateNotify( uint8_t TransSeq, zAddrType_t *dstAddr,
                                    uint8_t status, uint32_t scannedChannels,
                                    uint16_t totalTransmissions, uint16_t transmissionFailures,
                                    uint8_t listCount, uint8_t *energyValues, uint8_t txOptions,
                                    uint8_t securityEnable )
{
  uint8_t *buf;
  uint8_t *pBuf;
  uint8_t len;

  (void)securityEnable;  // Intentionally unreferenced parameter

  // Status + ScannedChannels + totalTransmissions + transmissionFailures + ListCount + energyValues
  len = 1 + 4 + 2 + 2 + 1 + listCount;

  buf = OsalPort_malloc( len+1 ); // +1 for transaction ID
  if ( buf == NULL )
  {
    return afStatus_MEM_FAIL;
  }

  pBuf = buf+1;

  *pBuf++ = status;

  pBuf = OsalPort_bufferUint32( pBuf, scannedChannels );

  *pBuf++ = LO_UINT16( totalTransmissions );
  *pBuf++ = HI_UINT16( totalTransmissions );

  *pBuf++ = LO_UINT16( transmissionFailures );
  *pBuf++ = HI_UINT16( transmissionFailures );

  *pBuf++ = listCount;

  if ( listCount > 0 )
    OsalPort_memcpy( pBuf, energyValues, listCount );

  FillAndSendBufferTxOptions( &TransSeq, dstAddr, Mgmt_NWK_Update_notify, len, buf, txOptions );
}

/*********************************************************************
 * Functions to register for ZDO Over-the-air messages
 */

/*********************************************************************
 * @fn          ZDO_RegisterForZDOMsg
 *
 * @brief       Call this function to register of an incoming over
 *              the air ZDO message - probably a response message
 *              but requests can also be received.
 *              Messages are delivered to the task with ZDO_CB_MSG
 *              as the message ID.
 *
 * @param       taskID - Where you would like the message delivered
 * @param       clusterID - What message?
 *                          ZDO_ALL_MSGS_CLUSTERID - all responses
 *                          and device announce
 *
 * @return      ZSuccess - successful, ZMemError if not
 */
ZStatus_t ZDO_RegisterForZDOMsg( uint8_t taskID, uint16_t clusterID )
{
  ZDO_MsgCB_t *pList;
  ZDO_MsgCB_t *pLast;
  ZDO_MsgCB_t *pNew;

  // Look for duplicate
  pList = pLast = zdoMsgCBs;
  while ( pList )
  {
    if ( pList->taskID == taskID && pList->clusterID == clusterID )
      return ( ZSuccess );
    pLast = pList;
    pList = (ZDO_MsgCB_t *)pList->next;
  }

  // Add to the list
  pNew = (ZDO_MsgCB_t *)OsalPort_malloc( sizeof ( ZDO_MsgCB_t ) );
  if ( pNew )
  {
    pNew->taskID = taskID;
    pNew->clusterID = clusterID;
    pNew->next = NULL;
    if ( zdoMsgCBs )
    {
      pLast->next = pNew;
    }
    else
      zdoMsgCBs = pNew;
    return ( ZSuccess );
  }
  else
    return ( ZMemError );
}

/*********************************************************************
 * @fn          ZDO_RemoveRegisteredCB
 *
 * @brief       Call this function if you don't want to receive the
 *              incoming message.
 *
 * @param       taskID - Where the messages are being delivered.
 * @param       clusterID - What message?
 *
 * @return      ZSuccess - successful, ZFailure if not found
 */
ZStatus_t ZDO_RemoveRegisteredCB( uint8_t taskID, uint16_t clusterID )
{
  ZDO_MsgCB_t *pList;
  ZDO_MsgCB_t *pLast = NULL;

  pList = zdoMsgCBs;
  while ( pList )
  {
    if ( pList->taskID == taskID && pList->clusterID == clusterID )
    {
      if ( pLast )
      {
        // remove this one from the linked list
        pLast->next = pList->next;
      }
      else if ( pList->next )
      {
        // remove the first one from the linked list
        zdoMsgCBs = pList->next;
      }
      else
      {
        // remove the only item from the list
        zdoMsgCBs = (ZDO_MsgCB_t *)NULL;
      }
      OsalPort_free( pList );
      return ( ZSuccess );
    }
    pLast = pList;
    pList = pList->next;
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn          ZDO_SendMsgCBs
 *
 * @brief       This function sends messages to registered tasks.
 *              Local to ZDO and shouldn't be called outside of ZDO.
 *
 * @param       inMsg - incoming message
 *
 * @return      TRUE if sent to at least 1 task, FALSE if not
 */
uint8_t ZDO_SendMsgCBs( zdoIncomingMsg_t *inMsg )
{
  uint8_t ret = FALSE;
  ZDO_MsgCB_t *pList = zdoMsgCBs;
  while ( pList )
  {
    if ( (pList->clusterID == inMsg->clusterID)
       || ((pList->clusterID == ZDO_ALL_MSGS_CLUSTERID)
           && ((inMsg->clusterID & ZDO_RESPONSE_BIT) || (inMsg->clusterID == Device_annce))) )
    {
      zdoIncomingMsg_t *msgPtr;

      // Send the address to the task
      msgPtr = (zdoIncomingMsg_t *)OsalPort_msgAllocate( sizeof( zdoIncomingMsg_t ) + inMsg->asduLen );
      if ( msgPtr )
      {
        // copy struct
        OsalPort_memcpy( msgPtr, inMsg, sizeof( zdoIncomingMsg_t ));

        if ( inMsg->asduLen )
        {
          msgPtr->asdu = (byte*)(((byte*)msgPtr) + sizeof( zdoIncomingMsg_t ));
          OsalPort_memcpy( msgPtr->asdu, inMsg->asdu, inMsg->asduLen );
        }

        msgPtr->hdr.event = ZDO_CB_MSG;
        OsalPort_msgSend( pList->taskID, (uint8_t *)msgPtr );
        ret = TRUE;
      }
    }
    pList = (ZDO_MsgCB_t *)pList->next;
  }
  return ( ret );
}

/*********************************************************************
 * Incoming message processor
 */

/*********************************************************************
 * @fn          ZDP_IncomingData
 *
 * @brief       This function indicates the transfer of a data PDU (ASDU)
 *              from the APS sub-layer to the ZDO.
 *
 * @param       pData - Incoming Message
 *
 * @return      none
 */
void ZDP_IncomingData( afIncomingMSGPacket_t *pData )
{
  uint8_t x = 0;
  uint8_t handled;
  zdoIncomingMsg_t inMsg;

  inMsg.srcAddr.addrMode = Addr16Bit;
  inMsg.srcAddr.addr.shortAddr = pData->srcAddr.addr.shortAddr;
  inMsg.wasBroadcast = pData->wasBroadcast;
  inMsg.clusterID = pData->clusterId;
  inMsg.SecurityUse = pData->SecurityUse;

  inMsg.asduLen = pData->cmd.DataLength-1;
  inMsg.asdu = pData->cmd.Data+1;
  inMsg.TransSeq = pData->cmd.Data[0];
  inMsg.macDestAddr = pData->macDestAddr;
  inMsg.macSrcAddr = pData->macSrcAddr;

  handled = ZDO_SendMsgCBs( &inMsg );

#if (defined MT_ZDO_CB_FUNC)
#if !defined MT_TASK
  if (zgZdoDirectCB)
#endif
  {
    MT_ZdoDirectCB( pData, &inMsg );
  }
#endif

  while ( zdpMsgProcs[x].clusterID != 0xFFFF )
  {
    if ( zdpMsgProcs[x].clusterID == inMsg.clusterID )
    {
      zdpMsgProcs[x].pFn( &inMsg );
      return;
    }
    x++;
  }

  // Handle unhandled messages
  if ( !handled )
    ZDApp_InMsgCB( &inMsg );
}


#ifdef APP_TP2
/*********************************************************************
 * @fn          ZDP_InvalidCmdReq
 *
 * @brief       This is to send an invalid ZDO cmd for Platform conformance testing.
 *              This has no usage on device products.
 *
 * @param       dstAddr - destination address
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_InvalidCmdReq( zAddrType_t *dstAddr )

{
  uint8_t index;
  for(index = 0; index < ZDO_INVALID_CMD_LEN; index++)
  {
      ZDP_TmpBuf[index] = 0;
  }

  return fillAndSend( &ZDP_SeqNum, dstAddr, ZDO_invalid_cmd_req, ZDO_INVALID_CMD_LEN );
}

#endif

/*********************************************************************
*********************************************************************/

