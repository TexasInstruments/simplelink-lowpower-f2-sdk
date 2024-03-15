/**************************************************************************************************
  Filename:       aps_mede.h
  Revised:        $Date: 2015-06-02 15:55:43 -0700 (Tue, 02 Jun 2015) $
  Revision:       $Revision: 43961 $

  Description:    Primitives of the Application Support Sub Layer Data Entity (APSDE) and
                  Management Entity (APSME).


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

#ifndef APSMEDE_H
#define APSMEDE_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * INCLUDES
 */
#include "zcomdef.h"
#include "nwk_globals.h"
#include "assoc_list.h"
#include "nwk_bufs.h"
#include "binding_table.h"
#include "ssp.h"

/******************************************************************************
 * MACROS
 */

/******************************************************************************
 * CONSTANTS
 */
// Frame control fields
#define APS_FRAME_TYPE_MASK         0x03
#define APS_DATA_FRAME              0x00
#define APS_CMD_FRAME               0x01
#define APS_ACK_FRAME               0x02
#define STUB_APS_FRAME              0x03

#define APS_DELIVERYMODE_MASK       0x0C
#define APS_FC_DM_UNICAST           0x00
#define APS_FC_DM_INDIRECT          0x04
#define APS_FC_DM_BROADCAST         0x08
#define APS_FC_DM_GROUP             0x0C

#define APS_FC_ACK_FORMAT           0x10
#define APS_FC_SECURITY             0x20
#define APS_FC_ACK_REQ              0x40
#define APS_FC_EXTENDED             0x80

#define APS_XFC_FRAG_MASK           0x03
#define APS_XFC_FIRST_FRAG          0x01
#define APS_XFC_FRAGMENT            0x02
#define APS_XFC_RESERVED            0xFC

#define APS_FRAME_CTRL_FIELD_LEN     0x01
#define APS_DSTEP_ID_FIELD_LEN       0x01
#define APS_GROUP_ID_FIELD_LEN       0x02
#define APS_SRCEP_ID_FIELD_LEN       0x01
#define APS_CLUSTERID_FIELD_LEN_V1_0 0x01
#define APS_CLUSTERID_FIELD_LEN      0x02
#define APS_PROFILEID_FIELD_LEN      0x02
#define APS_FRAME_CNT_FIELD_LEN      0x01
#define APS_XFRAME_CTRL_FIELD_LEN    0x01
#define APS_BLOCK_CNT_FIELD_LEN      0x01
#define APS_ACK_BITS_FIELD_LEN       0x01

// Tx Options (bitmap values)
#define APS_TX_OPTIONS_SECURITY_ENABLE  0x0001u
//#define APS_TX_OPTIONS_USE_NWK_KEY    0x0002u remove from spec
#define APS_TX_OPTIONS_ACK              0x0004u
#define APS_TX_OPTIONS_PERMIT_FRAGMENT  0x0008u
#define APS_TX_OPTIONS_SKIP_ROUTING     0x0010u
#define APS_TX_OPTIONS_FIRST_FRAGMENT   0x0020u
#define APS_TX_OPTIONS_PREPROCESS       0x0040u
#define APS_TX_OPTIONS_RETRY_MSG        0x0080u
#define APS_TX_OPTIONS_REFLECTED_MSG    0x0100u

// APSDE header fields
#define APS_HDR_FC 0

// APSME CMD id index
#define APSME_CMD_ID 0

// APS commands
#define APSME_CMD_TRANSPORT_KEY    0x05
#define APSME_CMD_UPDATE_DEVICE    0x06
#define APSME_CMD_REMOVE_DEVICE    0x07
#define APSME_CMD_REQUEST_KEY      0x08
#define APSME_CMD_SWITCH_KEY       0x09
#define APSME_CMD_TUNNEL           0x0E
#define APSME_CMD_VERIFY_KEY       0x0F
#define APSME_CMD_CONFIRM_KEY      0x10

