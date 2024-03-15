/**
   @file  zstack.h
   $Date: 2015-02-12 12:13:13 -0800 (Thu, 12 Feb 2015) $
   $Revision: 42527 $

   @brief ZStack API structures

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
#ifndef ZSTACK_H
#define ZSTACK_H

#include <stdbool.h>
#include <stdint.h>

#include "zglobals.h"
#include "nl_mede.h"
#include "bdb_interface.h"
#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
#include "cgp_stub.h"
#endif

#if defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
#include "bdb_touchlink.h"
#include "bdb_tl_commissioning.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
// Constants and definitions
//*****************************************************************************

/** Extended (IEEE) Address Length */
#if !defined (EXTADDR_LEN)
#define EXTADDR_LEN 8
#endif

/** Default radius - (2 * maximumm depth) */
#define DEFAULT_RADIUS  30


#if ZG_BUILD_COORDINATOR_TYPE
//By default, Coordiantor has Formation selected in the UI menu
#define DEFAULT_COMISSIONING_MODE (BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_NWK_FORMATION | BDB_COMMISSIONING_MODE_FINDING_BINDING)
#else
//By default, joining devices such as Router and ZED do not have formation selected. It can be enabled in the UI if needed.
#define DEFAULT_COMISSIONING_MODE (BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_FINDING_BINDING)
#endif

// Commissioning Modes availables
/** Commissioning mode to perform Touchlink commissioning as initiator */
#define BDB_COMMISSIONING_MODE_INITIATOR_TL         (1<<0)
/** Commissioning mode to perform network steering commissioning */
#define BDB_COMMISSIONING_MODE_NWK_STEERING         (1<<1)
/** Commissioning mode to perform network formation */
#define BDB_COMMISSIONING_MODE_NWK_FORMATION        (1<<2)
/** Commissioning mode to perform finding and binding on the active F&B
 * endpoint. Refer to Zstackapi_bdbSetIdentifyActiveEndpointReq */
#define BDB_COMMISSIONING_MODE_FINDING_BINDING      (1<<3)

typedef enum
{
  /** Instruct joining node to use Default Global Trust Center link key.
  No key buffer requiered */
  zstack_UseDefaultGlobalTrustCenterLinkKey,
  /** Instruct the joining node to use the provided install code (16 bytes
  + 2 CRC bytes) to derive APS Link key to be used during joining */
  zstack_UseInstallCode,
  /** Instruct the joining node to use the provided install code (16 bytes
  + 2 CRC bytes) to derive APS Link key to be used during joining.
  If it fails to decrypt Transport Key, it will automatically try Default
  Global Trust Center Link Key */
  zstack_UseInstallCodeWithFallback,
  /** Instruct the joining node to use the provided APS Link key to be used
  during joining (key size is 16 bytes) */
  zstack_UseAPSKey,
  /** Instruct the joining node to use the provided APS Link key to be used
  during joining (key size is 16 bytes). If it fails to decrypt Transport
  Key, it will automatically try Default Global Trust Center Link Key  */
  zstack_UseAPSKeyWithFallback,
}zstack_CentralizedLinkKeyModes_t;



/** General collection of status values */
typedef enum
{
    zstack_ZStatusValues_ZSuccess = 0x00,
    zstack_ZStatusValues_ZFailure = 0x01,
    zstack_ZStatusValues_ZInvalidParameter = 0x02,
    zstack_ZStatusValues_ZDecodeError = 0x03,
    zstack_ZStatusValues_ZMemError = 0x10,
    zstack_ZStatusValues_ZBufferFull = 0x11,
    zstack_ZStatusValues_ZUnsupportedMode = 0x12,
    zstack_ZStatusValues_ZMacMemError = 0x13,
    zstack_ZStatusValues_ZSapiInProgress = 0x20,
    zstack_ZStatusValues_ZSapiTimeout = 0x21,
    zstack_ZStatusValues_ZSapiInit = 0x22,
    zstack_ZStatusValues_ZNotAuthorized = 0x7E,
    zstack_ZStatusValues_ZMalformedCmd = 0x80,
    zstack_ZStatusValues_ZUnsupClusterCmd = 0x81,
    zstack_ZStatusValues_ZOtaAbort = 0x95,
    zstack_ZStatusValues_ZOtaImageInvalid = 0x96,
    zstack_ZStatusValues_ZOtaWaitForData = 0x97,
    zstack_ZStatusValues_ZOtaNoImageAvailable = 0x98,
    zstack_ZStatusValues_ZOtaRequireMoreImage = 0x99,
    zstack_ZStatusValues_ZApsFail = 0xb1,
    zstack_ZStatusValues_ZApsTableFull = 0xb2,
    zstack_ZStatusValues_ZApsIllegalRequest = 0xb3,
    zstack_ZStatusValues_ZApsInvalidBinding = 0xb4,
    zstack_ZStatusValues_ZApsUnsupportedAttrib = 0xb5,
    zstack_ZStatusValues_ZApsNotSupported = 0xb6,
    zstack_ZStatusValues_ZApsNoAck = 0xb7,
    zstack_ZStatusValues_ZApsDuplicateEntry = 0xb8,
    zstack_ZStatusValues_ZApsNoBoundDevice = 0xb9,
    zstack_ZStatusValues_ZApsNotAllowed = 0xba,
    zstack_ZStatusValues_ZApsNotAuthenticated = 0xbb,
    zstack_ZStatusValues_ZSecNoKey = 0xa1,
    zstack_ZStatusValues_ZSecOldFrmCount = 0xa2,
    zstack_ZStatusValues_ZSecMaxFrmCount = 0xa3,
    zstack_ZStatusValues_ZSecCcmFail = 0xa4,
    zstack_ZStatusValues_ZNwkInvalidParam = 0xc1,
    zstack_ZStatusValues_ZNwkInvalidRequest = 0xc2,
    zstack_ZStatusValues_ZNwkNotPermitted = 0xc3,
    zstack_ZStatusValues_ZNwkStartupFailure = 0xc4,
    zstack_ZStatusValues_ZNwkAlreadyPresent = 0xc5,
    zstack_ZStatusValues_ZNwkSyncFailure = 0xc6,
    zstack_ZStatusValues_ZNwkTableFull = 0xc7,
    zstack_ZStatusValues_ZNwkUnknownDevice = 0xc8,
    zstack_ZStatusValues_ZNwkUnsupportedAttribute = 0xc9,
    zstack_ZStatusValues_ZNwkNoNetworks = 0xca,
    zstack_ZStatusValues_ZNwkLeaveUnconfirmed = 0xcb,
    zstack_ZStatusValues_ZNwkNoAck = 0xcc,
    zstack_ZStatusValues_ZNwkNoRoute = 0xcd,
    zstack_ZStatusValues_ZAfDuplicateEndpoint = 0xd0,
    zstack_ZStatusValues_ZAfEndpointMax = 0xd1,
    zstack_ZStatusValues_ZIcallNoMsg = 0x30,
    zstack_ZStatusValues_ZIcallTimeout = 0x31
} zstack_ZStatusValues;

/** Address types */
typedef enum
{
    //! Address not present
    zstack_AFAddrMode_NONE = 0,
    //! Group Address (uint16_t)
    zstack_AFAddrMode_GROUP = 1,
    //! Short Address (uint16_t)
    zstack_AFAddrMode_SHORT = 2,
    //! Extended Address (8 bytes/64 bits)
    zstack_AFAddrMode_EXT = 3,
    //! Broadcast Address (uint16_t)
    zstack_AFAddrMode_BROADCAST = 15,
} zstack_AFAddrMode;

/** Reset types */
typedef enum
{
    //! Target Device Reset
    zstack_ResetTypes_DEVICE = 0,
    //! Serial Bootloader Reset
    zstack_ResetTypes_SERIAL_BOOTLOADER = 1,
} zstack_ResetTypes;

/** Reset reasons */
typedef enum
{
    zstack_ResetReasons_POWERUP = 0,
    zstack_ResetReasons_EXTERNAL = 1,
    zstack_ResetReasons_WATCHDOG = 2,
} zstack_ResetReasons;

/** Logical Device Types */
typedef enum
{
    zstack_LogicalTypes_COORDINATOR = 0,
    zstack_LogicalTypes_ROUTER = 1,
    zstack_LogicalTypes_ENDDEVICE = 2,
    zstack_LogicalTypes_UNKNOWN = 3,
} zstack_LogicalTypes;

/**
 * Network Latency values, for now only use
 * zstack_NetworkLatency_NO_LATENCY_REQS, for the AF Register Endpoint
 * request (@ref zstack_afRegisterReq_t)
 */
typedef enum
{
    zstack_NetworkLatency_NO_LATENCY_REQS = 0,
    zstack_NetworkLatency_FAST_BEACONS = 1,
    zstack_NetworkLatency_SLOW_BEACONDS = 2,
} zstack_NetworkLatency;

/**
 * Response type request for ZDO Nwk request (@ref zstack_zdoNwkAddrReq_t)
 * and IEEE Address requests (@ref zstack_zdoIeeeAddrReq_t)
 */
typedef enum
{
    /** The response will not contain a list of associated devices */
    zstack_NwkAddrReqType_SINGLE_DEVICE = 0,
    /** The response will contain a list of associated devices */
    zstack_NwkAddrReqType_EXTENDED_W_ASSOCDEVS = 1,
} zstack_NwkAddrReqType;

/**
 * Power Level values for the Power Descriptor
 * (@ref zstack_PowerDescriptor_t)
 */
typedef enum
{
    /** Critical power level */
    zstack_PowerLevel_LEVEL_CRITICAL = 0x00,
    /** Power level at 33% */
    zstack_PowerLevel_LEVEL_33 = 0x04,
    /** Power level at 66% */
    zstack_PowerLevel_LEVEL_66 = 0x08,
    /** Power level at 100% */
    zstack_PowerLevel_LEVEL_100 = 0x0C,
} zstack_PowerLevel;

/**
 * Transciever RF Power types for the Power Descriptor
 * (@ref zstack_PowerDescriptor_t)
 */
typedef enum
{
    /** Receiver synchronized with the receiver on when idle (nodeDesc) */
    zstack_PowerModes_POWER_SYNCHRONIZED = 0,
    /** Receiver comes on periodically (powerDesc) */
    zstack_PowerModes_POWER_PERIODIC = 1,
    /** Receiver comes on when stimulated, e.g. key press */
    zstack_PowerModes_POWER_STIMULATED = 2,
} zstack_PowerModes;

/**
 * ZDO status values, these are used in the status fields of ZDO
 * response messages (over-the-air)
 */
typedef enum
{
    /** Operation completed successfully */
    zstack_ZdpStatus_SUCCESS = 0,
    /** The supplied request type was invalid */
    zstack_ZdpStatus_INVALID_REQTYPE = 0x80,
    /** Reserved */
    zstack_ZdpStatus_DEVICE_NOT_FOUND = 0x81,
    /** Invalid endpoint value */
    zstack_ZdpStatus_INVALID_EP = 0x82,
    /** Endpoint not described by a simple desc. */
    zstack_ZdpStatus_NOT_ACTIVE = 0x83,
    /** Optional feature not supported */
    zstack_ZdpStatus_NOT_SUPPORTED = 0x84,
    /** Operation has timed out */
    zstack_ZdpStatus_TIMEOUT = 0x85,
    /** No match for end device bind */
    zstack_ZdpStatus_NO_MATCH = 0x86,
    /** Unbind request failed, no entry */
    zstack_ZdpStatus_NO_ENTRY = 0x88,
    /** Child descriptor not available */
    zstack_ZdpStatus_NO_DESCRIPTOR = 0x89,
    /** Insufficient space to support operation */
    zstack_ZdpStatus_INSUFFICIENT_SPACE = 0x8a,
    /** Not in proper state to support operation */
    zstack_ZdpStatus_NOT_PERMITTED = 0x8b,
    /** No table space to support operation */
    zstack_ZdpStatus_TABLE_FULL = 0x8c,
    /** Permissions indicate request not authorized */
    zstack_ZdpStatus_NOT_AUTHORIZED = 0x8d,
    /** No binding table space to support operation */
    zstack_ZdpStatus_BINDING_TABLE_FULL = 0x8e,
} zstack_ZdpStatus;

/**
 * Device States for the Network Information Read response (@ref
 * zstack_sysNwkInfoReadRsp_t) and the
 * Device State Change indication (@ref zstack_devStateChangeInd_t).
 */
typedef enum
{
    /** Initialized - not started automatically */
    zstack_DevState_HOLD = 0,
    /** Initialized - not connected to anything */
    zstack_DevState_INIT = 1,
    /** Discovering PAN's to join */
    zstack_DevState_NWK_DISC = 2,
    /** Joining a PAN */
    zstack_DevState_NWK_JOINING = 3,
    /**
     * ReJoining a PAN in secure mode scanning in current channel,
     * only for end devices
     */
    zstack_DevState_NWK_REJOIN_SEC_CURR_CHANNEL = 4,
    /** Joined but not yet authenticated by trust center */
    zstack_DevState_END_DEVICE_UNAUTH = 5,
    /** Started as device after authentication */
    zstack_DevState_DEV_END_DEVICE = 6,
    /** Device joined, authenticated and is a router */
    zstack_DevState_DEV_ROUTER = 7,
    /** Started as Zigbee Coordinator */
    zstack_DevState_COORD_STARTING = 8,
    /** Started as Zigbee Coordinator */
    zstack_DevState_DEV_ZB_COORD = 9,
    /** Device has lost information about its parent */
    zstack_DevState_NWK_ORPHAN = 10,
    /** Device is sending KeepAlive message to its parent */
    zstack_DevState_NWK_KA = 11,
    /** Device is waiting before trying to rejoin */
    zstack_DevState_NWK_BACKOFF = 12,
    /**
     * ReJoining a PAN in secure mode scanning in all channels,
     * only for end devices
     */
    zstack_DevState_NWK_REJOIN_SEC_ALL_CHANNEL = 13,
    /**
     * ReJoining a PAN in unsecure mode scanning in current channel,
     * only for end devices
     */
    zstack_DevState_NWK_TC_REJOIN_CURR_CHANNEL = 14,
    /**
     * ReJoining a PAN in unsecure mode scanning in all channels,
     * only for end devices
     */
    zstack_DevState_NWK_TC_REJOIN_ALL_CHANNEL = 15,
} zstack_DevState;

/**
 * Network States for the Device Force Network Settings Request
 * (@ref zstack_devForceNetworkSettingsReq_t)
 */
typedef enum
{
    /** Initialized - not connected to anything */
    zstack_NwkState_INIT = 0,
    /** Device has lost information about its parent.. */
    zstack_NwkState_JOINING_ORPHAN = 1,
    /** Discovering PAN's to join */
    zstack_NwkState_DISC = 2,
    /** Joining a PAN */
    zstack_NwkState_JOINING = 3,
    /** Started as device after authentication */
    zstack_NwkState_ENDDEVICE = 4,
    /** PAN Channel Selection */
    zstack_NwkState_PAN_CHNL_SELECTION = 5,
    /** PAN Channel Verification */
    zstack_NwkState_PAN_CHNL_VERIFY = 6,
    /** Starting a coordinator or router */
    zstack_NwkState_STARTING = 7,
    /** Device joined, authenticated and is a router */
    zstack_NwkState_ROUTER = 8,
    /** ReJoining a PAN, only for end devices */
    zstack_NwkState_REJOINING = 9,
} zstack_NwkState;

