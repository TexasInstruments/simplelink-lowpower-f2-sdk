/**************************************************************************************************
  Filename:       bdb_touchlink.h
  Revised:        $Date: 2013-08-30 16:09:11 -0700 (Fri, 30 Aug 2013) $
  Revision:       $Revision: 35197 $

  Description:    This file contains the Zigbee Cluster Library: Light Link
                  Profile definitions.


  Copyright 2011-2013 Texas Instruments Incorporated.

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

#ifndef TOUCHLINK_H
#define TOUCHLINK_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zd_profile.h"

/*********************************************************************
 * CONSTANTS
 */

// Reason for Leave command initiation
#define TOUCHLINK_LEAVE_TO_JOIN_NWK                              1
#define TOUCHLINK_LEAVE_TO_START_NWK                             2

//Scan modes
#define TOUCHLINK_SCAN_PRIMARY_CHANNELS                          1
#define TOUCHLINK_SCAN_SECONDARY_CHANNELS                        2
#define TOUCHLINK_SCAN_FOUND_NOTHING                             3

#define APLC_MAX_PERMIT_JOIN_DURATION                            60

#define APL_FREE_NWK_ADDR_RANGE_BEGIN                            0x0001
#define APL_FREE_NWK_ADDR_RANGE_END                              0xFFF7
#define APL_FREE_ADDR_THRESHOLD                                  10

#define APL_FREE_GROUP_ID_RANGE_BEGIN                            0x0001
#define APL_FREE_GROUP_ID_RANGE_END                              0xFEFF
#define APL_FREE_GROUP_ID_THRESHOLD                              10

#define TOUCHLINK_UPDATE_NV_NIB                                  0x01
#define TOUCHLINK_UPDATE_NV_RANGES                               0x02


// Only for development
#define No_Ch_offset                                             0
#define Ch_Plus_1                                                1
#define Ch_Plus_2                                                2
#define Ch_Plus_3                                                3

/*********************************************************************
 * MACROS
 */
#define TOUCHLINK_PRIMARY_CHANNEL( ch )         ((ch) == TOUCHLINK_FIRST_CHANNEL  || \
                                                 (ch) == TOUCHLINK_SECOND_CHANNEL || \
                                                 (ch) == TOUCHLINK_THIRD_CHANNEL  || \
                                                 (ch) == TOUCHLINK_FOURTH_CHANNEL )

#define TOUCHLINK_SAME_NWK( panId, ePanId )     ((_NIB.nwkPanId == (panId) ) && \
                                                 osal_ExtAddrEqual(_NIB.extendedPANID, (ePanId)))

/*********************************************************************
 * TYPEDEFS
 */

// Scan Request/Response "ZigBee information" field bitmap
typedef struct
{
  unsigned int logicalType:2;
  unsigned int rxOnWhenIdle:1;
  unsigned int reserved:5;
} zInfoBits_t;

// Scan Request/Response "ZigBee information" field
typedef union
{
  zInfoBits_t zInfoBits;
  uint8_t zInfoByte;
} zInfo_t;

// Scan Request/Response "TouchLink information" field bitmap
typedef struct
{
  unsigned int factoryNew:1;
  unsigned int addressAssignment:1;
  unsigned int reserved:2;
  unsigned int linkInitiator:1;
  unsigned int linkPriority:1;
  unsigned int reserved1:1;
  unsigned int profileInterop:1;
} touchLinkInfoBits_t;

// Scan Request/Response "TouchLink information" field
typedef union
{
  touchLinkInfoBits_t touchLinkInfoBits;
  uint8_t touchLinkInfoByte;
} touchLinkInfo_t;

typedef struct nwkParam
{
  struct nwkParam *nextParam;
  uint16_t PANID;
  uint8_t logicalChannel;
  uint8_t extendedPANID[Z_EXTADDR_LEN];
  uint16_t chosenRouter;
  uint8_t chosenRouterLinkQuality;
  uint8_t chosenRouterDepth;
  uint8_t routerCapacity;
  uint8_t deviceCapacity;
} touchLinkDiscoveredNwkParam_t;

// TouchLink Network parameters
typedef struct
{
  uint8_t extendedPANID[Z_EXTADDR_LEN]; // Extended PAN identifier
  uint8_t keyIndex;                     // Key index
  uint8_t nwkKey[SEC_KEY_LEN];          // Encrypted network key
  uint8_t logicalChannel;               // Logical channel
  uint16_t panId;                       // PAN identifier
  uint16_t nwkAddr;                     // Network address
  uint16_t grpIDsBegin;                 // Group identifiers begin
  uint16_t grpIDsEnd;                   // Group identifiers end
  uint16_t freeNwkAddrBegin;            // Free network address range begin
  uint16_t freeNwkAddrEnd;              // Free network address range end
  uint16_t freeGrpIDBegin;              // Free group identifier range begin
  uint16_t freeGrpIDEnd;                // Free group identifier range end
} bdbTLNwkParams_t;