// APSME CMD packet fields (APSME_CMD_TRANSPORT_KEY)
#define APSME_TK_KEY_TYPE      1
#define APSME_TK_KEY           2
#define APSME_TK_COMMON_LEN    (uint8_t)         \
                               (APSME_TK_KEY + \
                                SEC_KEY_LEN   )
#define APSME_TK_KEY_SEQ_LEN   1
#define APSME_TK_INITIATOR_LEN 1

#define APSME_TK_TC_DST_ADDR  18
#define APSME_TK_TC_SRC_ADDR  26
#define APSME_TK_TC_KEY_LEN   34

#define APSME_TK_NWK_KEY_SEQ      18
#define APSME_TK_NWK_DST_ADDR     19
#define APSME_TK_NWK_SRC_ADDR     27
#define APSME_TK_NWK_KEY_LEN      35

#define APSME_TK_APP_PARTNER_ADDR 18
#define APSME_TK_APP_INITIATOR    26
#define APSME_TK_APP_KEY_LEN      27

#define APSME_TK_APP_SOURCE_ADDR_OFFSET  42
#define APSME_REQ_KEY_CMD_OFFSET         15



// APSME CMD packet fields (APSME_CMD_UPDATE_DEVICE)
#define APSME_UD_STANDARD_SECURED_REJOIN        0
#define APSME_UD_STANDARD_UNSECURED_JOIN        1
#define APSME_UD_DEVICE_LEFT                    2
#define APSME_UD_STANDARD_TRUST_CENTER_REJOIN   3
#define APSME_UD_HIGH_SECURED_REJOIN            4
#define APSME_UD_HIGH_UNSECURED_JOIN            5
#define APSME_UD_HIGH_UNSECURED_REJOIN          7

#define APSME_UD_EADDR     1
#define APSME_UD_SADDR_LSB 9
#define APSME_UD_SADDR_MSB 10
#define APSME_UD_STATUS    11
#define APSME_UD_LEN       12

// APSME CMD packet fields (APSME_CMD_REMOVE_DEVICE)
#define APSME_RD_LEN   9
#define APSME_RD_EADDR 1

// APSME CMD packet fields (APSME_CMD_REQUEST_KEY)
#define APSME_RK_KEY_TYPE 1
#define APSME_RK_EADDR    2
#define APSME_RK_SYS_LEN  2
#define APSME_RK_APP_LEN  10

// APSME CMD packet fields (APSME_CMD_SWITCH_KEY)
#define APSME_SK_SEQ_NUM 1
#define APSME_SK_LEN     2

// APSME CMD packet fields (APSME_CMD_TUNNEL)
#define APSME_TUNNEL_DEA  1 //destination extended address
//devtag.pro.security.remove
//      APSME_TUNNEL_AUX  9 //auxillary header(obsolete)
#define APSME_TUNNEL_TCMD 9 //tunnelled command
#define APSME_TUNNEL_LEN  9

// APSME CMD packet fields (APSME_CMD_VERIFY_KEY)
#define APSME_VK_KEY_TYPE 1
#define APSME_VK_EADDR    2
#define APSME_VK_HASH     10
#define APSME_VK_APP_LEN  26

// APSME CMD packet fields (APSME_CMD_CONFIRM_KEY)
#define APSME_CK_STATUS   1
#define APSME_CK_KEY_TYPE 2
#define APSME_CK_EADDR    3
#define APSME_CK_APP_LEN  11

// APSME Coordinator/Trust Center NWK addresses
#define APSME_TRUSTCENTER_NWKADDR  NWK_PAN_COORD_ADDR

#if !defined( MAX_APS_FRAMECOUNTER_CHANGES )
  // The number of times the frame counter can change before
  // saving to NV
  #define MAX_APS_FRAMECOUNTER_CHANGES    1000
#endif

#if !defined( MAX_TCLK_FRAMECOUNTER_CHANGES )
  // The number of times the frame counter can change before
  // saving to NV
  #define MAX_TCLK_FRAMECOUNTER_CHANGES    10
#endif

// used for IcIndex entry of the APSME_TCLinkKeyNVEntry_t
#define APSME_IC_SET_MASK   0x80
#define APSME_IC_INDEX_MASK 0x7F