/**
 * Routing Table State/Status Values used in a routing record
 * (@ref zstack_routeItem_t)
 */
typedef enum
{
    zstack_RouteStatus_ROUTE_ACTIVE = 0,
    zstack_RouteStatus_ROUTE_DISCOVERY_UNDERWAY = 1,
    zstack_RouteStatus_ROUTE_DISCOVERY_FAILED = 2,
    zstack_RouteStatus_ROUTE_INACTIVE = 3,
    zstack_RouteStatus_ROUTE_VALIDATION_UNDERWAY = 4,
} zstack_RouteStatus;

/**
 * Rx On When Idle Types used in the neighbor record (@ref zstack_nwkLqiItem_t)
 */
typedef enum
{
    zstack_RxOnWhenIdleTypes_OFF = 0,
    zstack_RxOnWhenIdleTypes_ON = 1,
    zstack_RxOnWhenIdleTypes_UNKNOWN = 2,
} zstack_RxOnWhenIdleTypes;

/**
 * Neighbor's Relationship type used in the neighbor record
 * (@ref zstack_nwkLqiItem_t)
 */
typedef enum
{
    zstack_RelationTypes_PARENT = 0,
    zstack_RelationTypes_CHILD = 1,
    zstack_RelationTypes_SIBLING = 2,
    zstack_RelationTypes_UNKNOWN = 3,
    zstack_RelationTypes_PREVIOUS_CHILD = 4,
} zstack_RelationTypes;

/**
 * Neighbor's Permit Joining Status Type used in the neighbor record
 * (@ref zstack_nwkLqiItem_t)
 */
typedef enum
{
    zstack_PermitJoinTypes_NOT_ACCEPTING_JOIN = 0,
    zstack_PermitJoinTypes_ACCEPTING_JOIN = 1,
    zstack_PermitJoinTypes_UNKNOWN_JOIN = 2,
} zstack_PermitJoinTypes;

/**
 * ZStack Device Build Types used in the version response
 * (@ref zstack_sysVersionRsp_t)
 */
typedef enum
{
    zstack_BuildTypes_COORDINATOR = 0,
    zstack_BuildTypes_ROUTER = 1,
    zstack_BuildTypes_ENDDEVICE = 2,
    zstack_BuildTypes_ALLBUILD = 3,
} zstack_BuildTypes;

//*****************************************************************************
// Structures - Building blocks for the ZStack API structures
//*****************************************************************************

/**
 * Extended 64 bit IEEE Address type
 */
typedef uint8_t zstack_LongAddr_t[EXTADDR_LEN];

/**
 * This is a structure used to define an application address.
 * Depending on addrMode, only one shortAddr or extAddr should be used.
 */
typedef struct _zstack_afaddr_t
{
    /** Address Mode */
    zstack_AFAddrMode addrMode;
    /** Address union of 16 bit short address and 64 bit IEEE address */
    union
    {
        /** 16 bit network address */
        uint16_t shortAddr;
        /** 64 bit IEEE address */
        zstack_LongAddr_t extAddr;
    } addr;
    /** Endpoint address element, optional if addressing to the endpoint,
     * can be 0xFF to address all endpoints in a device.
     */
    uint8_t endpoint;
    /** PAN ID - for use with Inter-PAN */
    uint16_t panID;
} zstack_AFAddr_t;

/** Structure to select the startup states. */
typedef struct _zstack_startupoptions_t
{
    /** True to clear the previous network state */
    bool clearState;
    /** True to overwrite all the configuration parameters with defaults */
    bool clearConfig;
} zstack_StartupOptions_t;

/**
 * Structure for the Simple Descriptor. Each endpoint must have a Zigbee Simple
 * Descriptor. This descriptor describes the endpoint to the rest of the Zigbee
 * network. Another device can query an endpoint for it simple descriptor and
 * know what kind of device it is. This structure is defined by the application.
 */
typedef struct _zstack_simpledescriptor_t
{
    /**
     * The endpoint number 1-240 (0 is reserved). This is the subaddress of the
     * node, and is used to receive data.
     */
    uint8_t endpoint;
    /**
     * This field identifies the Profile ID supported on this endpoint. The
     * IDs shall be obtained from the ZigBee Alliance.
     */
    uint16_t profileID;
    /**
     * This field identifies the Device ID supported on this endpoint. The IDs
     * shall be obtained from the ZigBee Alliance.
     */
    uint16_t deviceID;
    /**
     * Identifies the version of the relevant Device Description that this
     * device implements on this endpoint. 0x00 is Version 1.0.
     */
    uint8_t deviceVer;
    /**
     * This indicates the number of input clusters supported by this endpoint.
     */
    uint8_t n_inputClusters;
    /** Pointer to inputClusters array */
    uint16_t *pInputClusters;
    /**
     * This indicates the number of output clusters supported by this endpoint.
     */
    uint8_t n_outputClusters;
    /** Pointer to outputClusters array */
    uint16_t *pOutputClusters;
} zstack_SimpleDescriptor_t;

/**
 * Structure for defining the transaction options when sending a data request.
 */
typedef struct _zstack_transoptions_t
{
    /** Set to request use of Wildcard Profile ID (OxFFFF) */
    bool wildcardProfileID;
    /**
     * Set to request APS acknowlegement - This is an application level
     * acknowledgement - meaning that the destination device will acknowledge
     * the message. Only used on messages send direct (unicast).
     */
    bool ackRequest;
    /** Set to limit the message to concentrators only */
    bool limitConcentrator;
    /** Set to suppress intermediate route discoveries (only initial node) */
    bool suppressRouteDisc;
    /** Set to enable APS security (link key) */
    bool apsSecurity;
    /**
     * Setting this option will cause the device to skip routing and try
     * to send the message directly (not multihop). End devices
     * will not send the message to its parent first. Good for only direct
     * (unicast) and broadcast messages.
     */
    bool skipRouting;
} zstack_TransOptions_t;

/** Structure defines the Capabilities Information bit field. */
typedef struct _zstack_capabilityinfo_t
{
    /** True if the device is a PAN Coordinator */
    bool panCoord;
    /** True if the device is a full function device (FFD) */
    bool ffd;
    /** True if the device is mains powered */
    bool mainsPower;
    /** True if the device's RX is on when the device is idle */
    bool rxOnWhenIdle;
    /** True if the device is capable of sending and receiving secured frames
      */
    bool security;
} zstack_CapabilityInfo_t;

/** Structure defines the Server Capabilities. */
typedef struct _zstack_servercapabilities_t
{
    /** Set to indicate the Primary Trust Center */
    bool primaryTrustCenter;
    /** Set to indicate the Backup Trust Center */
    bool backupTrustCenter;
    /** Set to indicate the Primary Binding Table Cache */
    bool primaryBindingTableCache;
    /** Set to indicate the Backup Binding Table Cache */
    bool backupBindingTableCache;
    /** Set to indicate the Primary Discovery Cache */
    bool primaryDiscoveryCache;
    /** Set to indicate the Backup Discovery Cache */
    bool backupDiscoveryCache;
    /** Set to indicate the Network Manager */
    bool networkManager;
} zstack_ServerCapabilities_t;

/** Structure for the Binding Information record. */
typedef struct _zstack_bindrec_t
{
    /** Targeted device's ieee address */
    zstack_LongAddr_t srcAddr;
    /** Targeted device's endpoint */
    uint8_t srcEndpoint;
    /** Cluster ID */
    uint16_t clusterID;
    /** Binding's destination address (IEEE address only) */
    zstack_AFAddr_t dstAddr;
} zstack_BindRec_t;

/** Options subfield of the leave request */
typedef struct _zstack_leaveoptions_t
{
    /** Set to request a rejoin */
    bool rejoin;
    /** Set to request the device's children to leave*/
    bool removeChildren;
} zstack_LeaveOptions_t;

/** Structure for the ZDO Node Descriptor */
typedef struct _zstack_nodedescriptor_t
{
    /** Device type */
    zstack_LogicalTypes logicalType;
    /** Set if the Complex Descriptor is available */
    bool complexDescAvail;
    /** Set if the User Descriptor is available */
    bool userDescAvail;
    /** (5 bits) Node Flags assigned for APS */
    uint8_t apsFlags;
    /** (3 bits) Identifies node frequency  band capabilities */
    uint8_t freqBand;
    /** MAC Capabilities */
    zstack_CapabilityInfo_t capInfo;
    /** Manufacturer Code allocated by Zigbee Alliance */
    uint16_t manufacturerCode;
    /** Maximum size of NPDU */
    uint8_t maxBufferSize;
    /** Maximum size of transfer up to 0x7FFF, reserved and shall be set to 0
      */
    uint16_t maxInTransferSize;
    /** Server Capabilities */
    zstack_ServerCapabilities_t serverMask;
    /** (uint16_t) Maximum size of Transfer up to 0x7FFF */
    uint16_t maxOutTransferSize;
    /** Descriptor capabilities */
    uint32_t descCap;
} zstack_NodeDescriptor_t;

/** Power Source options*/
typedef struct _zstack_powersource_t
{
    /** Set for constant (Mains) power */
    bool mains;
    /** Set for rechargeable battery */
    bool recharge;
    /** Set for disposable battery */
    bool dispose;
} zstack_PowerSource_t;

/** Structure for the Power Descriptor */
typedef struct _zstack_powerdescriptor_t
{
    /** Current power mode */
    zstack_PowerModes powerMode;
    /** Available power sources */
    zstack_PowerSource_t availPowerSource;
    /** Current power source */
    zstack_PowerSource_t currentPowerSource;
    /** Current power level */
    zstack_PowerLevel currentPowerLevel;
} zstack_PowerDescriptor_t;

/** Structure for the network information of a single network */
typedef struct _zstack_nwkdiscitem_t
{
    /** 64 bit extended PAN ID */
    zstack_LongAddr_t extendedPANID;
    /** Current logical channel */
    uint8_t logicalChan;
    /** A ZigBee profile identifier */
    uint8_t stackProfile;
    /** ZigBee Protocol version */
    uint8_t version;
    /** Beacon Order */
    uint8_t beaconOrder;
    /** Superframe Order */
    uint8_t superFrameOrder;
    /** True if Permit Join set */
    bool permitJoin;
} zstack_nwkDiscItem_t;

/** Structure to hold the LQI information of a single network. */
typedef struct _zstack_nwklqiitem_t
{
    /** Device's short address */
    uint16_t nwkAddr;
    /** PAN ID */
    uint16_t panID;
    /** 64 bit extended PAN ID */
    zstack_LongAddr_t extendedPANID;
    /** 64 bit extended Address */
    zstack_LongAddr_t extendedAddr;
    /** Receive LQI */
    uint8_t rxLqi;
    /** Type of neighbor device */
    zstack_LogicalTypes deviceType;
    /** Receiver On When Idle status */
    zstack_RxOnWhenIdleTypes rxOnWhenIdle;
    /** Neighbor's relationship */
    zstack_RelationTypes relationship;
    /** Neighbor's depth in the network */
    uint8_t depth;
    /** Permit Join Status */
    zstack_PermitJoinTypes permit;
} zstack_nwkLqiItem_t;

/** Structure to hold the information for a single route. */
typedef struct _zstack_routeitem_t
{
    /** Destination short address */
    uint16_t dstAddr;
    /** Next hop short address */
    uint16_t nextHop;
    /** Route status */
    zstack_RouteStatus status;
    /**
     * A flag indicating whether the device is a memory constrained concentrator
     */
    bool memoryConstrained;
    /** A flag indicating that the destination is a concentrator */
    bool manyToOne;
    /**
     * A flag indicating that a route record command frame should be sent before
     * data
     */
    bool routeRecordRequired;
} zstack_routeItem_t;

/** Structure to hold a binding record. */
typedef struct _zstack_binditem_t
{
    /** 64 bit source extended address */
    zstack_LongAddr_t srcAddr;
    /** Source endpoint */
    uint8_t srcEndpoint;
    /** Cluster ID */
    uint16_t clustedID;
    /** Destination address */
    zstack_AFAddr_t dstAddr;
} zstack_bindItem_t;

/**
 * Device type capable information, a device can be capable of one or
 * more of these types.
 */
typedef struct _zstack_devicetypes_t
{
    //! True if device is capable of being a coordinator
    bool coodinator;
    //! True if device is capable of being a router
    bool router;
    //! True if device is capable of being an end device
    bool enddevice;
} zstack_deviceTypes_t;

//*****************************************************************************
// System Interface Request Structures
//*****************************************************************************

/**
 * Structure to send a system reset request.
 */
typedef struct _zstack_sysresetreq_t
{
    /**
     * if the stack doesn't support a type of reset, the type will
     * be ignored and a reset will be performed.
     */
    zstack_ResetTypes type;
    /**
     * Set this flag to true to disregard the values stored in non-volatile
     * and restart fresh.
     */
    bool newNwkState;
} zstack_sysResetReq_t;

/**
 * Structure to send a system config read request.  Set to "true" each of the
 * fields that you would like returned in the response
 * [zstack_sysConfigReadRsp_t](@ref _zstack_sysconfigreadrsp_t)
 * structure.
 */
typedef struct _zstack_sysconfigreadreq_t
{
    /** True to return Pre Config Key Enable */
    bool preConfigKeyEnable;
    /** True to return Security Mode Enable (nwk) */
    bool securityModeEnable;
    /** True to return default TCLK flag */
    bool useDefaultTCLK;
    /**  True to return if the poll rate is disabled or not */
    bool disabledPollRate;
    /** True to return Poll Rate */
    bool pollRate;
    /** True to return APS Ack Waite Duration */
    bool apsAckWaitDuration;
    /** True to return Binding Time */
    bool bindingTime;
    /** True to return PAN ID */
    bool panID;
    /** True to return zgMaxMissingMacAckLinkFailure */
    bool maxMissingMacAckLinkFailure;
    /** True to return Indirect Message Timeout */
    bool indirectMsgTimeout;
    /** True to return APS Frame Retries */
    bool apsFrameRetries;
    /** True to return Broadcast Retries */
    bool bcastRetries;
    /** True to return Passive Ack Timeout */
    bool passiveAckTimeout;
    /** True to return Broadcast Delivery Time */
    bool bcastDeliveryTime;
    /** True to return Route Expiry Time */
    bool routeExpiryTime;
    /** True to return User Descriptor (16 bytes max) */
    bool userDesc;
    /** True to return PreConfig Key (16 bytes) */
    bool preConfigKey;
    /** True to return Channel List (bit masked) */
    bool chanList;
    /** True to return the Multicast radius */
    bool multicastRadius;
    /** True to return the extended PAN ID (APS USE) */
    bool extendedPANID;
    /** True to return the IEEE extended address */
    bool ieeeAddr;
    /** True to return the MAC's RxOnIdle parameter */
    bool macRxOnIdle;
    /** True to return the Sniffer Feature active parameter */
    bool snifferFeature;
    /** True to return the Concentrator Enable parameter */
    bool concentratorEnable;
    /** True to return the Concentrator Discovery Time parameter */
    bool concentratorDiscTime;
    /** True to return the Network Multicast Use parameter */
    bool nwkUseMultiCast;
    /** True to return if the device is already part of a network */
    bool devPartOfNetwork;
} zstack_sysConfigReadReq_t;

