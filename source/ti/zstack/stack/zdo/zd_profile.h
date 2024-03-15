/**************************************************************************************************
  Filename:       ZDProfile.h
  Revised:        $Date: 2015-02-10 15:42:13 -0800 (Tue, 10 Feb 2015) $
  Revision:       $Revision: 42485 $

  Description:    This file contains the interface to the Zigbee Device Object.


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

#ifndef ZDPROFILE_H
#define ZDPROFILE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcomdef.h"
#include "nl_mede.h"
#include "af.h"
#include "zd_config.h"

/*********************************************************************
 * CONSTANTS
 */

#define ZDO_EP 0   // Endpoint of ZDO
#define ZDO_PROFILE_ID            0
#define ZDO_WILDCARD_PROFILE_ID   0xFFFF

// IEEE_addr_req request types
#define ZDP_ADDR_REQTYPE_SINGLE     0
#define ZDP_ADDR_REQTYPE_EXTENDED   1

// ZDO Status Values
#define ZDP_SUCCESS            0x00  // Operation completed successfully
#define ZDP_INVALID_REQTYPE    0x80  // The supplied request type was invalid
#define ZDP_DEVICE_NOT_FOUND   0x81  // Reserved
#define ZDP_INVALID_EP         0x82  // Invalid endpoint value
#define ZDP_NOT_ACTIVE         0x83  // Endpoint not described by a simple desc.
#define ZDP_NOT_SUPPORTED      0x84  // Optional feature not supported
#define ZDP_TIMEOUT            0x85  // Operation has timed out
#define ZDP_NO_MATCH           0x86  // No match for end device bind
#define ZDP_NO_ENTRY           0x88  // Unbind request failed, no entry
#define ZDP_NO_DESCRIPTOR      0x89  // Child descriptor not available
#define ZDP_INSUFFICIENT_SPACE 0x8a  // Insufficient space to support operation
#define ZDP_NOT_PERMITTED      0x8b  // Not in proper state to support operation
#define ZDP_TABLE_FULL         0x8c  // No table space to support operation
#define ZDP_NOT_AUTHORIZED     0x8d  // Permissions indicate request not authorized
#define ZDP_BINDING_TABLE_FULL 0x8e  // No binding table space to support operation

#define ZDP_NETWORK_DISCRIPTOR_SIZE  8
#define ZDP_NETWORK_EXTENDED_DISCRIPTOR_SIZE  14
#define ZDP_RTG_DISCRIPTOR_SIZE      5
#define ZDP_BIND_DISCRIPTOR_SIZE  19

// Mgmt_Permit_Join_req fields
#define ZDP_MGMT_PERMIT_JOIN_REQ_DURATION 0
#define ZDP_MGMT_PERMIT_JOIN_REQ_TC_SIG   1
#define ZDP_MGMT_PERMIT_JOIN_REQ_SIZE     2

// Mgmt_Leave_req fields
#define ZDP_MGMT_LEAVE_REQ_REJOIN      1 << 7
#define ZDP_MGMT_LEAVE_REQ_RC          1 << 6   // Remove Children

// Mgmt_Lqi_rsp - (neighbor table) device type
#define ZDP_MGMT_DT_COORD  0x0
#define ZDP_MGMT_DT_ROUTER 0x1
#define ZDP_MGMT_DT_ENDDEV 0x2

// Mgmt_Lqi_rsp - (neighbor table) relationship
#define ZDP_MGMT_REL_PARENT  0x0
#define ZDP_MGMT_REL_CHILD   0x1
#define ZDP_MGMT_REL_SIBLING 0x2
#define ZDP_MGMT_REL_UNKNOWN 0x3

// Mgmt_Lqi_rsp - (neighbor table) unknown boolean value
#define ZDP_MGMT_BOOL_RECEIVER_OFF 0x00
#define ZDP_MGMT_BOOL_RECEIVER_ON  0x01
#define ZDP_MGMT_BOOL_UNKNOWN      0x02

// Status fields used by ZDO_ProcessMgmtRtgReq
#define ZDO_MGMT_RTG_ENTRY_ACTIVE                 0x00
#define ZDO_MGMT_RTG_ENTRY_DISCOVERY_UNDERWAY     0x01
#define ZDO_MGMT_RTG_ENTRY_DISCOVERY_FAILED       0x02
#define ZDO_MGMT_RTG_ENTRY_INACTIVE               0x03

#define ZDO_MGMT_RTG_ENTRY_MEMORY_CONSTRAINED     0x01
#define ZDO_MGMT_RTG_ENTRY_MANYTOONE              0x02
#define ZDO_MGMT_RTG_ENTRY_ROUTE_RECORD_REQUIRED  0x04

#ifdef APP_TP2
//ZDO cmd for testing
#define ZDO_INVALID_CMD_LEN         10
#endif