/******************************************************************************
 * TYPEDEFS
 */

// AIB item Ids
typedef enum
{
  apsAddressMap = 0xA0,

  // Proprietary Items
  apsMaxBindingTime,
  apsBindingTable,
  apsNumBindingTableEntries,
  apsUseExtendedPANID,
  apsUseInsecureJoin,
  apsTrustCenterAddress = 0xAB,
  apsMAX_AIB_ITEMS  // Must be the last entry
} ZApsAttributes_t;

// Type of information being queried
typedef enum
{
  NWK_ADDR_LIST,
  EXT_ADDRESS,
  SIMPLE_DESC,
  NODE_DESC,
  POWER_DESC,
  SVC_MATCH
} APSME_query_t;

#define APS_ILLEGAL_DEVICES             0x02

// Structure returned from APSME_GetRequest for apsBindingTable
typedef struct
{
  uint8_t srcAddr[Z_EXTADDR_LEN]; // Src address
  byte srcEP;                   // Endpoint/interface of source device
  uint16_t clusterID;             // Cluster ID
  zAddrType_t dstAddr;          // Destination address
  byte dstEP;                   // Endpoint/interface of dest device
} apsBindingItem_t;

typedef struct
{
  uint8_t FrmCtrl;
  uint8_t XtndFrmCtrl;
  uint8_t DstEndPoint;
  uint8_t SrcEndPoint;
  uint16_t GroupID;
  uint16_t ClusterID;
  uint16_t ProfileID;
  uint16_t macDestAddr;
  uint8_t wasBroadcast;
  uint8_t apsHdrLen;
  uint8_t *asdu;
  uint8_t asduLength;
  uint8_t ApsCounter;
  uint8_t transID;
  uint8_t BlkCount;
  uint8_t AckBits;
  uint16_t macSrcAddr;
} aps_FrameFormat_t;

typedef struct
{
  uint16_t tna; // tunnel network address
  uint8_t* dea; // destination extended address
} APSDE_FrameTunnel_t;

// APS Data Service Primitives
typedef struct
{
  zAddrType_t dstAddr;
  uint8_t       srcEP;
  uint8_t       dstEP;
  uint16_t      dstPanId;
  uint16_t      clusterID;
  uint16_t      profileID;
  uint16_t      asduLen;
  uint8_t*      asdu;
  uint16_t      txOptions;
  uint8_t       transID;
  uint8_t       discoverRoute;
  uint8_t       radiusCounter;
  uint8_t       apsCount;
  uint8_t       blkCount;
  uint8_t       apsRetries;
  uint8_t       nsduHandle;
} APSDE_DataReq_t;

typedef struct
{
  uint16_t dstAddr;
  uint8_t  dstEP;
  uint8_t  srcEP;
  uint8_t  transID;
  uint16_t clusterID;
  uint8_t  status;
} APSDE_DataCnf_t;

typedef struct
{
  uint8_t secure;
  uint8_t addressingMode; // Helps to identify the exact length of the payload.
} APSDE_DataReqMTU_t;

// APS Security Related Primitives
typedef struct
{
  uint16_t               dstAddr;
  uint8_t                keyType;
  uint8_t                keySeqNum;
  uint8_t*               key;
  uint8_t*               extAddr;
  uint8_t                initiator;
  uint8_t                apsSecure;
  uint8_t                nwkSecure;
  APSDE_FrameTunnel_t* tunnel;
} APSME_TransportKeyReq_t;

typedef struct
{
  uint16_t srcAddr;
  uint8_t  keyType;
  uint8_t  keySeqNum;
  uint8_t* key;
  uint8_t* dstExtAddr;
  uint8_t* srcExtAddr;
  uint8_t  initiator;
  uint8_t  secure;
} APSME_TransportKeyInd_t;

typedef struct
{
  uint16_t dstAddr;
  uint16_t devAddr;
  uint8_t* devExtAddr;
  uint8_t  status;
  uint8_t  apsSecure;
} APSME_UpdateDeviceReq_t;