// Device Info
PACKED_TYPEDEF_STRUCT
{
  uint16_t profileID;                   // Profile identifier
  uint16_t deviceID;                    // Device identifier
  uint8_t endpoint;                     // Endpoint identifier
  uint8_t version;                      // Version
  uint8_t grpIdCnt;                     // Group identifier count
} bdbTLDeviceInfo_t;

// Scan Response command format
typedef struct
{
  uint32_t transID;                     // Inter-PAN transaction idententifier
  uint8_t rssiCorrection;               // RSSI correction
  zInfo_t zInfo;                      // ZigBee information
  touchLinkInfo_t touchLinkInfo;      // TouchLink information
  uint16_t keyBitmask;                  // Key bitmask
  uint32_t responseID;                  // Response idententifier
  uint8_t extendedPANID[Z_EXTADDR_LEN]; // Extended PAN identifier
  uint8_t nwkUpdateId;                  // Network update identifier
  uint8_t logicalChannel;               // Logical channel
  uint16_t PANID;                       // PAN identifier
  uint16_t nwkAddr;                     // Network address
  uint8_t numSubDevices;                // Number of sub-devices
  uint8_t totalGrpIDs;                  // Total group identifiers

// The followings are only present when numSubDevices is equal to 1
bdbTLDeviceInfo_t deviceInfo;       // Device info

// shorthand "zInfo" access
#define zLogicalType          zInfo.zInfoBits.logicalType
#define zRxOnWhenIdle         zInfo.zInfoBits.rxOnWhenIdle

  // shorthand "touchLinkInfo" access
#define touchLinkFactoryNew         touchLinkInfo.touchLinkInfoBits.factoryNew
#define touchLinkAddressAssignment  touchLinkInfo.touchLinkInfoBits.addressAssignment
#define touchLinkInitiator          touchLinkInfo.touchLinkInfoBits.linkInitiator
#define touchLinklinkPriority       touchLinkInfo.touchLinkInfoBits.linkPriority
#define touchLinkProfileInterop     touchLinkInfo.touchLinkInfoBits.profileInterop
} bdbTLScanRsp_t;

// Network start request command frame
typedef struct
{
  uint32_t transID;                         // Inter-PAN transaction idententifier
  bdbTLNwkParams_t nwkParams;             // Network parameters
  uint8_t initiatorIeeeAddr[Z_EXTADDR_LEN]; // Initiator IEEE address
  uint16_t initiatorNwkAddr;                // Initiator network address
} bdbTLNwkStartReq_t;

// Network join router/end device request command frame
typedef struct
{
  uint32_t transID;                     // Inter-PAN transaction idententifier
  bdbTLNwkParams_t nwkParams;         // Network parameters
  uint8_t nwkUpdateId;                  // Network update identifier
} bdbTLNwkJoinReq_t;

// Network rejoin request command frame
typedef struct
{
  uint8_t extendedPANID[Z_EXTADDR_LEN]; // Extended PAN identifier
  uint8_t nwkUpdateId;                  // Network update identifier
  uint8_t logicalChannel;               // Logical channel
  uint16_t panId;                       // PAN identifier
  uint16_t nwkAddr;                     // Network address
} bdbTLNwkRejoin_t;

typedef union
{
  bdbTLNwkStartReq_t nwkStartReq;
  bdbTLNwkJoinReq_t nwkJoinReq;
} bdbTLReq_t;

/*********************************************************************
 * VARIABLES
 */
extern uint32_t touchLinkLastAcceptedTransID;

extern uint32_t touchLinkResponseID;
extern uint32_t touchLinkTransID;

extern uint16_t touchLinkGrpIDsBegin;
extern uint16_t touchLinkGrpIDsEnd;

extern uint8_t touchLinkLeaveInitiated;

extern touchLinkDiscoveredNwkParam_t discoveredTouchlinkNwk;

extern bool touchlinkFNReset;

extern uint8_t touchLinkTaskId;

extern bool touchlinkDistNwk;

extern uint16_t touchlinkPanId;

/*********************************************************************
 * FUNCTIONS
 */

/*
 *  Initialize the TOUCHLINK global and local variables
 */
extern void touchLink_InitVariables( bool initiator );

/*
 * Get the total number of sub-devices (endpoints) registered
 */
extern uint8_t touchLink_GetNumSubDevices( uint8_t startIndex );

/*
 * Get the total number of group IDs required by this device
 */
extern uint8_t touchLink_GetNumGrpIDs( void );

/*
 * Get the sub-device information
 */