/*********************************************************************
 * Message/Cluster IDS
 */

// ZDO Cluster IDs
#define ZDO_RESPONSE_BIT_V1_0   ((uint8_t)0x80)
#define ZDO_RESPONSE_BIT        ((uint16_t)0x8000)

#define NWK_addr_req            ((uint16_t)0x0000)
#define IEEE_addr_req           ((uint16_t)0x0001)
#define Node_Desc_req           ((uint16_t)0x0002)
#define Power_Desc_req          ((uint16_t)0x0003)
#define Simple_Desc_req         ((uint16_t)0x0004)
#define Active_EP_req           ((uint16_t)0x0005)
#define Match_Desc_req          ((uint16_t)0x0006)
#define NWK_addr_rsp            (NWK_addr_req | ZDO_RESPONSE_BIT)
#define IEEE_addr_rsp           (IEEE_addr_req | ZDO_RESPONSE_BIT)
#define Node_Desc_rsp           (Node_Desc_req | ZDO_RESPONSE_BIT)
#define Power_Desc_rsp          (Power_Desc_req | ZDO_RESPONSE_BIT)
#define Simple_Desc_rsp         (Simple_Desc_req | ZDO_RESPONSE_BIT)
#define Active_EP_rsp           (Active_EP_req | ZDO_RESPONSE_BIT)
#define Match_Desc_rsp          (Match_Desc_req | ZDO_RESPONSE_BIT)

#define Complex_Desc_req        ((uint16_t)0x0010)
#define User_Desc_req           ((uint16_t)0x0011)
#define Discovery_Cache_req     ((uint16_t)0x0012)
#define Device_annce            ((uint16_t)0x0013)
#define User_Desc_set           ((uint16_t)0x0014)
#define Server_Discovery_req    ((uint16_t)0x0015)
#define Parent_annce            ((uint16_t)0x001F)
#define Complex_Desc_rsp        (Complex_Desc_req | ZDO_RESPONSE_BIT)
#define User_Desc_rsp           (User_Desc_req | ZDO_RESPONSE_BIT)
#define Discovery_Cache_rsp     (Discovery_Cache_req | ZDO_RESPONSE_BIT)
#define User_Desc_conf          (User_Desc_set | ZDO_RESPONSE_BIT)
#define Server_Discovery_rsp    (Server_Discovery_req | ZDO_RESPONSE_BIT)
#define Parent_annce_rsp        (Parent_annce | ZDO_RESPONSE_BIT)

#define End_Device_Bind_req     ((uint16_t)0x0020)
#define Bind_req                ((uint16_t)0x0021)
#define Unbind_req              ((uint16_t)0x0022)
#define Bind_rsp                (Bind_req | ZDO_RESPONSE_BIT)
#define End_Device_Bind_rsp     (End_Device_Bind_req | ZDO_RESPONSE_BIT)
#define Unbind_rsp              (Unbind_req | ZDO_RESPONSE_BIT)

#define Mgmt_NWK_Disc_req       ((uint16_t)0x0030)
#define Mgmt_Lqi_req            ((uint16_t)0x0031)
#define Mgmt_Rtg_req            ((uint16_t)0x0032)
#define Mgmt_Bind_req           ((uint16_t)0x0033)
#define Mgmt_Leave_req          ((uint16_t)0x0034)
#define Mgmt_Direct_Join_req    ((uint16_t)0x0035)
#define Mgmt_Permit_Join_req    ((uint16_t)0x0036)
#define Mgmt_NWK_Update_req     ((uint16_t)0x0038)
#define Mgmt_NWK_Disc_rsp       (Mgmt_NWK_Disc_req | ZDO_RESPONSE_BIT)
#define Mgmt_Lqi_rsp            (Mgmt_Lqi_req | ZDO_RESPONSE_BIT)
#define Mgmt_Rtg_rsp            (Mgmt_Rtg_req | ZDO_RESPONSE_BIT)
#define Mgmt_Bind_rsp           (Mgmt_Bind_req | ZDO_RESPONSE_BIT)
#define Mgmt_Leave_rsp          (Mgmt_Leave_req | ZDO_RESPONSE_BIT)
#define Mgmt_Direct_Join_rsp    (Mgmt_Direct_Join_req | ZDO_RESPONSE_BIT)
#define Mgmt_Permit_Join_rsp    (Mgmt_Permit_Join_req | ZDO_RESPONSE_BIT)
#define Mgmt_NWK_Update_notify  (Mgmt_NWK_Update_req | ZDO_RESPONSE_BIT)

#ifdef APP_TP2
#define ZDO_invalid_cmd_req      ((uint16_t)0x00F0)
#endif

#define ZDO_ALL_MSGS_CLUSTERID  0xFFFF