/**
 * Structure to send a system config write request. Each config field has
 * 2 parts to it, the actual value field and its "has_" field.  The "has_" field
 * must be set to "true" for the value field to be valid.  For example, the
 * pollRate field is valid only if the has_pollRate field is set to true.
 */
typedef struct _zstack_sysconfigwritereq_t
{
    /** Set to true if preConfigKeyEnable is available, false if not */
    bool has_preConfigKeyEnable;
    /** Pre Config Key Enable */
    bool preConfigKeyEnable;

    /** Set to true if securityModeEnable is available, false if not */
    bool has_securityModeEnable;
    /** Security Mode Enable (nwk) */
    bool securityModeEnable;

    /** Set to true if useDefaultTCLK is available, false if not */
    bool has_useDefaultTCLK;
    /** Enable the use of the preconfig Trust Center Link Key */
    bool useDefaultTCLK;

    /** Set to true if disablePollRate is available. If this is enabled, then has_pollRate has no effect */
    bool has_disablePollRate;
    /**  Set to true to indicate the stack to stop polling completly (this overwrites EndDeviceTimeout stack requirements).
     *   Set to False to restore the current polling configuration. */
    bool disablePollRate;

    /** Set to true if pollRate and pollRateType is available, false if not */
    bool has_pollRate;
    /** End Device Poll Rate to set to pollRateType (not applicable for routers), in milliseconds.
     *  This must be bigger than MINIMUM_APP_POLL_RATE, when setting application poll rate as: POLL_RATE_TYPE_DEFAULT, POLL_RATE_TYPE_APP_1 and POLL_RATE_TYPE_APP_2 */
    uint32_t pollRate;
    /** Poll rate type to be set. This field is a bitmask. The poll associated
     * to the operations set in this bitmask parameter will be set to the pollRate parameter.
     * POLL_RATE_TYPE_DEFAULT          0x0001   Default pollrate to be used when no other stack operation is done.
     * POLL_RATE_TYPE_APP_1            0x0002   Application defined poll rate
     * POLL_RATE_TYPE_APP_2            0x0004   Application defined poll rate
     * POLL_RATE_TYPE_JOIN_REJOIN      0x0008   Poll rate used by the stack during join/rejoin operations
     * POLL_RATE_TYPE_QUEUED           0x0010   Poll rate used by the stack when expecting a response or retrieving all queued messages from parent.
     * POLL_RATE_TYPE_RESPONSE         0x0020   Poll rate used by the stack when expecting a response.
     * POLL_RATE_TYPE_GENERIC_1_SEC    0x1000   1 second poll rate used by the stack (cannot be modified)
     * POLL_RATE_DISABLED              0x4000   State to not perform poll rate. This is used by the stack (cannot be modified)
     * POLL_RATE_RX_ON_TRUE            0x8000   Indicates if ZED is RxOnIdle, device will not poll unless Parent request it as keep alive method (cannot be modified)
     * */
    uint16_t pollRateType;

    /** Set to true if apsAckWaitDuration is available, false if not */
    bool has_apsAckWaitDuration;
    /** APS Ack Waite Duration */
    uint16_t apsAckWaitDuration;

    /** Set to true if bindingTime is available, false if not */
    bool has_bindingTime;
    /** Binding Time */
    uint16_t bindingTime;

    /** Set to true if panID is available, false if not */
    bool has_panID;
    /** PAN ID */
    uint16_t panID;

    /** Set to true if maxMissingMacAckLinkFailure is available, false if not */
    bool has_maxMissingMacAckLinkFailure;
    /** End Device frames to its parent failure before forcing link failure (not applicable for routers) */
    uint8_t maxMissingMacAckLinkFailure;

    /** Set to true if indirectMsgTimeout is available, false if not */
    bool has_indirectMsgTimeout;
    /** Indirect Message Timeout */
    uint8_t indirectMsgTimeout;

    /** Set to true if apsFrameRetries is available, false if not */
    bool has_apsFrameRetries;
    /** APS Frame Retries */
    uint8_t apsFrameRetries;

    /** Set to true if bcastRetries is available, false if not */
    bool has_bcastRetries;
    /**  Broadcast Retries */
    uint8_t bcastRetries;

    /** Set to true if passiveAckTimeout is available, false if not */
    bool has_passiveAckTimeout;
    /** Passive Ack Timeout */
    uint8_t passiveAckTimeout;

    /** Set to true if bcastDeliveryTime is available, false if not */
    bool has_bcastDeliveryTime;
    /** Broadcast Delivery Time */
    uint8_t bcastDeliveryTime;

    /** Set to true if routeExpiryTime is available, false if not */
    bool has_routeExpiryTime;
    /** Route Expiry Time */
    uint8_t routeExpiryTime;

    /** Set to true if userDesc is available, false if not */
    bool has_userDesc;
    /** Size of the User Descriptor (16 bytes max) */
    uint8_t n_userDesc;
    /** Pointer to the User Descriptor */
    uint8_t *pUserDesc;

    /** Set to true if preConfigKey is available, false if not */
    bool has_preConfigKey;
    /** length of pPreConfigKey */
    uint8_t n_preConfigKey;
    /** Pointer to the PreConfig Key (16 bytes) */
    uint8_t *pPreConfigKey;

    /** Set to true if chanList is available, false if not */
    bool has_chanList;
    /**
     * Channel List (bitmap) - Channels are defined in the following:
     * 0 : 868 MHz (0x00000001),
     * 1 - 10 : 915 MHz (0x000007FE),
     * 11 - 26 : 2.4 GHz (0x07FFF800)
     */
    uint32_t chanList;

    /** Set to true if chanList is available, false if not */
    bool has_multicastRadius;
    /** Multicast radius */
    uint8_t multicastRadius;

    /** Set to true if extendedPANID is available, false if not */
    bool has_extendedPANID;
    /** 64 bit extended PAN ID (APS USE) */
    zstack_LongAddr_t extendedPANID;

    /** Set to true if ieeeAddr is available, false if not */
    bool has_ieeeAddr;
    /** 64 bit extended IEEE address */
    zstack_LongAddr_t ieeeAddr;

    /** Set to true if macRxOnIdle is available, false if not */
    bool has_macRxOnIdle;
    /** the MAC's RxOnIdle parameter */
    bool macRxOnIdle;

    /** Set to true if snifferFeature is available, false if not */
    bool has_snifferFeature;
    /** True to enable the Sniffer Feature, false to disable */
    bool snifferFeature;

    /** Set to true if concentratorEnable is available, false if not */
    bool has_concentratorEnable;
    /** True to enable the Concentrator, false to disable */
    bool concentratorEnable;

    /** Set to true if concentratorEnable is available, false if not */
    bool has_concentratorDiscTime;
    /** Concentrator Discovery Time value in seconds */
    uint8_t concentratorDiscTime;

    /** Set to true if nwkUseMultiCast is available, false if not */
    bool has_nwkUseMultiCast;
    /** True to enable the use of Multicast messaging */
    bool nwkUseMultiCast;
} zstack_sysConfigWriteReq_t;

/**
 * Structure to send a system TX power request.
 */
typedef struct _zstack_syssettxpowerreq_t
{
    /** Requested Tx power setting, in dBm */
    int_least8_t requestedTxPower;
} zstack_sysSetTxPowerReq_t;

//*****************************************************************************
// System Interface Response Structures
//*****************************************************************************

/**
 * Structure to return the system version response.
 */
typedef struct _zstack_sysversionrsp_t
{
    /** Transport protocol revision, this is set to a value of 2 */
    uint8_t transportRev;
    //! Product information - build types
    zstack_BuildTypes product;
    //! Major release number
    uint8_t majorRel;
    //! Minor release number
    uint8_t minorRel;
    //! Maintenance revision number
    uint8_t maintRel;
} zstack_sysVersionRsp_t;

/**
 * Structure to return the system config response.  Each config field has
 * 2 parts to it, the actual value field and its "has_" field.  The "has_" field
 * must be set to "true" for the value field to be valid.  For example, the
 * pollRate field is valid only if the has_pollRate field is set to true.
 */
typedef struct _zstack_sysconfigreadrsp_t
{
    /** Set to true if preConfigKeyEnable is available, false if not */
    bool has_preConfigKeyEnable;
    /** Pre Config Key Enable */
    bool preConfigKeyEnable;

    /** Set to true if securityModeEnable is available, false if not */
    bool has_securityModeEnable;
    /** Security Mode Enable (nwk) */
    bool securityModeEnable;

    /** Set to true if useDefaultTCLK is available, false if not */
    bool has_useDefaultTCLK;
    /** Enable the use of the preconfig Trust Center Link Key */
    bool useDefaultTCLK;

    /** Set to true if disabledPollRate is available, false if not */
    bool has_disabledPollRateState;
    /** True if the poll rate is disabled, false if normal polling operations
     * are running (check the has_pollRate and its related parameters)*/
    bool disabledPollRate;

    /** Set to true if pollRate is available, false if not */
    bool has_pollRate;

    /** End Device Poll Rate in use (not applicable for routers) */
    uint32_t CurrentPollRate;

    /** Current poll rates enabled */
    uint16_t CurrentPollRateTypesEnabled;

    /** Poll Rate configuration set for different app or stack process */
    uint32_t PollRateDefault;
    uint32_t PollRateApp1;
    uint32_t PollRateApp2;
    uint32_t PollRateJoinRejoin;
    uint32_t PollRateQueue;
    uint32_t PollRateResponse;

    /** Set to true if apsAckWaitDuration is available, false if not */
    bool has_apsAckWaitDuration;
    /** APS Ack Waite Duration */
    uint16_t apsAckWaitDuration;

    /** true if bindingTime is available, false if not */
    bool has_bindingTime;
    /** Binding Time */
    uint16_t bindingTime;

    /** Set to true if panID is available, false if not */
    bool has_panID;
    /** PAN ID */
    uint16_t panID;

    /** Set to true if maxMissingMacAckLinkFailure is available, false if not */
    bool has_maxMissingMacAckLinkFailure;
    /** End Device frames to its parent failure before forcing link failure (not applicable for routers) */
    uint8_t maxMissingMacAckLinkFailure;

    /** Set to true if indirectMsgTimeout is available, false if not */
    bool has_indirectMsgTimeout;
    /** Indirect Message Timeout */
    uint8_t indirectMsgTimeout;

    /** Set to true if apsFrameRetries is available, false if not */
    bool has_apsFrameRetries;
    /** APS Frame Retries */
    uint8_t apsFrameRetries;

    /** Set to true if bcastRetries is available, false if not */
    bool has_bcastRetries;
    /** Broadcast Retries */
    uint8_t bcastRetries;

    /** Set to true if passiveAckTimeout is available, false if not */
    bool has_passiveAckTimeout;
    /** Passive Ack Timeout */
    uint8_t passiveAckTimeout;

    /** Set to true if bcastDeliveryTime is available, false if not */
    bool has_bcastDeliveryTime;
    /** Broadcast Delivery Time */
    uint8_t bcastDeliveryTime;

    /** Set to true if routeExpiryTime is available, false if not */
    bool has_routeExpiryTime;
    /** Route Expiry Time */
    uint8_t routeExpiryTime;

    /** Set to true if userDesc is available, false if not */
    bool has_userDesc;
    /** Size of the User Descriptor (16 bytes max) */
    uint8_t n_userDesc;
    /** Pointer to the User Descriptor */
    uint8_t *pUserDesc;

    /** Set to true if preConfigKey is available, false if not */
    bool has_preConfigKey;
    /** Pointer to the PreConfig Key (16 bytes) */
    uint8_t *pPreConfigKey;

    /** Set to true if chanList is available, false if not */
    bool has_chanList;
    /**
     * Channel List (bitmap) - Channels are defined in the following:
     * 0 : 868 MHz (0x00000001),
     * 1 - 10 : 915 MHz (0x000007FE),
     * 11 - 26 : 2.4 GHz (0x07FFF800)
     */
    uint32_t chanList;

    /**  Set to true if multicastRadius is available, false if not */
    bool has_multicastRadius;
    /** Multicast radius */
    uint8_t multicastRadius;

    /** Set to true if extendedPANID is available, false if not */
    bool has_extendedPANID;
    /** 64 bit extended PAN ID (APS USE) */
    zstack_LongAddr_t extendedPANID;

    /** Set to true if ieeeAddr is available, false if not */
    bool has_ieeeAddr;
    /** 64 bit extended IEEE address */
    zstack_LongAddr_t ieeeAddr;

    /** Set to true if macRxOnIdle is available, false if not */
    bool has_macRxOnIdle;
    /** The MAC's RxOnIdle parameter */
    bool macRxOnIdle;

    /** Set to true if the snifferFeature is available, false if not */
    bool has_snifferFeature;
    /** True if the sniffer feature is enabled */
    bool snifferFeature;

    /** Set to true if the concentratorEnable is available, false if not */
    bool has_concentratorEnable;
    /** True if the concentrator feature is enabled */
    bool concentratorEnable;

    /** Set to true if the concentratorDiscTime is available, false if not */
    bool has_concentratorDiscTime;
    /** Concentrator Discovery Time */
    uint8_t concentratorDiscTime;

    /** Set to true if the nwkUseMultiCast is available, false if not */
    bool has_nwkUseMultiCast;
    /** True if the device is to send network Multicast message */
    bool nwkUseMultiCast;

    /** Set to true if devPartOfNetwork is available, false if not */
    bool has_devPartOfNetwork;
    /**
     * True if the device is already part of a network.
     * "Part of the Network" means that the device's network state is
     * that of an end device or router/coordinator in a network, and
     * not initialized or in an intermediate state (like scannning,
     * joining or rejoining).  A good use of this parameter would be
     * to get this parameter before starting the device, to
     * "pre"-determine if the device needs to join a network or just
     * start from the parameters already in NV.
     */
    bool devPartOfNetwork;

} zstack_sysConfigReadRsp_t;

/**
 * Structure to return the system TX power response.
 */
typedef struct _zstack_syssettxpowerrsp_t
{
    //! Actual TX power setting, in dBm
    int_least8_t txPower;
} zstack_sysSetTxPowerRsp_t;

/**
 * Structure to return the system network information read response.
 */
typedef struct _zstack_sysnwkinforeadrsp_t
{
    //! Currently assigned short address
    uint16_t nwkAddr;
    //! 64 bit IEEE Address
    zstack_LongAddr_t ieeeAddr;
    //! Current device state
    zstack_DevState devState;
    //! PAN ID
    uint16_t panId;
    //! 64 bit extended PAN ID
    zstack_LongAddr_t extendedPanId;
    //! Parent's short address
    uint16_t parentNwkAddr;
    //! 64 bit parent's extended address
    zstack_LongAddr_t parentExtAddr;
    //! possible device types
    zstack_deviceTypes_t devTypes;
    //! Current network logical channel
    uint8_t logicalChannel;
    //! Capability Flags
    zstack_CapabilityInfo_t capInfo;
} zstack_sysNwkInfoReadRsp_t;

