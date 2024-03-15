/**************************************************************************************************
  Filename:       AF.h
  Revised:        $Date: 2014-11-04 10:53:36 -0800 (Tue, 04 Nov 2014) $
  Revision:       $Revision: 40974 $

  Description:    This file contains the Application Framework definitions.


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
#ifndef AF_H
#define AF_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "zcomdef.h"
#include "nwk.h"
#include "aps_mede.h"

/*********************************************************************
 * CONSTANTS
 */

#define AF_BROADCAST_ENDPOINT              0xFF

#define AF_WILDCARD_PROFILEID              0x02   // Will force the message to use Wildcard ProfileID
#define AF_PREPROCESS                      0x04   // Will force APS to callback to preprocess before calling NWK layer
#define AF_LIMIT_CONCENTRATOR              0x08
#define AF_ACK_REQUEST                     0x10
#define AF_SUPRESS_ROUTE_DISC_NETWORK      0x20   // Supress Route Discovery for intermediate routes
                                                  // (route discovery preformed for initiating device)
#define AF_EN_SECURITY                     0x40
#define AF_SKIP_ROUTING                    0x80

#define AF_DISCV_ROUTE                     0x00   // This option is no longer available, and is included for backwards compatibility

// Backwards support for afAddOrSendMessage / afFillAndSendMessage.
#define AF_TX_OPTIONS_NONE                 0
#define AF_MSG_ACK_REQUEST                 AF_ACK_REQUEST

// Default Radius Count value
#define AF_DEFAULT_RADIUS                  DEF_NWK_RADIUS

/*********************************************************************
 * Node Descriptor
 */
#define AF_MAX_USER_DESCRIPTOR_LEN         16
#define AF_USER_DESCRIPTOR_FILL          0x20
typedef struct
{
  uint8_t len;     // Length of string descriptor
  uint8_t desc[AF_MAX_USER_DESCRIPTOR_LEN];
} UserDescriptorFormat_t;

// Node Logical Types
#define NODETYPE_COORDINATOR    0x00
#define NODETYPE_ROUTER         0x01
#define NODETYPE_DEVICE         0x02

// Node Frequency Band - bit map
#define NODEFREQ_800            0x01    // 868 - 868.6 MHz
#define NODEFREQ_900            0x04    // 902 - 928 MHz
#define NODEFREQ_2400           0x08    // 2400 - 2483.5 MHz

// Node MAC Capabilities - bit map
//   Use CAPINFO_ALTPANCOORD, CAPINFO_DEVICETYPE_FFD,
//       CAPINFO_DEVICETYPE_RFD, CAPINFO_POWER_AC,
//       and CAPINFO_RCVR_ON_IDLE from nl_mede.h

// Node Descriptor format structure
typedef struct
{
  uint8_t LogicalType:3;
  uint8_t ComplexDescAvail:1;  /* AF_V1_SUPPORT - reserved bit. */
  uint8_t UserDescAvail:1;     /* AF_V1_SUPPORT - reserved bit. */
  uint8_t Reserved:3;
  uint8_t APSFlags:3;
  uint8_t FrequencyBand:5;
  uint8_t CapabilityFlags;
  uint8_t ManufacturerCode[2];
  uint8_t MaxBufferSize;
  uint8_t MaxInTransferSize[2];
  uint16_t ServerMask;
  uint8_t MaxOutTransferSize[2];
  uint8_t DescriptorCapability;
} NodeDescriptorFormat_t;

// Bit masks for the ServerMask.
#define PRIM_TRUST_CENTER        0x01
#define BKUP_TRUST_CENTER        0x02
#define PRIM_BIND_TABLE          0x04
#define BKUP_BIND_TABLE          0x08
#define PRIM_DISC_TABLE          0x10
#define BKUP_DISC_TABLE          0x20
#define NETWORK_MANAGER          0x40


/*********************************************************************
 * Node Power Descriptor
 */

// Node Current Power Modes (CURPWR)
// Receiver permanently on or sync with coordinator beacon.
#define NODECURPWR_RCVR_ALWAYS_ON   0x00
// Receiver automatically comes on periodically as defined by the
// Node Power Descriptor.
#define NODECURPWR_RCVR_AUTO        0x01
// Receiver comes on when simulated, eg by a user pressing a button.
#define NODECURPWR_RCVR_STIM        0x02