/*********************************************************************
 * TYPEDEFS
 */

#define ZDP_BINDINGENTRY_SIZE   19

typedef struct
{
  OsalPort_EventHdr hdr;
  zAddrType_t      srcAddr;
  uint8_t            wasBroadcast;
  cId_t            clusterID;
  uint8_t            SecurityUse;
  uint8_t            TransSeq;
  uint8_t            asduLen;
  uint16_t           macDestAddr;
  uint8_t            *asdu;
  uint16_t           macSrcAddr;
} zdoIncomingMsg_t;

// This structure is used to build the Mgmt Network Discover Response
typedef struct
{
  uint8_t extendedPANID[Z_EXTADDR_LEN];   // The extended PAN ID
  uint16_t PANId;            // The network PAN ID
  uint8_t   logicalChannel;  // Network's channel
  uint8_t   stackProfile;    // Network's profile
  uint8_t   version;         // Network's Zigbee version
  uint8_t   beaconOrder;     // Beacon Order
  uint8_t   superFrameOrder;
  uint8_t   permitJoining;   // PermitJoining. 1 or 0
} mgmtNwkDiscItem_t;

// This structure is used to build the Mgmt LQI Response
typedef struct
{
  uint16_t nwkAddr;         // device's short address
  uint16_t PANId;           // The neighbor device's PAN ID
  uint8_t  extPANId[Z_EXTADDR_LEN]; // The neighbor device's Extended PanID
  uint8_t   txQuality;       // Transmit quality
  uint8_t   rxLqi;           // Receive LQI
} neighborLqiItem_t;
#define ZDP_NEIGHBORLQI_SIZE    12

// This structure is used to build the Mgmt_Lqi_rsp
typedef struct
{
  uint16_t panID;                  // PAN Id
  uint8_t  extPanID[Z_EXTADDR_LEN];// Extended Pan ID
  uint8_t  extAddr[Z_EXTADDR_LEN]; // Extended address
  uint16_t nwkAddr;                // Network address
  uint8_t  devType;                // Device type
  uint8_t  rxOnIdle;               // RxOnWhenIdle
  uint8_t  relation;               // Relationship
  uint8_t  permit;                 // Permit joining
  uint8_t  depth;                  // Depth
  uint8_t  lqi;                    // LQI
} ZDP_MgmtLqiItem_t;
// devType, rxOnIdle, relation, are all packed into 1 byte: 18-2=16.
#define ZDP_MGMTLQI_SIZE           15
#define ZDP_MGMTLQI_EXTENDED_SIZE  22   // One extra byte for permitJointing, also with extended PanID instead of PanID 15+8-2+1 = 22

// This structure is used to build the Mgmt Routing Response
// NOTICE: this structure must match "rtgEntry_t" in rtg.h
typedef struct
{
  uint16_t dstAddress;     // Destination short address
  uint16_t nextHopAddress; // next hop short address
  uint8_t  expiryTime;     // expiration time - not used for response
  uint8_t  status;         // route entry status
  uint8_t  options;
} rtgItem_t;
// expiryTime is not packed & sent OTA.
#define ZDP_ROUTINGENTRY_SIZE   5

typedef struct
{
  uint8_t  TransSeq;
  byte SecurityUse;
  uint16_t srcAddr;
  uint16_t localCoordinator;
  uint8_t  ieeeAddr[Z_EXTADDR_LEN];
  uint8_t  endpoint;
  uint16_t profileID;
  uint8_t  numInClusters;
  uint16_t *inClusters;
  uint8_t  numOutClusters;
  uint16_t *outClusters;
} ZDEndDeviceBind_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

extern byte ZDP_SeqNum;
extern byte ZDP_TxOptions;

/*********************************************************************
 * MACROS
 */

/*
 * Generic data send function
 */
extern afStatus_t ZDP_SendData( uint8_t *transSeq, zAddrType_t *dstAddr, uint16_t cmd, byte len,
                                              uint8_t *buf, byte SecurityEnable );

/*
 * ZDP_NodeDescReq - Request a Node Description
 *
 * @MT SPI_CMD_ZDO_NODE_DESC_REQ
 * (uint16_t DstAddr,
 *  uint16_t NWKAddrOfInterest,
 *  byte SecuritySuite)
 *
 */
#define ZDP_NodeDescReq( dstAddr, NWKAddrOfInterest, SecurityEnable ) \
                          ZDP_NWKAddrOfInterestReq(  dstAddr, \
                              NWKAddrOfInterest, Node_Desc_req, SecurityEnable )

/*
 * ZDP_PowerDescReq - Request a Power Description
 *
 * @MT  SPI_CMD_ZDO_POWER_DESC_REQ
 * (uint16_t DstAddr,
 *  uint16_t NWKAddrOfInterest,
 *  byte SecuritySuit)
 *
 */
