/**************************************************************************************************
  Filename:       nwk.h
  Revised:        $Date: 2014-12-01 14:58:34 -0800 (Mon, 01 Dec 2014) $
  Revision:       $Revision: 41287 $

  Description:    Network layer logic component interface.


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

#ifndef NWK_H
#define NWK_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

#include "zcomdef.h"
#include "zmac.h"
#include "nwk_bufs.h"
#include "nl_mede.h"
#include "ssp.h"

/*********************************************************************
 * MACROS
 */
#define NWK_BLACK_WHITE_LIST_SIZE  90

/*********************************************************************
 * CONSTANTS
 */

//NWK event identifiers
#define MAC_SCAN_REQ          0x01
#define NWK_NETWORKSTART_REQ  0x02
#define MAC_ASSOCIATE_REQ     0x03
#define NWK_REMOTE_GET_REQ    0x04
#define NWK_REMOTE_SET_REQ    0x05
#define NWK_ASSOCIATE_RESP    0x06
#define NWK_DISASSOCIATE_REQ  0x07

#define NWK_AUTO_POLL_EVT                    0x0001
#define NWK_ZED_KEEP_ALIVE_EVT               0x0002
#define NWK_NOT_EXPECTING_EVT                0x0004
#define NWK_STARTUP_REBROADCAST_TIMEOUT_EVT  0x0008
#define RTG_TIMER_EVENT                      0x0010
#define NWK_DATABUF_SEND                     0x0020
#define NWK_BCAST_TIMER_EVT                  0x0040
#define NWK_PERMITJOIN_EVT                   0x0080
#define NWK_LINK_STATUS_EVT                  0x0100
#define NWK_PID_UPDATE_EVT                   0x0200
#define NWK_REJOIN_TIMEOUT_EVT               0x0400
#define NWK_MTO_RTG_REQ_EVT                  0x0800
#define NWK_MTO_RTG_REQ_DELAY_EVT            0x1000
#define NWK_BROADCAST_MSG_EVT                0x2000
#define NWK_CHILD_AGE_TIMER_EVT              0x4000
// Event 0x8000 is Reserved for SYS_EVENT_MSG
#define NWK_JOIN_SECURITY_TIMEOUT_EVT        0x10000

//NWK PACKET: FIELD IDENTIFIERS
#define NWK_CMD_ID                  0
#define NWK_PARAMS_ID               1
#define NWK_REQ_ATTR_ID             1
#define NWK_REQ_ATTR                2
#define NWK_CMD_PYLD_BEGIN          NWK_HEADER_LEN
#define NWK_DEVICE_LIST_LEN_FIELD   NWK_HEADER_LEN + 1

#ifdef APP_TP2
#define CMD_ID_INVALID_CMD          0xF0
#define INVALID_CMD_LEN               10
#endif

// This value needs to be set or read from somewhere
#define ED_SCAN_MAXCHANNELS 27

/* MAX size the frame can have */
#define MAX_MAC_FRAME_SIZE       127
/* MAC Frame Header size without security*/
#define MAC_HEADER_FRAME_SIZE      9
/* MAC Footer */
#define MAC_FOOTER_FRAME_SIZE      2

// Max length of packet that can be sent to the MAC
#define MAX_DATA_PACKET_LEN      (MAX_MAC_FRAME_SIZE - MAC_HEADER_FRAME_SIZE - MAC_FOOTER_FRAME_SIZE)

#define NWK_TASK_ID              0
#define ASSOC_CAPABILITY_INFO    0
#define ASSOC_SECURITY_EN        0

#define DEF_DEST_EP              2
#define DEVICE_APPLICATION       0

#define MAC_ADDR_LEN             8

#define NWK_TXOPTIONS_ACK        0x01
#define NWK_TXOPTIONS_INDIRECT   0x04

// TxOptions for a data request - Indirect data and ACK required
#define NWK_TXOPTIONS_COORD      (NWK_TXOPTIONS_ACK | NWK_TXOPTIONS_INDIRECT)

// TxOptions for a data request - Direct data and ACK required
//#define NWK_TXOPTIONS_COORD       (NWK_TXOPTIONS_ACK)

