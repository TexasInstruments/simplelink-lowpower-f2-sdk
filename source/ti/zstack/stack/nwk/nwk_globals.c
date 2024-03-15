/**************************************************************************************************
  Filename:       nwk_globals.c
  Revised:        $Date: 2015-01-08 16:32:12 -0800 (Thu, 08 Jan 2015) $
  Revision:       $Revision: 41678 $

  Description:    User definable Network Parameters.


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
#include "osal_nv.h"
#include "addr_mgr.h"
#include "assoc_list.h"
#include "binding_table.h"
#include "nwk_util.h"
#include "nwk_globals.h"
#include "aps.h"
#include "ssp.h"
#include "rtg.h"
#include "zd_config.h"
#include "zglobals.h"
#include "zd_app.h"
#include "zd_sec_mgr.h"
#include "ti_zstack_config.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
// Maximums for the data buffer queue
#define NWK_MAX_DATABUFS_WAITING    8     // Waiting to be sent to MAC
#define NWK_MAX_DATABUFS_SCHEDULED  5     // Timed messages to be sent
#define NWK_MAX_DATABUFS_CONFIRMED  5     // Held after MAC confirms
#define NWK_MAX_DATABUFS_TOTAL      12    // Total number of buffers

// 1-255 (0 -> 256) X RTG_TIMER_INTERVAL
// A known shortcoming is that when a message is enqueued as "hold" for a
// sleeping device, the timer tick may have counted down to 1, so that msg
// will not be held as long as expected. If NWK_INDIRECT_MSG_TIMEOUT is set to 1
// the hold time will vary randomly from 0 - CNT_RTG_TIMER ticks.
// So the hold time will vary within this interval:
// { (NWK_INDIRECT_MSG_TIMEOUT-1)*CNT_RTG_TIMER,
//                                    NWK_INDIRECT_MSG_TIMEOUT*CNT_RTG_TIMER }

#define NWK_INDIRECT_CNT_RTG_TMR    1
// To hold msg for sleeping end devices for 30 secs:
// #define CNT_RTG_TIMER            1
// #define NWK_INDIRECT_MSG_TIMEOUT 30
// To hold msg for sleeping end devices for 30 mins:
// #define CNT_RTG_TIMER            60
// #define NWK_INDIRECT_MSG_TIMEOUT 30
// To hold msg for sleeping end devices for 30 days:
// #define CNT_RTG_TIMER            60
// #define NWK_INDIRECT_MSG_TIMEOUT (30 * 24 * 60)
// Maximum msgs to hold per associated device.
#define NWK_INDIRECT_MSG_MAX_PER    3
// Maximum total msgs to hold for all associated devices.
#define NWK_INDIRECT_MSG_MAX_ALL    \
                            (NWK_MAX_DATABUFS_TOTAL - NWK_INDIRECT_MSG_MAX_PER)

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * NWK GLOBAL VARIABLES
 */

// Variables for MAX list size
CONST deviceTableIndex_t gNWK_MAX_DEVICE_LIST = NWK_MAX_DEVICES;

// Variables for MAX data buffer levels
CONST uint8_t gNWK_MAX_DATABUFS_WAITING = NWK_MAX_DATABUFS_WAITING;
CONST uint8_t gNWK_MAX_DATABUFS_SCHEDULED = NWK_MAX_DATABUFS_SCHEDULED;
CONST uint8_t gNWK_MAX_DATABUFS_CONFIRMED = NWK_MAX_DATABUFS_CONFIRMED;
CONST uint8_t gNWK_MAX_DATABUFS_TOTAL = NWK_MAX_DATABUFS_TOTAL;

CONST uint8_t gNWK_INDIRECT_CNT_RTG_TMR = NWK_INDIRECT_CNT_RTG_TMR;
CONST uint8_t gNWK_INDIRECT_MSG_MAX_PER = NWK_INDIRECT_MSG_MAX_PER;
CONST uint8_t gNWK_INDIRECT_MSG_MAX_ALL = NWK_INDIRECT_MSG_MAX_ALL;