#define ZDP_PowerDescReq( dstAddr, NWKAddrOfInterest, SecurityEnable ) \
                          ZDP_NWKAddrOfInterestReq(  dstAddr, \
                              NWKAddrOfInterest, Power_Desc_req, SecurityEnable )

/*
 * ZDP_ActiveEPReq - Request a device's endpoint list
 *
 * @MT SPI_CMD_ZDO_ACTIVE_EPINT_REQ
 * (uint16_t DstAddr,
 *  uint16_t NWKAddrOfInterest,
 *  byte SecuritySuite)
 *
 */
#define ZDP_ActiveEPReq( dstAddr, NWKAddrOfInterest, SecurityEnable ) \
                          ZDP_NWKAddrOfInterestReq(  dstAddr, \
                            NWKAddrOfInterest, Active_EP_req, SecurityEnable )

/*
 * ZDP_ComplexDescReq - Request a device's complex description
 *
 * @MT SPI_CMD_ZDO_COMPLEX_DESC_REQ
 * (uint16_t DstAddr,
 *  uint16_t NWKAddrOfInterest,
 *  byte SecuritySuite)
 *
 */
#define ZDP_ComplexDescReq( dstAddr, NWKAddrOfInterest, SecurityEnable ) \
                          ZDP_NWKAddrOfInterestReq(  dstAddr, \
                            NWKAddrOfInterest, Complex_Desc_req, SecurityEnable )

/*
 * ZDP_UserDescReq - Request a device's user description
 *
 * @MT SPI_CMD_ZDO_USER_DESC_REQ
 * (uint16_t DstAddr,
 *  uint16_t NWKAddrOfInterest,
 *  byte SecuritySuite)
 *
 */
#define ZDP_UserDescReq( dstAddr, NWKAddrOfInterest, SecurityEnable ) \
                          ZDP_NWKAddrOfInterestReq(  dstAddr, \
                            NWKAddrOfInterest, User_Desc_req, SecurityEnable )

/*
 * ZDP_BindReq - bind request
 *
 * @MT SPI_CMD_ZDO_BIND_REQ
 * (uint16_t DstAddr,
 *  UInt64 SrcAddress,
 *  byte SrcEndpoint,
 *  uint16_t ClusterID,
 *  zAddrType_t *DstAddress,
 *  byte DstEndpoint,
 *  byte SecuritySuite)
 *
 */
#define ZDP_BindReq( dstAddr, SourceAddr, SrcEP, \
              ClusterID, DestinationAddr, DstEP, SecurityEnable ) \
                       ZDP_BindUnbindReq( Bind_req, dstAddr, \
                            SourceAddr, SrcEP, ClusterID, \
                            DestinationAddr, DstEP, SecurityEnable )

/*
 * ZDP_UnbindReq - Unbind request
 *
 * @MT SPI_CMD_ZDO_UNBIND_REQ
 * (uint16_t DstAddr,
 *  UInt64 SrcAddress,
 *  byte SrcEndpoint,
 *  uint16_t ClusterID,
 *  zAddrType_t DestinationAddr,
 *  byte DstEndpoint,
 *  byte SecuritySuite)
 *
 */
#define ZDP_UnbindReq( dstAddr, SourceAddr, SrcEP, \
              ClusterID, DestinationAddr, DstEP, SecurityEnable ) \
                       ZDP_BindUnbindReq( Unbind_req, dstAddr, \
                            SourceAddr, SrcEP, ClusterID, \
                            DestinationAddr, DstEP, SecurityEnable )

/*
 * ZDP_MgmtLqiReq - Send a Management LQI Request
 *
 * @MT SPI_CMD_ZDO_MGMT_LQI_REQ
 * (uint16_t DstAddr,
 *  byte StartIndex)
 *
 */
#define ZDP_MgmtLqiReq( dstAddr, StartIndex, SecurityEnable ) \
          ZDP_SendData( &ZDP_SeqNum, dstAddr, Mgmt_Lqi_req, 1, &StartIndex, SecurityEnable )

/*
 * ZDP_MgmtRtgReq - Send a Management Routing Table Request
 *
 * @MT SPI_CMD_ZDO_MGMT_RTG_REQ
 * (uint16_t DstAddr,
 *  byte StartIndex)
 *
 */
#define ZDP_MgmtRtgReq( dstAddr, StartIndex, SecurityEnable ) \
          ZDP_SendData( &ZDP_SeqNum, dstAddr, Mgmt_Rtg_req, 1, &StartIndex, SecurityEnable )

/*
 * ZDP_MgmtBindReq - Send a Management Binding Table Request
 *
 * @MT SPI_CMD_ZDO_MGMT_BIND_REQ
 * (uint16_t DstAddr,
 *  byte StartIndex)
 *
 */