typedef struct
{
  uint16_t srcAddr;
  uint8_t* devExtAddr;
  uint16_t devAddr;
  uint8_t  status;
} APSME_UpdateDeviceInd_t;

typedef struct
{
  uint16_t parentAddr;
  uint8_t* childExtAddr;
  uint8_t  apsSecure;
} APSME_RemoveDeviceReq_t;

typedef struct
{
  uint16_t srcAddr;
  uint8_t* childExtAddr;
} APSME_RemoveDeviceInd_t;

typedef struct
{
  uint8_t  dstAddr;
  uint8_t  keyType;
  uint8_t* partExtAddr;
} APSME_RequestKeyReq_t;

typedef struct
{
  uint16_t srcAddr;
  uint8_t  keyType;
  uint8_t* partExtAddr;
} APSME_RequestKeyInd_t;

typedef struct
{
  uint16_t dstAddr;
  uint8_t  keySeqNum;
  uint8_t  apsSecure;
} APSME_SwitchKeyReq_t;

typedef struct
{
  uint16_t srcAddr;
  uint8_t  keySeqNum;
} APSME_SwitchKeyInd_t;

typedef struct
{
  uint8_t* tcExtAddr;
  uint8_t  keyType;
} APSME_VerifyKeyReq_t;

typedef struct
{
  uint16_t srcAddr;
  uint8_t  keyType;
  uint8_t* partExtAddr;
  uint8_t* receivedInitiatorHashValue;
} APSME_VerifyKeyInd_t;

typedef struct
{
  uint16_t dstAddr;
  uint8_t  status;
  uint8_t* dstExtAddr;
  uint8_t  keyType;
} APSME_ConfirmKeyReq_t;

typedef struct
{
  uint16_t srcAddr;
  uint8_t  status;
  uint8_t* srcExtAddr;
  uint8_t  keyType;
} APSME_ConfirmKeyInd_t;

// APS Incoming Command Packet
typedef struct
{
  OsalPort_EventHdr hdr;
  uint8_t*           asdu;
  uint8_t            asduLen;
  uint8_t            secure;
  uint16_t           nwkAddr;
  uint8_t            nwkSecure;
} APSME_CmdPkt_t;

typedef struct
{
  uint8_t  key[SEC_KEY_LEN];
  uint32_t txFrmCntr;
  uint32_t rxFrmCntr;
} APSME_ApsLinkKeyNVEntry_t;

typedef struct
{
  uint8_t   frmCtrl;
  uint8_t   xtndFrmCtrl;
  uint8_t   srcEP;
  uint8_t   dstEP;
  uint16_t  groupID;
  uint16_t  clusterID;
  uint16_t  profileID;
  uint8_t   asduLen;
  uint8_t*  asdu;
  uint8_t   hdrLen;
  uint8_t   apsCounter;
  uint8_t   transID;
  uint8_t   blkCount;
  uint8_t   ackBits;
} APSDE_FrameData_t;

typedef struct
{
  uint8_t  frmCtrl;
  uint8_t  xtndFrmCtrl;
  uint8_t  srcEP;
  uint8_t  dstEP;
  uint16_t clusterID;
  uint16_t profileID;
  uint8_t  asduLen;
  uint16_t dstAddr;
  uint8_t  transID;
  uint8_t  apsCounter;
} APSDE_StoredFrameData_t;

typedef struct
{
//ZMacDataReq_t     mfd;
  NLDE_FrameData_t  nfd;
  APSDE_FrameData_t afd;
} APSDE_FrameFormat_t;

typedef struct
{
  uint16_t               dstAddr;
  uint8_t                frmCtrl;
  uint8_t                xtndFrmCtrl;
  uint8_t                asduLen;
  uint8_t                nwkSecure;
  APSDE_FrameTunnel_t* tunnel;
} APSDE_FrameAlloc_t;

typedef struct
{
  //input
  APSDE_FrameAlloc_t   fa;

  //output
  APSDE_FrameFormat_t* aff;
  SSP_Info_t*          si;
  uint8_t                status;
} APSDE_FrameBlk_t;