// Minimum lqi value that is required for association
uint8_t gMIN_TREE_LQI = MIN_LQI_COST_3;

// Statically defined Associated Device List
associated_devices_t AssociatedDevList[NWK_MAX_DEVICES];

CONST uint8_t gMAX_NWK_SEC_MATERIAL_TABLE_ENTRIES = MAX_NWK_SEC_MATERIAL_TABLE_ENTRIES;
CONST uint16_t gNWK_MIN_ROUTER_CHILDREN = NWK_MIN_ROUTER_CHILDREN;
CONST uint16_t gNWK_MIN_ENDDEVICE_CHILDREN = NWK_MIN_ENDDEVICE_CHILDREN;

CONFIG_ITEM rtgTableIndex_t gMAX_RTG_ENTRIES = MAX_RTG_ENTRIES;
CONFIG_ITEM srcRtgTableIndex_t gMAX_RTG_SRC_ENTRIES = MAX_RTG_SRC_ENTRIES;
CONFIG_ITEM rreqTableIndex_t gMAX_RREQ_ENTRIES = MAX_RREQ_ENTRIES;

CONFIG_ITEM neighborTableIndex_t gMAX_NEIGHBOR_ENTRIES = MAX_NEIGHBOR_ENTRIES;

 // Table of neighboring nodes (not including child nodes)
neighborEntry_t neighborTable[MAX_NEIGHBOR_ENTRIES];

CONST uint8_t gMAX_SOURCE_ROUTE = MAX_SOURCE_ROUTE;

CONST uint8_t gMAX_BROADCAST_QUEUED = MAX_BROADCAST_QUEUED;

CONST uint8_t gLINK_DOWN_TRIGGER = LINK_DOWN_TRIGGER;

CONST uint8_t gGOOD_LINK_COST = GOOD_LINK_COST;

CONST uint8_t gMAX_PASSIVE_ACK_CNT = MAX_PASSIVE_ACK_CNT;

// Routing table
rtgEntry_t rtgTable[MAX_RTG_ENTRIES];

rtgSrcEntry_t rtgSrcTable[MAX_RTG_SRC_ENTRIES];
uint16_t rtgSrcRelayList[MAX_SOURCE_ROUTE];


// Table of current RREQ packets in the network
rtDiscEntry_t rtDiscTable[MAX_RREQ_ENTRIES];

// Table of data broadcast packets currently in circulation.
bcastEntry_t bcastTable[MAX_BCAST];

// These 2 arrays are to be used as an array of struct { uint8_t, uint32_t }.
uint8_t bcastHoldHandle[MAX_BCAST];
uint32_t bcastHoldAckMask[MAX_BCAST];

CONFIG_ITEM bcastTableIndex_t gMAX_BCAST = MAX_BCAST;

// For tree addressing, this switch allows the allocation of a
// router address to an end device when end device address are
// all used up.  If this option is enabled, address space
// could be limited.
CONST uint8_t gNWK_TREE_ALLOCATE_ROUTERADDR_FOR_ENDDEVICE = FALSE;

// number of link status periods after the last received address conflict report
// (network status command)
CONST uint8_t gNWK_CONFLICTED_ADDR_EXPIRY_TIME = NWK_CONFLICTED_ADDR_EXPIRY_TIME;

#if defined ( ZIGBEE_FREQ_AGILITY ) || defined ( ZBIT )
CONST uint8_t gNWK_FREQ_AGILITY_ALL_MAC_ERRS = NWK_FREQ_AGILITY_ALL_MAC_ERRS;
#endif

// The time limited to one MTO RReq (Concentrator Announce) in milliseconds.
CONST uint16_t gMTO_RREQ_LIMIT_TIME = MTO_RREQ_LIMIT_TIME;

// The number of seconds a MTO routing entry will last.
CONST uint8_t gMTO_ROUTE_EXPIRY_TIME = MTO_ROUTE_EXPIRY_TIME;

// Route Discovery Request Default Radius
CONST uint8_t gDEFAULT_ROUTE_REQUEST_RADIUS = DEFAULT_ROUTE_REQUEST_RADIUS;