#define ZDP_MgmtBindReq( dstAddr, StartIndex, SecurityEnable ) \
         ZDP_SendData( &ZDP_SeqNum, dstAddr, Mgmt_Bind_req, 1, &StartIndex, SecurityEnable )

/*
 * ZDP_ParentAnnceReq - Send a ParentAnnce Request
 */
#define ZDP_ParentAnnceReq( dstAddr, numberOfChildren, childInfo, SecurityEnable ) \
           ZDP_ParentAnnce( &ZDP_SeqNum, &dstAddr, numberOfChildren, childInfo, \
                            Parent_annce, SecurityEnable )

/*
 * ZDP_ActiveEPRsp - Send an list of active endpoint
 */
#define ZDP_ActiveEPRsp( TransSeq, dstAddr, Status, nwkAddr, Count, \
                  pEPList, SecurityEnable ) \
                      ZDP_EPRsp( Active_EP_rsp, TransSeq, dstAddr, Status, \
                           nwkAddr, Count, pEPList, SecurityEnable )

/*
 * ZDP_MatchDescRsp - Send an list of endpoint that match
 */
#define ZDP_MatchDescRsp( TransSeq, dstAddr, Status, nwkAddr, Count, \
                  pEPList, SecurityEnable ) \
                      ZDP_EPRsp( Match_Desc_rsp, TransSeq, dstAddr, Status, \
                           nwkAddr, Count, pEPList, SecurityEnable )

/*
 * ZDP_ComplexDescRsp - This message isn't supported until we fix it.
 */
#define ZDP_ComplexDescRsp( dstAddr, SecurityEnable ) \
        ZDP_GenericRsp( dstAddr, Complex_Desc_rsp, SecurityEnable )

/*
 * ZDP_UserDescConf - Send a User Descriptor Set Response
 */
#define ZDP_UserDescConf( TransSeq, dstAddr, Status, SecurityEnable ) \
            ZDP_SendData( &TransSeq, dstAddr, User_Desc_conf, 1, &Status, SecurityEnable )

/*
 * ZDP_EndDeviceBindRsp - Send a End Device Bind Response
 */
#define ZDP_EndDeviceBindRsp( TransSeq, dstAddr, Status, SecurityEnable ) \
       ZDP_SendData( &TransSeq, dstAddr, End_Device_Bind_rsp, 1, &Status, SecurityEnable )

/*
 * ZDP_BindRsp - Send a Bind Response
 */
#define ZDP_BindRsp( TransSeq, dstAddr, Status, SecurityEnable ) \
                  ZDP_SendData( &TransSeq, dstAddr, Bind_rsp, 1, &Status, SecurityEnable )

/*
 * ZDP_UnbindRsp - Send an Unbind Response
 */
#define ZDP_UnbindRsp( TransSeq, dstAddr, Status, SecurityEnable ) \
                ZDP_SendData( &TransSeq, dstAddr, Unbind_rsp, 1, &Status, SecurityEnable )

/*
 * ZDP_MgmtLeaveRsp - Send a Management Leave Response
 */
#define ZDP_MgmtLeaveRsp( TransSeq, dstAddr, Status, SecurityEnable ) \
            ZDP_SendData( &TransSeq, dstAddr, Mgmt_Leave_rsp, 1, &Status, SecurityEnable )

/*
 * ZDP_MgmtPermitJoinRsp - Send a Management Permit Join Response
 */
#define ZDP_MgmtPermitJoinRsp( TransSeq, dstAddr, Status, SecurityEnable ) \
      ZDP_SendData( &TransSeq, dstAddr, Mgmt_Permit_Join_rsp, 1, &Status, SecurityEnable )

/*
 * ZDP_MgmtDirectJoinRsp - Send a Mgmt_DirectJoining_Rsp Response
 */
#define ZDP_MgmtDirectJoinRsp( TransSeq, dstAddr, Status, SecurityEnable ) \
      ZDP_SendData( &TransSeq, dstAddr, Mgmt_Direct_Join_rsp, 1, &Status, SecurityEnable )

/*
 * ZDP_ParentAnnceRsp - Send a ParentAnnceRsp Response
 */
#define ZDP_ParentAnnceRsp( TransSeq, dstAddr, numberOfChildren, childInfo, SecurityEnable ) \
           ZDP_ParentAnnce( &TransSeq, &dstAddr, numberOfChildren, childInfo, \
                            Parent_annce_rsp, SecurityEnable )

/*********************************************************************
 * FUNCTIONS - API
 */

/*
 * ZDP_NWKAddrOfInterestReq - Send request with NWKAddrOfInterest as parameter
 */
extern afStatus_t ZDP_NWKAddrOfInterestReq( zAddrType_t *dstAddr,
                              uint16_t nwkAddr, byte cmd, byte SecurityEnable );