// Node Available Power Sources (AVAILPWR) - bit map
//   Can be used for AvailablePowerSources or CurrentPowerSource
#define NODEAVAILPWR_MAINS          0x01  // Constant (Mains) power
#define NODEAVAILPWR_RECHARGE       0x02  // Rechargeable Battery
#define NODEAVAILPWR_DISPOSE        0x04  // Disposable Battery

// Power Level
#define NODEPOWER_LEVEL_CRITICAL    0x00  // Critical
#define NODEPOWER_LEVEL_33          0x04  // 33%
#define NODEPOWER_LEVEL_66          0x08  // 66%
#define NODEPOWER_LEVEL_100         0x0C  // 100%

// Node Power Descriptor format structure
typedef struct
{
  unsigned int PowerMode:4;
  unsigned int AvailablePowerSources:4;
  unsigned int CurrentPowerSource:4;
  unsigned int CurrentPowerSourceLevel:4;
} NodePowerDescriptorFormat_t;

/*********************************************************************
 * Simple Descriptor
 */

// AppDevVer values
#define APPDEVVER_1               0x01

// AF_V1_SUPPORT AppFlags - bit map
#define APPFLAG_NONE                0x00  // Backwards compatibility to AF_V1.

// AF-AppFlags - bit map
#define AF_APPFLAG_NONE             0x00
#define AF_APPFLAG_COMPLEXDESC      0x01  // Complex Descriptor Available
#define AF_APPFLAG_USERDESC         0x02  // User Descriptor Available

typedef uint16_t  cId_t;
// Simple Description Format Structure
typedef struct
{
  uint8_t          EndPoint;
  uint16_t         AppProfId;
  uint16_t         AppDeviceId;
  uint8_t          AppDevVer:4;
  uint8_t          Reserved:4;             // AF_V1_SUPPORT uses for AppFlags:4.
  uint8_t          AppNumInClusters;
  cId_t         *pAppInClusterList;
  uint8_t          AppNumOutClusters;
  cId_t         *pAppOutClusterList;
} SimpleDescriptionFormat_t;

/*********************************************************************
 * AF Message Format
 */

// Frame Types
#define FRAMETYPE_KVP          0x01     // 0001
#define FRAMETYPE_MSG          0x02     // 0010

#define ERRORCODE_SUCCESS               0x00

#define AF_HDR_KVP_MAX_LEN   0x08  // Max possible AF KVP header.
#define AF_HDR_V1_0_MAX_LEN  0x03  // Max possible AF Ver 1.0 header.
#define AF_HDR_V1_1_MAX_LEN  0x00  // Max possible AF Ver 1.1 header.

// Generalized MSG Command Format
typedef struct
{
  uint16_t  DataLength;              // Number of bytes in TransData
  uint8_t  *Data;
} afMSGCommandFormat_t;

typedef enum
{
  noLatencyReqs,
  fastBeacons,
  slowBeacons
} afNetworkLatencyReq_t;

/*********************************************************************
 * Endpoint  Descriptions
 */

typedef enum
{
  afAddrNotPresent = AddrNotPresent,
  afAddr16Bit      = Addr16Bit,
  afAddr64Bit      = Addr64Bit,
  afAddrGroup      = AddrGroup,
  afAddrBroadcast  = AddrBroadcast
} afAddrMode_t;

typedef struct
{
  union
  {
    uint16_t      shortAddr;
    ZLongAddr_t extAddr;
  } addr;
  afAddrMode_t addrMode;
  uint8_t endPoint;
  uint16_t panId;  // used for the INTER_PAN feature
}  afAddrType_t;


typedef struct
{
  OsalPort_EventHdr hdr;     /* OSAL Message header */
  uint16_t groupId;           /* Message's group ID - 0 if not set */
  uint16_t clusterId;         /* Message's cluster ID */
  afAddrType_t srcAddr;     /* Source Address, if endpoint is STUBAPS_INTER_PAN_EP,
                               it's an InterPAN message */
  uint16_t macDestAddr;       /* MAC header destination short address */
  uint8_t endPoint;           /* destination endpoint */
  uint8_t wasBroadcast;       /* TRUE if network destination was a broadcast address */
  uint8_t LinkQuality;        /* The link quality of the received data frame */
  uint8_t correlation;        /* The raw correlation value of the received data frame */
  int8_t  rssi;               /* The received RF power in units dBm */
  uint8_t SecurityUse;        /* deprecated */
  uint32_t timestamp;         /* receipt timestamp from MAC */
  uint8_t nwkSeqNum;          /* network header frame sequence number */
  afMSGCommandFormat_t cmd; /* Application Data */
  uint16_t macSrcAddr;        /* MAC header source short address */
  uint8_t radius;
} afIncomingMSGPacket_t;