// Network message radius
CONST uint8_t gDEF_NWK_RADIUS = DEF_NWK_RADIUS;

#if ( ZSTACK_ROUTER_BUILD ) || defined ( ZBIT )
CONST uint16_t gLINK_STATUS_JITTER_MASK = LINK_STATUS_JITTER_MASK;
#endif

// Maximum number of devices either aged out or not recognized as child
CONST uint8_t gMAX_NOT_MYCHILD_DEVICES = MAX_NOT_MYCHILD_DEVICES;

// Child table Management timeout values
CONST uint32_t timeoutValue[15] =
{   10, // 0	10 seconds
     2, // 1	2 minutes
     4, // 2	4 minutes
     8, // 3	8 minutes
    16, // 4	16 minutes
    32, // 5	32 minutes
    64, // 6	64 minutes
   128, // 7	128 minutes
   256, // 8	256 minutes
   512, // 9	512 minutes
  1024, // 10	1024 minutes
  2048, // 11	2048 minutes
  4096, // 12	4096 minutes
  8192, // 13	8192 minutes
 16384 // 14	16384 minutes
};

// This table stores devices that have been aged out by the Child Aging Table
// mechanism or have never been a child of this device
nwkNotMyChild_t notMyChildList[MAX_NOT_MYCHILD_DEVICES];

CONST uint32_t gMAX_NWK_FRAMECOUNTER_CHANGES = MAX_NWK_FRAMECOUNTER_CHANGES;

#if defined ( FEATURE_MNP )
  uint8_t gMAX_MNP_QUEUED = MAX_MNP_QUEUED;
#endif

// This stores the max broadcast jitter when rebroadcasting packets (in 2 millisecond periods)
CONFIG_ITEM uint16_t gNWK_MAX_BROADCAST_JITTER = NWK_MAX_BROADCAST_JITTER;

CONST uint32_t gNWK_JOIN_SECURITY_TIMEOUT_PERIOD = NWK_JOIN_SECURITY_TIMEOUT_PERIOD;

/*********************************************************************
 * APS GLOBAL VARIABLES
 */

// The Maximum number of binding records
// This number is defined in f8wConfig.cfg - change it there.
CONFIG_ITEM bindTableIndex_t gNWK_MAX_BINDING_ENTRIES = NWK_MAX_BINDING_ENTRIES;

// The Maximum number of cluster IDs in a binding record
CONFIG_ITEM uint8_t gMAX_BINDING_CLUSTER_IDS = MAX_BINDING_CLUSTER_IDS;

CONST uint16_t gBIND_REC_SIZE = sizeof( BindingEntry_t );

// Binding Table
BindingEntry_t BindingTable[NWK_MAX_BINDING_ENTRIES];

// Maximum number allowed in the groups table.
CONFIG_ITEM uint8_t gAPS_MAX_GROUPS = APS_MAX_GROUPS;

// APS End Device Broadcast Table
#if ( ZG_BUILD_ENDDEVICE_TYPE )
  apsEndDeviceBroadcast_t apsEndDeviceBroadcastTable[APS_MAX_ENDDEVICE_BROADCAST_ENTRIES];
  uint8_t gAPS_MAX_ENDDEVICE_BROADCAST_ENTRIES = APS_MAX_ENDDEVICE_BROADCAST_ENTRIES;
#endif

/*********************************************************************
 * SECURITY GLOBAL VARIABLES
 */

// This is the default pre-configured key,
// change this to make a unique key
// SEC_KEY_LEN is defined in ssp.h.


#if (ZG_BUILD_JOINING_TYPE)
//Distributed key is only used by devices with joining capabilities (router and ZED)
CONFIG_ITEM uint8_t distributedDefaultKey[SEC_KEY_LEN] = DISTRIBUTED_GLOBAL_LINK_KEY;
#endif