/*
 * ZDP_NwkAddrReq - Request a Network address
 *
 * @MT SPI_CMD_ZDO_NWK_ADDR_REQ
 * (UInt64 IEEEAddress,
 *  byte ReqType,
 *  byte StarIndex,
 *  byte SecurityEnable)
 *
 */
extern afStatus_t ZDP_NwkAddrReq( uint8_t *IEEEAddress, byte ReqType,
                               byte StartIndex, byte SecurityEnable );

/*
 * ZDP_IEEEAddrReq - Request an IEEE address
 *
 * @MT SPI_CMD_ZDO_IEEE_ADDR_REQ
 * (uint16_t shortAddr,
 *  byte ReqType,
 *  byte StartIndex,
 *  byte SecurityEnable)
 *
 */
extern afStatus_t ZDP_IEEEAddrReq( uint16_t shortAddr, byte ReqType,
                                byte StartIndex, byte SecurityEnable );

/*
 * ZDP_MatchDescReq - Request matching device's endpoint list
 *
 * @MT SPI_CMD_ZDO_MATCH_DESC_REQ
 * (uint16_t DstAddr,
 *  uint16_t NWKAddrOfInterest,
 *  uint16_t ProfileID,
 *  byte NumInClusters,
 *  uint16_t InClusterList[15],
 *  byte NumOutClusters,
 *  uint16_t OutClusterList[15],
 *  byte SecuritySuite)
 *
 */
extern afStatus_t ZDP_MatchDescReq( zAddrType_t *dstAddr, uint16_t nwkAddr,
                                uint16_t ProfileID,
                                byte NumInClusters, uint16_t *InClusterList,
                                byte NumOutClusters, uint16_t *OutClusterList,
                                byte SecurityEnable );

/*
 * ZDP_SimpleDescReq - Request Simple Descriptor
 *
 * @MT SPI_CMD_ZDO_SIMPLE_DESC_REQ
 * (uint16_t DstAddr,
 *  uint16_t NWKAddrOfInterest,
 *  byte Endpoint,
 *  byte Security)
 *
 */
extern afStatus_t ZDP_SimpleDescReq( zAddrType_t *dstAddr, uint16_t nwkAddr,
                                    byte ep, byte SecurityEnable );

/*
 * ZDP_UserDescSet - Set the User Descriptor
 *
 * @MT SPI_CMD_ZDO_USER_DESC_SET
 * (uint16_t DstAddr,
 *  uint16_t NWKAddrOfInterest,
 *  byte DescLen,
 *  byte Descriptor[15],
 *  byte SecuritySuite)
 *
 */
extern afStatus_t ZDP_UserDescSet( zAddrType_t *dstAddr, uint16_t nwkAddr,
                          UserDescriptorFormat_t *UserDescriptor,
                          byte SecurityEnable );

/*
 * ZDP_ServerDiscReq - Build and send a Server_Discovery_req request message.
 */
afStatus_t ZDP_ServerDiscReq( uint16_t serverMask, byte SecurityEnable );

/*
 * ZDP_DeviceAnnce - Device Announce
 *
 * @MT SPI_CMD_ZDO_DEV_ANNCE
 * (uint16_t DevAddr,
 *  byte DeviceAddress,
 *  byte SecuritySuite)
 *
 */
extern afStatus_t ZDP_DeviceAnnce( uint16_t nwkAddr, uint8_t *IEEEAddr,
                         byte capabilities, byte SecurityEnable );

/*
 * ZDP_ParentAnnce - Parent Announce and Parent Announce Rsp
 */
extern afStatus_t ZDP_ParentAnnce( uint8_t *TransSeq,
                                   zAddrType_t *dstAddr,
                                   uint8_t numberOfChildren,
                                   uint8_t *childInfo,
                                   cId_t clusterID,
                                   uint8_t SecurityEnable );

/*
 * ZDP_EndDeviceBindReq - End Device (hand) bind request
 *
 * @MT SPI_CMD_ZDO_END_DEV_BIND_REQ
 * (uint16_t DstAddr,
 *  uint16_t LocalCoordinator,
 *  byte Endpoint,
 *  uint16_t Profile,
 *  byte NumInClusters,
 *  uint16_t InClusterList[15],
 *  byte NumOutClusters,
 *  uint16_t OutClusterList[15],
 *  byte SecuritySuite)
 *
 */
extern afStatus_t ZDP_EndDeviceBindReq( zAddrType_t *dstAddr,
                              uint16_t LocalCoordinator,
                              byte ep,
                              uint16_t ProfileID,
                              byte NumInClusters, uint16_t *InClusterList,
                              byte NumOutClusters, uint16_t *OutClusterList,
                              byte SecurityEnable );

/*
 * ZDP_BindUnbindReq - bind request
 */
