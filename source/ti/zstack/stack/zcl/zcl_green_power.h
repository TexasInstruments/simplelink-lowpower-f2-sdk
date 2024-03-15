/**************************************************************************************************
  Filename:       zcl_green_power.h
  Revised:        $Date: 2014-10-14 13:03:14 -0700 (Tue, 14 Oct 2014) $
  Revision:       $Revision: 40629 $

  Description:    This file contains the ZCL Green Power definitions.


  Copyright (c) 2019, Texas Instruments Incorporated
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  *  Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

  *  Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

  *  Neither the name of Texas Instruments Incorporated nor the names of
      its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************/

/**
 *  @defgroup ZCL_GREEN_POWER ZCL Green Power feature
 *  @brief This module implements the Zigbee Green Power feature
 *  @{
 *
 *  @file  zcl_green_power.h
 *  @brief      ZCL Green Power header
 */


#ifndef ZCL_GREEN_POWER_H
#define ZCL_GREEN_POWER_H

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "cgp_stub.h"
#include "gp_bit_fields.h"
#include "ti_zstack_config.h"

 /*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/**
 * @defgroup GP_SIMPLE_DESCRIPTOR Green Power Simple Descriptor
 * @{
 * @brief This group defines the values for the
 * simple descriptor of the internal Green Power endpoint
 */

/// Reserved Green Power endpoint
#define GREEN_POWER_INTERNAL_ENDPOINT                       0xF2
/// Reserved Green Power Profile ID
#define ZCL_GP_PROFILE_ID                                 0xA1E0
/// Green Power Proxy device ID
#define ZCL_GP_DEVICEID_PROXY_BASIC                       0x0061
/** @} End GP_SIMPLE_DESCRIPTOR */

/**
 * @defgroup ATTRID_GP Green Power Cluster Attribute IDs
 * @{
 * @brief This group defines the IDs used for each
 * Green Power Cluster attribute
 */
/********************************/
/*** Green Power Cluster Attributes ***/
/********************************/

/**
 * @defgroup ATTRID_GPS Green Power Sink Attribute IDs
 * @{
 * @brief These attributes are used by Green Power Sinks
 */
/// The maximum number of Sink Table entries that can be stored by this sink.
#define ATTRID_GP_GPS_MAX_SINK_TABLE_ENTRIES              0x0000
/// The Sink Table attribute contains the pairings configured for this sink.
#define ATTRID_GP_SINK_TABLE                              0x0001
/// The gpsCommunicationMode attribute contains the communication mode
/// required by this sink.
#define ATTRID_GP_GPS_COMMUNICATION_MODE                  0x0002
/// The gpsCommissioningExitMode attribute contains the information on commissioning
/// mode exit requirements of this sink.
#define ATTRID_GP_GPS_COMMISSIONING_EXIT_MODE             0x0003
/// The gpsCommissioningWindow attribute contains the information on the time,
/// in seconds, during which this sink accepts pairing changes
#define ATTRID_GP_GPS_COMMISSIONING_WINDOW                0x0004
/// The gpsSecurityLevel attribute contains the minimum security level this
/// sink requires the paired GPDs to support.
#define ATTRID_GP_GPS_SECURITY_LEVEL                      0x0005
/// The gpsFunctionality attribute indicates support of the GP functionality by this device.
#define ATTRID_GP_GPS_FUNCTIONALITY                       0x0006
/// The gpsActiveFunctionality attribute indicates which GP functionality
/// supported by this device is currently enabled.
#define ATTRID_GP_GPS_ACTIVE_FUNCTIONALITY                0x0007
/** @} End ATTRID_GPS */

/**
 * @defgroup ATTRID_GPP Green Power Proxy Attributes
 * @{
 * @brief These attributes are used by Green Power Proxies
 */
/// Maximum number of Proxy Table entries this node can hold.
#define ATTRID_GP_GPP_MAX_PROXY_TABLE_ENTRIES             0x0010
/// The Proxy Table attribute contains the information on GPDs active in the
/// system and the corresponding sinks.
#define ATTRID_GP_PROXY_TABLE                             0x0011
/// This attribute defines the maximum number of retransmissions in case a
/// GP Notification Response.
#define ATTRID_GP_GPP_NOTIFICATION_RETRY_NUMBER           0x0012
/// This attribute defines the time to wait for GP Notification Response command
/// after sending full unicast GP Notification command.
#define ATTRID_GP_GPP_NOTIFICATION_RETRY_TIMER            0x0013
/// This attribute defines the maximum value the Search Counter can take, before it rolls over.
#define ATTRID_GP_GPP_MAX_SEARCH_COUNTER                  0x0014
/// The gppBlockedGPDID attribute contains the information on GPDs active in the vicinity.
#define ATTRID_GP_GPP_BLOCKED_GPD_ID                      0x0015
/// The gppFunctionality attribute indicates support of the GP functionality by this device.
#define ATTRID_GP_GPP_FUNCTIONALITY                       0x0016
/// The gppActiveFunctionality attribute indicates which GP functionality supported
/// by this device is currently enabled.
#define ATTRID_GP_GPP_ACTIVE_FUNCTIONALITY                0x0017
/** @} End ATTRID_GPP */

/**
 * @defgroup ATTRID_SHARED Green Power Shared Attributes
 * @{
 * @brief These attributes are shared by client and server
 */
/// The gpSharedSecurityKeyType attribute stores the key type of the shared security key.
#define ATTRID_GP_SHARED_SEC_KEY_TYPE                     0x0020
/// The gpSharedSecurityKey attribute stores the shared security key of the key type
/// as indicated in the gpSecurityKeyType attribute.
#define ATTRID_GP_SHARED_SEC_KEY                          0x0021
/// The gpLinkKey attribute stores the Link Key, used to encrypt the key transmitted
/// in the Commissioning GPDF and Commissioning Reply GPDF.
#define ATTRID_GP_LINK_KEY                                0x0022
/** @} End ATTRID_SHARED */

/** @} End ATTRID_GP */

/**
 * @defgroup SINK_DEFAULTS Default Values for GP Sink
 * @{
 * @brief These are default values used by GP Sinks
 */