typedef struct
{
  OsalPort_EventHdr hdr;
  uint8_t endpoint;
  uint8_t transID;
  uint16_t clusterID;
} afDataConfirm_t;

// Reflect Error Message - sent when there is an error occurs
// during a reflected message.
typedef struct
{
  OsalPort_EventHdr hdr;  // hdr.status contains the error indication (ie. ZApsNoAck)
  uint8_t endpoint;        // destination endpoint
  uint8_t transID;         // transaction ID of sent message
  uint8_t dstAddrMode;     // destination address type: 0 - short address, 1 - group address
  uint16_t dstAddr;        // destination address - depends on dstAddrMode
} afReflectError_t;

// Endpoint Table - this table is the device description
// or application registration.
// There will be one entry in this table for every
// endpoint defined.
typedef struct
{
  uint8_t endPoint;
  uint8_t epType;
  uint8_t *task_id;  // Pointer to location of the Application task ID.
  SimpleDescriptionFormat_t *simpleDesc;
  afNetworkLatencyReq_t latencyReq;
} endPointDesc_t;

// Typedef for callback function to retrieve an endpoints
//   descriptors, contained in the endpoint descriptor.
//   This will allow an application to dynamically change
//   the descriptor and not use the RAM/ROM.
typedef void *(*pDescCB)( uint8_t type, uint8_t endpoint );

// Typedef for callback function to control the AF transaction ID
//   used when sending messages.
//   This allows the application to verify if the transaction ID
//   is not duplicated of a pending message.
typedef void (*pApplCB)( APSDE_DataReq_t *req );

// Descriptor types used in the above callback
#define AF_DESCRIPTOR_SIMPLE            1
#define AF_DESCRIPTOR_PROFILE_ID        2

// Bit definitions for epList_t flags.
typedef enum
{
  eEP_AllowMatch = 1,
  eEP_NotUsed
} eEP_Flags;

typedef struct {
  uint8_t frameDelay;
  uint8_t windowSize;
} afAPSF_Config_t;

typedef struct _epList_t {
  struct _epList_t *nextDesc;
  endPointDesc_t *epDesc;
  pDescCB  pfnDescCB;     // Don't use if this function pointer is NULL.
  afAPSF_Config_t apsfCfg;
  eEP_Flags flags;
  pApplCB pfnApplCB;    // Don't use it if it has not been set to a valid function pointer by the application
} epList_t;

/*********************************************************************
 * TYPEDEFS
 */

#define afStatus_SUCCESS            ZSuccess           /* 0x00 */
#define afStatus_FAILED             ZFailure           /* 0x01 */
#define afStatus_INVALID_PARAMETER  ZInvalidParameter  /* 0x02 */
#define afStatus_MEM_FAIL           ZMemError          /* 0x10 */
#define afStatus_NO_ROUTE           ZNwkNoRoute        /* 0xCD */

typedef ZStatus_t afStatus_t;

typedef struct
{
  uint8_t              kvp;
  APSDE_DataReqMTU_t aps;
} afDataReqMTU_t;

/*********************************************************************
 * Globals
 */

extern epList_t *epList;

/*********************************************************************
 * FUNCTIONS
 */

 /*
  * afInit - Initialize the AF.
  */
  //extern void afInit( void );
  #define afInit()  // No work to do for now.

 /*
  * afRegisterExtended - Register an Application's EndPoint description
  *           with a callback function for descriptors and
  *           with an Application callback function to control
  *           the AF transaction ID.
  *
  */
  extern epList_t *afRegisterExtended( endPointDesc_t *epDesc, pDescCB descFn, pApplCB applFn );

 /*
  * afRegister - Register an Application's EndPoint description.
  *
  */
  extern afStatus_t afRegister( endPointDesc_t *epDesc );

 /*
  * afDelete - Delete an Application's EndPoint descriptor and frees the memory.
  *
  */
  extern afStatus_t afDelete( uint8_t EndPoint );

 /*
  * afDataConfirm - APS will call this function after a data message
  *                 has been sent.
  */
  extern void afDataConfirm( uint8_t endPoint, uint8_t transID, uint16_t clusterID, ZStatus_t status );

 /*
  * afReflectError - APS will call this function for an error with a reflected data message.
  */
  extern void afReflectError( uint8_t srcEP, uint8_t dstAddrMode, uint16_t dstAddr, uint8_t dstEP,
                              uint8_t transID, ZStatus_t status );

 /*
  * afIncomingData - APS will call this function when an incoming
  *                   message is received.
  */
  extern void afIncomingData( aps_FrameFormat_t *aff, zAddrType_t *SrcAddress, uint16_t SrcPanId,
                       NLDE_Signal_t *sig, uint8_t nwkSeqNum, uint8_t SecurityUse, uint32_t timestamp, uint8_t radius );

  afStatus_t AF_DataRequest( afAddrType_t *dstAddr, endPointDesc_t *srcEP,
                             uint16_t cID, uint16_t len, uint8_t *buf, uint8_t *transID,
                             uint8_t options, uint8_t radius );