/**
 * Structure to return the system application message.
 */
typedef struct _zstack_sysAppMsg_t
{
    /** destination endpoint */
    uint8_t dstEndpoint;
    //! Application data lenght
    uint8_t appDataLen;
    //! pointer to App Data
    uint8_t *pAppData;
} zstack_sysAppMsg_t;

/**
 * Structure to return the system OTA message.
 */
typedef struct _zstack_sysOtaMsg_t
{
    /** Command */
    uint8_t cmd;
    //! pointer to App Data
    uint8_t *pData;
} zstack_sysOtaMsg_t;

//*****************************************************************************
// Device Interface Requests
//*****************************************************************************

/**
 * Structure to send a Device Start Request.
 */
typedef struct _zstack_devstartreq_t
{
    /** Time delay (milliseconds) before the device starts */
    uint16_t startDelay;
} zstack_devStartReq_t;

/**
 * Structure to send a Device Network Discovery Request.
 */
typedef struct _zstack_devnwkdiscreq_t
{
    //! Bit Mask of channels to scan
    uint32_t scanChannels;
    //! Scanning Time
    uint8_t scanDuration;
} zstack_devNwkDiscReq_t;

/**
 * Structure to send a set nwk frame forward notification Request.
 */
typedef struct zstack_setnwkframefwdnotificationreq_t
{
    //! Flag to enable/disable Frame Forward notifications
    uint8_t Enabled;
} zstack_setNwkFrameFwdNotificationReq_t;


/**
 * Structure to send a Device Join Requst.
 */
typedef struct _zstack_devjoinreq_t
{
    /** Channel where the PAN is located */
    uint8_t logicalChannel;
    /** ID of the PAN to join */
    uint16_t panID;
    /** 64 bit extended PAN ID, use all 0xFF if don't care */
    zstack_LongAddr_t extendedPANID;
    /** Short address of parent device chosen to join */
    uint16_t chosenParent;
    /** Depth of the parent */
    uint8_t parentDepth;
    /** Stack profile of the network to join */
    uint8_t stackProfile;
} zstack_devJoinReq_t;

/**
 * Structure to send a Device Rejoin Request.
 */
typedef struct _zstack_devrejoinreq_t
{
    /** Set to True to perform an unsecure rejoin */
    bool unsecure;
    /** Channel where the PAN is located*/
    uint8_t logicalChannel;
    /** 64 bit extended PAN ID, use all 0xFF if don't care */
    zstack_LongAddr_t extendedPANID;
} zstack_devRejoinReq_t;

/**
 * Structure to send a Device ZDO Callback Indications, Confirmations
 * and/or Respones Request.
 */
typedef struct _zstack_devzdocbreq_t
{
    /** Set to true if srcRtgIndCB is available, false if not */
    bool has_srcRtgIndCB;
    /** Set to true to receive Source Route Indication */
    bool srcRtgIndCB;

    /** Set to true if concentratorIndCb is available, false if not */
    bool has_concentratorIndCb;
    /** Set to true to receive Concentrator Indication */
    bool concentratorIndCb;

    /** Set to true if nwkDiscCnfCB is available, false if not */
    bool has_nwkDiscCnfCB;
    /** Set to true to receive Network Discovery Confirmation */
    bool nwkDiscCnfCB;

    /** Set to true if beaconNotIndCB is available, false if not */
    bool has_beaconNotIndCB;
    /** Set to true to receive Beacon Notify Indication */
    bool beaconNotIndCB;

    /** Set to true if joinCnfCB is available, false if not */
    bool has_joinCnfCB;
    /** Set to true to receive Join Confirmation */
    bool joinCnfCB;

    /** Set to true if leaveCnfCB is available, false if not */
    bool has_leaveCnfCB;
    /** Set to true to receive Leave Confirmation */
    bool leaveCnfCB;

    /** Set to true if leaveIndCB is available, false if not */
    bool has_leaveIndCB;
    /** Set to true to receive Leave Indication */
    bool leaveIndCB;

    /** Set to true if nwkAddrRsp is available, false if not */
    bool has_nwkAddrRsp;
    /** Set to true to receive ZDO Network Address Response */
    bool nwkAddrRsp;

    /** Set to true if ieeeAddrRsp is available, false if not */
    bool has_ieeeAddrRsp;
    /** Set to true to receive ZDO IEEE Address Response */
    bool ieeeAddrRsp;

    /** Set to true if nodeDescRsp is available, false if not */
    bool has_nodeDescRsp;
    /** Set to true to receive ZDO Node Descriptor Response */
    bool nodeDescRsp;

    /** Set to true if powerDescRsp is available, false if not */
    bool has_powerDescRsp;
    /** Set to true to receive ZDO Power Descriptor Response */
    bool powerDescRsp;

    /** Set to true if simpleDescRsp is available, false if not */
    bool has_simpleDescRsp;
    /** Set to true to receive ZDO Simple Descriptor Response */
    bool simpleDescRsp;

    /** Set to true if activeEndpointRsp is available, false if not */
    bool has_activeEndpointRsp;
    /** Set to true to receive ZDO Active Endpoint Response */
    bool activeEndpointRsp;

    /** Set to true if matchDescRsp is available, false if not */
    bool has_matchDescRsp;
    /** Set to true to receive ZDO Match Descriptor Response */
    bool matchDescRsp;

    /** Set to true if complexDescRsp is available, false if not */
    bool has_complexDescRsp;
    /** Set to true to receive ZDO Complex Descriptor Response */
    bool complexDescRsp;

    /** Set to true if userDescRsp is available, false if not */
    bool has_userDescRsp;
    /** Set to true to receive ZDO User Descriptor Response */
    bool userDescRsp;

    /** Set to true if discoveryCacheRsp is available, false if not */
    bool has_discoveryCacheRsp;
    /** Set to true to receive ZDO Discovery Cache Response */
    bool discoveryCacheRsp;

    /** Set to true if userDescCnf is available, false if not */
    bool has_userDescCnf;
    /** Set to true to receive ZDO User Descriptor Confirmation */
    bool userDescCnf;

    /** Set to true if serverDiscoveryRsp is available, false if not */
    bool has_serverDiscoveryRsp;
    /** Set to true to receive ZDO Server Discovery Response */
    bool serverDiscoveryRsp;

    /** Set to true if bindRsp is available, false if not */
    bool has_bindRsp;
    /** Set to true to receive ZDO Bind Response */
    bool bindRsp;

    /** Set to true if endDeviceBindRsp is available, false if not */
    bool has_endDeviceBindRsp;
    /** Set to true to receive ZDO End Device Bind Response */
    bool endDeviceBindRsp;

    /** Set to true if unbindRsp is available, false if not */
    bool has_unbindRsp;
    /** Set to true to receive ZDO Unbind Response */
    bool unbindRsp;

    /** Set to true if mgmtNwkDiscRsp is available, false if not */
    bool has_mgmtNwkDiscRsp;
    /** Set to true to receive ZDO Management Network Discovery Response */
    bool mgmtNwkDiscRsp;

    /** Set to true if mgmtLqiRsp is available, false if not */
    bool has_mgmtLqiRsp;
    /** Set to true to receive ZDO Management LQI Response */
    bool mgmtLqiRsp;

    /** Set to true if mgmtRtgRsp is available, false if not */
    bool has_mgmtRtgRsp;
    /** Set to true to receive ZDO Management Routing Response */
    bool mgmtRtgRsp;

    /** Set to true if mgmtBindRsp is available, false if not */
    bool has_mgmtBindRsp;
    /** Set to true to receive ZDO Management Bind Response */
    bool mgmtBindRsp;

    /** Set to true if mgmtLeaveRsp is available, false if not */
    bool has_mgmtLeaveRsp;
    /** Set to true to receive ZDO Management Leave Response */
    bool mgmtLeaveRsp;

    /** Set to true if mgmtDirectJoinRsp is available, false if not */
    bool has_mgmtDirectJoinRsp;
    /** Set to true to receive ZDO Management Direct Join Response */
    bool mgmtDirectJoinRsp;

    /** Set to true if mgmtPermitJoinRsp is available, false if not */
    bool has_mgmtPermitJoinRsp;
    /** Set to true to receive ZDO Management Permit Join Response */
    bool mgmtPermitJoinRsp;

    /** Set to true if mgmtNwkUpdateNotify is available, false if not */
    bool has_mgmtNwkUpdateNotify;
    /** Set to true to receive ZDO Management Network Update Notify */
    bool mgmtNwkUpdateNotify;

    /** Set to true if deviceAnnounce is available, false if not */
    bool has_deviceAnnounce;
    /** Set to true to receive Device Announce */
    bool deviceAnnounce;

    /** Set to true if devStateChange is available, false if not */
    bool has_devStateChange;
    /** Set to true to receive Device State Change Indication */
    bool devStateChange;

    /** Set to true if tcDeviceInd is available, false if not */
    bool has_tcDeviceInd;
    /** Set to true to receive the Trust Center Device Indication */
    bool tcDeviceInd;

    /** Set to true if devPermitJoinInd is available, false if not */
    bool has_devPermitJoinInd;
    /** Set to true to receive the Permit Join Change Indication */
    bool devPermitJoinInd;
} zstack_devZDOCBReq_t;

/**
 * Structure to send a Device Network Route Request.
 */
typedef struct _zstack_devnwkroutereq_t
{
    /** Network address to discover */
    uint16_t dstAddr;
    /** True if you are announcing a concentrator */
    bool mtoRoute;
    /**
     * True if the concentrator has limited cache (only set if mtoRoute is set)
     */
    bool mtoNoCache;
    /** True if the route requested is for a multicast address */
    bool multicast;
    /** Radius of the message */
    uint8_t radius;
} zstack_devNwkRouteReq_t;

/**
 * Structure to send a Device Network Check Route Request.
 */
typedef struct _zstack_devnwkcheckroutereq_t
{
    //! Network address to check if a route exists
    uint16_t dstAddr;
    //! True if you're check for a Many-to-one route
    bool mtoRoute;
} zstack_devNwkCheckRouteReq_t;

/**
 * Structure to send a Device Permit Join Indication.
 */
typedef struct _zstack_devpermitjoinind_t
{
    /**
     * Duration of join. 0=disabled, 0xFF= Is converted to 0xFE, 0x01-0xFE number seconds
     * to permit join
     */
    uint8_t duration;
} zstack_devPermitJoinInd_t;

/**
 * Structure to send a Device Update Neighbor's TxCost Request.
 */
typedef struct _zstack_devupdateneighbortxcostreq_t
{
    //! Network address of neighbor
    uint16_t nwkAddr;
    //! PAN ID of neighbor
    uint16_t panID;
    /**
     * Updated TxCost: Range 0 - 7, 0 = link down, 7 = MaxCost,
     * 1 = Default cost
     */
    uint8_t txCost;
} zstack_devUpdateNeighborTxCostReq_t;

/**
 * Structure to send a Device Force Network Settings Request.
 */
typedef struct _zstack_devforcenetworksettingsreq_t
{
    //! Network State
    zstack_NwkState state;
    //! Network address of device
    uint16_t nwkAddr;
    //! Channel List (bit mask)
    uint32_t channelList;
    //! logical channel
    uint8_t logicalChannel;
    //! Parent's network address
    uint16_t parentNwkAddr;
    //! device's PAN ID
    uint16_t panID;
    //! Node depth within network
    uint8_t depth;
    /**
     * Maximum number of routers supported in associated device list
     * (routers only)
     */
    uint8_t maxRouters;
    /**
     * Maximum number of children supported in associated device list
     * (routers only)
     */
    uint8_t maxChildren;
    //! Allocated Router Addresses
    uint32_t allocatedRouterAddresses;
    //! Allocated End Device Addresses
    uint32_t allocatedEndDeviceAddresses;
    //! Network Update ID
    uint8_t updateID;
    //! 64 bit extended PAN ID, use all 0xFF if don't care
    zstack_LongAddr_t extendedPANID;
} zstack_devForceNetworkSettingsReq_t;

/**
 * Structure to send a Device Force Network Update Request.
 */
typedef struct _zstack_devforcenetworkupdatereq_t
{
    //! Network Update ID
    uint8_t updateID;
    //! Logical Channel
    uint8_t logicalChannel;
} zstack_devForceNetworkUpdateReq_t;

/**
 * Structure to send a Device Force MAC parameters Request.
 */
typedef struct _zstack_devforcemacparamsreq_t
{
    //! Network address
    uint16_t nwkAddr;
    //! device's PAN ID
    uint16_t panID;
    //! Logical Channel
    uint8_t logicalChannel;
} zstack_devForceMacParamsReq_t;

//*****************************************************************************
// APS Interface Request Messages
//*****************************************************************************

/**
 * Structure to send an APS Remove Group Request.
 */
typedef struct _zstack_apsremovegroup_t
{
    //! Endpoint to remove group from
    uint8_t endpoint;
    //! Group ID to remove
    uint16_t groupID;
} zstack_apsRemoveGroup_t;

/**
 * Structure to send an APS Remove All Groups Request.
 */
typedef struct _zstack_apsremoveallgroups_t
{
    //! Endpoint to remove all groups from
    uint8_t endpoint;
} zstack_apsRemoveAllGroups_t;

/**
 * Structure to send an APS Find All Groups Request.
 */
typedef struct _zstack_apsfindallgroupsreq_t
{
    //! Endpoint to find all groups
    uint8_t endpoint;
} zstack_apsFindAllGroupsReq_t;

/**
 * Structure to send an APS Find All Groups Response.
 */
typedef struct _zstack_apsfindallgroupsrsp_t
{
    /** Number of items in pGroupList */
    uint8_t numGroups;
    /**
     * List of groups - this memory is allocate by the ZStack Thread
     * and must be deallocated [OsalPort_free(pGroupList)] by the receiving
     * application thread
     */
    uint16_t *pGroupList;
} zstack_apsFindAllGroupsRsp_t;

/**
 * Structure to send an APS Find Group Request.
 */
typedef struct _zstack_apsfindgroupreq_t
{
    //! Endpoint to use to find the group
    uint8_t endpoint;
    //! Group ID to find
    uint16_t groupID;
} zstack_apsFindGroupReq_t;

/**
 * Structure to send an APS Find Group Response.
 */
typedef struct _zstack_apsfindgrouprsp_t
{
    /** group ID, not include if not found */
    uint16_t groupID;
    /** length of "pName" */
    uint8_t n_name;
    /**
     * Pointer to the Human readable "name" of group, and is not a zero
     * terminated string.
     * This memory is allocate by the ZStack Thread and must be deallocated
     * [OsalPort_free(pName)] by the receiving application thread.
     */
    uint8_t *pName;
} zstack_apsFindGroupRsp_t;

/**
 * Structure to send an APS Add Group Request.
 */
typedef struct _zstack_apsaddgroup_t
{
    //! The endpoint that will receive messages sent to the group in the group field.
    uint8_t endpoint;
    //! This is the group ID that is set over the air.
    uint16_t groupID;
    //! length of "name"
    uint8_t n_name;
    //! pointer to the Human readable "name" of group
    uint8_t *pName;
} zstack_apsAddGroup_t;