typedef struct
{
  uint32_t txFrmCntr;
  uint32_t rxFrmCntr;
  uint8_t  extAddr[Z_EXTADDR_LEN];
  uint8_t  keyAttributes;
  uint8_t  keyType;
  uint8_t  SeedShift;
  uint8_t  IcIndex;
} APSME_TCLinkKeyNVEntry_t;

typedef struct
{
  uint32_t txFrmCntr;
  uint32_t rxFrmCntr;
  uint8_t  pendingFlag;
} APSME_ApsLinkKeyRAMEntry_t;

typedef struct
{
  uint32_t txFrmCntr;
  uint32_t rxFrmCntr;
  uint8_t  pendingFlag;
  uint8_t  entryUsed;
} APSME_TCLinkKeyRAMEntry_t;

// Function pointer prototype to preprocess messages before calling NWK layer
typedef void (*apsPreProcessDataReq_t)( APSDE_FrameBlk_t *blk );

/******************************************************************************
 * GLOBAL VARIABLES
 */
// Store Frame Counters in RAM and update NV periodically
extern APSME_ApsLinkKeyRAMEntry_t ApsLinkKeyRAMEntry[];
extern APSME_TCLinkKeyRAMEntry_t TCLinkKeyRAMEntry[];

/******************************************************************************
 * APS Data Service
 *   APSDE-DATA
 */

/*
 * This function requests the transfer of data from the next higher layer
 * to a single peer entity.
 */
extern ZStatus_t APSDE_DataReq( APSDE_DataReq_t* req );

/*
 * This function requests the MTU(Max Transport Unit) of the APS Data Service
 */
extern uint8_t APSDE_DataReqMTU( APSDE_DataReqMTU_t* fields );

/*
 * This function reports the results of a request to transfer a data
 * PDU (ASDU) from a local higher layer entity to another single peer entity.
 */
extern void APSDE_DataConfirm( nwkDB_t *rec, ZStatus_t Status );
extern void APSDE_DataCnf( APSDE_DataCnf_t* cnf );

/*
 * This function indicates the transfer of a data PDU (ASDU) from the
 * APS sub-layer to the local application layer entity.
 */
extern void APSDE_DataIndication( aps_FrameFormat_t *aff, zAddrType_t *SrcAddress,
                                uint16_t SrcPanId, NLDE_Signal_t *sig, uint8_t nwkSeqNum,
                                byte SecurityUse, uint32_t timestamp, uint8_t radius  );

/******************************************************************************
 * APS Management Service
 *   APSME-BIND
 *   APSME-UNBIND
 */

/*
 * This function allows the next higher layer to request to bind two devices together
 * either by proxy or directly, if issued on the coordinator.
 *
 * NOTE: The APSME-BIND.confirm is returned by this function and is not a
 *       seperate callback function.
 */
extern ZStatus_t APSME_BindRequest( byte SrcEndpInt, uint16_t ClusterId,
                                   zAddrType_t *DstAddr, byte DstEndpInt);

/*
 * This function allows the next higher layer to request to unbind two devices
 * either by proxy or directly, if issued on the coordinator.
 *
 * NOTE: The APSME-UNBIND.confirm is returned by this function and is not a
 *       seperate callback function.
 */
extern ZStatus_t APSME_UnBindRequest( byte SrcEndpInt,
                            uint16_t ClusterId, zAddrType_t *DstAddr, byte DstEndpInt);

/*
 * This function allows the next higher layer to read the value of an attribute
 * from the AIB (APS Information Base)
 */
extern ZStatus_t APSME_GetRequest( ZApsAttributes_t AIBAttribute,
                                    uint16_t Index, byte *AttributeValue );

/*
 * This function allows the next higher layer to write the value of an attribute
 * into the AIB (APS Information Base)
 */
extern ZStatus_t APSME_SetRequest( ZApsAttributes_t AIBAttribute,
                                    uint16_t Index, byte *AttributeValue );

/*
 * This function gets the EXT address based on the NWK address.
 */