/*********************************************************************
 * @fn      AF_DataRequestSrcRtg
 *
 * @brief   Common functionality for invoking APSDE_DataReq() for both
 *          SendMulti and MSG-Send.
 *
 * input parameters
 *
 * @param  *dstAddr - Full ZB destination address: Nwk Addr + End Point.
 * @param  *srcEP - Origination (i.e. respond to or ack to) End Point Descr.
 * @param   cID - A valid cluster ID as specified by the Profile.
 * @param   len - Number of bytes of data pointed to by next param.
 * @param  *buf - A pointer to the data bytes to send.
 * @param  *transID - A pointer to a byte which can be modified and which will
 *                    be used as the transaction sequence number of the msg.
 * @param   options - Valid bit mask of Tx options.
 * @param   radius - Normally set to AF_DEFAULT_RADIUS.
 * @param   relayCnt - Number of devices in the relay list
 * @param   pRelayList - Pointer to the relay list
 *
 * output parameters
 *
 * @param  *transID - Incremented by one if the return value is success.
 *
 * @return  afStatus_t - See previous definition of afStatus_... types.
 */

afStatus_t AF_DataRequestSrcRtg( afAddrType_t *dstAddr, endPointDesc_t *srcEP,
                           uint16_t cID, uint16_t len, uint8_t *buf, uint8_t *transID,
                           uint8_t options, uint8_t radius, uint8_t relayCnt,
                           uint16_t* pRelayList );

/*********************************************************************
 * Direct Access Functions - ZigBee Device Object
 */

 /*
  *	afFindEndPointDesc - Find the endpoint description entry from the
  *                      endpoint number.
  */
  extern endPointDesc_t *afFindEndPointDesc( uint8_t endPoint );

 /*
  *	afFindSimpleDesc - Find the Simple Descriptor from the endpoint number.
  *   	  If return value is not zero, the descriptor memory must be freed.
  */
  extern uint8_t afFindSimpleDesc( SimpleDescriptionFormat_t **ppDesc, uint8_t EP );

 /*
  *	afDataReqMTU - Get the Data Request MTU(Max Transport Unit)
  */
  extern uint8_t afDataReqMTU( afDataReqMTU_t* fields );

 /*
  *	afGetMatch - Get the action for the Match Descriptor Response
  *             TRUE allow match descriptor response
  */
  extern uint8_t afGetMatch( uint8_t ep );

 /*
  *	afSetMatch - Set the action for the Match Descriptor Response
  *             TRUE allow match descriptor response
  */
  extern uint8_t afSetMatch( uint8_t ep, uint8_t action );

 /*
  *	afNumEndPoints - returns the number of endpoints defined.
  */
  extern uint8_t afNumEndPoints( void );

 /*
  *	afEndPoints - builds an array of endpoints.
  */
  extern void afEndPoints( uint8_t *epBuf, uint8_t skipZDO );

 /*
  * afCopyAddress
  */
extern void afCopyAddress (afAddrType_t *afAddr, zAddrType_t *zAddr);

 /*
  *	afAPSF_ConfigGet - ascertain the fragmentation configuration for the specified EndPoint.
  */
void afAPSF_ConfigGet(uint8_t endPoint, afAPSF_Config_t *pCfg);

 /*
  *	afAPSF_ConfigSet - set the fragmentation configuration for the specified EndPoint.
  */
afStatus_t afAPSF_ConfigSet(uint8_t endPoint, afAPSF_Config_t *pCfg);

 /*
  *	afSetApplCB - Sets the pointer to the Application Callback function for a
  *               specific EndPoint.
  */
uint8_t afSetApplCB( uint8_t endPoint, pApplCB pApplFn );

#ifdef __cplusplus
}
#endif
#endif
/**************************************************************************************************
*/
