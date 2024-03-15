/**************************************************************************************************
  Filename:       nwk_globals.h
  Revised:        $Date: 2015-01-21 19:28:52 -0800 (Wed, 21 Jan 2015) $
  Revision:       $Revision: 41954 $

  Description:    User definable Z-Stack parameters.


  Copyright 2007-2015 Texas Instruments Incorporated.

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

#ifndef ZGLOBALS_H
#define ZGLOBALS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcomdef.h"

/*********************************************************************
 * MACROS
 */
#if defined( BUILD_ALL_DEVICES ) && !defined( ZSTACK_DEVICE_BUILD )
  #define ZSTACK_DEVICE_BUILD  (DEVICE_BUILD_COORDINATOR | DEVICE_BUILD_ROUTER | DEVICE_BUILD_ENDDEVICE)
#endif

// Setup to work with the existing (old) compile flags
#if !defined ( ZSTACK_DEVICE_BUILD )
  #if defined ( ZDO_COORDINATOR )

    #define ZSTACK_DEVICE_BUILD  (DEVICE_BUILD_COORDINATOR)
  #elif defined ( RTR_NWK )

    #define ZSTACK_DEVICE_BUILD  (DEVICE_BUILD_ROUTER)
  #else

    #define ZSTACK_DEVICE_BUILD  (DEVICE_BUILD_ENDDEVICE)
  #endif
#endif

// Use the following to macros to make device type decisions
#define ZG_BUILD_COORDINATOR_TYPE  (ZSTACK_DEVICE_BUILD & DEVICE_BUILD_COORDINATOR)
#define ZG_BUILD_RTR_TYPE          (ZSTACK_DEVICE_BUILD & (DEVICE_BUILD_COORDINATOR | DEVICE_BUILD_ROUTER))
#define ZG_BUILD_ENDDEVICE_TYPE    (ZSTACK_DEVICE_BUILD & DEVICE_BUILD_ENDDEVICE)
#define ZG_BUILD_RTRONLY_TYPE      (ZSTACK_DEVICE_BUILD == DEVICE_BUILD_ROUTER)
#define ZG_BUILD_JOINING_TYPE      (ZSTACK_DEVICE_BUILD & (DEVICE_BUILD_ROUTER | DEVICE_BUILD_ENDDEVICE))
#define ZG_BUILD_ALL_DEVICES_TYPE  (ZSTACK_DEVICE_BUILD & (DEVICE_BUILD_COORDINATOR | DEVICE_BUILD_ROUTER | DEVICE_BUILD_ENDDEVICE))

#if ( ZSTACK_DEVICE_BUILD == DEVICE_BUILD_COORDINATOR )
  #define ZG_DEVICE_COORDINATOR_TYPE 1
#else
  #define ZG_DEVICE_COORDINATOR_TYPE (zgDeviceLogicalType == ZG_DEVICETYPE_COORDINATOR)
#endif

#if ( ZSTACK_DEVICE_BUILD == (DEVICE_BUILD_ROUTER | DEVICE_BUILD_COORDINATOR) )
  #define ZG_DEVICE_RTR_TYPE 1
#else
  #define ZG_DEVICE_RTR_TYPE ((zgDeviceLogicalType == ZG_DEVICETYPE_COORDINATOR) || (zgDeviceLogicalType == ZG_DEVICETYPE_ROUTER))
#endif

#if ( ZSTACK_DEVICE_BUILD == DEVICE_BUILD_ROUTER )
  #define ZG_DEVICE_RTRONLY_TYPE  1
#else
  #define ZG_DEVICE_RTRONLY_TYPE (zgDeviceLogicalType == ZG_DEVICETYPE_ROUTER)
#endif


#if ( ZSTACK_DEVICE_BUILD == DEVICE_BUILD_ENDDEVICE )
  #define ZG_DEVICE_ENDDEVICE_TYPE 1
#else
  #define ZG_DEVICE_ENDDEVICE_TYPE (zgDeviceLogicalType == ZG_DEVICETYPE_ENDDEVICE)