extern afStatus_t ZDP_BindUnbindReq( uint16_t BindOrUnbind, zAddrType_t *dstAddr,
                            uint8_t *SourceAddr, byte SrcEP,
                            cId_t  ClusterID,
                            zAddrType_t *DestinationAddr, byte DstEP,
                            byte SecurityEnable );

/*
 * ZDP_MgmtNwkDiscReq - Send a Management Network Discovery Request
 *
 * @MT SPI_CMD_ZDO_MGMT_NWKDISC_REQ
 * (uint16_t DstAddr,
 *  uint32_t ScanChannels,
 *  byte StartIndex)
 *
 */
extern afStatus_t ZDP_MgmtNwkDiscReq( zAddrType_t *dstAddr,
                            uint32_t ScanChannels,
                            byte ScanDuration,
                            byte StartIndex,
                            byte SecurityEnable );

/*
 * ZDP_MgmtDirectJoinReq - Send a Management Direct Join Request
 *
 * @MT SPI_CMD_ZDO_MGMT_DIRECT_JOIN_REQ
 * (uint16_t DstAddr,
 *  UInt64 DeviceAddress,
 *  byte CapInfo)
 *
 */
extern afStatus_t ZDP_MgmtDirectJoinReq( zAddrType_t *dstAddr,
                               uint8_t *deviceAddr,
                               byte capInfo,
                               byte SecurityEnable );

/*
 * ZDP_MgmtLeaveReq - Send a Management Leave Request
 *
 * @MT SPI_CMD_ZDO_MGMT_LEAVE_REQ
 * (uint16_t DstAddr,
 *  UInt64 DeviceAddress
 *  uint8_t RemoveChildren
 *  uint8_t Rejoin
 *  uint8_t SecurityEnable)
 */
extern afStatus_t ZDP_MgmtLeaveReq( zAddrType_t *dstAddr,
                                   uint8_t *IEEEAddr,
                                   uint8_t RemoveChildren,
                                   uint8_t Rejoin,
                                   uint8_t SecurityEnable );
/*
 * ZDP_MgmtPermitJoinReq - Send a Management Permit Join Request
 *
 * @MT SPI_CMD_ZDO_MGMT_PERMIT_JOIN_REQ
 * (uint16_t DstAddr,
 *  byte duration,
 *  byte TcSignificance)
 *
 */
extern afStatus_t ZDP_MgmtPermitJoinReq( zAddrType_t *dstAddr,
                               byte duration,
                               byte TcSignificance,
                               byte SecurityEnable );

/*
 * ZDP_MgmtNwkUpdateReq - Send a Management NWK Update Request
 *
 * @MT SPI_CMD_ZDO_MGMT_NWK_UPDATE_REQ
 * (uint16_t dstAddr,
 *  uint32_t ChannelMask,
 *  uint8_t ScanDuration,
 *  uint8_t ScanCount,
 *  uint16_t NwkManagerAddr )
 *
 */
extern afStatus_t ZDP_MgmtNwkUpdateReq( zAddrType_t *dstAddr,
                                        uint32_t ChannelMask,
                                        uint8_t ScanDuration,
                                        uint8_t ScanCount,
                                        uint8_t NwkUpdateId,
                                        uint16_t NwkManagerAddr );

/*********************************************************************
 * @fn      ZDP_AddrRsp
 *
 * @brief   Build and send a NWK_addr_rsp or IEEE_addr_rsp message.
 *
 * @param   cId - ClusterID of the rsp, either NWK_addr_rsp or IEEE_addr_rsp.
 * @param   seq - Message sequence number of the corresponding request.
 * @param   dst - Destination address for the response.
 * @param   stat - Response status: ZDP_SUCCESS or other value from ZDProfile.h
 * @param   ieee - 64-bit IEEE address of the response.
 * @param   reqType - Type of response requested (single, extended, etc.)
 * @param   nwkAddr - 16-bit network address of the response.
 * @param   devCnt  - Number of associated devices in the device address list.
 * @param   strtIdx - Starting index into the dev addr array if extended rsp.
 * @param   devAddr - Array of 16-bit network addresses of the associated devs.
 * @param   secOpt  - Security Enable Options.
 *
 * @return  afStatus_t
 */
afStatus_t ZDP_AddrRsp( byte cId, byte seq, zAddrType_t *dst, byte stat,
  uint8_t *ieee, byte reqType, uint16_t nwkAddr, byte devCnt, byte strtIdx,
  uint16_t *devAddr, byte secOpt );

/*
 * ZDP_NodeDescMsg - Send a Node Descriptor message.
 */
extern afStatus_t ZDP_NodeDescMsg( zdoIncomingMsg_t *inMsg,
                    uint16_t nwkAddr, NodeDescriptorFormat_t *pNodeDesc );