/// Maximum number of Sink Table entries supported by this device
#define GPS_MAX_SINK_TABLE_ENTRIES         0x05
/// Default communication mode requested by this sink
#define GPS_COMMUNICATION_MODE             0x01
/// Conditions for the commissioning mode sink to exit the commissioning mode
#define GPS_COMMISSIONING_EXIT_MODE        0x02
/// Default duration of the Commissioning window duration, in seconds,
/// as requested by this sink
#define GPS_COMMISSIONING_WINDOW           0x00B4
/// The minimum required security level to be supported by the paired GPDs
#define GPS_SECURITY_LEVEL                 0x06
/// The radius value
#define GPS_RADIUS                         0x08
/** @} End SINK_DEFAULTS */

/**
 * @defgroup ZIGBEE_PRO_GPS_SPEC Zigbee PRO Green Power Sink specification
 * @{
 * @brief Bit masks showing which GPS functions supported
 */
/**********************************************************************************
ZigBee PRO Green Power feature specification
Basic functionality set
Version 1.0

Table 28 � Format of the gpsFunctionality attribute
----------------------------------------------------------------------------------
Indication   | Functionality                                        | Basic Proxy
-------------|------------------------------------------------------|-------------
b0           | GP feature                                           | 0b1
b1           | Direct communication (reception of GPDF via GP stub) | 0b1
b2           | Derived groupcast communication                      | 0b1
b3           | Pre-commissioned groupcast communication             | 0b1
b4           | Full unicast communication                           | 0b0
b5           | Lightweight unicast communication                    | 0b1
b6           | Proximity bidirectional operation                    | 0b0
b7           | Bidirectional operation                              | 0b0
b8           | Proxy Table maintenance (active and passive,         | 0b0
             | for GPD mobility and GPP robustness)                 |
b9           | Reserved                                             | 0b0
b10          | GP commissioning                                     | 0b1
b11          | CT-based commissioning                               | 0b1
b12          | Maintenance of GPD (deliver channel/key              | 0b0
             | during operation)                                    |
b13          | gpdSecurityLevel  = 0b00                             | 0b1
b14          | Deprecated: gpdSe curityLevel = 0b01                 | 0b0
b15          | gpdSecurityLevel  = 0b10                             | 0b1
b16          | gpdSecurityLevel  = 0b11                             | 0b1
b17          | Sink Table-based groupcast forwarding                | 0b0
b18          | Translation Table                                    | 0b0
b19          | GPD IEEE address                                     | 0b1
b20 � b23    | Reserved                                             | 0b0
----------------------------------------------------------------------------------
***********************************************************************************/
//                                   b0-b7       b8-b15      b16-23
//                                 0b00101111  0b10101100  0b00001001
/// gpsFunctionality attribute value
#define GPS_FUNCTIONALITY          {   0x2F,      0xAC,       0x09  }  // Table 42 – Format of the gppFunctionality attribute

/*** The optional GP functionality supported by this proxy that
     is active ***/
/// gpsActiveFunctionality attribute value
#define GPS_ACTIVE_FUNCTIONALITY   {   0x2F,      0xAC,       0x09  }
/** @} End ZIGBEE_PRO_GPS_SPEC */


/**
 * @defgroup GPP_DEFAULTS GP Proxy Defaults
 * @{
 * @brief Bit masks showing which GP functions supported
 */
/// Maximum number of Proxy Table entries supported by this device
#define GPP_MAX_PROXY_TABLE_ENTRIES        0x05

/// Number of full unicast GP Notification retries on lack of GP
/// otification Response
#define GPP_NOTIFICATION_RETRY_NUMBER      0x02

/// Time in ms between full unicast GP Notification retries on
///  lack of GP Notification Response
#define GPP_NOTIFICATION_RETRY_TIMER       0x64

/// The frequency of sink rediscovery for inactive Proxy
/// Table entries
#define GPP_MAX_SEARCH_COUNTER             0x0A

/// A list holding information about blocked GPD IDs
#define GPP_BLOCKED_GPD_ID                 0x00
/** @} End GPP_DEFAULTS */

/**
 * @defgroup ZIGBEE_PRO_GPP_SPEC Zigbee PRO Green Power Proxy specification
 * @{
 * @brief Bit masks showing which GPP functions supported
 */
/**********************************************************************************
ZigBee PRO Green Power feature specification
Basic functionality set
Version 1.0

Table 42 � Format of the gppFunctionality attribute
----------------------------------------------------------------------------------
Indication   | Functionality                                        | Basic Proxy
-------------|------------------------------------------------------|-------------
b0           | GP feature                                           | 0b1
b1           | Direct communication (reception of GPDF via GP stub) | 0b1
b2           | Derived groupcast communication                      | 0b1
b3           | Pre-commissioned groupcast communication             | 0b1
b4           | Full unicast communication                           | 0b0
b5           | Lightweight unicast communication                    | 0b1
b6           | Reserved                                             | 0b0
b7           | Bidirectional operation                              | 0b0
b8           | Proxy Table maintenance (active and passive,         | 0b0
             | for GPD mobility and GPP robustness)                 |
b9           | Reserved                                             | 0b0
b10          | GP commissioning                                     | 0b1
b11          | CT-based commissioning                               | 0b1
b12          | Maintenance of GPD (deliver channel/key              | 0b0
             | during operation)                                    |
b13          | gpdSecurityLevel  = 0b00                             | 0b1
b14          | Deprecated: gpdSe curityLevel = 0b01                 | 0b0
b15          | gpdSecurityLevel  = 0b10                             | 0b1
b16          | gpdSecurityLevel  = 0b11                             | 0b1
b17          | Reserved                                             | 0b0
b18          | Reserved                                             | 0b0
b19          | GPD IEEE address                                     | 0b1
b20 � b23    | Reserved                                             | 0b0
----------------------------------------------------------------------------------
***********************************************************************************/
//                                   b0-b7       b8-b15      b16-23
//                                 0b00101111  0b10101100  0b00001001
/// gppFunctionality attribute value
#define GPP_FUNCTIONALITY          {   0x2F,      0xAC,       0x09  }  // Table 42 – Format of the gppFunctionality attribute