//Assume value until defined By SuperApp or design spec
#define DEF_MAX_NUM_COORDINATORS 15        //Default value
#define DEF_CHANNEL_SCAN_BITMAP  MAX_CHANNELS_24GHZ
#define SOFT_SCAN_DURATION       1         //in secs

#define DEF_SCAN_DURATION        2

#define NO_BEACONS              15

#define DEF_BEACON_ORDER         NO_BEACONS
//#define DEF_BEACON_ORDER         10   // 15 seconds
//#define DEF_BEACON_ORDER         9    // 7.5 seconds
//#define DEF_BEACON_ORDER         8    // 3.75 seconds
//#define DEF_BEACON_ORDER         6    // 1 second
//#define DEF_BEACON_ORDER         1    // 30 millisecond

//#define DEF_SUPERFRAMEORDER      2
#define DEF_SUPERFRAMEORDER      DEF_BEACON_ORDER
#define NWK_SECURITY_ENABLE      FALSE
#define NWK_MAC_ASSOC_CNF_LEN    4
#define FIXED_SIZ_MAC_DATA_CNF   4         //Length of all fixed params except data
#define FIXED_SIZ_MAC_DATA_IND   26
#define FIXED_SIZ_MAC_SCAN_CNF   7

#define ALL_PAIRING_TABLE_ENTRIES   0
#define SIZE_OF_PAIRING_TABLE_ENTRY 6 //Two short addr and two endpts
#define SIZE_OF_DEVICE_LIST_ENTRY   2 //short addr in dev list is 2 bytes

#define NWK_SEND_TIMER_INTERVAL         2
#define NWK_BCAST_TIMER_INTERVAL        100 // NWK_BCAST_TIMER_EVT duration
#define NWK_CHILD_AGE_TIMER_INTERVAL    1000  // One Second 1(s) * 1000(ms)

#define INVALID_NODE_ADDR                           0xFFFE

// Link cost constants
#define DEF_LINK_COST              1   // Default link cost
#define MAX_LINK_COST              7   // max link cost
#define LINK_DOWN_COST             0   // link is down if txCost is equal to LINK_DOWN_COST
#define LINK_AGEOUT_COST           0   // Set link cost to zero if the neighbor age out

#define DEF_LQI                    170   // Default lqi
#define LOWEST_LQI_STILL_ACTIVE    1
#define LINK_AGEOUT_LQI            0   // Set lqi to zero if the neighbor age out

// Link counter constants
#define DEF_LINK_COUNTER           ((gLINK_DOWN_TRIGGER+1) / 2)   // Starting tx counter
#define LINK_ACTIVE_TRIGGER        2   // link is up if txCounter goes below this

//NWK Callback subscription IDs
#define CB_ID_APP_ANNOUNCE_CNF          0x00
#define CB_ID_APP_ASSOCIATE_CNF         0x01
#define CB_ID_APP_ASSOCIATE_IND         0x02
#define CB_ID_APP_DATA_CNF              0x03
#define CB_ID_APP_DATA_IND              0x04
#define CB_ID_APP_DISASSOCIATE_CNF      0x05
#define CB_ID_APP_DISASSOCIATE_IND      0x06
#define CB_ID_APP_NETWORK_DETECT_CNF    0x07
#define CB_ID_APP_REMOTE_GET_CNF        0x08
#define SPI_CB_APP_REMOTE_SET_CNF       0x09
#define CB_ID_APP_SERVICE_CNF           0x0a
#define CB_ID_APP_SERVICE_IND           0x0b
#define CB_ID_APP_START_CNF             0x0c

#define NUM_PING_ROUTE_ADDRS            12
#define PING_ROUTE_ADDRS_INDEX          8

#define NWK_GetNodeDepth()              (_NIB.nodeDepth)
#define NWK_GetTreeDepth()              (0)

#define BEACON_MAX_DEPTH                0x0F

// The value of this event should larger than the maximum value of the MAC events
#define NWK_CHILD_TABLE_MGMT            100

// Status of child device
#define NWK_CHILD_NOT_IN_TABLE          1