extern uint8_t APSME_LookupExtAddr( uint16_t nwkAddr, uint8_t* extAddr );

/*
 * This function gets the NWK address based on the EXT address.
 */
extern uint8_t APSME_LookupNwkAddr( uint8_t* extAddr, uint16_t* nwkAddr );

#if 0     // NOT IMPLEMENTED
/*
 * This function allows the next higher layer to be notified of the results of its
 * request to unbind two devices directly or by proxy.
 */
extern void APSME_UnbindConfirm( zAddrType_t CoorAddr,ZStatus_t Status,
                           uint16_t SrcAddr, byte SrcEndpInt, byte ObjectId,
                           uint16_t DstAddr, byte DstEndpInt);
/*
 * This function allows the next higher layer to be notified of the results of its
 * request to bind two devices directly or by proxy.
 */
extern void APSME_BindConfirm( zAddrType_t CoorAddr,ZStatus_t Status,
                           uint16_t SrcAddr, byte SrcEndpInt, byte ObjectId,
                           uint16_t DstAddr, byte DstEndpInt);
#endif  // NOT IMPLEMENTED

/*
 * Set the Preprocess function pointer.  The APS Layer will call this function
 * right before calling APSDE_FrameSend() [setup function that calls NLDE_DataReq()].
 */
extern void APSDE_SetPreProcessFnp( apsPreProcessDataReq_t pfnCB );


/******************************************************************************
 * APS Incoming Command Packet Handler
 */

/*
 * APSME_CmdPkt handles APS CMD packets.
 */
extern void APSME_CmdPkt( APSME_CmdPkt_t* pkt );

/******************************************************************************
 * APS Frame Allocation And Routing
 */

/*
 * APSDE_FrameAlloc allocates an APS frame.
 */
extern void APSDE_FrameAlloc( APSDE_FrameBlk_t* blk );

/*
 * APSDE_FrameSend sends an APS frame.
 */
extern void APSDE_FrameSend( APSDE_FrameBlk_t* blk );

/*
 * APSME_HoldDataRequests holds all data request for a timeout.
 */
void APSME_HoldDataRequests( uint16_t holdTime );

/******************************************************************************
 * APS Security Related Functions
 */

/*
 * APSME_FrameSecurityRemove removes security from APS frame.
 */
extern ZStatus_t APSME_FrameSecurityRemove(uint16_t             srcAddr,
                                           aps_FrameFormat_t* aff);

/*
 * APSME_FrameSecurityApply applies security to APS frame.
 */
extern ZStatus_t APSME_FrameSecurityApply(uint16_t             dstAddr,
                                          aps_FrameFormat_t* aff);

/*
 * Configure APS security mode
 */
extern void APSME_SecurityNM( void );   // NULL MODE        - NO SECURITY
extern void APSME_SecurityCM_ED( void );// COMMERCIAL MODE  - END DEVICE
extern void APSME_SecurityCM_RD( void );// COMMERCIAL MODE  - ROUTER DEVICE
extern void APSME_SecurityCM_CD( void );// COMMERCIAL MODE  - COORD DEVICE

/******************************************************************************
 * APS Security Service Primitives - API, NHLE Calls Routines
 *
 *   APSME_TransportKeyReq
 *   APSME_UpdateDeviceReq
 *   APSME_RemoveDeviceReq
 *   APSME_RequestKeyReq
 *   APSME_SwitchKeyReq
 *   APSME_ConfirmKeyReq    // added for confirm key service
 */

/*
 * APSME_TransportKeyReq primitive.
 */
extern ZStatus_t APSME_TransportKeyReq( APSME_TransportKeyReq_t* req );

/*
 * APSME_UpdateDeviceReq primitive.
 */
extern ZStatus_t APSME_UpdateDeviceReq( APSME_UpdateDeviceReq_t* req );

/*
 * APSME_RemoveDeviceReq primitive.
 */
extern ZStatus_t APSME_RemoveDeviceReq( APSME_RemoveDeviceReq_t* req );

/*
 * APSME_RequestKeyReq primitive.
 */