/*** The optional GP functionality supported by this proxy that
     is active ***/
/// gppActiveFunctionality attribute value
#define GPP_ACTIVE_FUNCTIONALITY   {   0x2F,      0xAC,       0x09  }
/** @} End ZIGBEE_PRO_GPP_SPEC */

/**
 * @defgroup OTHER_GP_CONSTANTS Other GP Constants
 * @brief Other relevant GP constants
 * @{
 */
/// gpSharedSecurityKeyType attribute value
#define GP_SHARED_SEC_KEY_TYPE             0x02
/// gpLinkKey attribute value
#define GP_LINK_LEY                        DEFAULT_TC_LINK_KEY
/// ClusterRevision attribute value
#define GP_CLUSTER_REVISION                0x0002
/** @} End OTHER_GP_CONSTANTS */


/**
 * @defgroup GP_CLUSTER_CMDS Green Power Cluster Commands
 * @{
 * @brief Green Power Cluster Commands
 */
/// From proxy to sink to tunnel GP frame.
#define COMMAND_GP_NOTIFICATION                           0x00
/// From proxy to the sinks in entire network to get pairing indication related
/// to GPD for Proxy Table update
#define COMMAND_GP_PAIRING_SEARCH                         0x01
/// From proxy to neighbor proxies to indicate GP Notification sent in full unicast mode.
#define COMMAND_GP_TUNNELING_STOP                         0x03
/// From proxy to sink to tunnel GPD commissioning data.
#define COMMAND_GP_COMMISSIONING_NOTIFICATION             0x04
/// To enable commissioning mode of the sink, over the air
#define COMMAND_GP_COMMISSIONING_MODE                     0x05
/// To configure GPD Command Translation Table
#define COMMAND_GP_TRANSLATION_TABLE_UPDATE_COMMAND       0x07
/// To provide GPD Command Translation Table content
#define COMMAND_GP_TRANSLATION_TABLE_REQUEST              0x08
/// To configure Sink Table
#define COMMAND_GP_PAIRING_CONFIGURATION                  0x09
/// To read out selected Sink Table entries, by index or by GPD ID
#define COMMAND_GP_SINK_TABLE_REQUEST                     0x0A
/// To receive information on requested selected Proxy Table entries, by index or by GPD ID
#define COMMAND_GP_PROXY_TABLE_RESPONSE                   0x0B


/// From sink to a proxy to acknowledge GP Notification received in full unicast mode.
#define COMMAND_GP_NOTIFICATION_RESPONSE                  0x00
/// From sink to the entire network to (de)register for tunneling service, or for
/// removing GPD from the network
#define COMMAND_GP_PAIRING                                0x01
/// From sink to proxies in the whole network to indicate commissioning mode
#define COMMAND_GP_PROXY_COMMISSIONING_MODE               0x02
/// From sink to selected proxies, to provide data to be transmitted to Rx-capable GPD
#define COMMAND_GP_RESPONSE                               0x06
/// To provide GPD Command Translation Table content
#define COMMAND_GP_TRANSLATION_TABLE_RESPONSE             0x08
/// To send selected Sink Table entries
#define COMMAND_GP_SINK_TABLE_RESPONSE                    0x0A
/// To requested selected Proxy Table entries
#define COMMAND_GP_PROXY_TABLE_REQUEST                    0x0B
/** @} End GP_CLUSTER_CMDS */

/*********************************************************************
 * TYPEDEFS
 */

/**
 * @defgroup GP_STACK_MSGS Green Power Stack Messages
 * @{
 * @brief gp stack messages
 */
/*** Format for gp stack messages ***/
typedef struct gpNotificationMsg
{
  afAddrType_t                 addr;     //!< Destination address
  uint16_t                       nwkAlias; //!< For APS header addressing when required
  uint8_t                        secNum;   //!< Sequence Number
  uint8_t                        radius;   //!< Message radius
  uint8_t*                       pMsg;     //!< Pointer to the message
  struct gpNotificationMsg*    pNext;    //!< Pointer to next message
}gpNotificationMsg_t;

typedef struct gpCmdPayloadMsg
{
  uint8_t                        secNum; //!< Sequence Number
  uint8_t                        lenght; //!< Message lenght
  uint8_t*                       pMsg;   //!< Pointer to the message
  struct gpCmdPayloadMsg*      pNext;  //!< Pointer to next message
}gpCmdPayloadMsg_t;

/** @} End GP_STACK_MSGS */

/**
 * @defgroup ATTRIBUTE_FORMATS Attribute formats
 * @{
 * @brief Attribute formats
 */
/*** Format of entries in the Proxy Table attribute ***/
typedef struct
{
  uint16_t  options;                     //!< This parameter specifies the tunneling
                                       //!< options.
  uint32_t  gpdId;                       //!< ID of the GPD.
  uint8_t   endPoint;                    //!< GPD endpoint.
  uint16_t  gpdAssignedAlias;            //!< The commissioned 16-bit ID to
                                       //!< be used as alias for this GPD.
  uint8_t   securityOptions;             //!< The security options.
  uint32_t  gpdSecurityFramecounter;     //!< The incoming security frame
                                       //!< counter for the GPD.
  uint8_t   gpdKey[SEC_KEY_LEN];         //!< The security key for the GPD.
  uint8_t  *pLightweightSinkAddressList; //!< IEEE and short address of the sink.
  uint8_t  *pSinkGroupList;              //!< GroupIDs and Aliases for the sinks
                                       //!< that require the tunneling in
                                       //!< groupcast communication mode.
  uint8_t   groupcastRadius;             //!< To limit the range of the group-cast.
  uint8_t   SearchCounter;               //!< For inactive/invalid entries,
                                       //!< allows for Sink re-discovery
                                       //!< when Search Counter equals 0.
  uint16_t  ExtendedOptions;             //!< This parameter specifies
                                       //!< exten-sions to the tunneling options.
  uint8_t  *pFullUnicastSinkAddressList; //!< IEEE and short address of the sink.
} ProxyTableEntryFormat_t;