#endif

#define ZG_DEVICE_JOINING_TYPE     ((zgDeviceLogicalType == ZG_DEVICETYPE_ROUTER) || (zgDeviceLogicalType == ZG_DEVICETYPE_ENDDEVICE))

#if ( ZG_BUILD_RTR_TYPE )
  #if ( ZG_BUILD_ENDDEVICE_TYPE )
    #define ZSTACK_ROUTER_BUILD         (ZG_BUILD_RTR_TYPE && ZG_DEVICE_RTR_TYPE)
  #else
    #define ZSTACK_ROUTER_BUILD         1
  #endif
#else
  #define ZSTACK_ROUTER_BUILD           0
#endif

#if ( ZG_BUILD_ENDDEVICE_TYPE )
  #if ( ZG_BUILD_RTR_TYPE )
    #define ZSTACK_END_DEVICE_BUILD     (ZG_BUILD_ENDDEVICE_TYPE && ZG_DEVICE_ENDDEVICE_TYPE)
  #else
    #define ZSTACK_END_DEVICE_BUILD     1
  #endif
#else
  #define ZSTACK_END_DEVICE_BUILD       0
#endif

#if !defined ( BUILD_FLEXABLE )
  #define BUILD_FLEXABLE 0
#endif

#if !defined ( BEACON_MESH )
#define BEACON_MESH 0
#endif


/*********************************************************************
 * CONSTANTS
 */

// Values for ZCD_NV_LOGICAL_TYPE (zgDeviceLogicalType)
#define ZG_DEVICETYPE_COORDINATOR      0x00
#define ZG_DEVICETYPE_ROUTER           0x01
#define ZG_DEVICETYPE_ENDDEVICE        0x02

//#define DEVICE_LOGICAL_TYPE ZG_DEVICETYPE_COORDINATOR

// Default Device Logical Type
#if !defined ( DEVICE_LOGICAL_TYPE )
  #if ( ZG_BUILD_COORDINATOR_TYPE )
    // If capable, default to coordinator
    #define DEVICE_LOGICAL_TYPE ZG_DEVICETYPE_COORDINATOR
  #elif ( ZG_BUILD_RTR_TYPE )
    #define DEVICE_LOGICAL_TYPE ZG_DEVICETYPE_ROUTER
  #elif ( ZG_BUILD_ENDDEVICE_TYPE )
    // Must be an end device
    #define DEVICE_LOGICAL_TYPE ZG_DEVICETYPE_ENDDEVICE
  #else
    #error ZSTACK_DEVICE_BUILD must be defined as something!
  #endif
#endif

// Transmission retries numbers
#if !defined ( MAX_MISSING_MAC_ACK_LINK_FAILURE )
  #define MAX_MISSING_MAC_ACK_LINK_FAILURE 3
#endif
#if !defined ( MAX_DATA_RETRIES )
  #define MAX_DATA_RETRIES         2
#endif

// NIB parameters
#if !defined ( MAX_BCAST_RETRIES )
  #define MAX_BCAST_RETRIES        2
#endif
#if !defined ( PASSIVE_ACK_TIMEOUT )
  #define PASSIVE_ACK_TIMEOUT      5
#endif
#if !defined ( BCAST_DELIVERY_TIME )
  #define BCAST_DELIVERY_TIME      30
#endif

#if !defined ( ROUTE_DISCOVERY_TIME )
  #define ROUTE_DISCOVERY_TIME     5
#endif

#if !defined ( APS_DEFAULT_MAXBINDING_TIME )
  #define APS_DEFAULT_MAXBINDING_TIME 16000
#endif

#if !defined ( APS_DEFAULT_NONMEMBER_RADIUS )
  #define APS_DEFAULT_NONMEMBER_RADIUS   2
#endif

#if !defined ( APSF_DEFAULT_WINDOW_SIZE )
  #define APSF_DEFAULT_WINDOW_SIZE       1
#endif

#if !defined ( APSF_DEFAULT_INTERFRAME_DELAY )
  #define APSF_DEFAULT_INTERFRAME_DELAY  50