#if defined ( DEFAULT_KEY )
CONFIG_ITEM uint8_t defaultKey[SEC_KEY_LEN] = DEFAULT_KEY;
#else
CONST uint8_t defaultKey[SEC_KEY_LEN] =
{
#if defined ( APP_TP ) || defined ( APP_TP2 )
  // Key for ZigBee Conformance Testing
  0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa
#else
  // Key for In-House Testing
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
#endif
};
#endif

// This is the default pre-configured Trust Center Link key,
// change this to make a unique key, SEC_KEY_LEN is defined in ssp.h.
CONST uint8_t defaultTCLinkKey[SEC_KEY_LEN] = DEFAULT_TC_LINK_KEY;

/*********************************************************************
 * STATUS STRINGS
 */
#if defined ( LCD_SUPPORTED )
  const char PingStr[]         = "Ping Rcvd from";
  const char AssocCnfStr[]     = "Assoc Cnf";
  const char SuccessStr[]      = "Success";
  const char EndDeviceStr[]    = "EndDevice:";
  const char ParentStr[]       = "Parent:";
  const char ZigbeeCoordStr[]  = "ZigBee Coord";
  const char NetworkIDStr[]    = "Network ID:";
  const char RouterStr[]       = "Router:";
  const char OrphanRspStr[]    = "Orphan Response";
  const char SentStr[]         = "Sent";
  const char FailedStr[]       = "Failed";
  const char AssocRspFailStr[] = "Assoc Rsp fail";
  const char AssocIndStr[]     = "Assoc Ind";
  const char AssocCnfFailStr[] = "Assoc Cnf fail";
  const char EnergyLevelStr[]  = "Energy Level";
  const char ScanFailedStr[]   = "Scan Failed";
#endif

/*********************************************************************
 * @fn       nwk_globals_init()
 *
 * @brief
 *
 *   Initialize nwk layer globals.  These are the system defaults and
 *   should be changed by the user here.  The default definitions are
 *   defined in nwk.h or nl_mede.h.
 *
 * @param   none
 *
 * @return  none
 */
void nwk_globals_init( void )
{
  AddrMgrInit( NWK_MAX_ADDRESSES );

#if defined ( ZIGBEE_FREQ_AGILITY )
  NwkFreqAgilityInit();
#endif

  if ( ( ZSTACK_ROUTER_BUILD ) && ( zgChildAgingEnable == TRUE ) )
  {
    // Set the function pointers for the Child Aging feature
    NwkInitChildAging();
  }
}

/*********************************************************************
 * @fn       NIB_init()
 *
 * @brief
 *
 *   Initialize attribute values in NIB
 *
 * @param   none
 *
 * @return  none
 */