/** @} End ATTRIBUTE_FORMATS */

/**
 * @defgroup GP_ZCL_CALLBACK_STRUCTS GP callback structs
 * @{
 * @brief Structs used for GP callbacks
 */

typedef struct
{
  afAddrType_t  *srcAddr; //!< requestor's address
  uint8_t          actions; //!< Pairing actions
  uint16_t         options; //!< Pairing Options
  uint8_t         *pData;   //!< Command data
} zclGpPairingConfig_t;

/*** GP Notification ***/
typedef struct
{
  gpdID_t  gpdId;                //!< Address identifier structure for Green Power Device.
                                 //!< Power device that generates the notification
  uint8_t    GPDEndpoint;          //!< Endpoint of the GP if application ID is 0b010.
  uint32_t   securityFrameCounter; //!< Security frame counter from Green Power.
                                 //!< Device frame.
  uint8_t    gpCmdId;              //!< GreenPower Command Id.
  uint8_t    DataLen;              //!< Command payload length.
  uint8_t   *pData;                //!< Command payload.
  uint16_t   GPPAddress;           //!< Address of the GPP that generates the notification.
  uint8_t    GPPLink;              //!< Link quality of between the GPD and GPP.
} zclGpNotification_t;

/*** GP Commissioning Notification ***/
typedef struct
{
  gpCommissioningNotificationOptions_t  options;              //!< Commissioning notification options bitfield.
  gpdID_t                               gpdId;                //!< Address identifier structure for Green Power Device.
  uint8_t                                 GPDEndpoint;          //!< Endpoint of the GP if application ID is 0b010.
  uint32_t                                securityFrameCounter; //!< Security frame counter from Green Power Device frame.
  uint8_t                                 gpCmdId;              //!< GreenPower Command Id.
  uint8_t                                 DataLen;              //!< Command payload length.
  uint8_t                                *pData;                //!< Command payload.
  uint16_t                                GPPAddress;           //!< Address of the GPP that generates the Notification.
  uint8_t                                 GPPLink;              //!< Link quality of between the GPD and GPP.
  uint32_t                                MIC;                  //!< MIC value for security failed frames.
} zclGpCommissioningNotification_t;

/*** GP Pairing ***/
typedef struct
{
  afAddrType_t  *srcAddr;       //!< Local device address.
  uint8_t       wasBroadcast;   //! TRUE if network destination was a broadcast address.
  uint8_t          options[3];    //!< Pairing options bitfield.
  uint8_t         *pData;         //!< Pairing command payload.
} zclGpPairing_t;

/*** GP Proxy Commissioning Mode ***/
typedef struct
{
  uint16_t  srcAddr;   //!< Local device address.
  uint8_t   options;   //!< Commissioning mode options.
  uint8_t  *pData;     //!< Commissioning mode data.
} zclGpProxyCommissioningMode_t;

/*** GP Response ***/
typedef struct
{
  uint16_t  dstAddr;               //!< Remote device address.
  uint8_t   options;               //!< Green Power Response command options.
  uint16_t  tempMasterShortAddr;   //!< Temp Master address.
  uint8_t   tempMasterTxChannel;   //!< Temp Master channel.
  uint8_t  *pData;                 //!< Response data.
} zclGpResponse_t;

/*** GP Response payload***/
typedef struct
{
  gpdID_t gpdId;                 //!< Address identifier structure for Green Power Device.
  uint8_t   GPDEndpoint;           //!< Endpoint of the GP if application ID is 0b010.
  uint8_t   options;               //!< Green Power Response command options.
  uint8_t   gpCmdId;               //!< GreenPower Command Id.
  uint8_t  *pData;                 //!< Response data.
} zclGpResponsePayload_t;


/*** GP Table Request ***/
typedef struct
{
  afAddrType_t *srcAddr;    //!< Requestor's address.
  uint8_t        options;     //!< Options field.
  uint8_t        *pData;      //!< Request data.
} zclGpTableRequest_t;

/*** GP Table Response ***/
typedef struct
{
  uint8_t  status;            //!< Response status.
  uint8_t  tableEntriesTotal; //!< Total number of entries.
  uint8_t  startIndex;        //!< Index of the first entry on the response.
  uint8_t  entriesCount;      //!< Number of entries in the response.
  uint8_t *entry;             //!< List of entries.
} zclGpTableResponse_t;
/** @} End GP_ZCL_CALLBACK_STRUCTS */

/**
 * @defgroup GP_COMMANDS GP commands
 * @{
 * @brief GP commands
 */
/*** GP Pairing Configuration Command ***/
typedef struct
{
  uint8_t  actions;                     //!< Actions field of the GP Pairing
                                      //!< Configuration command.
  uint16_t options;                     //!< Options parameter of the GP Pairing
                                      //!< Configuration command.
  uint32_t gpdId;                       //!< ID of the GPD.
  uint8_t  gpdIEEE[Z_EXTADDR_LEN];      //!< IEEE of the GPD if application ID is
                                      //!< 0b010.
  uint8_t  ep;                          //!< Endpoint of the GP if application ID
                                      //!< is 0b010.
  uint8_t  deviceId;                    //!< The DeviceID for GPD.
  uint8_t  groupEntriesCount;           //!< Number of groups.
  uint8_t *pGroupList;                  //!< The 16-bit GroupID and alias for the
                                      //!< group communication.
  uint16_t gpdAssignedAlias;            //!< The commissioned alias for this GPD.
  uint8_t  forwardingRadius;            //!< To limit the range of the groupcast.
  uint8_t  securityOptions;             //!< The security options.
  uint32_t gpdSecurityFrameCounter;     //!< The incoming security frame counter.
  uint8_t  gpdSecurityKey[SEC_KEY_LEN]; //!< The security key for the GPD.
  uint8_t  numOfPairedEndpoints;        //!< Indicates the number of endpoints.
  uint8_t *pPairedEndpoints;            //!< List of paired endpoints.
  uint8_t  appInfo;                     //!< Application information bitfield.
  uint16_t manufacturerId;              //!< ManufacturerID value.
  uint16_t modeId;                      //!< ModelID value.
  uint8_t  numberOfGpdCmd;              //!< Number of GP commands defines the number
                                      //!< of items in the GP command list field.
  uint8_t  pGpdCmdList;                 //!< Commmand list.
  uint8_t *pClusterList;                //!< Cluster list.
} gpPairingConfigCmd_t;