/*
 * ZDP_PowerDescMsg - Send a Node Power Descriptor message.
 */
extern afStatus_t ZDP_PowerDescMsg( zdoIncomingMsg_t *inMsg,
 uint16_t nwkAddr, NodePowerDescriptorFormat_t *pPowerDesc );

/*
 * ZDP_SimpleDescMsg - Send a Simple Descriptor message.
 */
extern afStatus_t ZDP_SimpleDescMsg( zdoIncomingMsg_t *inMsg,
                     byte Status, SimpleDescriptionFormat_t *pSimpleDesc );

/*
 * ZDP_EPRsp - Send a list of endpoint
 */
extern afStatus_t ZDP_EPRsp( uint16_t MsgType, byte TransSeq, zAddrType_t *dstAddr, byte Status,
                                uint16_t nwkAddr, byte Count, uint8_t *pEPList,
                                byte SecurityEnable );

/*
 * ZDP_GenericRsp - Sends a response message with only the parameter response
 *                                     byte and the addr of interest for data.
 */
extern afStatus_t ZDP_GenericRsp( byte TransSeq, zAddrType_t *dstAddr,
                    byte status, uint16_t aoi, uint16_t rspID, byte SecurityEnable );

/*
 * ZDP_MgmtNwkDiscRsp - Sends the Management Network Discovery Response.
 */
extern afStatus_t ZDP_MgmtNwkDiscRsp( byte TransSeq, zAddrType_t *dstAddr,
                            byte Status,
                            byte NetworkCount,
                            byte StartIndex,
                            byte NetworkCountList,
                            networkDesc_t *NetworkList,
                            byte SecurityEnable );

/*
 * ZDP_MgmtLqiRsp - Sends the Management LQI Response.
 */
extern ZStatus_t ZDP_MgmtLqiRsp( byte TransSeq, zAddrType_t *dstAddr,
                          byte Status,
                          byte NeighborLqiEntries,
                          byte StartIndex,
                          byte NeighborLqiCount,
                          ZDP_MgmtLqiItem_t* NeighborList,
                          byte SecurityEnable );

/*
 * ZDP_MgmtRtgRsp - Sends the Management Routing Response.
 */
extern ZStatus_t ZDP_MgmtRtgRsp( byte TransSeq, zAddrType_t *dstAddr,
                            byte Status,
                            byte RoutingTableEntries,
                            byte StartIndex,
                            byte RoutingListCount,
                            rtgItem_t *RoutingTableList,
                            byte SecurityEnable );

/*
 * ZDP_MgmtBindRsp - Sends the Management Binding Response.
 */
extern ZStatus_t ZDP_MgmtBindRsp( byte TransSeq, zAddrType_t *dstAddr,
                            byte Status,
                            byte BindingTableEntries,
                            byte StartIndex,
                            byte BindingTableListCount,
                            apsBindingItem_t *BindingTableList,
                            byte SecurityEnable );
/*
 * ZDP_MgmtNwkUpdateNotify - Sends the Management Netwotk Update Notify.
 */
extern afStatus_t ZDP_MgmtNwkUpdateNotify( uint8_t TransSeq, zAddrType_t *dstAddr,
                                    uint8_t status, uint32_t scannedChannels,
                                    uint16_t totalTransmissions, uint16_t transmissionFailures,
                                    uint8_t listCount, uint8_t *energyValues, uint8_t txOptions,
                                    uint8_t securityEnable );

/*
 * ZDP_UserDescRsp - Sends the user descriptor response message.
 */
extern ZStatus_t ZDP_UserDescRsp( byte TransSeq, zAddrType_t *dstAddr,
                uint16_t nwkAddrOfInterest, UserDescriptorFormat_t *userDesc,
                byte SecurityEnable );

/*
 * ZDP_ServerDiscRsp - Build and send the User Decriptor Response.
 */
ZStatus_t ZDP_ServerDiscRsp( byte transID, zAddrType_t *dstAddr, byte status,
                           uint16_t aoi, uint16_t serverMask, byte SecurityEnable );

/*
 * ZDP_IncomingData - Incoming data callback from AF layer
 */
extern void ZDP_IncomingData( afIncomingMSGPacket_t *pData );

extern ZStatus_t ZDO_RegisterForZDOMsg( uint8_t taskID, uint16_t clusterID );
extern ZStatus_t ZDO_RemoveRegisteredCB( uint8_t taskID, uint16_t clusterID );


#ifdef APP_TP2

/*
 * ZDP_InvalidCmdReq - Send Invalid Cmd. For testing only
 *
 * (zAddrType_t DstAddr)
 *
 */
extern afStatus_t ZDP_InvalidCmdReq( zAddrType_t *dstAddr );
#endif

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZDPROFILE_H */