#endif

// Concentrator values
#if !defined ( CONCENTRATOR_ENABLE )
  #define CONCENTRATOR_ENABLE          false // true if concentrator is enabled
#endif

#if !defined ( CONCENTRATOR_DISCOVERY_TIME )
  #define CONCENTRATOR_DISCOVERY_TIME  0
#endif

#if !defined ( CONCENTRATOR_ROUTE_CACHE )
  #define CONCENTRATOR_ROUTE_CACHE     false // true if concentrator has route cache
#endif

// The hop count radius for concentrator route discoveries
#if !defined ( CONCENTRATOR_RADIUS )
  #define CONCENTRATOR_RADIUS          0x0a
#endif

#if !defined ( START_DELAY )
  #define START_DELAY                  0x0a
#endif

#if !defined ( SAPI_ENDPOINT )
  #define SAPI_ENDPOINT                0xe0
#endif

#define ZG_STARTUP_CLEAR               0x00
#define ZG_STARTUP_SET                 0xFF

// Backwards compatible - AMI changed name to SE
#if defined ( AMI_PROFILE ) || defined ( SE_PROFILE )
#define TC_LINKKEY_JOIN
#endif

// APS Duplication Rejection Table Values
#if !defined ( APS_DUP_REJ_ENTRIES )
  #define APS_DUP_REJ_ENTRIES                     5
#endif
#if !defined ( DEFAULT_APS_DUP_REJ_TIMEOUT_INCREMENT )
  #define DEFAULT_APS_DUP_REJ_TIMEOUT_INCREMENT   1000
#endif
#if !defined ( DEFAULT_APS_DUP_REJ_TIMEOUT )
  #define DEFAULT_APS_DUP_REJ_TIMEOUT             10
#endif

// Child aging management default values
// Values are specified in table of nwk_globals.h module
//timeoutValue[15]
//    10, // 0	10 seconds
//     2, // 1	2 minutes
//     4, // 2	4 minutes
//     8, // 3	8 minutes
//    16, // 4	16 minutes
//    32, // 5	32 minutes
//    64, // 6	64 minutes
//   128, // 7	128 minutes
//   256, // 8	256 minutes
//   512, // 9	512 minutes
//  1024, // 10	1024 minutes
//  2048, // 11	2048 minutes
//  4096, // 12	4096 minutes
//  8192, // 13	8192 minutes
// 16384 // 14	16384 minutes
//

//Timeout after which an EndDevice will be removed from from the indirect MAC messages queue
   // NOTE: End devices which poll rate is slower than this will not receive the leave request
#if !defined ( NWK_END_DEVICE_LEAVE_TIMEOUT )
  #define NWK_END_DEVICE_LEAVE_TIMEOUT 9
#endif

// Value used by END DEVICE when sending End Device Timeout Request
#if !defined ( END_DEV_CONFIGURATION )
  #define END_DEV_CONFIGURATION   0     // Per ZigBee Core spec R21, 0 is the only valid value
#endif

//Number of keep alive messages that the stack will send to parent per timeout, to ensure it wont get expired.
//Zigbee spec recommends this value to be at least 3. This value cannot be 0.
//NOTE: If parent supports MAC DATA POLL and application configures a bigger poll rate than
//END_DEV_TIMEOUT_VALUE/END_DEVICE_KEEP_ALIVE_MESSAGES_PER_TIMEOUT Z-stack will ignore application
//poll rate and will use END_DEV_TIMEOUT_VALUE/END_DEVICE_KEEP_ALIVE_MESSAGES_PER_TIMEOUT instead
//to ensure the device do not get expired.
#if !defined (END_DEVICE_KEEP_ALIVE_MESSAGES_PER_TIMEOUT)
  #define END_DEVICE_KEEP_ALIVE_MESSAGES_PER_TIMEOUT    3
#endif

#if (END_DEVICE_KEEP_ALIVE_MESSAGES_PER_TIMEOUT==0)
  #error "Invalid valid value for END_DEVICE_KEEP_ALIVE_MESSAGES_PER_TIMEOUT"