/*** GP Pairing Command ***/
typedef struct
{
  uint32_t options;                  //!< Options field of the GP Pairing command.
  uint32_t gpdId;                    //!< ID of the GPD.
  uint8_t  gpdIEEE[Z_EXTADDR_LEN];   //!< IEEE of the GPD if application ID is
                                   //!< 0b010.
  uint8_t  ep;                       //!< Endpoint of the GP if application ID
                                   //!< is 0b010.
  uint8_t  sinkIEEE[Z_EXTADDR_LEN];  //!< Sink IEEE address value.
  uint16_t sinkNwkAddr;              //!< Sink Network address value.
  uint16_t sinkGroupID;              //!< Sink group ID value.
  uint8_t  deviceId;                 //!< The DeviceID for GPD.
  uint32_t gpdSecCounter;            //!< The security frame counter.
  uint8_t  gpdKey[SEC_KEY_LEN];      //!< The security key for the GPD.
  uint16_t assignedAlias;            //!< The commissioned alias for this GPD.
  uint8_t  forwardingRadius;         //!< To limit the range of the groupcast.
} gpPairingCmd_t;

/*** GP Table Request Command ***/
typedef struct
{
  uint8_t  options;                 //!< Options for table request
  uint32_t gpdId;                   //!< ID of the GPD.
  uint8_t  gpdIEEE[Z_EXTADDR_LEN];  //!< IEEE of the GPD if application ID is
                                  //!< 0b010.
  uint8_t  ep;                      //!< Endpoint of the GP if application ID
                                  //!< is 0b010.
  uint8_t  index;                   //!< index value of the Proxy Table entry being
                                  //!< requested.
} gpTableReqCmd_t;

/*** GP Notification Command ***/
typedef struct
{
  uint16_t options;                //!< Options field of the GP Notification command.
  uint32_t gpdId;                  //!< ID of the GPD.
  uint8_t  gpdIEEE[Z_EXTADDR_LEN]; //!< IEEE of the GPD if application ID is 0b010.
  uint8_t  ep;                     //!< Endpoint of the GP if application ID is 0b010.
  uint32_t gpdSecCounter;          //!< The security frame counter.
  uint8_t  cmdId;                  //!< Command ID.
  uint8_t  payloadLen;             //!< Command payload length.
  uint8_t  *cmdPayload;            //!< Command payolad.
  uint16_t gppShortAddr;           //!< Address of the GPP that generates the Notification.
  uint8_t  gppGpdLink;             //!< Link quality of between the GPD and GPP.
} gpNotificationCmd_t;

/*** GP Commissioning Notification Command ***/
typedef struct
{
  uint16_t options;                //!< Options field of the GP Commissioning
                                 //!< Notification command.
  uint32_t gpdId;                  //!< ID of the GPD.
  uint8_t  gpdIEEE[Z_EXTADDR_LEN]; //!< IEEE of the GPD if application ID is 0b010.
  uint8_t  ep;                     //!< Endpoint of the GP if application ID is 0b010.
  uint32_t gpdSecCounter;          //!< The security frame counter.
  uint8_t  cmdId;                  //!< Command ID.
  uint8_t  payloadLen;             //!< Command payload length.
  uint8_t  *cmdPayload;            //!< Command payolad.
  uint16_t gppShortAddr;           //!< Address of the GPP that generates the
                                 //!< Notification.
  uint8_t  gppGpdLink;             //!< Link quality of between the GPD and GPP.
  uint32_t mic;                    //!< Security MIC.
} gpCommissioningNotificationCmd_t;

typedef struct
{
    afAddrType_t addr;           //!< APS addressing for ZCL commissioning notification
    uint16_t aliasNwk;             //!< Alias for network address
    uint8_t seqNum;                //!< Sequence number for ZCL frame
    uint8_t len;                   //!< Length of buffer to be sent
    uint8_t *pbuf;                 //!< Pointer to buffer that holds the message
} gpCommissioningNotificationMsg_t;

/*** GP Response Command ***/
typedef struct
{
  uint8_t  options;                //!< Options field of the GP Response command.
  uint16_t tempMasterShortAddr;    //!< indicates the address of the proxy which
                                 //!< will transmit the response GPDF to the GPD.
  uint8_t  tempMasterChannel;      //!< indicates the channel the Response GPDF will be sent on.
  uint32_t gpdSrcId;               //!< ID of the GPD.
  uint8_t  gpdIEEE[Z_EXTADDR_LEN]; //!< IEEE of the GPD if application ID is 0b010.
  uint8_t  ep;                     //!< Endpoint of the GP if application ID is 0b010.
  uint8_t  cmdId;                  //!< Command ID.
  uint8_t  payloadLen;             //!< Command payload length.
  uint8_t  *cmdPayload;            //!< Command payolad.
} gpResponseCmd_t;

/** @} End GP_COMMANDS */

/**
 * @defgroup COMMAND_OPTIONS Options for commands
 * @{
 * @brief Options for commands
 */
/*** GPD Commissioning Command ***/
typedef struct _gpdCommissioningCmd_t
{
  uint8_t deviceId;                                    //!< The DeviceID for GPD.
  gpdCommissioningOptions_t options;                 //!< Options field.
  gpdCommissioningExtOptions_t extOptions;           //!< Extended options field.
  uint8_t  gpdKey[SEC_KEY_LEN];                        //!< The security key for the GPD.
  uint32_t gpdKeyMic;                                  //!< Key MIC.
  uint32_t gpdOutCounter;                              //!< Security frame counter.
  gpdCommissioningAppInfo_t appInfo;                 //!< Application information
  uint16_t manufacturerId;                             //!< ManufacturerID value.
  uint16_t modeId;                                     //!< ModelID value.
  uint8_t numberOfGpdCommands;                         //!< Number of GP commands
                                                     //!< defines the number of
                                                     //!< items in the GP command
                                                     //!< list field.
  uint8_t *pGpdCommandList;                            //!< Commmand list.
  gpdCommissioningClusterLength_t clusterListLength; //!< Cluster list lenght.
  uint16_t *pServerList;                               //!< Server list.
  uint16_t *pClientList;                               //!< Client list.
} gpdCommissioningCmd_t;