//*****************************************************************************
// AF Interface Request Structures
//*****************************************************************************

/**
 * Structure to send an AF Register Request.
 */
typedef struct _zstack_afregisterreq_t
{
    /**
     * The endpoint number 1-240 (0 is reserved). This is the subaddress of the
     * node, and is used to receive data.
     */
    uint8_t endpoint;
    /** Pointer to the Simple Descriptor structure */
    zstack_SimpleDescriptor_t *pSimpleDesc;
    /**
     * Network Latency Requirements: use zstack_NetworkLatency_NO_LATENCY_REQS
     */
    zstack_NetworkLatency latencyReq;
} zstack_afRegisterReq_t;

/**
 * Structure to send an AF UnRegister Request.
 */
typedef struct _zstack_afunregisterreq_t
{
    //! Endpoint to unregister
    uint8_t endpoint;
} zstack_afUnRegisterReq_t;

/**
 * Structure to send an AF Data Request.
 */
typedef struct _zstack_afdatareq_t
{
    /**
     * Destination address - The address mode in this structure must be either:
     * afAddrNotPresent to let the reflector (source binding) figure out the
     * destination address; zstack_AFAddrMode_GROUP to send to a group;
     * zstack_AFAddrMode_BROADCAST to send a broadcast message; or
     * zstack_AFAddrMode_SHORT to send directly (unicast) to a node.
     */
    zstack_AFAddr_t dstAddr;
    /** Number of entries in relayList (use 0 for non-source routing) */
    uint8_t n_relayList;
    /** Pointer to an optional arrary of hops (source routing) */
    uint16_t *pRelayList;
    /** Source Endpoint */
    uint8_t srcEndpoint;
    /** Cluster ID */
    uint16_t clusterID;
    /** Transaction ID */
    uint8_t *transID;
    /** The options to send this message */
    zstack_TransOptions_t options;
    /** Network radius - use DEFAULT_RADIUS */
    uint8_t radius;
    /** "payload" size */
    uint16_t n_payload;
    /** Pointer to the "payload" */
    uint8_t *pPayload;
} zstack_afDataReq_t;

/**
 * Structure to send an AF Inter-PAN Control Request.
 */
typedef struct _zstack_afinterpanctlreq_t
{
    /** Set to turn on inter-PAN, Clear to switch back to NIB */
    bool enable;
    /** Rrue if channel is available, false if not */
    bool has_channel;
    /** InterPan channel */
    uint8_t channel;
    /** True if endpoint is available, false if not */
    bool has_endpoint;
    /** Endpoint controller */
    uint8_t endpoint;
    /** True if panID is available, false if not */
    bool has_panID;
    /** PAN ID */
    uint16_t panID;
} zstack_afInterPanCtlReq_t;

/**
 * Structure to send an AF Config Get Request.
 */
typedef struct _zstack_afconfiggetreq_t
{
    //! Endpoint
    uint8_t endpoint;
} zstack_afConfigGetReq_t;

/**
 * Send this message to the ZStack Server to set the
 * AF/APS Fragmentation parameters.
 * The command ID for this message is AF_CONFIG_SET_REQ.
 * The synchronous response to this message is
 * zstack_DefaultRsp_t(cmdID:AF_CONFIG_SET_REQ).
 */
typedef struct _zstack_afconfigsetreq_t
{
    //! Endpoint
    uint8_t endpoint;
    //! Fragmentation frame delay
    uint8_t frameDelay;
    //! Fragmentation window size
    uint8_t windowSize;
} zstack_afConfigSetReq_t;

//*****************************************************************************
// AF Interface Response Structures
//*****************************************************************************

/**
 * Structure to send an AF Config Get Response.
 */
typedef struct _zstack_afconfiggetrsp_t
{
    //! Endpoint
    uint8_t endpoint;
    //! Fragmentation frame delay
    uint8_t frameDelay;
    //! Fragmentation window size
    uint8_t windowSize;
} zstack_afConfigGetRsp_t;

//*****************************************************************************
// AF Interface Indication Structures
//*****************************************************************************

/**
 * Structure to send an AF Data Confirm Indication.
 */
typedef struct _zstack_afdataconfirmind_t
{
    //! status
    zstack_ZStatusValues status;
    //! Endpoint the message was sent from
    uint8_t endpoint;
    //! transaction ID of the AF Data Request
    uint8_t transID;
    //! cluster ID of the AF Data Request
    uint16_t clusterID;
} zstack_afDataConfirmInd_t;

/**
 * Structure to send an AF Incoming Message Indication.
 */
typedef struct _zstack_afincomingmsgind_t
{
    /** Source Address, if endpoint is 0xFE, it's an interPAN message */
    zstack_AFAddr_t srcAddr;
    /** Message's group ID - 0 if not group message */
    uint16_t groupID;
    /** Message's clusterID */
    uint16_t clusterId;
    /** MAC header destination short address */
    uint16_t macDestAddr;
    /** Destination endpoint */
    uint8_t endpoint;
    /** Set if network destination was a broadcast address */
    bool wasBroadcast;
    /** Set if APS security was used */
    bool securityUse;
    /** The link quality of the received data frame */
    uint8_t linkQuality;
    /** The raw correlation value of the received data frame */
    uint8_t correlation;
    /** The received RF power in units dBm */
    int_least8_t rssi;
    /** Receipt timestamp from MAC */
    uint32_t timestamp;
    /** Network header frame sequence number */
    uint8_t nwkSeqNum;
    /** MAC header source short address */
    uint16_t macSrcAddr;
    /** Message's network radius, 0 mean internal */
    uint8_t radius;
    /** Length of the message "payload" */
    uint16_t n_payload;
    /** Pointer to the message "payload" */
    uint8_t *pPayload;
} zstack_afIncomingMsgInd_t;

/**
 * Structure to send an AF Reflection Error Indication.
 */
typedef struct _zstack_afreflecterrorind_t
{
    //! Status of APSDE_DATA_REQUEST
    zstack_ZStatusValues status;
    //! Destination Address
    zstack_AFAddr_t dstAddr;
    //! Transaction ID
    uint8_t transID;
} zstack_afReflectErrorInd_t;

//*****************************************************************************
// ZDO Interface Request Structures
//*****************************************************************************

/**
 * Structure to send a ZDO Network Address Request.
 */
typedef struct _zstack_zdonwkaddrreq_t
{
    //! Known IEEE Address
    zstack_LongAddr_t ieeeAddr;
    //! Request Type
    zstack_NwkAddrReqType type;
    //! Starting index into the list of children.
    uint8_t startIndex;
} zstack_zdoNwkAddrReq_t;

/**
 * Structure to send a ZDO IEEE Address Request.
 */
typedef struct _zstack_zdoieeeaddrreq_t
{
    //! Known short address
    uint16_t nwkAddr;
    //! Request Type
    zstack_NwkAddrReqType type;
    //! Starting index into the list of children.
    uint8_t startIndex;
} zstack_zdoIeeeAddrReq_t;

/**
 * Structure to send a ZDO Node Descriptor Request.
 */
typedef struct _zstack_zdonodedescreq_t
{
    /** Destination address */
    uint16_t dstAddr;
    /** The network address of the destination device being queried. */
    uint16_t nwkAddrOfInterest;
} zstack_zdoNodeDescReq_t;

/**
 * Structure to send a ZDO Power Descriptor Request.
 */
typedef struct _zstack_zdopowerdescreq_t
{
    /** Destination address */
    uint16_t dstAddr;
    /** The network address of the destination device being queried. */
    uint16_t nwkAddrOfInterest;
} zstack_zdoPowerDescReq_t;

/**
 * Structure to send the ZDO Simple Descriptor Request.
 */
typedef struct _zstack_zdosimpledescreq_t
{
    /** Destination address */
    uint16_t dstAddr;
    /** The network address of the destination device being queried. */
    uint16_t nwkAddrOfInterest;
    /** Application endpoint being queried */
    uint8_t endpoint;
} zstack_zdoSimpleDescReq_t;

/**
 * Structure to send a ZDO Active Endpoint Request.
 */
typedef struct _zstack_zdoactiveendpointreq_t
{
    /** Destination address */
    uint16_t dstAddr;
    /** The network address of the destination device being queried. */
    uint16_t nwkAddrOfInterest;
} zstack_zdoActiveEndpointReq_t;

/**
 * Structure to send a ZDO Match Descriptor Request.
 */
typedef struct _zstack_zdomatchdescreq_t
{
    /** Destination address */
    uint16_t dstAddr;
    /** The network address of the destination device being queried. */
    uint16_t nwkAddrOfInterest;
    /** The profile ID of the device */
    uint16_t profileID;
    /** Number of input clusters */
    uint8_t n_inputClusters;
    /** Pointer to array of input clusters */
    uint16_t *pInputClusters;
    /** Number of output clusters */
    uint8_t n_outputClusters;
    /** Pointer to array of output clusters */
    uint16_t *pOutputClusters;
} zstack_zdoMatchDescReq_t;

/**
 * Structure to send a ZDO Complex Descriptor Request.
 */
typedef struct _zstack_zdocomplexdescreq_t
{
    /** Destination address */
    uint16_t dstAddr;
    /** The network address of the destination device being queried. */
    uint16_t nwkAddrOfInterest;
} zstack_zdoComplexDescReq_t;

/**
 * Structure to send a ZDO User Descriptor Request.
 */
typedef struct _zstack_zdouserdescreq_t
{
    /** Destination address */
    uint16_t dstAddr;
    /** The network address of the destination device being queried. */
    uint16_t nwkAddrOfInterest;
} zstack_zdoUserDescReq_t;

/**
 * Structure to send a ZDO Set Bind Unbind Authenticate Address Request. None of these parameters is persistent in Nv
 */
typedef struct _zstack_zdosetbindunbindauthaddrreq_t
{
    /** Address of the remote device authorized to perform
     * Bind/Unbind operations in local device. If set to FF's anyone can do it */
    zstack_LongAddr_t AuthAddress;
    /** Binds to this cluster are restricted to only be modified by AuthAddress,
     * if FF's then all clusters are protected and only AuthAddress can modify binds */
    uint16_t ClusterId;
    /** Binds to this endpoint are restricted to only be modified by AuthAddress,
     * if FF's then all clusters are protected and only AuthAddress can modify binds */
    uint8_t  Endpoint;
} zstack_zdoSetBindUnbindAuthAddr_t;

/**
 * Structure to send a ZDO Device Announce.
 */
typedef struct _zstack_zdodeviceannouncereq_t
{
    //! Network address of device generating
    uint16_t nwkAddr;
    //! 64 bit IEEE Address of device
    zstack_LongAddr_t ieeeAddr;
    //! MAC capabilities
    zstack_CapabilityInfo_t capabilities;
} zstack_zdoDeviceAnnounceReq_t;

/**
 * Structure to send a ZDO User Decriptor Set Request.
 */
typedef struct _zstack_zdouserdescsetreq_t
{
    /** Destination address */
    uint16_t dstAddr;
    /** The network address of the destination device being queried. */
    uint16_t nwkAddrOfInterest;
    /** Length of of the User Descriptor (16 max) */
    uint8_t n_userDescriptor;
    /** Pointer to the descriptor */
    uint8_t *pUserDescriptor;
} zstack_zdoUserDescSetReq_t;

/**
 * Structure to send a ZDO Server Discovery Request.
 */
typedef struct _zstack_zdoserverdiscreq_t
{
    //! Looking for these capabilities
    zstack_ServerCapabilities_t serverMask;
} zstack_zdoServerDiscReq_t;

/**
 * Structure to send a ZDO End Device .
 */
typedef struct _zstack_zdoenddevicebindreq_t
{
    /** Destination address, should be 0x0000 for coordinator */
    uint16_t dstAddr;
    /**
     * The address of the target for the binding. This can be either the primary
     * binding cache device or the short address of the local device.
     */
    uint16_t bindingTarget;
    /** Source device's endpoint */
    uint8_t endpoint;
    /**
     * ProfileID which is to be matched between two End_Device_Bind_req
     * received at the ZigBee Coordinator within the timeout value pre-configured
     * in the ZigBee Coordinator.
     */
    uint16_t profileID;
    /** Number of input clusters */
    uint8_t n_inputClusters;
    /** Pointer to array of input clusters */
    uint16_t *pInputClusters;
    /** Number of output clusters */
    uint8_t n_outputClusters;
    /** Pointer to array of output clusters */
    uint16_t *pOutputClusters;
} zstack_zdoEndDeviceBindReq_t;

/**
 * Structure to send a ZDO Bind Request.
 */
typedef struct _zstack_zdobindreq_t
{
    //! Targeted device
    uint16_t nwkAddr;
    //! Binding Record
    zstack_BindRec_t bindInfo;
} zstack_zdoBindReq_t;

/**
 * Structure to send a ZDO Unbind Request.
 */
typedef struct _zstack_zdounbindreq_t
{
    //! Targeted device
    uint16_t nwkAddr;
    //! Binding Record
    zstack_BindRec_t bindInfo;
} zstack_zdoUnbindReq_t;

/**
 * Structure to send a ZDO Mgmt Network Discovery Request.
 */
typedef struct _zstack_zdomgmtnwkdiscreq_t
{
    //! Targeted device
    uint16_t nwkAddr;
    //! Bit Mask of channels to scan
    uint32_t scanChannels;
    /**
     *  Scanning Time:
     *     0 is 30 milliseconds,
     *     1 is 60 milliseconds,
     *     2 is 120 milliseconds,
     *     3 is 240 milliseconds,
     *     4 is 480 milliseconds,
     *     5 is 960 milliseconds,
     *     6 is 1920 milliseconds,
     *     7 is 3840 milliseconds,
     *     8 is 7680 milliseconds,
     *     9 is 15360 milliseconds,
     *     10 is 30720 milliseconds,
     *     11 is 61440 milliseconds,
     *     12 is 122880 milliseconds,
     *     13 is 245760 milliseconds,
     *     14 is 491520 milliseconds
     */
    uint8_t scanDuration;
    //! Start index for the response array list.
    uint8_t startIndex;
} zstack_zdoMgmtNwkDiscReq_t;

/**
 * Structure to send a ZDO Mgmt LQI Request.
 */
typedef struct _zstack_zdomgmtlqireq_t
{
    //! Targeted device
    uint16_t nwkAddr;
    //! Start index for the response array list.
    uint8_t startIndex;
} zstack_zdoMgmtLqiReq_t;

/**
 * Structure to send a ZDO Mgmt Routing Request.
 */
typedef struct _zstack_zdomgmtrtgreq_t
{
    //! Targeted device
    uint16_t nwkAddr;
    //! Start index for the response array list.
    uint8_t startIndex;
} zstack_zdoMgmtRtgReq_t;

/**
 * Structure to send a ZDO Mgmt Bind Request.
 */
typedef struct _zstack_zdomgmtbindreq_t
{
    //! Targeted device
    uint16_t nwkAddr;
    //! Start index for the response array list.
    uint8_t startIndex;
} zstack_zdoMgmtBindReq_t;