#endif

#if !defined (MAX_SUPPORTED_ENDPOINTS)
#define MAX_SUPPORTED_ENDPOINTS 5
#endif

// number of secure rejoin attempts will perform before trying
// unseure/trust center rejoin
#if !defined (BDB_MAX_SECURE_REJOIN_ATTEMPTS)
#define BDB_MAX_SECURE_REJOIN_ATTEMPTS 3
#endif

// Determines whether or not we will attempt to perform unsecure rejoins
#if !defined (BDB_ATTEMPT_UNSECURE_REJOIN)
#define BDB_ATTEMPT_UNSECURE_REJOIN TRUE
#endif

// Determines whether or not routers will perform silent rejoin
// By default, routers will perform silent rejoins
#if !defined (ZR_SILENT_REJOIN)
#define ZR_SILENT_REJOIN TRUE
#endif

//--------------------------------------------------------------------
// Security settings
//--------------------------------------------------------------------

// Values for zgApsLinkKeyType
#define ZG_UNIQUE_LINK_KEY        0x00
#define ZG_GLOBAL_LINK_KEY        0x01

// Values for KeyAttributes
#define ZG_PROVISIONAL_KEY          0x00  //Used for IC derived keys
#define ZG_UNVERIFIED_KEY           0x01  //Unique key that is not verified
#define ZG_VERIFIED_KEY             0x02  //Unique key that got verified by ZC
//Internal keyAttribute definitions
#define ZG_DISTRIBUTED_DEFAULT_KEY  0xFC  //Use default key to join
#define ZG_NON_R21_NWK_JOINED       0xFD  //Joined a network which is not R21 nwk, so TCLK process finished.
#define ZG_VERIFIED_KEY_JOINING_DEV 0xFE  //Unique key that got verified by Joining device. This means that key is stored as plain text (not seed hashed)
#define ZG_DEFAULT_KEY              0xFF  //Entry using default key


#define ZG_IC_NOT_SUPPORTED           0x00
#define ZG_IC_SUPPORTED_NOT_REQUIRED  0x01
#define ZG_IC_MUST_USED               0x02

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * NWK GLOBAL VARIABLES
 */
// Variables for number of transmission retries
extern uint8_t zgMaxDataRetries;
extern uint8_t zgMaxMissingMacAckLinkFailure;

extern uint32_t zgDefaultChannelList;
#define zgDefaultStartingScanDuration  bdbAttributes.bdbScanDuration
extern uint8_t zgStackProfile;

extern uint8_t zgIndirectMsgTimeout;
extern uint8_t zgSecurePermitJoin;
extern uint8_t zgAllowRejoinsWithWellKnownKey;
extern uint8_t zgAllowInstallCodes;
extern uint8_t zgAllowRemoteTCPolicyChange;
extern uint8_t zgApsTrustCenterAddr[];
extern uint8_t zgRouteDiscoveryTime;
extern uint8_t zgRouteExpiryTime;

extern uint8_t zgExtendedPANID[];

extern uint8_t zgMaxBcastRetires;
extern uint8_t zgPassiveAckTimeout;
extern uint8_t zgBcastDeliveryTime;

extern uint8_t zgConcentratorEnable;
extern uint8_t zgConcentratorDiscoveryTime;
extern uint8_t zgConcentratorRadius;
extern uint8_t zgConcentratorRC;
extern uint8_t zgNwkSrcRtgExpiryTime;

extern uint8_t zgRouterOffAssocCleanup;

extern uint8_t zgNwkLeaveRequestAllowed;

extern uint8_t zgNwkProcessLocalLeaveCnf;

extern uint8_t zgNwkParentInformation;

extern uint8_t zgNwkEndDeviceTimeoutDefault;

extern uint8_t zgNwkEndDeviceLeaveTimeoutDefault;

extern uint8_t zgEndDeviceKeepAliveMessagesPerTimeout;
extern uint8_t zgEndDeviceTimeoutValue;