/** @} End COMMAND_OPTIONS */

#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
/// From CT to Sink to (de)register for tunneling service or to remove GPD from the network
typedef void (*zclGCB_GP_PairingConfig_t)( zclGpPairingConfig_t *pCmd );

/// From Proxy to sink device to process GPD command frame
typedef void (*zclGCB_GP_Notification_t)( zclGpNotification_t *pCmd );

/// From Proxy to sink device to process GPD command frame during commissioning
typedef void (*zclGCB_GP_CommissioningNotification_t)( zclGpCommissioningNotification_t *pCmd );
#endif

/// From sink to proxies to (de)register for tunneling service or to remove GPD from the network
typedef void (*zclGCB_GP_Pairing_t)( zclGpPairing_t *pCmd );

/// From sink to selected proxies, to provide data to be transmitted to Rx-capable GPD
typedef void (*zclGCB_GP_Response_t)( zclGpResponse_t *pCmd );

/// From sink to proxies in the whole network to indicate commissioning mode
typedef void (*zclGCB_GP_Proxy_Commissioning_Mode_t)( zclGpProxyCommissioningMode_t *pCmd );

/// To request selected Proxy Table entries, by index or by GPD ID
typedef void (*zclGCB_GP_Proxy_Table_Request_t)( zclGpTableRequest_t *pRsp );

/**
 * @brief Register Callbacks table entry - enter function pointers for callbacks
 * that the green power application would like to receive
 */
typedef struct
{
#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
/// Green Power Pairing Configuration command callback
  zclGCB_GP_PairingConfig_t             pfnGpPairingConfigCmd;
/// Green Power Notiffication command callback
  zclGCB_GP_Notification_t              pfnGpNotificationCmd;
/// Green Power Commissioning Notiffication command callback
  zclGCB_GP_CommissioningNotification_t pfnGpCommissioningNotificationCmd;
#endif
/// Green Power Pairing command callback
  zclGCB_GP_Pairing_t                   pfnGpPairingCmd;
/// Green Power Proxy Table Request command callback
  zclGCB_GP_Proxy_Table_Request_t       pfnGpProxyTableReqCmd;
/// Green Power Commissioning Mode command callback
  zclGCB_GP_Proxy_Commissioning_Mode_t  pfnGpProxyCommissioningModeCmd;
/// Green Power Response command callback
  zclGCB_GP_Response_t                  pfnGpResponseCommand;
} zclGp_AppCallbacks_t;