/**
 * Structure to send a ZDO Mgmt Leave Request.
 */
typedef struct _zstack_zdomgmtleavereq_t
{
    /** Targeted device */
    uint16_t nwkAddr;
    /** 64 bit IEEE address of the device to leave the network */
    zstack_LongAddr_t deviceAddress;
    /** Leave options */
    zstack_LeaveOptions_t options;
} zstack_zdoMgmtLeaveReq_t;

/**
 * Structure to send a ZDO Mgmt Direct Join Request.
 */
typedef struct _zstack_zdomgmtdirectjoinreq_t
{
    /** Targeted device */
    uint16_t nwkAddr;
    /** 64 bit IEEE address of the device to be joined to nwkAddr */
    zstack_LongAddr_t deviceAddress;
    /** MAC capabilities */
    zstack_CapabilityInfo_t capInfo;
} zstack_zdoMgmtDirectJoinReq_t;

/**
 * Structure to send a ZDO Mgmt Permit Join Request.
 */
typedef struct _zstack_zdomgmtpermitjoinreq_t
{
    /** Targeted device */
    uint16_t nwkAddr;
    /**
     * Duration of join. 0=disabled, 0xFF=enabled,
     * 0x01-0xFE number seconds to permit join
     */
    uint8_t duration;
    /** If set, and the target device is TC, it affects TC auth policy */
    bool tcSignificance;
} zstack_zdoMgmtPermitJoinReq_t;

/**
 * Structure to send a ZDO Mgmt Network Update Request.
 */
typedef struct _zstack_zdomgmtnwkupdatereq_t
{
    /** Targeted device (unicast or 0xFFFD) */
    uint16_t dstAddr;
    /** Bit masked channel selection */
    uint32_t channelMask;
    /** MAC Scan duration. Range 0x00-0x05, 0xFE or 0xFF */
    uint8_t scanDuration;
    /** Number of energy scans to perform and report */
    uint8_t scanCount;
    /**
     * This value is set by the Network Channel Manager
     * prior to sending the message
     */
    uint8_t nwkUpdateId;
    /** Network address of the Network Manager */
    uint16_t nwkMgrAddr;
} zstack_zdoMgmtNwkUpdateReq_t;

//*****************************************************************************
// ZDO Interface Response Structures
//*****************************************************************************

/**
 * Structure to send ZDO Network Address Response Indication.
 */
typedef struct _zstack_zdonwkaddrrspind_t
{
    /** Response status */
    zstack_ZdpStatus status;
    /** 64 bit IEEE address of source device */
    zstack_LongAddr_t ieeeAddr;
    /** Network address of the responding device */
    uint16_t nwkAddr;
    /** Starting index into the list of associated devices */
    uint8_t startIndex;
    /** Number of associated devices in the list */
    uint8_t n_assocDevList;
    /** Array of (uint16_t) associated devices */
    uint16_t *pAssocDevList;
} zstack_zdoNwkAddrRspInd_t;

/**
 * Structure to send a ZDO IEEE Address Response Indication.
 */
typedef struct _zstack_zdoieeeaddrrspind_t
{
    /** Response status */
    zstack_ZdpStatus status;
    /** 64 bit IEEE address of source device */
    zstack_LongAddr_t ieeeAddr;
    /** Network address of the responding device */
    uint16_t nwkAddr;
    /** Starting index into the list of associated devices */
    uint8_t startIndex;
    /** Number of associated devices in the list */
    uint8_t n_assocDevList;
    /** Array of (uint16_t) associated devices */
    uint16_t *pAssocDevList;
} zstack_zdoIeeeAddrRspInd_t;

/**
 * Structure to send a ZDO Node Descriptor Response Indication.
 */
typedef struct _zstack_zdonodedescrspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
    //! Network address of this node descriptor
    uint16_t nwkAddrOfInterest;
    //! Node Descriptor
    zstack_NodeDescriptor_t nodeDesc;
} zstack_zdoNodeDescRspInd_t;

/**
 * Structure to send a ZDO Power Descriptor Response Indication.
 */
typedef struct _zstack_zdopowerdescrspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
    //! Nwk address of this power descriptor
    uint16_t nwkAddrOfInterest;
    //! Power Descriptor
    zstack_PowerDescriptor_t powerDesc;
} zstack_zdoPowerDescRspInd_t;

/**
 * Structure to send a ZDO Simple Descriptor Response Indication.
 */
typedef struct _zstack_zdosimpledescrspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
    //! Nwk address of this this descriptor
    uint16_t nwkAddrOfInterest;
    //! Simple Descriptor
    zstack_SimpleDescriptor_t simpleDesc;
} zstack_zdoSimpleDescRspInd_t;

/**
 * Structure to send a ZDO Active Endpoint Response Indication.
 */
typedef struct _zstack_zdoactiveendpointsrspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
    //! Nwk address of this this descriptor
    uint16_t nwkAddrOfInterest;
    //! Num active endpoints in the activeEPList
    uint8_t n_activeEPList;
    //! Array of endpoints (uint8_t)
    uint8_t *pActiveEPList;
} zstack_zdoActiveEndpointsRspInd_t;

/**
 * Structure to send a ZDO Match Descriptor Response Indication.
 */
typedef struct _zstack_zdomatchdescrspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
    //! Nwk address of this this descriptor
    uint16_t nwkAddrOfInterest;
    //! Number of matching endpoints
    uint8_t n_matchList;
    //! Array of matching endpoints (uint8_t)
    uint8_t *pMatchList;
} zstack_zdoMatchDescRspInd_t;

/**
 * Structure to send a ZDO User Descriptor Response Indication.
 */
typedef struct _zstack_zdouserdescrspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
    //! Nwk address of this this descriptor
    uint16_t nwkAddrOfInterest;
    //! Number of bytes in the descriptor
    uint8_t n_desc;
    //! Array of descriptor bytes (uint8_t)
    uint8_t *pDesc;
} zstack_zdoUserDescRspInd_t;

/**
 * Structure to send a ZDO Server Discovery Response Indication.
 */
typedef struct _zstack_zdoserverdiscoveryrspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
    //! Server Capabilities
    zstack_ServerCapabilities_t serverCap;
} zstack_zdoServerDiscoveryRspInd_t;

/**
 * Structure to send a ZDO Bind Response Indication.
 */
typedef struct _zstack_zdobindrspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
} zstack_zdoBindRspInd_t;

/**
 * Structer to send a ZDO End Device Bind Response Indication.
 */
typedef struct _zstack_zdoenddevicebindrspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
} zstack_zdoEndDeviceBindRspInd_t;

/**
 * Structure to send a ZDO Unbind Response Indication.
 */
typedef struct _zstack_zdounbindrspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
} zstack_zdoUnbindRspInd_t;

/**
 * Structure to send a ZDO Device Announce Indication.
 */
typedef struct _zstack_zdodeviceannounceind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Device short address
    uint16_t devAddr;
    //! Devices IEEE Address (64-bit)
    zstack_LongAddr_t devExtAddr;
    //! Device's Capabilities
    zstack_CapabilityInfo_t capInfo;
} zstack_zdoDeviceAnnounceInd_t;

/**
 * Structure to send a ZDO Mgmt Network Discovery Response Indication.
 */
typedef struct _zstack_zdomgmtnwkdiscrspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
    //! The total number of networks reported
    uint8_t networkCount;
    //! The starting point in the networkList
    uint8_t startIndex;
    //! Number of items in the netList
    uint8_t n_netList;
    //! Array of networks
    zstack_nwkDiscItem_t *pNetList;
} zstack_zdoMgmtNwkDiscRspInd_t;

/**
 * Structure to send a ZDO Mgmt LQI Response Indication.
 */
typedef struct _zstack_zdomgmtlqirspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
    //! The total number of neighbors
    uint8_t neighborLqiEntries;
    //! The starting point in the LQI List
    uint8_t startIndex;
    //! Number of items in the lqiList
    uint8_t n_lqiList;
    //! Array of neighbor LQI info
    zstack_nwkLqiItem_t *pLqiList;
} zstack_zdoMgmtLqiRspInd_t;

/**
 * Structure to send a ZDO Mgmt Routing Response Indication.
 */
typedef struct _zstack_zdomgmtrtgrspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
    //! The total number of routing entries available
    uint8_t rtgEntries;
    //! The starting point in the routing List
    uint8_t startIndex;
    //! Number of items in the rtgList
    uint8_t n_rtgList;
    //! Array of route items
    zstack_routeItem_t *pRtgList;
} zstack_zdoMgmtRtgRspInd_t;

/**
 * Structure to send a ZDO Mgmt Bind Response Indication.
 */
typedef struct _zstack_zdomgmtbindrspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
    //! The total binding entries available
    uint8_t bindEntries;
    //! The starting point in the binding List
    uint8_t startIndex;
    //! Number of items in the bindList
    uint8_t n_bindList;
    //! Array of binding items
    zstack_bindItem_t *pBindList;
} zstack_zdoMgmtBindRspInd_t;

/**
 * Structure to send a ZDO Mgmt Leave Response Indication.
 */
typedef struct _zstack_zdomgmtleaverspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
} zstack_zdoMgmtLeaveRspInd_t;

/**
 * Structure to send a ZDO Mgmt Direct Join Response Indication.
 */
typedef struct _zstack_zdomgmtdirectjoinrspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
} zstack_zdoMgmtDirectJoinRspInd_t;

/**
 * Structure to send a ZDO Mgmt Permit Join Response Indication.
 */
typedef struct _zstack_zdomgmtpermitjoinrspind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Response status
    zstack_ZdpStatus status;
} zstack_zdoMgmtPermitJoinRspInd_t;

/**
 * Structure to send a ZDO Mgmt Network Update Notify Indication.
 */
typedef struct _zstack_zdomgmtnwkupdatenotifyind_t
{
    /** Source Address */
    uint16_t srcAddr;
    /** Response Status */
    zstack_ZdpStatus status;
    /** Bitmap - list of channels scanned by the request. */
    uint32_t scannedChannels;
    /** Count of the total transmissions */
    uint16_t totalTrans;
    /** Sum of the total transmission failures */
    uint16_t transFails;
    /** Number of items in the energyValuesList */
    uint8_t n_energyValuesList;
    /**
     * Array of bytes, each representing the energy value for each scanned channel
     */
    uint8_t *pEnergyValuesList;
} zstack_zdoMgmtNwkUpdateNotifyInd_t;

/**
 * Structure to send a ZDO Trust Center Device Indication.
 */
typedef struct _zstack_zdotcteviceind_t
{
    /** The joining device's network address */
    uint16_t nwkAddr;
    /** The joining device's parent's network address */
    uint16_t parentAddr;
    /** The joining device's 64-bit extended address */
    zstack_LongAddr_t extendedAddr;
} zstack_zdoTcDeviceInd_t;

//*****************************************************************************
// ZDO Interface Confirm/Indication Structures
//*****************************************************************************

/**
 * Structure to send a ZDO Source Route Indication.
 */
typedef struct _zstack_zdosrcrtgind_t
{
    //! Source address
    uint16_t srcAddr;
    //! Number of relay addresses in the list
    uint8_t n_relay;
    //! Array of relay short addresses
    uint16_t *pRelay;
} zstack_zdoSrcRtgInd_t;

/**
 * Structure to send a ZDO Concentrator Indication.
 */
typedef struct _zstack_zdocncntrtrind_t
{
    //! Network address
    uint16_t nwkAddr;
    //! pkt Cost
    uint8_t pktCost;
    //! 64 bit IEEE address of concentrator
    zstack_LongAddr_t ieeeAddr;
} zstack_zdoCncntrtrInd_t;

/**
 * Structure to send a ZDO Network Discovery Indication.
 */
typedef struct _zstack_zdonwkdisccnf_t
{
    //! Network discovery status
    uint8_t status;
} zstack_zdoNwkDiscCnf_t;

/**
 * Structure to send a ZDO Beacon Notify Indication.
 */
typedef struct _zstack_zdobeaconnotifyind_t
{
    //! Source address
    uint16_t sourceAddr;
    //! PAN ID
    uint16_t panID;
    //! Logical channel
    uint8_t logicalChannel;
    //! Set if joining allowed
    bool permitJoining;
    //! Set if there is router capacity available
    bool routerCapacity;
    //! Set if there is device capacity available
    bool deviceCapacity;
    //! Protocol version
    uint8_t protocolVersion;
    //! Stack profile
    uint8_t stackProfile;
    //! LQI
    uint8_t lqi;
    //! Network depth
    uint8_t depth;
    //! Update ID
    uint8_t updateID;
    //! 64 bit extended PAN ID
    zstack_LongAddr_t extendedPANID;
} zstack_zdoBeaconNotifyInd_t;

/**
 * Structure to send a ZDO Join Confirm Indication.
 */
typedef struct _zstack_zdojoinconf_t
{
    //! Network address of joining
    uint16_t devAddr;
    //! Parent's network address
    uint16_t parentAddr;
} zstack_zdoJoinConf_t;

/**
 * Structure to send a ZDO Leave Confirm Indication.
 */
typedef struct _zstack_zdoleavecnf_t
{
    /** Destination address */
    uint16_t dstAddr;
    /** 64 extended address */
    zstack_LongAddr_t extendedAddr;
    /** Set if remove children option set */
    bool removeChildren;
    /** Set if a rejoin is expected, Cleared if not to rejoin network */
    bool rejoin;
} zstack_zdoLeaveCnf_t;

/**
 * Structure to send a ZDO Join Indication.
 */
typedef struct _zstack_zdoleaveind_t
{
    /** Source address */
    uint16_t srcAddr;
    /** 64 extended address */
    zstack_LongAddr_t extendedAddr;
    /** Request */
    uint8_t request;
    /** Set if remove children option set */
    bool removeChildren;
    /** Set if a rejoin is expected, Cleared if not to rejoin network */
    bool rejoin;
} zstack_zdoLeaveInd_t;


/**
 * Structure to send a Network Frame Forward Notification.
 */
typedef struct _zstack_nwkframefwdnotification_t
{
    /** Structure with information about the frame being forwarded*/
    nwkFrameFwdNotification_t nwkFrameFwdNotification;
} zstack_nwkFrameFwdNotification_t;




//*****************************************************************************
// Device Interface Indication Structures
//*****************************************************************************

/**
 * Structure to send a Device State Change Indication.
 */
typedef struct _zstack_devstatechangeind_t
{
    //! New state
    zstack_DevState state;
} zstack_devStateChangeInd_t;

//*****************************************************************************
// Security Interface Request Structures
//*****************************************************************************

/**
 * Structure to send a Security Network Key Get Request.
 */
typedef struct _zstack_secnwkkeygetreq_t
{
    /** True to get the active key information, False to get the alternate key
      */
    bool activeKey;
} zstack_secNwkKeyGetReq_t;

/**
 * Structure to send a Security Network Key Set Request.
 */
typedef struct _zstack_secnwkkeysetreq_t
{
    /**  True to set the active key information, False to set the alternate key
      */
    bool activeKey;
    /** Key sequence number */
    uint8_t seqNum;
    /** True if key is included */
    bool has_key;
    /**
     * (optional) 16 bytes of key data, if not included the key is
     * randomly generated.
     */
    uint8_t key[16];
} zstack_secNwkKeySetReq_t;