void NIB_init()
{
  _NIB.SequenceNum = LO_UINT16(OsalPort_rand());

  _NIB.nwkProtocolVersion = ZB_PROT_VERS;
  _NIB.MaxDepth = MAX_NODE_DEPTH;
  _NIB.beaconOrder = BEACON_ORDER_NO_BEACONS;
  _NIB.superFrameOrder = BEACON_ORDER_NO_BEACONS;

  // BROADCAST SETTINGS:
  // *******************
  //   Broadcast Delivery Time
  //     - set to multiples of 100ms
  //     - should be 500ms more than the retry time
  //       -  "retry time" = PassiveAckTimeout * (MaxBroadcastRetries + 1)
  //   Passive Ack Timeout
  //     - set to multiples of 100ms
  _NIB.BroadcastDeliveryTime = zgBcastDeliveryTime;
  _NIB.PassiveAckTimeout     = zgPassiveAckTimeout;
  _NIB.MaxBroadcastRetries   = zgMaxBcastRetires;

  _NIB.ReportConstantCost = 0;
  _NIB.RouteDiscRetries = 0;
  _NIB.SecureAllFrames = USE_NWK_SECURITY;
  _NIB.nwkAllFresh = NWK_ALL_FRESH;

  _NIB.SecurityLevel = SECURITY_LEVEL;

  _NIB.SymLink = TRUE;

  _NIB.CapabilityFlags = ZDO_Config_Node_Descriptor.CapabilityFlags;

  _NIB.TransactionPersistenceTime = zgIndirectMsgTimeout;

  _NIB.RouteDiscoveryTime = zgRouteDiscoveryTime;
  _NIB.RouteExpiryTime = zgRouteExpiryTime;

  _NIB.nwkDevAddress = INVALID_NODE_ADDR;
  _NIB.nwkLogicalChannel = 0;
  _NIB.nwkCoordAddress = INVALID_NODE_ADDR;
  memset( _NIB.nwkCoordExtAddress, 0, Z_EXTADDR_LEN );
  _NIB.nwkPanId = INVALID_NODE_ADDR;

  osal_cpyExtAddr( _NIB.extendedPANID, zgExtendedPANID );

  _NIB.nwkKeyLoaded = FALSE;

  _NIB.nwkLinkStatusPeriod = NWK_LINK_STATUS_PERIOD;
  _NIB.nwkRouterAgeLimit = NWK_ROUTE_AGE_LIMIT;

  //MTO and source routing
  _NIB.nwkConcentratorDiscoveryTime = zgConcentratorDiscoveryTime;
  _NIB.nwkIsConcentrator = zgConcentratorEnable;
  _NIB.nwkConcentratorRadius = zgConcentratorRadius;
  _NIB.nwkUseMultiCast = MULTICAST_ENABLED;

#if defined ( NV_RESTORE )
  if ( osal_nv_read( ZCD_NV_NWKMGR_ADDR, 0, sizeof( _NIB.nwkManagerAddr ),
                     &_NIB.nwkManagerAddr ) != SUCCESS )
#endif
  {
    _NIB.nwkManagerAddr = 0x0000;
  }

  NLME_SetUpdateID( 0 );
  _NIB.nwkTotalTransmissions = 0;
}

/*********************************************************************
 * @fn       nwk_Status()
 *
 * @brief
 *
 *   Status report.
 *
 * @param   statusCode
 * @param   statusValue
 *
 * @return  none
 */
void nwk_Status( uint16_t statusCode, uint16_t statusValue )
{
#if defined ( SERIAL_DEBUG_SUPPORTED ) || (defined ( LEGACY_LCD_DEBUG ) && defined (LCD_SUPPORTED))
  switch ( statusCode )
  {
    case NWK_STATUS_COORD_ADDR:
      if ( ZSTACK_ROUTER_BUILD )
      {

      if ( _NIB.nwkDevAddress == NWK_PAN_COORD_ADDR )
      {
        HalLcdWriteString( (char*)ZigbeeCoordStr, HAL_LCD_DEBUG_LINE_1 );
      }
      if ( (_NIB.nwkDevAddress != NWK_PAN_COORD_ADDR) &&
           (_NIB.nwkDevAddress != INVALID_NODE_ADDR) )
      {
        HalLcdWriteStringValue( (char*)RouterStr, _NIB.nwkDevAddress, 16, HAL_LCD_DEBUG_LINE_1 );
      }

        HalLcdWriteStringValue( (char*)NetworkIDStr, statusValue, 16, HAL_LCD_DEBUG_LINE_2 );
        BuzzerControl( BUZZER_BLIP );
      }
      break;

    case NWK_STATUS_ROUTER_ADDR:
      if ( ZSTACK_ROUTER_BUILD )
      {
        HalLcdWriteStringValue( (char*)RouterStr, statusValue, 16, HAL_LCD_DEBUG_LINE_1 );
      }
      break;

    case NWK_STATUS_ORPHAN_RSP:
      if ( ZSTACK_ROUTER_BUILD )
      {
        if ( statusValue == ZSuccess )
          HalLcdWriteScreen( (char*)OrphanRspStr, (char*)SentStr );
        else
          HalLcdWriteScreen( (char*)OrphanRspStr, (char*)FailedStr );
      }
      break;

    case NWK_ERROR_ASSOC_RSP:
      if ( ZSTACK_ROUTER_BUILD )
      {
        HalLcdWriteString( (char*)AssocRspFailStr, HAL_LCD_DEBUG_LINE_1 );
        HalLcdWriteValue( (uint32_t)(statusValue), 16, HAL_LCD_DEBUG_LINE_2 );
      }
      break;

    case NWK_STATUS_ED_ADDR:
      if ( ZSTACK_END_DEVICE_BUILD )
      {
        HalLcdWriteStringValue( (char*)EndDeviceStr, statusValue, 16, HAL_LCD_DEBUG_LINE_1 );
      }
      break;

    case NWK_STATUS_PARENT_ADDR:
            HalLcdWriteStringValue( (char*)ParentStr, statusValue, 16, HAL_LCD_DEBUG_LINE_2 );
      break;

    case NWK_STATUS_ASSOC_CNF:
      HalLcdWriteScreen( (char*)AssocCnfStr, (char*)SuccessStr );
      break;

    case NWK_ERROR_ASSOC_CNF_DENIED:
      HalLcdWriteString((char*)AssocCnfFailStr, HAL_LCD_DEBUG_LINE_1 );
      HalLcdWriteValue( (uint32_t)(statusValue), 16, HAL_LCD_DEBUG_LINE_2 );
      break;

    case NWK_ERROR_ENERGY_SCAN_FAILED:
      HalLcdWriteScreen( (char*)EnergyLevelStr, (char*)ScanFailedStr );
      break;
  }
#endif
}