// Router parent capabilities information bitmask
// Bits   Value    Description
//   0    0x01     MAC Data Poll Keepalive Supported
//   1    0x01     EndDeviceTimeoutMsg as Keepalive supported
#define NWK_PARENT_INFO_UNDEFINED               0x00
#define NWK_PARENT_INFO_MAC_DATA_POLL           0x01
#define NWK_PARENT_INFO_END_DEVICE_TIMEOUT_MSG  0x02

#define PARENT_INFO_VALID_PARAMETERS (NWK_PARENT_INFO_MAC_DATA_POLL | NWK_PARENT_INFO_END_DEVICE_TIMEOUT_MSG)

/*********************************************************************
 * TYPEDEFS
 */
typedef enum
{
  NWK_INIT,
  NWK_JOINING_ORPHAN,
  NWK_DISC,
  NWK_JOINING,
  NWK_ENDDEVICE,
  PAN_CHNL_SELECTION,
  PAN_CHNL_VERIFY,
  PAN_STARTING,
  NWK_ROUTER,
  NWK_REJOINING
} nwk_states_t;

// MAC Command Buffer types
typedef enum
{
  MACCMDBUF_NONE,
  MACCMDBUF_ASSOC_REQ,
  MACCMDBUF_DISASSOC_REQ
} nwkMacCmds_t;

typedef struct
{
  byte  SequenceNum;
  byte  PassiveAckTimeout;
  byte  MaxBroadcastRetries;
  byte  MaxChildren;
  byte  MaxDepth;
  byte  MaxRouters;

  byte  dummyNeighborTable;     // to make everything a byte!!

  byte  BroadcastDeliveryTime;
  byte  ReportConstantCost;
  byte  RouteDiscRetries;

  byte  dummyRoutingTable;      // to make everything a byte!!

  byte  SecureAllFrames;
  byte  SecurityLevel;
#if defined ( COMPATIBILITY_221 )   // Obsolete - do not use
  byte  nwkAllFresh;
#endif
  byte  SymLink;
  byte  CapabilityFlags;

  uint16_t TransactionPersistenceTime;

  byte   nwkProtocolVersion;

  // non-standard attributes
  byte  RouteDiscoveryTime;
  byte  RouteExpiryTime;        // set to 255 to turn off expiration of routes

  // non-settable
  uint16_t  nwkDevAddress;
  byte    nwkLogicalChannel;
  uint16_t  nwkCoordAddress;
  byte    nwkCoordExtAddress[Z_EXTADDR_LEN];
  uint16_t  nwkPanId;

  // Other global items - non-settable
  nwk_states_t  nwkState;
  uint32_t        channelList;
  byte          beaconOrder;
  byte          superFrameOrder;
  byte          scanDuration;
  byte          battLifeExt;
  uint32_t        allocatedRouterAddresses;
  uint32_t        allocatedEndDeviceAddresses;
  byte          nodeDepth;

  // Version 1.1 - extended PAN ID
  uint8_t         extendedPANID[Z_EXTADDR_LEN];

  // Network key flag
  uint8_t      nwkKeyLoaded;
  // Key information - Moved out of the NIB structure after ZStack 2.3.0
  // If these elements are going to be reused make sure to consider the size
  // of the structures and padding specific to the target where the stack is
  // going to be running.
  nwkKeyDesc spare1;    // Not used
  nwkKeyDesc spare2;    // Not used

  // Zigbee Pro extensions
  uint8_t      spare3;                // nwkAddrAlloc deprecated - not used anymore
  uint8_t      spare4;                // nwkUniqueAddr deprecated - not used anymore
  uint8_t      nwkLinkStatusPeriod;   // The time in seconds betwee link status
                                    // command frames
  uint8_t      nwkRouterAgeLimit;     // The number of missed link status
                                    // command frames before resetting the
                                    // link cost to zero
  uint8_t      nwkUseMultiCast;
  // ZigBee Pro extentions: MTO routing
  uint8_t      nwkIsConcentrator;             // If set, then the device is concentrator
  uint8_t      nwkConcentratorDiscoveryTime;  // Time period between two consecutive MTO route discovery
  uint8_t      nwkConcentratorRadius;         // Broadcast radius of the MTO route discovery

#if defined ( COMPATIBILITY_221 )   // Obsolete - do not use
  uint8_t      nwkMaxSourceRoute;
  uint8_t      nwkSrcRtgExpiryTime;
#else
  uint8_t      nwkAllFresh;
#endif

  uint16_t     nwkManagerAddr;        // Network Manager Address
  uint16_t     nwkTotalTransmissions;
  uint8_t      nwkUpdateId;
} nwkIB_t;