/// From GPD to sink to process identify command
typedef void (*GPDFCB_GP_identify_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process recall scene command
typedef void (*GPDFCB_GP_RecallScene_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Store scene command
typedef void (*GPDFCB_GP_StoreScene_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Off command
typedef void (*GPDFCB_GP_Off_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process On command
typedef void (*GPDFCB_GP_On_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Toggle command
typedef void (*GPDFCB_GP_Toggle_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Level Control Stop command
typedef void (*GPDFCB_GP_LevelControlStop_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Level control move up command
typedef void (*GPDFCB_GP_MoveUp_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Level control move down command
typedef void (*GPDFCB_GP_MoveDown_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Level control step up command
typedef void (*GPDFCB_GP_StepUp_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Level control move down command
typedef void (*GPDFCB_GP_StepDown_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Level control move up with on/off command
typedef void (*GPDFCB_GP_MoveUpWithOnOff_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Level control move down with on/off command
typedef void (*GPDFCB_GP_MoveDownWithOnOff_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Level control step up with on/off command
typedef void (*GPDFCB_GP_StepUpWithOnOff_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Level control step down with on/off command
typedef void (*GPDFCB_GP_StepDownWithOnOff_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Color Control Hue Stop command
typedef void (*GPDFCB_GP_MoveHueStop_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Color Control Move hue up command
typedef void (*GPDFCB_GP_MoveHueUp_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Color Control Move hue down command
typedef void (*GPDFCB_GP_MoveHueDown_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Color Control step hue up command
typedef void (*GPDFCB_GP_StepHueUp_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Color Control step hue down command
typedef void (*GPDFCB_GP_StepHueDown_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Color Control Move Saturation stop command
typedef void (*GPDFCB_GP_MoveSaturationStop_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Color Control Move Saturation up command
typedef void (*GPDFCB_GP_MoveSaturationUp_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Color Control Move Saturation down  command
typedef void (*GPDFCB_GP_MoveSaturationDown_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Color Control Step Saturation up command
typedef void (*GPDFCB_GP_StepSaturationUp_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Color Control Step Saturation down command
typedef void (*GPDFCB_GP_StepSaturationDown_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Color Control Move Color command
typedef void (*GPDFCB_GP_MoveColor_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Color Control Step Color command
typedef void (*GPDFCB_GP_StepColor_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process DoorLock Lock command
typedef void (*GPDFCB_GP_LockDoor_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process DoorLock Unlock command
typedef void (*GPDFCB_GP_UnlockDoor_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Attribute Reporting command
typedef void (*GPDFCB_GP_AttributeReporting_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Manufacturer Specific Reporting command
typedef void (*GPDFCB_GP_MfrSpecificReporting_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process MultiCluster Reporting command
typedef void (*GPDFCB_GP_MultiClusterReporting_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Manufacturer Specific MultiCluster Reporting command
typedef void (*GPDFCB_GP_MfrSpecificMultiReporting_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Request Attributes command
typedef void (*GPDFCB_GP_RequestAttributes_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process Read Attribute Response command
typedef void (*GPDFCB_GP_ReadAttributeRsp_t)( zclGpNotification_t *zclGpNotification );

/// From GPD to sink to process ZCL Tunneling command
typedef void (*GPDFCB_GP_zclTunneling_t)( zclGpNotification_t *zclGpNotification );

/// To process Commissioning Mode command
typedef void (*zclGPCB_CommissioningMode_t)(bool isEntering, bool hasTime, uint16_t time);

/**
 * @brief Register Callbacks table entry - enter function pointers for callbacks
 * that the application would like to receive when processing sink functionality.
 */
/// callbacks for GPDF commands handlers
typedef struct
{
#ifdef ZCL_IDENTIFY
  GPDFCB_GP_identify_t                  pfnGpdfIdentifyCmd;
#endif
#ifdef ZCL_SCENES
  GPDFCB_GP_RecallScene_t               pfnGpdfRecallSceneCmd;
  GPDFCB_GP_StoreScene_t                pfnGpdfStoreSceneCmd;
#endif
#ifdef ZCL_ON_OFF
  GPDFCB_GP_Off_t                       pfnGpdfOffCmd;
  GPDFCB_GP_On_t                        pfnGpdfOnCmd;
  GPDFCB_GP_Toggle_t                    pfnGpdfToggleCmd;
#endif
#ifdef ZCL_LEVEL_CTRL
  GPDFCB_GP_LevelControlStop_t          pfnGpdfLevelControlStopCmd;
  GPDFCB_GP_MoveUp_t                    pfnGpdfMoveUpCmd;
  GPDFCB_GP_MoveDown_t                  pfnGpdfMoveDownCmd;
  GPDFCB_GP_StepUp_t                    pfnGpdfStepUpCmd;
  GPDFCB_GP_StepDown_t                  pfnGpdfStepDownCmd;
  GPDFCB_GP_MoveUpWithOnOff_t           pfnGpdfMoveUpWithOnOffCmd;
  GPDFCB_GP_MoveDownWithOnOff_t         pfnGpdfMoveDownWithOnOffCmd;
  GPDFCB_GP_StepUpWithOnOff_t           pfnGpdfStepUpWithOnOffCmd;
  GPDFCB_GP_StepDownWithOnOff_t         pfnGpdfStepDownWithOnOffCmd;
#endif
  GPDFCB_GP_MoveHueStop_t               pfnGpdfMoveHueStopCmd;
  GPDFCB_GP_MoveHueUp_t                 pfnGpdfMoveHueUpCmd;
  GPDFCB_GP_MoveHueDown_t               pfnGpdfMoveHueDownCmd;
  GPDFCB_GP_StepHueUp_t                 pfnGpdfStepHueUpCmd;
  GPDFCB_GP_StepHueDown_t               pfnGpdfStepHueDownCmd;
  GPDFCB_GP_MoveSaturationStop_t        pfnGpdfMoveSaturationStopCmd;
  GPDFCB_GP_MoveSaturationUp_t          pfnGpdfMoveSaturationUpCmd;
  GPDFCB_GP_MoveSaturationDown_t        pfnGpdfMoveSaturationDownCmd;
  GPDFCB_GP_StepSaturationUp_t          pfnGpdfStepSaturationUpCmd;
  GPDFCB_GP_StepSaturationDown_t        pfnGpdfStepSaturationDownCmd;
  GPDFCB_GP_MoveColor_t                 pfnGpdfMoveColorCmd;
  GPDFCB_GP_StepColor_t                 pfnGpdfStepColorCmd;
#ifdef ZCL_DOORLOCK
  GPDFCB_GP_LockDoor_t                  pfnGpdfLockDoorCmd;
  GPDFCB_GP_UnlockDoor_t                pfnGpdfUnlockDoorCmd;
#endif
  GPDFCB_GP_AttributeReporting_t        pfnGpdfAttributeReportingCmd;
  GPDFCB_GP_MfrSpecificReporting_t      pfnGpdfMfrSpecificReportingCmd;
  GPDFCB_GP_MultiClusterReporting_t     pfnGpdfMultiClusterReportingCmd;
  GPDFCB_GP_MfrSpecificMultiReporting_t pfnGpdfMfrSpecificMultiReportingCmd;
  GPDFCB_GP_RequestAttributes_t         pfnGpdfRequestAttributesCmd;
  GPDFCB_GP_ReadAttributeRsp_t          pfnGpdfReadAttributeRspCmd;
  GPDFCB_GP_zclTunneling_t              pfnGpdfzclTunnelingCmd;
} GpSink_AppCallbacks_t;

/*********************************************************************
 * FUNCTION MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      zclGp_RegisterCmdCallbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate. Otherwise, returns ZSuccess.
 */
extern ZStatus_t zclGp_RegisterCmdCallbacks( uint8_t endpoint, zclGp_AppCallbacks_t *callbacks );

#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
/*********************************************************************
 * @fn      zclGp_SendGpSinkTableResponse
 *
 * @param   dstAddr - where to send the request
 * @param   groupID - pointer to the group structure
 * @param   groupName - pointer to Group Name.  This is a Zigbee
 *          string data type, so the first byte is the length of the
 *          name (in bytes), then the name.
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclGp_SendGpSinkTableResponse( afAddrType_t *dstAddr, zclGpTableResponse_t *rsp,
                                          uint8_t seqNum );

/*********************************************************************
 * @fn      zclGp_SendGpPairing
 *
 * @param   dstAddr - The destination address
 * @param   pEntry - Pointer to Sink Table entry
 * @param   actions - Pairing Actions
 * @param   secFrameCnt - Security Frame Counter
 * @param   seqNum - The identification number for the transaction
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclGp_SendGpPairing(uint8_t* pEntry, uint8_t actions,
                                     uint32_t secFrameCnt, uint8_t seqNum);

/*********************************************************************
 * @fn      zclGp_SendGpProxyCommissioningModeCommand
 *
 * @param   pAddr - Remote device address
 * @param   pCmd - Pointer to GP Proxy Commissioning Mode Struct
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclGp_SendGpProxyCommissioningModeCommand( afAddrType_t *pAddr,
                                                    gpProxyCommissioningModeCmd_t *pCmd );
/*********************************************************************
 * @fn      zclGp_SendGpResponseCommand
 *
 * @param   pAddr - Remote device address
 * @param   pCmd - Pointer to GP Response Struct
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclGp_SendGpResponseCommand( afAddrType_t *pAddr, gpResponseCmd_t *pCmd );

#endif

/*********************************************************************
 * @fn      zclGp_SendGpProxyTableResponse
 *
 * @param   dstAddr - where to send the request
 * @param   groupID - pointer to the group structure
 * @param   groupName - pointer to Group Name.  This is a Zigbee
 *          string data type, so the first byte is the length of the
 *          name (in bytes), then the name.
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclGp_SendGpProxyTableResponse( afAddrType_t *dstAddr, zclGpTableResponse_t *rsp,
                                          uint8_t seqNum );

/*********************************************************************
 * @fn      zclGp_SendGpNotificationCommand
 *
 * @param   pCmd - Pointer to GP Notification Struct
 *          secNum - Sequence number given by the GPDF
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclGp_SendGpNotificationCommand( gpNotificationCmd_t *cmd, uint8_t secNum );

/*********************************************************************
 * @fn      zclGp_SendGpCommissioningNotificationCommand
 *
 * @param   pCmd - Pointer to GP Commissioning Notification Struct
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclGp_SendGpCommissioningNotificationCommand( gpCommissioningNotificationCmd_t *pCmd, uint8_t secNumber, gpdID_t* pGpdID, uint8_t* entry );

/*********************************************************************
 * @fn      gp_CreateNotificationMsgList
 *
 * @param   pHead - pointer to a pointer of the list head
 *
 * @return  none
 */
void gp_CreateNotificationMsgList( gpNotificationMsg_t **pHead );

/*********************************************************************
 * @fn      gp_CreateCmdPayloadMsgList
 *
 * @param   pHead - pointer to a pointer of the list head
 *
 * @return  none
 */
void gp_CreateCmdPayloadMsgList( gpCmdPayloadMsg_t **pHead );

/*********************************************************************
 * @fn      gp_AddNotificationMsgNode
 *
 * @param   pHead - pointer to a pointer of the list head
 *
 * @return  pointer to new node
 */
gpNotificationMsg_t* gp_AddNotificationMsgNode( gpNotificationMsg_t **pHead, gpCmdPayloadMsg_t *pMsg );

/*********************************************************************
 * @fn      gp_AddCmdPayloadMsgNode
 *
 * @param   pHead - pointer to a pointer of the list head
 *
 * @return  pointer to new node
 */
gpCmdPayloadMsg_t* gp_AddCmdPayloadMsgNode( gpCmdPayloadMsg_t **pHead, uint8_t* pBuf, uint8_t len );

/*********************************************************************
 * @fn      gp_GetHeadNotificationMsg
 *
 * @param   none
 *
 * @return  pointer to head
 */
gpNotificationMsg_t* gp_GetHeadNotificationMsg(void);

/*********************************************************************
 * @fn      gp_GetPHeadNotification
 *
 * @param   none
 *
 * @return  pointer to head
 */
gpNotificationMsg_t** gp_GetPHeadNotification(void);

/*********************************************************************
 * @fn      gp_GetHeadCmdPayloadMsg
 *
 * @param   none
 *
 * @return  pointer to head
 */
gpCmdPayloadMsg_t* gp_GetHeadCmdPayloadMsg(void);

/*********************************************************************
 * @fn      gp_GetHeadCmdPayloadMsg
 *
 * @param   none
 *
 * @return  pointer to head
 */
gpCmdPayloadMsg_t** gp_GetPHeadCmdPayload(void);

/*********************************************************************
 * @fn      gp_NotificationMsgClean
 *
 * @param   pHead - begin of the Notification Msg list
 *
 * @return  status
 */
void gp_NotificationMsgClean( gpNotificationMsg_t **pHead );

/*********************************************************************
 * @fn      gp_CmdPayloadMsgClean
 *
 * @param   pHead - begin of the Notification Msg list
 *
 * @return
 */
void gp_CmdPayloadMsgClean( gpCmdPayloadMsg_t **pHead );

 /*********************************************************************
 * @fn          gp_getProxyTableByIndex
 *
 * @param       nvIndex - NV Id of proxy table
 *              pEntry  - pointer to PROXY_TBL_LEN array
 *
 * @return
 */
uint8_t gp_getProxyTableByIndex( uint16_t nvIndex, uint8_t *pEntry );

/*********************************************************************
* @fn          gp_getSinkTableByIndex
*
* @param       nvIndex - NV Id of sink table
*              pEntry  - pointer to SINK_TBL_LEN array
*
* @return
*/
uint8_t gp_getSinkTableByIndex( uint16_t nvIndex, uint8_t *pEntry );

 /*********************************************************************
 * @fn          gp_getProxyTableByGpId
 *
 * @param       gpd_ID  - address to look for in the table
 *              pEntry  - buffer in which the entry of the table will be copied
 *
 * @return
 */
uint8_t gp_getProxyTableByGpId(gpdID_t *gpd_ID, uint8_t *pEntry, uint16_t* NvProxyTableIndex);

/*********************************************************************
* @fn          gp_getSinkTableByGpId
*
* @param       gpd_ID  - address to look for in the table
*              pEntry  - buffer in which the entry of the table will be copied
*
* @return
*/
uint8_t gp_getSinkTableByGpId(gpdID_t *gpd_ID, uint8_t *pEntry,uint16_t *NvSinkTableIndex);
/*********************************************************************
 * @fn      gp_ResetProxyTblEntry
 *
 * @param   entry - proxy table entry to be removed
 *
 * @return  none
 */
void gp_ResetProxyTblEntry( uint8_t* entry );

/*********************************************************************
 * @fn      gp_ResetSinkTblEntry
 *
 * @param   entry - sink table entry to be removed
 *
 * @return  none
 */
void gp_ResetSinkTblEntry( uint8_t* entry );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE) */
#endif /* ZCL_GREEN_POWER_H */

/** @} End ZCL_GREEN_POWER */