/*********************************************************************
 * @fn      nwk_adjustDelay()
 *
 * @brief   Adjust the retransmit delay. Modify this function to
 *          change the default delay behavior.
 *
 * @param   existingDelay - default delay
 * @param   confirmStatus - data confirm status
 * @param   bufOptions - network buffer options
 *
 * @return  delay value - this number is the number of
 *          network event ticks (~2ms).
 */
uint16_t nwk_adjustDelay( uint16_t existingDelay, uint8_t confirmStatus, uint16_t bufOptions )
{
  uint16_t result = existingDelay;
  uint16_t mask = 0;       // Random mask
  uint16_t startValue = 0; // Start value

  switch ( confirmStatus )
  {
    case ZMacTransactionOverFlow:
    case ZMacChannelAccessFailure:
      if ( bufOptions & (HANDLE_DELAY | HANDLE_HI_DELAY) )
      {
        // Delay 4 - 18 ms
        mask = 0x0007;
        startValue = 2;
      }
      break;

    case ZNwkNoRoute:
    case ZMAC_NO_RESOURCES:
    case ZMacNoACK:
    default:
      // leave default delay
      mask = 0;
      startValue = 0;
      break;
  }

  if ( (mask > 0) && (startValue > 0) )
  {
    result = (OsalPort_rand() & mask) + startValue;
  }

  return ( result );
}

/*********************************************************************
 * @fn          NLME_InitializeDefaultPollRates
 *
 * @brief       Initialize the default values of the poll rates used by the stack
 *
 * @return      none
 */
void nwk_InitializeDefaultPollRates(void)
{
    if( ZG_DEVICE_ENDDEVICE_TYPE && zgRxAlwaysOn == FALSE )
    {
        //Disable all Poll Rates mode
        nwk_SetCurrentPollRateType(0xFFFF,FALSE);

        //Configure the stack pollrates defined
        nwk_SetConfigPollRate(POLL_RATE_TYPE_DEFAULT, POLL_RATE);
        nwk_SetConfigPollRate(POLL_RATE_TYPE_JOIN_REJOIN, REJOIN_POLL_RATE);
        nwk_SetConfigPollRate(POLL_RATE_TYPE_QUEUED, QUEUED_POLL_RATE);
        nwk_SetConfigPollRate(POLL_RATE_TYPE_RESPONSE, RESPONSE_POLL_RATE);
        nwk_SetConfigPollRate(POLL_RATE_TYPE_GENERIC_1_SEC, GENERIC_POLL_RATE_1_SEC);

        //Update the Poll Rate table in NV
        osal_nv_write(ZCD_NV_POLL_RATE, sizeof(nwk_pollRateTable), &nwk_pollRateTable);
    }
}

/*********************************************************************
*********************************************************************/