extern ZStatus_t APSME_RequestKeyReq( APSME_RequestKeyReq_t* req );

/*
 * APSME_SwitchKeyReq primitive.
 */
extern ZStatus_t APSME_SwitchKeyReq( APSME_SwitchKeyReq_t* req );

/*
 * APSME_VerifyKeyReq_t primitive.
 */
extern ZStatus_t APSME_VerifyKeyReq( APSME_VerifyKeyReq_t* req );

/*
 * APSME_SwitchKeyReq primitive.
 */
extern ZStatus_t APSME_ConfirmKeyReq( APSME_ConfirmKeyReq_t* req );

/******************************************************************************
 * APS Security Primitive Stubs - API, NHLE Implements Callback Stubs
 *
 *   APSME_TransportKeyInd
 *   APSME_UpdateDeviceInd
 *   APSME_RemoveDeviceInd
 *   APSME_RequestKeyInd
 *   APSME_SwitchKeyInd
 */

/*
 * APSME_TransportKeyInd primitive.
 */
extern void APSME_TransportKeyInd( APSME_TransportKeyInd_t* ind );

/*
 * APSME_UpdateDeviceInd primitive.
 */
extern void APSME_UpdateDeviceInd( APSME_UpdateDeviceInd_t* ind );

/*
 * APSME_RemoveDeviceInd primitive.
 */
extern void APSME_RemoveDeviceInd( APSME_RemoveDeviceInd_t* ind );

/*
 * APSME_RequestKeyInd primitive.
 */
extern void APSME_RequestKeyInd( APSME_RequestKeyInd_t* ind );

/*
 * APSME_SwitchKeyInd primitive.
 */
extern void APSME_SwitchKeyInd( APSME_SwitchKeyInd_t* ind );

/*
 * APSME_VerifyKeyInd primitive.
 */
extern void APSME_VerifyKeyInd( APSME_VerifyKeyInd_t* ind );

/*
 * APSME_ConfirmKeyInd primitive.
 */
extern void APSME_ConfirmKeyInd( APSME_ConfirmKeyInd_t* apsmeInd );


/*
 * APSME_EraseICEntry
 */
extern void APSME_EraseICEntry(uint8_t *IcIndex);

/*
 * APSME_AddTCLinkKey Interface to add TC link key derived from install codes.
 */
extern ZStatus_t APSME_AddTCLinkKey(uint8_t* pTCLinkKey, uint8_t* pExt);

/*
 * APSME_SetDefaultKey Interface to set the centralized default key to defaultTCLinkKey
 */
extern ZStatus_t APSME_SetDefaultKey(void);

/*
 * APSME_SearchTCLinkKeyEntry Interface search for the TCLK entry
 */
extern uint16_t APSME_SearchTCLinkKeyEntry(uint8_t *pExt,uint8_t* found, APSME_TCLinkKeyNVEntry_t* tcLinkKeyAddrEntry);
/******************************************************************************
 * APS Security Support - NHLE Implements Callback Stubs
 *
 *   APSME_LinkKeySet
 *   APSME_LinkKeyNVIdGet
 *   APSME_KeyFwdToChild
 */

/*
 * APSME_LinkKeySet stub.
 */
extern ZStatus_t APSME_LinkKeySet( uint8_t* extAddr, uint8_t* key );


/*
 * APSME_LinkKeyNVIdGet stub.
 */
extern ZStatus_t APSME_LinkKeyNVIdGet(uint8_t* extAddr, uint16_t *pKeyNvId);

/*
 * APSME_IsLinkKeyValid stub.
 */
extern uint8_t APSME_IsLinkKeyValid(uint8_t* extAddr);

/*
 * APSME_KeyFwdToChild stub.
 */
extern uint8_t APSME_KeyFwdToChild( APSME_TransportKeyInd_t* ind );

/*
 * APSME_IsDistributedSecurity - Is APS configured for distributed secrity network
 * (not Trust Center protected).
 */
extern uint8_t APSME_IsDistributedSecurity( void );



/******************************************************************************
******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* APSMEDE_H */