/**
 * Structure to send a Security Network Key Update Request.
 */
typedef struct _zstack_secnwkkeyupdatereq_t
{
    uint8_t* key;
    /** Key sequence number */
    uint8_t seqNum;
    /**
     * Destination address - network address or broadcast
     * address (0xFFFF, 0xFFFC, or 0xFFFD)
     */
    uint16_t dstAddr;
} zstack_secNwkKeyUpdateReq_t;

/**
 * Structure to send a Security Network Key Switch Request.
 */
typedef struct _zstack_secnwkkeyswitchReq_t
{
    /** Key sequence number */
    uint8_t seqNum;
    /**
     * Destination address - network address or broadcast
     * address (0xFFFF, 0xFFFC, or 0xFFFD)
     */
    uint16_t dstAddr;
} zstack_secNwkKeySwitchReq_t;

/**
 * Structure to send a Security APS Link Key Get Request.
 */
typedef struct _zstack_secapslinkkeygetreq_t
{
    /** True if TC Link Key, False if link key */
    bool tcLinkKey;
    /** IEEE Address of the Link Key wanted*/
    zstack_LongAddr_t ieeeAddr;
} zstack_secApsLinkKeyGetReq_t;

/**
 * Structure to send a Security APS Link Key Set Request.
 */
typedef struct _zstack_secapslinkkeysetreq_t
{
    /** True if the key information is for a TC Link Key, false if not */
    bool tcLinkKey;
    /** Must be included for a new non-TC Link Key (ignored for TC Link Key) */
    uint16_t shortAddr;
    /**
     * IEEE Address of the Link Key, USE 0xFFFFFFFFFFFFFFFF for
     * global(only for TC Link Key)
     */
    zstack_LongAddr_t ieeeAddr;
    /** 16 bytes of key data */
    uint8_t key[16];
    /** Transmit frame counter (0 if new Link Keys) */
    uint32_t txFrmCntr;
    /** Receive frame counter (0 if new Link Keys) */
    uint32_t rxFrmCntr;
} zstack_secApsLinkKeySetReq_t;

/**
 * Structure to send a Security APS Link Key Remove Request.
 */
typedef struct _zstack_secapslinkkeyremovereq_t
{
    /** True if the key information is for a TC Link Key, false if not */
    bool tcLinkKey;
    /** IEEE Address of the Link Key */
    zstack_LongAddr_t ieeeAddr;
} zstack_secApsLinkKeyRemoveReq_t;

/**
 * Structure to send a Security APS Remove Request.
 */
typedef struct _zstack_secapsremovereq_t
{
    //! Parent's Short Address
    uint16_t parentAddr;
    //! Device's NWK Address
    uint16_t nwkAddr;
    //! Device's Extended IEEE Address
    zstack_LongAddr_t extAddr;
} zstack_secApsRemoveReq_t;

/**
 * Structure to send a Security Mgr Setup Partner Request.
 */
typedef struct _zstack_secmgrsetuppartnerreq_t
{
    //! Partner's Short Address
    uint16_t partNwkAddr;
    //! Partner's Extended IEEE Address
    uint8_t * partExtAddr;
} zstack_secMgrSetupPartnerReq_t;

/**
 * Structure to send a Security Mgr App Key Type Set Request.
 */
typedef struct _zstack_secmgrappkeytypesetreq_t
{
    /* Key Type */
    uint8_t  keytype;
}zstack_secMgrAppKeyTypeSetReq_t;

/**
 * Structure to send a Security Mgr App Key Request.
 */
typedef struct _zstack_secmgrappkeyreq_t
{
    /* Ext Addr of secure partner */
    uint8_t            partExtAddr[Z_EXTADDR_LEN];
}zstack_secMgrAppKeyReq_t;

/**
 * Structure to send a BDB Start Commissioning Request.
 */
typedef struct _zstack_bdbstartcommissioningreq_t
{
  /** Commissioning modes mask. Refer to DEFAULT_COMISSIONING_MODE */
  uint8_t commissioning_mode;
} zstack_bdbStartCommissioningReq_t;

/**
 * Structure to send a Set Identify Active Endpoint Request.
 */
typedef struct _zstack_bdbsetidentifyactiveendpointreq_t
{
  /** Set the endpoint which will perform the finding and binding (either Target or
  Initiator) */
  uint8_t activeEndpoint;
} zstack_bdbSetIdentifyActiveEndpointReq_t;

/**
 * Structure to return the finding and binding active endpoint response.
 */
typedef struct _zstack_bdbgetidentifyactiveendpointrsp_t
{
  /** Get the endpoint which will perform the finding and binding (either Target or
  Initiator) */
  uint8_t activeEndpoint;
} zstack_bdbGetIdentifyActiveEndpointRsp_t;

/**
 * Structure to return the next ZCL Frame Counter for packet sequence number
 * response.
 */
typedef struct _zstack_getzclframecounterrsp_t
{
  /** Get the stack zcl frame counter increased by one */
  uint8_t zclFrameCounter;
}zstack_getZCLFrameCounterRsp_t;

/**
 * Structure to send to the stack the identify command parameters.
 */
typedef struct _zstack_bdbzclidentifycmdindreq_t
{
  /** Parameters of identify command */
  uint16_t identifyTime;
  uint8_t  endpoint;
}zstack_bdbZCLIdentifyCmdIndReq_t;


/**
 * Structure to return the active endpoint on F&B.
 */
typedef struct _zstack_bdbsetepdesclisttoactiveendpointrsp_t
{
  /** Active endpoint for F&B */
  uint8_t  EndPoint;
}zstack_bdbSetEpDescListToActiveEndpointRsp_t;


/**
  * Structure to send a BDB Set Attributes Request. Each config field has
  * 2 parts to it, the actual value field and its "has_" field.  The "has_" field
  * must be set to "true" for the value field to be valid.  For example, the
  * pollRate field is valid only if the has_pollRate field is set to true.
  */
typedef struct _zstack_bdbsetattributesreq_t
{
    /** Secondary Channel Set */
    uint32_t bdbSecondaryChannelSet;
    /** Set to true if bdbSecondaryChannelSet is available, false if not */
    bool has_bdbSecondaryChannelSet;
    /** Primary Channel Set */
    uint32_t bdbPrimaryChannelSet;
    /** Set to true if bdbPrimaryChannelSet is available, false if not */
    bool has_bdbPrimaryChannelSet;
    /** Group ID for Finding and Binding */
    uint16_t bdbCommissioningGroupID;
    /** Set to true if bdbCommissioningGroupID is available, false if not */
    bool has_bdbCommissioningGroupID;
    /** Scan Duration */
    uint8_t bdbScanDuration;
    /** Set to true if bdbScanDuration is available, false if not */
    bool has_bdbScanDuration;
#if (ZG_BUILD_COORDINATOR_TYPE)
    /** Is install codes mandated for joining devices? (ZC only) */
    bool bdbJoinUsesInstallCodeKey;
    /** Set to true if bdbJoinUsesInstallCodeKey is available, false if not */
    bool has_bdbJoinUsesInstallCodeKey;
    /** Timeout to allow joining devices to perform TCLK exchange (ZC only) */
    uint8_t bdbTrustCenterNodeJoinTimeout;
    /** Set to true if bdbTrustCenterNodeJoinTimeout is available, false if not */
    bool has_bdbTrustCenterNodeJoinTimeout;
    /** Is mandated to perform TCLK exchange for joining devices? (ZC only) */
    bool bdbTrustCenterRequireKeyExchange;
    /** Set to true if bdbTrustCenterRequireKeyExchange is available, false if not*/
    bool has_bdbTrustCenterRequireKeyExchange;
#endif
#if (ZG_BUILD_JOINING_TYPE)
    /** Maximum number of attempts to join the same network will be done before give
    up (ZR/ZED only) */
    uint8_t bdbTCLinkKeyExchangeAttemptsMax;
    /** Set to true if bdbTCLinkKeyExchangeAttemptsMax is available, false if not */
    bool has_bdbTCLinkKeyExchangeAttemptsMax;
    /** Standard TCLK or CBKE (ZR/ZED only) */
    uint8_t bdbTCLinkKeyExchangeMethod;
    /** Set to true if bdbTCLinkKeyExchangeMethod is available, false if not */
    bool has_bdbTCLinkKeyExchangeMethod;
#endif

} zstack_bdbSetAttributesReq_t;

/**
 * Structure to send a BDB Get Attribute Request. This is used to
 * retrieve the attributes currently set for bdb commissioning procedures.
 */
typedef struct _zstack_bdbgetattributesrsp_t
{
    /** Secondary Channel Set */
    uint32_t bdbSecondaryChannelSet;
    /** Primary Channel Set */
    uint32_t bdbPrimaryChannelSet;
    /** Group ID for Finding and Binding */
    uint16_t bdbCommissioningGroupID;
    /** Commissioning status from last call. */
    uint8_t  bdbCommissioningStatus;
    /** Current commissioning methods in progress (read only) */
    uint8_t  bdbCommissioningMode;
    /** Commissioning capabilities (read only) */
    uint8_t  bdbNodeCommissioningCapability;
    /** Scan Duration */
    uint8_t  bdbScanDuration;
    /** Is the device commissioned to a network (read only) */
    bool   bdbNodeIsOnANetwork;
#if (ZG_BUILD_COORDINATOR_TYPE)
    /** Is install codes mandated for joining devices? (ZC only) */
    bool   bdbJoinUsesInstallCodeKey;
    /** Timeout to allow joining devices to perform TCLK exchange (ZC only) */
    uint8_t  bdbTrustCenterNodeJoinTimeout;
    /** Is mandated to perform TCLK exchange for joining devices? (ZC only) */
    bool   bdbTrustCenterRequireKeyExchange;
#endif
#if (ZG_BUILD_JOINING_TYPE)
    /** LinkKey used by the joining device (read only)(ZR/ZED only) */
    uint8_t  bdbNodeJoinLinkKeyType;
    /** Number of attempts to perform association in the current network (read only)
    (ZR/ZED only) */
    uint8_t  bdbTCLinkKeyExchangeAttempts;
    /** Maximum number of attempts to join the same network will be done before
    giveup (ZR/ZED only) */
    uint8_t  bdbTCLinkKeyExchangeAttemptsMax;
    /** Standard TCLK or CBKE (ZR/ZED only) */
    uint8_t  bdbTCLinkKeyExchangeMethod;
#endif
} zstack_bdbGetAttributesRsp_t;

/**
 * Structure to return the finding and binding initiator status response.
 */
typedef struct _zstack_bdbgetfbinitiatorstatusrsp_t
{
  /** Remaining time in seconds */
  uint8_t RemainingTime;
  /** Number of attempts left to find targets */
  uint8_t AttemptsLeft;
}
zstack_bdbGetFBInitiatorStatusRsp_t;


/**
 * Structure to send a bdb generate Install Code Request.
 */
typedef struct _zstack_bdbgenerateinstallcodecrcreq_t
{
  /** Install code from which CRC will be generated */
  uint8_t installCode[16];
} zstack_bdbGenerateInstallCodeCRCReq_t;

/**
 * Structure to send a bdb generate Install Code Request.
 */
typedef struct _zstack_bdbgenerateinstallcodecrcrsp_t
{
  /** CRC generated */
  uint16_t CRC;
} zstack_bdbGenerateInstallCodeCRCRsp_t;

#ifdef BDB_REPORTING
/**
 * Structure to send a BDB Reportorting Attribute Configure Record Default Request.
 */
typedef struct _zstack_bdbrepaddattrcfgrecorddefaulttolistreq_t
{
  /** Endpoint */
  uint8_t endpoint;
  /** Cluster */
  uint16_t cluster;
  /** Attribute ID */
  uint16_t attrID;
  /** Value for minimum reportable interval */
  uint16_t minReportInt;
  /** Value for maximum reportable interval. If no change to the attribute is done,
  this is the period with which the attribute is reported. */
  uint16_t maxReportInt;
  /** Attribute value that is the delta change to trigger a report */
  uint8_t reportableChange[8];
} zstack_bdbRepAddAttrCfgRecordDefaultToListReq_t;

/**
 * Structure to send a BDB Report Changed Attribute Value Request.
 */
typedef struct _zstack_bdbrepchangedattrvaluereq_t
{
  /** Endpoint */
  uint8_t endpoint;
  /** Cluster */
  uint16_t cluster;
  /** Attribute ID */
  uint16_t attrID;
} zstack_bdbRepChangedAttrValueReq_t;



/**
 * Structure to send a ZCL Configure Reporting Command Request to the stack.
 */
typedef struct _zstack_bdbprocessinconfigreportingreq_t
{
  /** zcl incomming message with configure report */
  zclIncomingMsg_t *pZclIncommingMsg;
} zstack_bdbProcessInConfigReportReq_t;


/**
 * Structure to send a ZCL Read Report Configuration Command Request to the stack.
 */
typedef struct _zstack_bdbprocessinreadconfigreportcfgreq_t
{
  /** zcl incomming message with configure report */
  zclIncomingMsg_t *pZclIncommingMsg;
} zstack_bdbProcessInReadReportCfgReq_t;



#endif

/**
 * Structure to send a BDB Add Install Code Request.
 */
typedef struct _zstack_bdbaddinstallcodereq_t
{
  /** Install Code with CRC, buffer size of 18 bytes (16 bytes install code + 2
  bytes CRC) */
  uint8_t pInstallCode[18];
  /** Extended address of the joining node */
  uint8_t pExt[EXTADDR_LEN];
} zstack_bdbAddInstallCodeReq_t;

/**
 * Structure to send a BDB Touchlink Target Enable Commissioning Request.
 */
typedef struct _zstack_bdbtouchlinktargetenablecommissioningreq_t
{
  /** Time that touchlink commissioning as target will be enabled.
  A value equal or bigger than TOUCHLINK_TARGET_PERPETUAL will enable
  this commissioning method until disabled by the application. */
  uint32_t timeoutTime;
} zstack_bdbTouchLinkTargetEnableCommissioningReq_t;


typedef struct _zstack_bdbtouchlinktargetgettimerrsp_t
{
  /* Time left for touchlink commissioning as target */
  uint32_t Time;
} zstack_bdbTouchLinkTargetGetTimerRsp_t;

/**
 * Structure to send a BDB Set Active Centralized Link Key Request.
 */
typedef struct _zstack_bdbsetactivecentralizedlinkkeyreq_t
{
  /** Format of the key to be used during joining procedure into a centralized
  network */
  zstack_CentralizedLinkKeyModes_t zstack_CentralizedLinkKeyModes;
  /** Buffer that holds the expected key in the format indicated by
  zstack_CentralizedLinkKeyModes */
  uint8_t *pKey;
} zstack_bdbSetActiveCentralizedLinkKeyReq_t;

/**
 * Structure to send a BDB CBKE TrustCenter Key Exchange Attempt Request.
 */