extern void touchLink_GetSubDeviceInfo( uint8_t index, bdbTLDeviceInfo_t *pInfo );

/*
 * Copy new Network Parameters to the NIB
 */
extern void touchLink_SetNIB( nwk_states_t nwkState, uint16_t nwkAddr, uint8_t *pExtendedPANID,
                        uint8_t logicalChannel, uint16_t panId, uint8_t nwkUpdateId );

/*
 * Updates NV with NIB and free ranges items
 */
extern void touchLink_UpdateNV( uint8_t enables );

/*
 * Set our channel
 */
extern void touchLink_SetChannel( uint8_t newChannel );

/*
 * Encrypt the network key to be sent to the Target
 */
extern void touchLink_EncryptNwkKey( uint8_t *pNwkKey, uint8_t keyIndex, uint32_t transID, uint32_t responseID );

/*
 * Decrypt the network key received from the Initiator
 */
extern void touchLink_DecryptNwkKey( uint8_t *pNwkKey, uint8_t keyIndex, uint32_t transID, uint32_t responseID );

/*
 * Fill buffer with random bytes
 */
extern void touchLink_GenerateRandNum( uint8_t *pNum, uint8_t numSize );

/*
 * Get randomly chosen TOUCHLINK primary channel
 */
extern uint8_t touchLink_GetRandPrimaryChannel( void );

/*
 * Get the supported network key bitmask
 */
extern uint16_t touchLink_GetNwkKeyBitmask( void );

/*
 * Update our local network update id and logical channel
 */
extern void touchLink_ProcessNwkUpdate( uint8_t nwkUpdateId, uint8_t logicalChannel );

/*
 * Configure MAC with our Network Parameters
 */
extern void touchLink_SetMacNwkParams( uint16_t nwkAddr, uint16_t panId, uint8_t channel );

/*
 * Send out a Leave Request command
 */
extern ZStatus_t touchLink_SendLeaveReq( void );

/*
 * Pop an avaialble short address out of the free network addresses range
 */
extern uint16_t touchLink_PopNwkAddress( void );

/*
 * Update the TOUCHLINK free range global variables
 */
extern void touchLink_UpdateFreeRanges( bdbTLNwkParams_t *pParams );

/*
 * Get the TOUCHLINK free range global variables
 */
extern void touchLink_GerFreeRanges( bdbTLNwkParams_t *pParams );

/*
 * Checks to see if the free ranges can be split
 */
extern bool touchLink_IsValidSplitFreeRanges( uint8_t totalGrpIDs );

/*
 * Split our own free network address and group ID ranges
 */
extern void touchLink_SplitFreeRanges( uint16_t *pAddrBegin, uint16_t *pAddrEnd,
                               uint16_t *pGrpIdBegin, uint16_t *pGrpIdEnd );

/*
 * Pop the requested number of group IDs out of the free group IDs range.
 */
extern void touchLink_PopGrpIDRange( uint8_t numGrpIDs, uint16_t *pGrpIDsBegin, uint16_t *pGrpIDsEnd );

/*
 * Get the RSSI of the message just received through a ZCL callback
 */
extern int8_t touchLink_GetMsgRssi( void );

/*
 * Determine the new network update id.
 */
extern uint8_t touchLink_NewNwkUpdateId( uint8_t ID1, uint8_t ID2 );

/*
 * Update the network key.
 */
extern void touchLink_UpdateNwkKey( uint8_t *pNwkKey, uint8_t keyIndex );

/*
 * Perform a Network Discovery scan
 */
extern void touchLink_PerformNetworkDisc( uint32_t scanChannelList );

/*
 * Free any network discovery data
 */
extern void touchLink_FreeNwkParamList( void );

/*
 * Transaction ID Filter for Touch-Link received commands
 */
extern bool touchLink_IsValidTransID( uint32_t transID );

/*
 * Process incoming ZDO messages (for routers)
 */
extern void touchLink_RouterProcessZDOMsg( zdoIncomingMsg_t *inMsg );

/*
 * Set the router permit join flag
 */
extern ZStatus_t touchLink_PermitJoin( uint8_t duration );

/*
 * @brief   Perform a network rejoin
 */
extern void touchLink_DevRejoin( bdbTLNwkRejoin_t *rejoinInf );

/*
 * To request a network start
 */
extern void touchLinkStartRtr( bdbTLNwkParams_t *pParams, uint32_t transID );

/*
 * Set the touchlink initiator flag
 */
extern void touchLink_DeviceIsInitiator( bool initiator );

/*
 * Get the touchlink initiator flag
 */
extern bool touchLink_GetDeviceInitiator( void );

/*
 * @brief   Initialize the TOUCHLINK free range global variables.
 */
extern void touchLink_InitFreeRanges( bool initiator );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* TOUCHLINK_H */