// Scanned PAN IDs to be used for Network Report command
typedef struct
{
  uint16_t panId;
  void   *next;
} nwkPanId_t;

typedef enum
{
NWK_FRAME_FWD_MSG_QUEUED,           // Frame has been queued into nwk buffers
NWK_FRAME_FWD_MSG_SENT,             // Sent to the MAC
}nwkFrameFwdstate_t;


typedef struct
{
uint16_t srcAddr;
uint16_t dstAddr;
uint8_t  handle;
nwkFrameFwdstate_t  frameState;
uint8_t  status;                     // MAC status values
}nwkFrameFwdNotification_t;

typedef void  (*nwkFrameFwdNotificationCB_t) (nwkFrameFwdNotification_t *nwkFrameFwdNotification);

typedef struct
{
    uint8_t  isWhiteList;
    uint16_t ElementList[NWK_BLACK_WHITE_LIST_SIZE];
}NwkLayerBlackWhiteList_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern nwkIB_t _NIB;
extern byte NWK_TaskID;
extern networkDesc_t *NwkDescList;
extern byte nwkExpectingMsgs;
extern byte nwk_beaconPayload[ZMAC_MAX_BEACON_PAYLOAD_LEN];
extern byte nwk_beaconPayloadSize;

extern uint8_t nwkSendMTOReq;


/*********************************************************************
 * FUNCTIONS
 */

 /*
 * NWK Task Initialization
 */
extern void nwk_init( byte task_id );

 /*
 * Calls mac_data_req then handles the buffering
 */
extern ZStatus_t nwk_data_req_send( nwkDB_t* db );

 /*
 * NWK Event Loop
 */
extern uint32_t nwk_event_loop( byte task_id, uint32_t events );

 /*
 * Process incoming command packet
 */
//extern void CommandPktProcessing( NLDE_FrameFormat_t *ff );

/*
* Start a coordinator
*/
extern ZStatus_t nwk_start_coord( void );

/*
 * Free any network discovery data
 */
extern void nwk_desc_list_free( void );

/*
 * This function sets to null the discovery nwk list
 */
extern void nwk_desc_list_release(void);

extern networkDesc_t *nwk_getNetworkDesc( uint8_t *ExtendedPANID, uint16_t PanId, byte Channel );
extern networkDesc_t *nwk_getNwkDescList( void );

extern void nwk_BeaconFromNative(byte* buff, byte size, beaconPayload_t* beacon);
extern void nwk_BeaconToNative(beaconPayload_t* beacon, byte* buff, byte size);

/*
 * Set NWK task's state
 */
extern void nwk_setStateIdle( uint8_t idle );

/*
 * Returns TRUE if NWK state is idle, FALSE otherwise.
 */
extern uint8_t nwk_stateIdle( void );

/*********************************************************************
 * Functionality - not to be called directly.
 */
extern void nwk_ScanPANChanSelect( ZMacScanCnf_t *param );
extern void nwk_ScanPANChanVerify( ZMacScanCnf_t *param );

/*
 *  Special Send Leave Posts the message directly to MAC without buffering it
 */
extern ZStatus_t nwk_send_direct_leave_req( nwkDB_t* db );

#if (ZSTACK_ROUTER_BUILD)

/*
 *  Enable/disable notifications about frames being forwared by local routing device
 */
extern void setNwkFrameFwdNotification(uint8_t enabled);

/*
 *  This register a callback to which notify about a frame being forwarded by local routing device
 */
extern void nwk_RegisteNwkFrameFwdNotifyCB(nwkFrameFwdNotificationCB_t nwkFrameFwdNotificationCB);
#endif


/*********************************************************************
*********************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* NWK_H */