typedef struct _zstack_bdbcbketclinkkeyexchangeattemptreq_t
{
  /** Indicate the stack if the process was success or not. If no
  Certificate Based Key Exchange (CBKE) procedures was performed,
  return false so the stack can perform the default Trust Center Link Key
  exchange procedure */
  bool didSuccess;
} zstack_bdbCBKETCLinkKeyExchangeAttemptReq_t;

/**
 * Structure to send a BDB Network Descriptor Free Request.
 */
typedef struct _zstack_bdbnwkdescfreereq_t
{
  /** Pointer to the network descriptor to be released from the list
  of networks discovered during network steering */
  networkDesc_t* nodeDescToRemove;
} zstack_bdbNwkDescFreeReq_t;

/**
 * Structure to send a BDB Touchlink Set Allow Stealing Request.
 */
typedef struct _zstack_bdbtouchlinksetallowstealingreq_t
{
  /** Flag (TRUE or FALSE) to indicate if touchlink stealing is allowed */
  bool allowStealing;
} zstack_bdbTouchlinkSetAllowStealingReq_t;


/**
 * Structure to send a BDB Touchlink Get Allow Stealing Request.
 */
typedef struct _zstack_bdbtouchlinkgetallowstealingrsp_t
{
  /** Flag (TRUE or FALSE) to indicate if touchlink stealing is allowed */
  bool allowStealing;
} zstack_bdbTouchlinkGetAllowStealingRsp_t;

/**
 * Structure to send a BDB to Recover Network Request.
 */
typedef struct _zstack_bdbRecoverNwkRsp_t
{
  /** Indicate if the request was process or not. The request will fail
  if the device has not been commissioned yet */
  uint8_t status;
}zstack_bdbRecoverNwkRsp_t;


/**
 * Structure to send a GP Allow Change Channel Request.
 */
typedef struct _zstack_gpallowchangechannelreq_t
{
  /** Indicate if the application is allowing GP commissioning procedures
  to change channel. DO NOT ALLOW DURING APPLICATION PROCESS. BDB process
  sets its own flag to not allow change channel which is independent from
  applications flag*/
  uint8_t allowChangeChannel;
}zstack_gpAllowChangeChannelReq_t;

/**
 * Structure to send a GP Commissioning mode.
 */
typedef struct zstackmsg_gpcommissioningmodeind_t
{
    /** Flag to enter in commissioning mode */
    bool isEntering;
    /** Flag for Commissioning window time */
    bool hasTime;
    /** Commissioning window time value */
    uint16_t time;
}zstack_gpCommissioningModeInd_t;

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
/**
 * Structure to send a GP Data Indication.
 */
typedef struct _zstack_gpdataind_t
{
  /** The data indication Handler */
  gp_DataIndSecReq_t  SecReqHandling;
  /** The Timestamp delivered by the MAC on receipt of this frame */
  uint32_t              timestamp;
  /** Status code, as returned by dGP stub */
  uint8_t               status;
  /** The RSSI delivered by the MAC on receipt of this frame */
  int8_t                Rssi;
  /** The LQI delivered by the MAC on receipt of this frame */
  uint8_t               LinkQuality;
  /** The sequence number from MAC header of the received MPDU */
  uint8_t               SeqNumber;
  /** The device address of the GPD entity from which the ASDU was received */
  sAddr_t             srcAddr;
  /** The 16-bit PAN identifier of the GPD entity from which the ASDU was received */
  uint16_t              srcPanID;
  /** The ApplicationID, corresponding to the received MPDU.
  ApplicationID 0x00 indicates the usage of the SrcID;
  ApplicationID 0x02 indicates the usage of the GPD IEEE address */
  uint8_t               appID;
  /** The security level, corresponding to the received MPDU */
  uint8_t               GPDFSecLvl;
  /** The security key type, which was successfully used for
  security processing the received MPDU */
  uint8_t               GPDFKeyType;
  /** The Auto-Commissioning sub-field, copied from the received GPDF */
  bool                AutoCommissioning;
  /** The RxAfterTx sub-field, copied from the received GPDF */
  bool                RxAfterTx;
  /** The identifier of the GPD entity from which the ASDU was received */
  uint32_t              SrcId;
  /** The identifier of the GPD endpoint used in combination with
  the GPD IEEE address if ApplicationID = 0b010.
  If ApplicationID = 0b000 this parameter is ignored */
  uint8_t               EndPoint;
  /** The security frame counter value used on transmission by the
  GPD entity from which the ASDU was received */
  uint32_t              GPDSecFrameCounter;
  /** The identifier of the command, within the GP specification,
  which defines the application semantics of the ASDU */
  uint8_t               GPDCmmdID;
  /** The sequence of octets forming the MIC for the received
  GPD MPDU */
  uint32_t              MIC;
  /** The number of octets in the received GPD ASDU */
  uint8_t               GPDasduLength;
  /** The sequence of octets forming the received GPD ASDU */
  uint8_t               GPDasdu[1];
}zstack_gpDataInd_t;
#endif

/**
 * Structure of Green Power Commissioning Mode.
 */
typedef struct _zstack_gpCommissioningMode_t
{
    /** The flag to enter in commissioning mode  */
    uint8_t isEntering;

    /** The flag for commissioning time */
    uint8_t hasTime;

    /** The commissioning timeout  */
    uint16_t time;
}zstack_gpCommissioningMode_t;

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
/**
 * Structure to send a GP Security Request.
 */
typedef struct _zstack_gpSecReq_t
{
  /** Green power device ID parameters */
  gpdID_t      gpdID;
  /** The identifier of the GPD endpoint used in combination with
  the GPD IEEE address if ApplicationID = 0b010.
  If ApplicationID = 0b000 this parameter is ignored */
  uint8_t        EndPoint;
  /** Security parameters */
  gp_SecData_t gp_SecData;
  /* The handle used between dGP stub and the higher layers, to
  match the request with the response */
  uint8_t        dGPStubHandle;
}zstack_gpSecReq_t;

/**
 * Structure to send a GP Security Response.
 */
typedef struct _zstack_gpSecRsp_t
{
  /** The status code, as returned by the Green Power EndPoint */
  uint8_t         Status;
  /** The handle used between dGP stub and the higher layers, to
  match the request with the response */
  uint8_t         dGPStubHandle;
  /* Green power device ID parameters */
  gpdID_t       gpdID;
  /** The identifier of the GPD endpoint used in combination with
  the GPD IEEE address if ApplicationID = 0b010.
  If ApplicationID = 0b000 this parameter is ignored */
  uint8_t         EndPoint;
  /** Security parameters */
  gp_SecData_t  gp_SecData;
  /** The security key to be used for GPDF security processing */
  uint8_t         GPDKey[SEC_KEY_LEN];
}zstack_gpSecRsp_t;
#endif

typedef struct _zstack_gpcheckannounce_t
{
  /** Annoinced nwk address */
  uint16_t      nwkAddr;
  /** Announced IEEE address */
  uint8_t       extAddr[Z_EXTADDR_LEN];
} zstack_gpCheckAnnounce_t;

typedef struct _zstack_gpaddrconflict_t
{
  /** Nwk address with conclict */
  uint16_t      nwkAddr;
  /** Device IEEE address */
  uint8_t       extAddr[Z_EXTADDR_LEN];
  /** Conflict resolution flag */
  bool        conflictResolution;
} zstack_gpAddrConflict_t;

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
/**
 * Structure to send gp data indication to decryption.
 */
typedef struct _zstack_gpdecryptdataind_t
{
  /** Green Power Data Indication Encrypted */
  gp_DataInd_t *pGpDataInd;
  /** key */
  uint8_t         key[SEC_KEY_LEN];
} zstack_gpDecryptDataInd_t;

/**
 * Structure to send gp data indication to decryption.
 */
typedef struct _zstack_gpdecryptdataindrsp_t
{
  /** Green Power Data Indication Decrypted*/
  gp_DataInd_t *pGpDataInd;
  /** status of CCM star operation **/
  uint8_t         status;
} zstack_gpDecryptDataIndRsp_t;

/**
 * Structure to send a GPDF commissioning key for encryption/decryption.
 */
typedef struct _zstack_gpencryptdecryptcommissioningkey_t
{
  /** Encrypt */
  bool        encrypt;
  /** Green PowerDevice ID */
  gpdID_t     gpdId;
  /** Key Type **/
  uint8_t       keyType;
  /** Encrypted key */
  uint8_t       key[SEC_KEY_LEN];
  /** MIC for the encrypted key **/
  uint32_t      keyMic;
  /** Security Key **/
  uint8_t       secKey[SEC_KEY_LEN];
  /** Current Security Frame Counter */
  uint32_t      secFrameCounter;
} zstack_gpEncryptDecryptCommissioningKey_t;

/**
 * Structure hold the gp commissioning success.
 */
typedef struct zstack_gpcommissioningsuccess_t
{
    /** Sink Table Index **/
    uint16_t       index;
    /** GPD alias nwk address **/
    uint16_t       alias;
    /** Green power device ID parameters */
    gpdID_t      gpdID;
} zstack_gpCommissioningSuccess_t;
#endif

/**
 * Structure hold the CCM star GPD key result.
 */
typedef struct _zstack_gpencryptdecryptcommissioningkeyrsp_t
{
  /** Security Key **/
  uint8_t       key[SEC_KEY_LEN];
  /** MIC for the encrypted key **/
  uint32_t      keyMic;
  /** Current Security Frame Counter */
  uint32_t      secFrameCounter;
  /** status of CCM star operation **/
  uint8_t       status;
} zstack_gpEncryptDecryptCommissioningKeyRsp_t;

/**
 * Structure hold the GPD alias address.
 */
typedef struct _zstackmsg_gpaliasnwkaddress_t
{
    /** GPD alias nwk address **/
    uint16_t       alias;
} zstack_gpAliasNwkAddress_t;

#if defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
/**
 * Structure to send a Touchlink Scan Request.
 */
typedef struct _zstack_tlScanReq_t
{
  /** Device address, must be extended address mode */
  afAddrType_t     addr;
  /** The inter-PAN transaction identifier field is 32-bits in
  length and specifies an identifier for the inter-PAN transaction */
  uint32_t           transID;
  /** Sequence Number */
  uint8_t            seqNum;
}zstack_tlScanReq_t;

/**
 * Structure to send a Touchlink Target Network Start Request.
 */
typedef struct _zstack_tlTargetNwkStartReq_t
{
  /** Device address, must be extended address mode */
  afAddrType_t        addr;
  /* Start request parameters */
  bdbTLNwkStartReq_t  nwkStartReq;
  /** Sequence Number */
  uint8_t               seqNum;
  /** Allow stealing flag */
  bool                allowStealing;
}zstack_tlTargetNwkStartReq_t;

/**
 * Structure to send a Touchlink Target Network Join Request.
 */
typedef struct _zstack_touchlinkNwkJoinReq_t
{
  /** Device address, must be extended address mode */
  afAddrType_t        addr;
  /** Join request parameters */
  bdbTLNwkJoinReq_t   nwkJoinReq;
  /** Sequence Number */
  uint8_t               seqNum;
#if defined ( BDB_TL_TARGET )
  /** Allow stealing flag */
  bool                allowStealing;
#endif
}zstack_touchlinkNwkJointReq_t;

/**
 * Structure to send a Touchlink Target Network Join Request.
 */
typedef struct _zstack_touchlinkNwkUpdateReq_t
{
  /** The inter-PAN transaction identifier field is 32-bits in
  length and specifies an identifier for the inter-PAN transaction */
  uint32_t        transID;
  /** IEEE address */
  uint8_t         extendedPANID[Z_EXTADDR_LEN];
  /** Update identifier */
  uint8_t         nwkUpdateId;
  /** Logical channel */
  uint8_t         logicalChannel;
  /** Network Pan ID */
  uint16_t        PANID;
  /** Nwk address */
  uint16_t        nwkAddr;
} zstack_touchlinkNwkUpdateReq_t;

/**
 * Structure to send a Touchlink Get Scan Base time.
 */
typedef struct _zstack_touchlinkGetScanBaseTime_t
{
  /** Touchlink Scan Base remaining time */
  uint16_t time;
  /** Flag (TRUE or FALSE) to indicate if the timer will be stopped */
  bool   stopTimer;
} zstack_touchlinkGetScanBaseTime_t;

/**
 * Structure to send a Touchlink scan response.
 */
typedef struct _zstack_touchlinkScanRsp_t
{
  /** Device address, must be extended address mode */
  afAddrType_t addr;
  /** Scan response parameters */
  bdbTLScanRsp_t scanRsp;
} zstack_touchlinkScanRsp_t;

/**
 * Structure to send a Touchlink device information response.
 */
typedef struct _zstack_touchlinkDevInfoRsp_t
{
  /** Device address, must be extended address mode */
  afAddrType_t addr;
  /** Device information response parameters */
  bdbTLDeviceInfoRsp_t devInfoRsp;
} zstack_touchlinkDevInfoRsp_t;

/**
 * Structure to send a Touchlink device information response.
 */
typedef struct _zstack_touchlinkNwkStartRsp_t
{
  /** Network start response parameters */
  bdbTLNwkStartRsp_t nwkStartRsp;
} zstack_touchlinkNwkStartRsp_t;

/**
 * Structure to send a Touchlink device information response.
 */
typedef struct _zstack_touchlinkNwkJoinRsp_t
{
  /** Device address, must be extended address mode */
  afAddrType_t addr;
  /** Join respose parameters */
  bdbTLNwkJoinRsp_t  nwkJoinRsp;
} zstack_touchlinkNwkJoinRsp_t;
#endif

//*****************************************************************************
// Security Interface Response Structures
//*****************************************************************************

/**
 * Structure to send a Security Network Key Get Response.
 */
typedef struct _zstack_secnwkkeygetrsp_t
{
    /**
     * True for the active key information, False for the alternate key
     * information
     */
    bool activeKey;
    /** Key sequence number */
    uint8_t seqNum;
    /** 16 bytes of key data */
    uint8_t key[16];
} zstack_secNwkKeyGetRsp_t;

/**
 * Structure to send a Security APS Link Key Get Response.
 */
typedef struct _zstack_secapslinkkeygetrsp_t
{
    /** True if the key information is for a TC Link Key, false if not */
    bool tcLinkKey;
    /** IEEE Address of the Link Key wanted */
    zstack_LongAddr_t ieeeAddr;
    /** 16 bytes of key data */
    uint8_t key[16];
    /** Transmit frame counter */
    uint32_t txFrmCntr;
    /** Receive frame counter */
    uint32_t rxFrmCntr;
} zstack_secApsLinkKeyGetRsp_t;

//*****************************************************************************
// Generic Response Structure
//*****************************************************************************

/**
 * Structure to send a Generic Default Response.
 */
typedef struct _zstack_defaultrsp_t
{
    //! Response status - default status
    zstack_ZStatusValues status;
} zstack_DefaultRsp_t;

//*****************************************************************************
//*****************************************************************************

/**
 * Structure to send a device Pause/Resume Request
 */
typedef struct _zstack_pauseResumeDeviceReq_t
{
    /*
     * Set this to true to pause the device
     * Set this to false to resume the device
     */
    bool pause;
} zstack_pauseResumeDeviceReq_t;


#ifdef __cplusplus
}
#endif

#endif /* ZSTACK_H */