extern uint8_t zgEndDeviceConfiguration;

extern uint8_t zgChildAgingEnable;

extern uint8_t zTouchLinkNwkStartRtr;

extern uint8_t zgAllowRadioRxOff;

extern uint8_t zgRxAlwaysOn;

/*********************************************************************
 * APS GLOBAL VARIABLES
 */

extern uint8_t  zgApscMaxFrameRetries;
extern uint16_t zgApscAckWaitDurationPolled;
extern uint8_t  zgApsAckWaitMultiplier;
extern uint16_t zgApsDefaultMaxBindingTime;
extern uint8_t  zgApsUseExtendedPANID[Z_EXTADDR_LEN];
extern uint8_t  zgApsUseInsecureJoin;
extern uint8_t  zgApsNonMemberRadius;

extern uint16_t zgApscDupRejTimeoutInc;
extern uint8_t  zgApscDupRejTimeoutCount;
extern uint16_t zgApsMinDupRejTableSize;

#if defined ( APP_TP2 )
extern uint8_t guApsMaxFragBlockSize;
#endif

/*********************************************************************
 * SECURITY GLOBAL VARIABLES
 */

extern uint8_t zgPreConfigKeys;
extern uint8_t zgApsLinkKeyType;
extern uint8_t zgUseDefaultTCLK;

#if defined ( APP_TP2_TEST_MODE )
#define TP_GU_ALL            0
#define TP_GU_SEC_ONLY       1
#define TP_GU_NONSEC_ONLY    2
#define TP_GU_BOTH           2   // use both NoAPS and APS security

extern uint8_t guTxApsSecON;
extern uint8_t guEnforceRxApsSec;
extern uint8_t guEnableKeyExchange;
extern uint16_t guNextJoinerNwkAddr;
#endif

extern uint8_t zgApsAllowR19Sec;
extern uint8_t zgSwitchCoordKey;
extern uint8_t zgSwitchCoordKeyIndex;

extern uint8_t zgClearTCLKOnDeviceLeft;

/*********************************************************************
 * ZDO GLOBAL VARIABLES
 */

extern uint16_t zgConfigPANID;
extern uint8_t  zgDeviceLogicalType;
extern uint8_t  zgNwkMgrMinTransmissions;
#if !defined MT_TASK
extern uint8_t zgZdoDirectCB;
#endif

/*********************************************************************
 * BDB GLOBAL VARIABLES
 */

extern uint8_t zgBdbAttemptUnsecureRejoin;
extern uint8_t zgBdbMaxSecureRejoinAttempts;

/*********************************************************************
 * APPLICATION VARIABLES
 */

extern uint8_t zgNwkMgrMode;

extern uint8_t zgRouterSilentRejoin;

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Initialize the Z-Stack Globals.
 */
extern uint8_t zgInit( void );

/*
 * Initialize the RAM Items table with NV values.
 *    setDefault - if calling from outside ZGlobal use FALSE.
 */
extern void zgInitItems( uint8_t setDefault );

/*
 * Get Startup Options (ZCD_NV_STARTUP_OPTION NV Item)
 */
extern uint8_t zgReadStartupOptions( void );

/*
 * Write Startup Options (ZCD_NV_STARTUP_OPTION NV Item)
 *
 *      action - ZG_STARTUP_SET set bit, ZG_STARTUP_CLEAR to clear bit.
 *               The set bit is an OR operation, and the clear bit is an
 *               AND ~(bitOptions) operation.
 *      bitOptions - which bits to perform action on:
 *                      ZCD_STARTOPT_DEFAULT_CONFIG_STATE
 *                      ZDC_STARTOPT_DEFAULT_NETWORK_STATE
 *
 * Returns - ZSUCCESS if successful
 */
extern uint8_t zgWriteStartupOptions( uint8_t action, uint8_t bitOptions );

/*
 *  Set RAM variables from set-NV, if it exist in the zgItemTable
 */
extern void zgSetItem( uint16_t id, uint16_t len, void *buf );

/*********************************************************************
*********************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* ZGLOBALS_H */
