/******************************************************************************

 @file api_mac.h

 @brief TI-15.4 Stack API

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
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

 ******************************************************************************
 
 
 *****************************************************************************/
#ifndef API_MAC_H
#define API_MAC_H

/******************************************************************************
 Includes
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/*!
 @mainpage TI-15.4 Stack API

 The following is the application programming interface (API) for the Texas
 Instruments 802.15.4 MAC software.  This API provides an interface to the
 management and data services of the 802.15.4 stack.

 Callback Functions
 ============================
 These functions must be implemented by the application and are used to pass
 events and data from the MAC to the application.  Data accessed through
 callback function parameters (such as a pointer to data) are only valid for
 the execution of the function and should not be considered valid when the
 function returns.  These functions execute in the context of the MAC.
 The callback function implementation should avoid using critical sections
 and CPU intensive operations.  The [callback table structure]
 (@ref ApiMac_callbacks_t) should be setup application then call
 ApiMac_registerCallbacks() to register the table.

 Common Constants and Structures
 ============================
 - <b>Address Type</b> - the common address type used by the MAC is the
 [ApiMac_sAddr_t](@ref ApiMac_sAddr_t).
 - <b>Status Values</b> - the common MAC status type is [ApiMac_status_t]
 (@ref ApiMac_status_t).
 - <b>MAC Security Level</b> - The security level ([ApiMac_secLevel_t]
 (@ref ApiMac_secLevel_t)) defines the encryption and/or authentication
 methods used on the message frame.
 - <b>Key Identifier Modes</b> - The key identifier mode
 ([ApiMac_keyIdMode_t](@ref ApiMac_keyIdMode_t))
 defines how the key is determined from the key index
 - <b>Security Type</b>- MAC security structure ([ApiMac_sec_t]
 (@ref ApiMac_sec_t)).


 Initialization and Task Interfaces
 ===============================
 - ApiMac_init()
 - ApiMac_registerCallbacks()
 - ApiMac_processIncoming()

 Data Interfaces
 ===============================
 - ApiMac_mcpsDataReq()
 - ApiMac_mcpsPurgeReq()

 Management Interfaces
 ===============================
 - ApiMac_mlmeAssociateReq()
 - ApiMac_mlmeAssociateRsp()
 - ApiMac_mlmeDisassociateReq()
 - ApiMac_mlmeOrphanRsp()
 - ApiMac_mlmePollReq()
 - ApiMac_mlmeResetReq()
 - ApiMac_mlmeScanReq()
 - ApiMac_mlmeStartReq()
 - ApiMac_mlmeSyncReq()
 - ApiMac_mlmeWSAsyncReq()

 Management Attribute Interfaces
 ===============================
 The MAC attributes can be read and written to by use of the following Get and
 Set functions, which are organized by the attributes data type:
 - ApiMac_mlmeGetReqBool()
 - ApiMac_mlmeGetReqUint8()
 - ApiMac_mlmeGetReqUint16()
 - ApiMac_mlmeGetReqUint32()
 - ApiMac_mlmeGetReqArray()
 - ApiMac_mlmeGetFhReqUint8()
 - ApiMac_mlmeGetFhReqUint16()
 - ApiMac_mlmeGetFhReqUint32()
 - ApiMac_mlmeGetFhReqArray()
 - ApiMac_mlmeGetSecurityReqUint8()
 - ApiMac_mlmeGetSecurityReqUint16()
 - ApiMac_mlmeGetSecurityReqArray()
 - ApiMac_mlmeGetSecurityReqStruct()
 - ApiMac_mlmeSetReqBool()
 - ApiMac_mlmeSetReqUint8()
 - ApiMac_mlmeSetReqUint16()
 - ApiMac_mlmeSetReqUint32()
 - ApiMac_mlmeSetReqArray()
 - ApiMac_mlmeSetFhReqUint8()
 - ApiMac_mlmeSetFhReqUint16()
 - ApiMac_mlmeSetFhReqUint32()
 - ApiMac_mlmeSetFhReqArray()
 - ApiMac_mlmeSetSecurityReqUint8()
 - ApiMac_mlmeSetSecurityReqUint16()
 - ApiMac_mlmeSetSecurityReqArray()
 - ApiMac_mlmeSetSecurityReqStruct()

 Simplified Security Interfaces
 ===============================
 - ApiMac_secAddDevice()
 - ApiMac_secDeleteDevice()
 - ApiMac_secDeleteKeyAndAssocDevices()
 - ApiMac_secDeleteAllDevices()
 - ApiMac_secGetDefaultSourceKey()
 - ApiMac_secAddKeyInitFrameCounter()

 Extension Interfaces
 ===============================
 - ApiMac_randomByte()
 - ApiMac_updatePanId()
 - ApiMac_startFH()
 - ApiMac_enableFH()
 - ApiMac_parsePayloadGroupIEs()
 - ApiMac_parsePayloadSubIEs()
 - ApiMac_freeIEList()
 - ApiMac_convertCapabilityInfo()
 - ApiMac_buildMsgCapInfo()

 */

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/*! Key Length */
#define APIMAC_KEY_MAX_LEN  16

/*! IEEE Address Length */
#define APIMAC_SADDR_EXT_LEN 8

 /* Security Implementation specific defines. The values depend on memory resources.
  */
 #ifndef MAX_KEY_TABLE_ENTRIES
#ifdef FEATURE_SECURE_COMMISSIONING
/* Key entry numbers = max device number + network key */
#define MAX_KEY_TABLE_ENTRIES             MAX_DEVICE_TABLE_ENTRIES+1
#else
#define MAX_KEY_TABLE_ENTRIES             2
#endif
 #endif

 #ifndef MAX_SECURITY_LEVEL_TABLE_ENTRIES
   #define MAX_SECURITY_LEVEL_TABLE_ENTRIES  2
 #endif

 /* MAC key table related constants */
 #ifndef MAX_KEY_ID_LOOKUP_ENTRIES
   #define MAX_KEY_ID_LOOKUP_ENTRIES         1
 #endif

 #ifndef MAX_KEY_DEVICE_TABLE_ENTRIES
#ifdef FEATURE_SECURE_COMMISSIONING
#define MAX_KEY_DEVICE_TABLE_ENTRIES        1
#else
     #define MAX_KEY_DEVICE_TABLE_ENTRIES        MAX_DEVICE_TABLE_ENTRIES
#endif
 #endif

 /*Max keys/node */
 #ifndef MAX_NODE_KEY_ENTRIES
     #define MAX_NODE_KEY_ENTRIES                1
 #endif

 #ifndef MAX_KEY_USAGE_TABLE_ENTRIES
   #define MAX_KEY_USAGE_TABLE_ENTRIES       2
 #endif

/*! Maximum number of key table entries */
#define APIMAC_MAX_KEY_TABLE_ENTRIES MAX_KEY_TABLE_ENTRIES

 /*! Maximum number of keys/device */
#define API_MAX_NODE_KEY_ENTRIES   MAX_NODE_KEY_ENTRIES

/*! Key identifier field length - Implicit mode */
#define APIMAC_KEYID_IMPLICIT_LEN 0
/*! Key identifier field length - mode 1 */
#define APIMAC_KEYID_MODE1_LEN 1
/*! Key Identifier field length - mode 4 */
#define APIMAC_KEYID_MODE4_LEN 5
/*! Key Identifier field length - mode 8 */
#define APIMAC_KEYID_MODE8_LEN 9

/*! Key source maximum length in bytes */
#define APIMAC_KEY_SOURCE_MAX_LEN 8

/*! Key index length in bytes */
#define APIMAC_KEY_INDEX_LEN 1

/*! Frame counter length in bytes */
#define APIMAC_FRAME_COUNTER_LEN 4

/*! Key lookup data length in bytes - short length */
#define APIMAC_KEY_LOOKUP_SHORT_LEN 5
/*! Key lookup data length in bytes - long length */
#define APIMAC_KEY_LOOKUP_LONG_LEN 9
/*! Key lookup data length in bytes - lookup length */
#define APIMAC_MAX_KEY_LOOKUP_LEN APIMAC_KEY_LOOKUP_LONG_LEN

/*! Bytes required for MAC header in data frame */
#define APIMAC_DATA_OFFSET 24

/*! Maximum length allowed for the beacon payload */
#define APIMAC_MAX_BEACON_PAYLOAD 16

/*! Length required for MIC-32 authentication */
#define APIMAC_MIC_32_LEN 4
/*! Length required for MIC-64 authentication */
#define APIMAC_MIC_64_LEN 8
/*! Length required for MIC-128 authentication */
#define APIMAC_MIC_128_LEN 16

/*!
 MHR length for received frame
 - FCF (2) + Seq (1) + Addr Fields (20) + Security HDR (14)
 */
#define APIMAC_MHR_LEN 37

/*! Channel Page - standard-defined SUN PHY operating modes */
#define APIMAC_CHANNEL_PAGE_9 9
/*! Channel Page - MR-FSK Generic-PHY-defined PHY modes */
#define APIMAC_CHANNEL_PAGE_10 10
 /*! Channel Page - none for IEEE modes */
#define APIMAC_CHANNEL_PAGE_NONE 0

/*! Maximum number of Standard PHY descriptor entries */
#define APIMAC_STANDARD_PHY_DESCRIPTOR_ENTRIES  5
/*! Maximum number of Generic PHY descriptor entries */
#define APIMAC_GENERIC_PHY_DESCRIPTOR_ENTRIES   10

 /*! PHY IDs - 915MHz Frequency band operating mode # 1 */
 #define APIMAC_50KBPS_915MHZ_PHY_1                 1
 /*! 868MHz Frequency band operating mode #1 */
 #define APIMAC_50KBPS_868MHZ_PHY_3               3
  /*! 433MHz Frequency band operating mode #1 */
 #define APIMAC_50KBPS_433MHZ_PHY_128        128
  /*! PHY IDs - 915MHz LRM Frequency band operating mode # 1 */
 #define APIMAC_5KBPS_915MHZ_PHY_129       129
  /*! 433MHz LRM Frequency band operating mode #1 */
 #define APIMAC_5KBPS_433MHZ_PHY_130    130
  /*! 868MHz LRM Frequency band operating mode #1 */
 #define APIMAC_5KBPS_868MHZ_PHY_131     131
 /*! PHY IDs - 915MHz Frequency band operating mode # 3 */
#define APIMAC_200KBPS_915MHZ_PHY_132            132
 /*! 868MHz Frequency band operating mode #3 */
#define APIMAC_200KBPS_868MHZ_PHY_133          133
 /*! Channel Page - none for IEEE modes */
#define APIMAC_250KBPS_IEEE_PHY_0                      0

/*! PHY IDs - MRFSK Standard Phy ID start */
#define APIMAC_MRFSK_STD_PHY_ID_BEGIN APIMAC_50KBPS_915MHZ_PHY_1
/*! PHY IDs - MRFSK Standard Phy ID end */
#define APIMAC_MRFSK_STD_PHY_ID_END ( APIMAC_MRFSK_STD_PHY_ID_BEGIN + APIMAC_STANDARD_PHY_DESCRIPTOR_ENTRIES - 1 )

 /*! PHY IDs - MRFSK Generic Phy ID start */
 #define APIMAC_MRFSK_GENERIC_PHY_ID_BEGIN APIMAC_50KBPS_433MHZ_PHY_128
 /*! PHY IDs - MRFSK Generic Phy ID end */
 #define APIMAC_MRFSK_GENERIC_PHY_ID_END ( APIMAC_MRFSK_GENERIC_PHY_ID_BEGIN + APIMAC_GENERIC_PHY_DESCRIPTOR_ENTRIES - 1 )


/*! PHY descriptor table entry */
#define APIMAC_PHY_DESCRIPTOR 0x01

/*! Range Extender Mode disable high gain */
#define APIMAC_NO_EXTENDER      0x00
/*! Range Extender Mode enable high gain */
#define APIMAC_HIGH_GAIN_MODE   0x01

/*! High PA Type default PA */
#define APIMAC_DEFAULT_PA       0x00
/*! High PA Type high PA */
#define APIMAC_HIGH_PA          0x01

/*!
 Special address value - Short address value indicating extended address
 is used
 */
#define APIMAC_ADDR_USE_EXT 0xFFFE
/*! Special address value - Broadcast short address */
#define APIMAC_SHORT_ADDR_BROADCAST 0xFFFF
/*! Special address value - Short address when there is no short address */
#define APIMAC_SHORT_ADDR_NONE 0xFFFF

/*! The length of the random seed is set for maximum requirement which is 32 */
#define APIMAC_RANDOM_SEED_LEN 32

/*! Frequency Hopping UTT IE Selection Bit */
#define APIMAC_FH_UTT_IE 0x00000002
/*! Frequency Hopping BT IE Selection Bit */
#define APIMAC_FH_BT_IE 0x00000008
/*! Frequency Hopping US IE Selection Bit */
#define APIMAC_FH_US_IE 0x00010000
/*! Frequency Hopping BS IE Selection Bit */
#define APIMAC_FH_BS_IE 0x00020000

/*! Frequency hopping header IE's mask */
#define APIMAC_FH_HEADER_IE_MASK 0x000000FF

/*!
 Frequency hopping Protocol dispatch values - Protocol dispatch none
 */
#define APIMAC_FH_PROTO_DISPATCH_NONE 0x00
/*!
 Frequency hopping Protocol dispatch values - Protocol dispatch MHD-PDU
 */
#define APIMAC_FH_PROTO_DISPATCH_MHD_PDU 0x01
/*!
 Frequency hopping Protocol dispatch values - Protocol dispatch 6LOWPAN
 */
#define APIMAC_FH_PROTO_DISPATCH_6LOWPAN 0x02

/*! Maximum number of channels */
#define APIMAC_154G_MAX_NUM_CHANNEL    129

/*! Bitmap size to hold the channel list */
#define APIMAC_154G_CHANNEL_BITMAP_SIZ \
        ((APIMAC_154G_MAX_NUM_CHANNEL + 7) / 8)

/*! Maximum number of header IEs */
#define APIMAC_HEADER_IE_MAX 2
/*! Maximum number of payload-IEs */
#define APIMAC_PAYLOAD_IE_MAX 2
/*! Maximum number of sub-IEs */
#define APIMAC_PAYLOAD_SUB_IE_MAX 4

/*!
 MACRO that returns the beacon order from the superframe specification
 */
#define APIMAC_SFS_BEACON_ORDER(s) ((s) & 0x0F)

/*!
 MACRO that returns the superframe order from the superframe specification
 */
#define APIMAC_SFS_SUPERFRAME_ORDER(s) (((s) >> 4) & 0x0F)

/*!
 MACRO that returns the final CAP slot from the superframe specification
 */
#define APIMAC_SFS_FINAL_CAP_SLOT(s) (((s) >> 8) & 0x0F)

/*!
 MACRO that returns the battery life extension bit from the superframe
 specification
 */
#define APIMAC_SFS_BLE(s) (((s) >> 12) & 0x01)

/*!
 MACRO that returns the PAN coordinator bit from the superframe specification
 */
#define APIMAC_SFS_PAN_COORDINATOR(s) (((s) >> 14) & 0x01)

/*!
 MACRO that returns the Associate Permit bit from the superframe
 specification
 */
#define APIMAC_SFS_ASSOCIATION_PERMIT(s) (((s) >> 15) & 0x01)

/*! Max size of the Frequency Hopping Channel Map Size */
#define APIMAC_FH_MAX_BIT_MAP_SIZE 32

/*! Max size of the Frequency Hopping Network Name */
#define APIMAC_FH_NET_NAME_SIZE_MAX 32

/*! Size of the Frequency Hopping GTK Hash Size */
#define APIMAC_FH_GTK_HASH_SIZE 8

/*! Associate Response status types */
typedef enum
{
    /*! Success, join allowed */
    ApiMac_assocStatus_success = 0,
    /*! PAN at capacity */
    ApiMac_assocStatus_panAtCapacity = 1,
    /*! PAN access denied */
    ApiMac_assocStatus_panAccessDenied = 2
} ApiMac_assocStatus_t;

/*!
 Address types - used to set addrMode field of the ApiMac_sAddr_t structure.
 */
typedef enum
{
    /*! Address not present */
    ApiMac_addrType_none = 0,
    /*! Short Address (16 bits) */
    ApiMac_addrType_short = 2,
    /*! Extended Address (64 bits) */
    ApiMac_addrType_extended = 3
} ApiMac_addrType_t;

/*! Beacon types in the ApiMac_mlmeBeaconNotifyInd_t structure. */
typedef enum
{
    /*! normal beacon type */
    ApiMac_beaconType_normal = 0,
    /*! enhanced beacon type */
    ApiMac_beaconType_enhanced = 1,
} ApiMac_beaconType_t;

/*! Disassociate Reasons */
typedef enum
{
    /*! The coordinator wishes the device to disassociate */
    ApiMac_disassocateReason_coord = 1,
    /*! The device itself wishes to disassociate */
    ApiMac_disassocateReason_device = 2
} ApiMac_disassocateReason_t;

/*! Comm Status Indication Reasons */
typedef enum
{
    /*!
     Reason for comm status indication was in response to an Associate Response
     */
    ApiMac_commStatusReason_assocRsp = 0,
    /*!
     Reason for comm status indication was in response to an Orphan Response
     */
    ApiMac_commStatusReason_orphanRsp = 1,
    /*!
     Reason for comm status indication was result of recieving a secure frame
     */
    ApiMac_commStatusReason_rxSecure = 2
} ApiMac_commStatusReason_t;

/*! General MAC Status values */
typedef enum
{
    /*! Operation successful */
    ApiMac_status_success = 0,
    /*! MAC Co-Processor only - Subsystem Error */
    ApiMac_status_subSystemError = 0x25,
    /*! MAC Co-Processor only - Command ID error */
    ApiMac_status_commandIDError = 0x26,
    /*! MAC Co-Processor only - Length error */
    ApiMac_status_lengthError = 0x27,
    /*! MAC Co-Processor only - Unsupported Extended Type */
    ApiMac_status_unsupportedType = 0x28,
    /*! The AUTOPEND pending all is turned on */
    ApiMac_status_autoAckPendingAllOn = 0xFE,
    /*! The AUTOPEND pending all is turned off */
    ApiMac_status_autoAckPendingAllOff = 0xFF,
    /*! The beacon was lost following a synchronization request */
    ApiMac_status_beaconLoss = 0xE0,
    /*!
     The operation or data request failed because of activity on the channel
     */
    ApiMac_status_channelAccessFailure = 0xE1,
    /*!
     The frame counter puportedly applied by the originator of the received
     frame is invalid
     */
    ApiMac_status_counterError = 0xDB,
    /*! The MAC was not able to enter low power mode */
    ApiMac_status_denied = 0xE2,
    /*! Unused */
    ApiMac_status_disabledTrxFailure = 0xE3,
    /*!
     The received frame or frame resulting from an operation
     or data request is too long to be processed by the MAC
     */
    ApiMac_status_frameTooLong = 0xE5,
    /*!
     The key purportedly applied by the originator of the
     received frame is not allowed
     */
    ApiMac_status_improperKeyType = 0xDC,
    /*!
     The security level purportedly applied by the originator of
     the received frame does not meet the minimum security level
     */
    ApiMac_status_improperSecurityLevel = 0xDD,
    /*!
     The data request failed because neither the source address nor
     destination address parameters were present
     */
    ApiMac_status_invalidAddress = 0xF5,
    /*! Unused */
    ApiMac_status_invalidGts = 0xE6,
    /*! The purge request contained an invalid handle */
    ApiMac_status_invalidHandle = 0xE7,
    /*! Unused */
    ApiMac_status_invalidIndex = 0xF9,
    /*! The API function parameter is out of range */
    ApiMac_status_invalidParameter = 0xE8,
    /*!
     The scan terminated because the PAN descriptor storage limit
     was reached
     */
    ApiMac_status_limitReached = 0xFA,
    /*!
     The operation or data request failed because no
     acknowledgement was received
     */
    ApiMac_status_noAck = 0xE9,
    /*!
     The scan request failed because no beacons were received or the
     orphan scan failed because no coordinator realignment was received
     */
    ApiMac_status_noBeacon = 0xEA,
    /*!
     The associate request failed because no associate response was received
     or the poll request did not return any data
     */
    ApiMac_status_noData = 0xEB,
    /*! The short address parameter of the start request was invalid */
    ApiMac_status_noShortAddress = 0xEC,
    /*! Unused */
    ApiMac_status_onTimeTooLong = 0xF6,
    /*! Unused */
    ApiMac_status_outOfCap = 0xED,
    /*!
     A PAN identifier conflict has been detected and
     communicated to the PAN coordinator
     */
    ApiMac_status_panIdConflict = 0xEE,
    /*! Unused */
    ApiMac_status_pastTime = 0xF7,
    /*! A set request was issued with a read-only identifier */
    ApiMac_status_readOnly = 0xFB,
    /*! A coordinator realignment command has been received */
    ApiMac_status_realignment = 0xEF,
    /*! The scan request failed because a scan is already in progress */
    ApiMac_status_scanInProgress = 0xFC,
    /*! Cryptographic processing of the received secure frame failed */
    ApiMac_status_securityError = 0xE4,
    /*! The beacon start time overlapped the coordinator transmission time */
    ApiMac_status_superframeOverlap = 0xFD,
    /*!
     The start request failed because the device is not tracking
     the beacon of its coordinator
     */
    ApiMac_status_trackingOff = 0xF8,
    /*!
     The associate response, disassociate request, or indirect
     data transmission failed because the peer device did not respond
     before the transaction expired or was purged
     */
    ApiMac_status_transactionExpired = 0xF0,
    /*! The request failed because MAC data buffers are full */
    ApiMac_status_transactionOverflow = 0xF1,
    /*! Unused */
    ApiMac_status_txActive = 0xF2,
    /*!
     The operation or data request failed because the
     security key is not available
     */
    ApiMac_status_unavailableKey = 0xF3,
    /*! The set or get request failed because the attribute is not supported */
    ApiMac_status_unsupportedAttribute = 0xF4,
    /*!
     The received frame was secured with legacy security which is
     not supported
     */
    ApiMac_status_unsupportedLegacy = 0xDE,
    /*! The security of the received frame is not supported */
    ApiMac_status_unsupportedSecurity = 0xDF,
    /*! The operation is not supported in the current configuration */
    ApiMac_status_unsupported = 0x18,
    /*! The operation could not be performed in the current state */
    ApiMac_status_badState = 0x19,
    /*!
     The operation could not be completed because no
     memory resources were available
     */
    ApiMac_status_noResources = 0x1A,
    /*! For internal use only */
    ApiMac_status_ackPending = 0x1B,
    /*! For internal use only */
    ApiMac_status_noTime = 0x1C,
    /*! For internal use only */
    ApiMac_status_txAborted = 0x1D,
    /*!
     For internal use only - A duplicated entry is added to the source
     matching table
     */
    ApiMac_status_duplicateEntry = 0x1E,
    /*! Frequency Hopping - General error */
    ApiMac_status_fhError = 0x61,
    /*! Frequency Hopping - IE is not supported */
    ApiMac_status_fhIeNotSupported = 0x62,
    /*! Frequency Hopping - There is no ASYNC message in the MAC TX queue */
    ApiMac_status_fhNotInAsync = 0x63,
    /*! Frequency Hopping - Destination address is not in neighbor table */
    ApiMac_status_fhNotInNeighborTable = 0x64,
    /*! Frequency Hopping - Not in UC or BC dwell time slot */
    ApiMac_status_fhOutSlot = 0x65,
    /*! Frequency Hopping - Invalid address */
    ApiMac_status_fhInvalidAddress = 0x66,
    /*! Frequency Hopping - IE format is wrong */
    ApiMac_status_fhIeFormatInvalid = 0x67,
    /*! Frequency Hopping - PIB is not supported */
    ApiMac_status_fhPibNotSupported = 0x68,
    /*! Frequency Hopping - PIB is read only */
    ApiMac_status_fhPibReadOnly = 0x69,
    /*! Frequency Hopping - PIB API invalid parameter */
    ApiMac_status_fhPibInvalidParameter = 0x6A,
    /*! Frequency Hopping - Invalid frame type */
    ApiMac_status_fhInvalidFrameType = 0x6B,
    /*! Frequency Hopping - Expired node */
    ApiMac_status_fhExpiredNode = 0x6C,
    /*! Frequency Hopping - No Patch Needed */
    ApiMac_status_fhNoPatch = 0x6D
} ApiMac_status_t;

/*! MAC Security Levels */
typedef enum
{
    /*! No security is used */
    ApiMac_secLevel_none = 0,
    /*! MIC-32 authentication is used */
    ApiMac_secLevel_mic32 = 1,
    /*! MIC-64 authentication is used */
    ApiMac_secLevel_mic64 = 2,
    /*! MIC-128 authentication is used */
    ApiMac_secLevel_mic128 = 3,
    /*! AES encryption is used */
    ApiMac_secLevel_enc = 4,
    /*! AES encryption and MIC-32 authentication are used */
    ApiMac_secLevel_encMic32 = 5,
    /*! AES encryption and MIC-64 authentication are used */
    ApiMac_secLevel_encMic64 = 6,
    /*! AES encryption and MIC-128 authentication are used */
    ApiMac_secLevel_encMic128 = 7
} ApiMac_secLevel_t;

/*! Key Identifier Mode */
typedef enum
{
    /*! Key is determined implicitly */
    ApiMac_keyIdMode_implicit = 0,
    /*! Key is determined from the 1-byte key index */
    ApiMac_keyIdMode_1 = 1,
    /*! Key is determined from the 4-byte key index */
    ApiMac_keyIdMode_4 = 2,
    /*! Key is determined from the 8-byte key index */
    ApiMac_keyIdMode_8 = 3,
} ApiMac_keyIdMode_t;

/*! Standard PIB Get and Set Attributes - size bool */
typedef enum
{
    /*! TRUE if a coordinator is currently allowing association */
    ApiMac_attribute_associatePermit = 0x41,
    /*!
     TRUE if a device automatically sends a data request if its address
     is listed in the beacon frame
     */
    ApiMac_attribute_autoRequest = 0x42,
    /*! TRUE if battery life extension is enabled */
    ApiMac_attribute_battLifeExt = 0x43,
    /*! TRUE if the PAN coordinator accepts GTS requests */
    ApiMac_attribute_gtsPermit = 0x4D,
    /*! TRUE if the MAC is in promiscuous mode */
    ApiMac_attribute_promiscuousMode = 0x51,
    /*! TRUE if the MAC enables its receiver during idle periods */
    ApiMac_attribute_RxOnWhenIdle = 0x52,
    /*! TRUE if the device is associated to the PAN coordinator */
    ApiMac_attribute_associatedPanCoord = 0x56,
    /*! TRUE if the MAC supports RX and TX timestamps */
    ApiMac_attribute_timestampSupported = 0x5C,
    /*! TRUE if security is enabled */
    ApiMac_attribute_securityEnabled = 0x5D,
    /*! TRUE if MPM IE needs to be included */
    ApiMac_attribute_includeMPMIE = 0x62,
    /*! FCS type */
    ApiMac_attribute_fcsType = 0xE9,
} ApiMac_attribute_bool_t;

/*! Standard PIB Get and Set Attributes - size uint8_t */
typedef enum
{
    /*! The maximum number of symbols to wait for an acknowledgment frame */
    ApiMac_attribute_ackWaitDuration = 0x40,
    /*!
     The number of backoff periods during which the receiver is
     enabled following a beacon in battery life extension mode
     */
    ApiMac_attribute_battLifeExtPeriods = 0x44,
    /*!
     The length in bytes of the beacon payload, the maximum value for this
     parameters is APIMAC_MAX_BEACON_PAYLOAD.
     */
    ApiMac_attribute_beaconPayloadLength = 0x46,
    /*! How often the coordinator transmits a beacon */
    ApiMac_attribute_beaconOrder = 0x47,
    /*! The beacon sequence number */
    ApiMac_attribute_bsn = 0x49,
    /*! The data or MAC command frame sequence number */
    ApiMac_attribute_dsn = 0x4C,
    /*!
     The maximum number of backoffs the CSMA-CA algorithm will attempt
     before declaring a channel failure
     */
    ApiMac_attribute_maxCsmaBackoffs = 0x4E,
    /*!
     The minimum value of the backoff exponent in the CSMA-CA algorithm.
     If this value is set to 0, collision avoidance is disabled during
     the first iteration of the algorithm. Also for the slotted version
     of the CSMA-CA algorithm with the battery life extension enabled,
     the minimum value of the backoff exponent will be at least 2
     */
    ApiMac_attribute_backoffExponent = 0x4F,
    /*! This specifies the length of the active portion of the superframe */
    ApiMac_attribute_superframeOrder = 0x54,
    /*! The maximum value of the backoff exponent in the CSMA-CA algorithm */
    ApiMac_attribute_maxBackoffExponent = 0x57,
    /*! The maximum number of retries allowed after a transmission failure */
    ApiMac_attribute_maxFrameRetries = 0x59,
    /*!
     The maximum number of symbols a device shall wait for a response
     command to be available following a request command in multiples
     of aBaseSuperframeDuration
     */
    ApiMac_attribute_responseWaitTime = 0x5A,
    /*! The timestamp offset from SFD in symbols */
    ApiMac_attribute_syncSymbolOffset = 0x5B,
    /*! Enhanced beacon sequence number */
    ApiMac_attribute_eBeaconSequenceNumber = 0x5E,
    /*! Enhanced beacon order in a beacon enabled network */
    ApiMac_attribute_eBeaconOrder = 0x5F,
    /*! Offset time slot from the beacon */
    ApiMac_attribute_offsetTimeslot = 0x61,
    /*!
     Duplicate transmit power attribute in signed
     (2's complement) dBm unit
     */
    ApiMac_attribute_phyTransmitPowerSigned = 0xE0,
    /*! The logical channel */
    ApiMac_attribute_logicalChannel = 0xE1,
    /*! alternate minimum backoff exponent */
    ApiMac_attribute_altBackoffExponent = 0xE3,
    /*! Device beacon order */
    ApiMac_attribute_deviceBeaconOrder = 0xE4,
    /*! valid values are true and false */
    ApiMac_attribute_rf4cePowerSavings = 0xE5,
    /*!
     Currently supports 0 and 1.
     If 0, frame Version is always 0 and set to 1 only for secure frames.
     If 1, frame version will be set to 1 only if packet len > 102 or
     for secure frames
     */
    ApiMac_attribute_frameVersionSupport = 0xE6,
    /*! Channel Page */
    ApiMac_attribute_channelPage = 0xE7,
    /*!
     PHY Descriptor ID, used to support channel page number and index into
     descriptor table
     */
    ApiMac_attribute_phyCurrentDescriptorId = 0xE8,
    /*!
     RSSI threshold for CCA, expressed as a signed int with range -127 to 127
     */
    ApiMac_attribute_rssiThreshold = 0xF4,
    /*!
     Range Extender Mode from 0 to 2
     */
    ApiMac_attribute_rangeExtender = 0xF5,
    /*!
     enable Ack Pending for Data Pkts
     */
    ApiMac_attribute_enDataAckPending = 0xF6,
    /*!
     RF Freq Selection from 1 to 2
     */
    ApiMac_attribute_rfFreq = 0xF7,
    /*!
     RF PA Selection from 0 to 1
     */
    ApiMac_attribute_paType = 0xF8,
} ApiMac_attribute_uint8_t;

/*! Standard PIB Get and Set Attributes - size uint16_t */
typedef enum
{
    /*!
     The short address assigned to the coordinator with which the
     device is associated.  A value of MAC_ADDR_USE_EXT indicates
     that the coordinator is using its extended address
     */
    ApiMac_attribute_coordShortAddress = 0x4B,
    /*!
     The PAN identifier.  If this value is 0xffff, the device is
     not associated
     */
    ApiMac_attribute_panId = 0x50,
    /*!
     The short address that the device uses to communicate in the PAN.
     If the device is a PAN coordinator, this value shall be set before
     calling MAC_StartReq().  Otherwise the value is allocated during
     association.  Value MAC_ADDR_USE_EXT indicates that the device is
     associated but not using a short address
     */
    ApiMac_attribute_shortAddress = 0x53,
    /*!
     The maximum time in beacon intervals that a transaction is stored by
     a coordinator and indicated in the beacon
     */
    ApiMac_attribute_transactionPersistenceTime = 0x55,
    /*!
     The maximum number of CAP symbols in a beacon-enabled PAN, or
     symbols in a non beacon-enabled PAN, to wait for a frame intended
     as a response to a data request frame
     */
    ApiMac_attribute_maxFrameTotalWaitTime = 0x58,
    /*! Enhanced beacon order in a non-beacon enabled network */
    ApiMac_attribute_eBeaconOrderNBPAN = 0x60,
} ApiMac_attribute_uint16_t;


/*! Standard PIB Get and Set Attributes - size uint32_t */
typedef enum
{
    /*!
     The time the device transmitted its last beacon frame,
     in backoff period units
     */
    ApiMac_attribute_beaconTxTime = 0x48,
    /*! Diagnostics PIB - Received CRC pass counter */
    ApiMac_attribute_diagRxCrcPass = 0xEA,
    /*! Diagnostics PIB - Received CRC fail counter */
    ApiMac_attribute_diagRxCrcFail = 0xEB,
    /*! Diagnostics PIB - Received broadcast counter */
    ApiMac_attribute_diagRxBroadcast = 0xEC,
    /*! Diagnostics PIB - Transmitted broadcast counter */
    ApiMac_attribute_diagTxBroadcast = 0xED,
    /*! Diagnostics PIB - Received unicast counter */
    ApiMac_attribute_diagRxUnicast = 0xEE,
    /*! Diagnostics PIB - Transmitted unicast counter */
    ApiMac_attribute_diagTxUnicast = 0xEF,
    /*! Diagnostics PIB - Transmitted unicast retry counter */
    ApiMac_attribute_diagTxUnicastRetry = 0xF0,
    /*! Diagnostics PIB - Transmitted unicast fail counter */
    ApiMac_attribute_diagTxUnicastFail = 0xF1,
    /*! Diagnostics PIB - Received Security fail counter */
    ApiMac_attribute_diagRxSecureFail = 0xF2,
    /*! Diagnostics PIB - Transmit Security fail counter */
    ApiMac_attribute_diagTxSecureFail = 0xF3
} ApiMac_attribute_uint32_t;

/*! Standard PIB Get and Set Attributes - these attributes are array of bytes */
typedef enum
{
    /*! The contents of the beacon payload */
    ApiMac_attribute_beaconPayload = 0x45,
    /*!
     The extended address of the coordinator with which the device
     is associated
     */
    ApiMac_attribute_coordExtendedAddress = 0x4A,
    /*! The extended address of the device */
    ApiMac_attribute_extendedAddress = 0xE2,
} ApiMac_attribute_array_t;

/*! Security PIB Get and Set Attributes - size uint8_t  */
typedef enum
{
    /*! The number of entries in macSecurityLevelTable */
    ApiMac_securityAttribute_securityLevelTableEntries = 0x83,
    /*! The security level used for automatic data requests */
    ApiMac_securityAttribute_autoRequestSecurityLevel = 0x85,
    /*!The key identifier mode used for automatic data requests */
    ApiMac_securityAttribute_autoRequestKeyIdMode = 0x86,
    /*! The index of the key used for automatic data requests */
    ApiMac_securityAttribute_autoRequestKeyIndex = 0x88,
} ApiMac_securityAttribute_uint8_t;

/*! Security PIB Get and Set Attributes - size uint16_t  */
typedef enum
{
    /*! The number of entries in macKeyTable */
    ApiMac_securityAttribute_keyTableEntries = 0x81,

    /*! The number of entries in macDeviceTable */
    ApiMac_securityAttribute_deviceTableEntries = 0x82,

    /*!The 16-bit short address assigned to the PAN coordinator */
    ApiMac_securityAttribute_panCoordShortAddress = 0x8B,

} ApiMac_securityAttribute_uint16_t;

/*! Security PIB Get and Set Attributes - array of bytes */
typedef enum
{
    /*! The originator of the key used for automatic data requests */
    ApiMac_securityAttribute_autoRequestKeySource = 0x87,
    /*! The originator of the default key used for key ID mode 0x01 */
    ApiMac_securityAttribute_defaultKeySource = 0x89,
    /*! The 64-bit address of the PAN coordinator */
    ApiMac_securityAttribute_panCoordExtendedAddress = 0x8A,
} ApiMac_securityAttribute_array_t;

/*! Security PIB Get and Set Attributes - these attributes are structures */
typedef enum
{
    /*!
     A table of KeyDescriptor, entries, each containing keys and related
     information required for secured communications.  This is a SET only
     attribute.  Call ApiMac_mlmeSetSecurityReqStruct() with pValue set
     to NULL, the MAC will build the table.
     */
    ApiMac_securityAttribute_keyTable = 0x71,
    /*!
     The key lookup table entry, part of an entry of the key table.
     To GET or SET to this attribute, setup the keyIndex and keyIdLookupIndex
     fields of ApiMac_securityPibKeyIdLookupEntry_t, call
     ApiMac_mlmeGetSecurityReqStruct() or ApiMac_mlmeSetSecurityReqStruct()
     with a pointer to the ApiMac_securityPibKeyIdLookupEntry_t structure.
     For the GET, the lookupEntry field will contain the required data.
     */
    ApiMac_securityAttribute_keyIdLookupEntry = 0xD0,
    /*!
     The key device entry, part of an entry of the key table.
     To GET or SET to this attribute, setup the keyIndex and keyDeviceIndex
     fields of ApiMac_securityPibKeyDeviceEntry_t, call
     ApiMac_mlmeGetSecurityReqStruct() or ApiMac_mlmeSetSecurityReqStruct()
     with a pointer to the ApiMac_securityPibKeyDeviceEntry_t structure.
     For the GET, the deviceEntry field will contain the required data.
     */
    ApiMac_securityAttribute_keyDeviceEntry = 0xD1,
    /*!
     The key usage entry, part of an entry of the key table.
     To GET or SET to this attribute, setup the keyIndex and keyUsageIndex
     fields of ApiMac_securityPibKeyUsageEntry_t, call
     ApiMac_mlmeGetSecurityReqStruct() or ApiMac_mlmeSetSecurityReqStruct()
     with a pointer to the ApiMac_securityPibKeyUsageEntry_t structure.
     For the GET, the usageEntry field will contain the required data.
     */
    ApiMac_securityAttribute_keyUsageEntry = 0xD2,
    /*!
     The MAC key entry, an entry of the key table.
     To GET or SET to this attribute, setup the keyIndex
     field of ApiMac_securityPibKeyEntry_t, call
     ApiMac_mlmeGetSecurityReqStruct() or ApiMac_mlmeSetSecurityReqStruct()
     with a pointer to the ApiMac_securityPibKeyEntry_t structure.
     For the GET, the rest of the fields will contain the required data.
     */
    ApiMac_securityAttribute_keyEntry = 0xD3,
    /*!
     The MAC device entry, an entry of the device table.
     To GET or SET to this attribute, setup the deviceIndex
     field of ApiMac_securityPibDeviceEntry_t, call
     ApiMac_mlmeGetSecurityReqStruct() or ApiMac_mlmeSetSecurityReqStruct()
     with a pointer to the ApiMac_securityPibDeviceEntry_t structure.
     For the GET, the deviceEntry field will contain the required data.
     */
    ApiMac_securityAttribute_deviceEntry = 0xD4,
    /*!
     The MAC security level entry, an entry of the security level table.
     To GET or SET to this attribute, setup the levelIndex
     field of ApiMac_securityPibSecurityLevelEntry_t, call
     ApiMac_mlmeGetSecurityReqStruct() or ApiMac_mlmeSetSecurityReqStruct()
     with a pointer to the ApiMac_securityPibSecurityLevelEntry_t structure.
     For the GET, the levelEntry field will contain the required data.
     */
    ApiMac_securityAttribute_securityLevelEntry = 0xD5
} ApiMac_securityAttribute_struct_t;

/*! Frequency Hopping PIB Get and Set Attributes - size uint8_t */
typedef enum
{
    /*! Duration of node's unicast slot (in milliseconds) - uint8_t */
    ApiMac_FHAttribute_unicastDwellInterval = 0x2004,
    /*! Duration of node's broadcast slot (in milliseconds) - uint8_t */
    ApiMac_FHAttribute_broadcastDwellInterval = 0x2005,
    /*! Clock drift in PPM - uint8_t */
    ApiMac_FHAttribute_clockDrift = 0x2006,
    /*! Timing accuracy in 10 microsecond resolution - uint8_t */
    ApiMac_FHAttribute_timingAccuracy = 0x2007,
    /*! Unicast channel hopping function - uint8_t */
    ApiMac_FHAttribute_unicastChannelFunction = 0x2008,
    /*! Broadcast channel hopping function - uint8_t */
    ApiMac_FHAttribute_broadcastChannelFunction = 0x2009,
    /*! Node is propagating parent's BS-IE - uint8_t */
    ApiMac_FHAttribute_useParentBSIE = 0x200A,
    /*! Estimate of routing path ETX to the PAN coordinator - uint8_t */
    ApiMac_FHAttribute_routingCost = 0x200F,
    /*! RPL(1), MHDS(0) - uint8_t */
    ApiMac_FHAttribute_routingMethod = 0x2010,
    /*! Node can accept EAPOL message - uint8_t */
    ApiMac_FHAttribute_eapolReady = 0x2011,
    /*! Wi-SUN FAN version - uint8_t */
    ApiMac_FHAttribute_fanTPSVersion = 0x2012,
    /*! Additional base wait time to sense target channel */
    ApiMac_FHAttribute_CsmaBaseBacoff = 0x201A,

} ApiMac_FHAttribute_uint8_t;

/*! Frequency Hopping PIB Get and Set Attributes - size uint16_t */
typedef enum
{
    /*!
     Broadcast schedule ID for broadcast channel hopping sequence - uint16_t
     */
    ApiMac_FHAttribute_broadcastSchedId = 0x200B,
    /*! Unicast channel number when no hopping - uint16_t */
    ApiMac_FHAttribute_unicastFixedChannel = 0x200C,
    /*! Broadcast channel number when no hopping - uint16_t */
    ApiMac_FHAttribute_broadcastFixedChannel = 0x200D,
    /*! Number of nodes in the PAN - uint16_t */
    ApiMac_FHAttribute_panSize = 0x200E,
    /*! PAN version to notify PAN configuration changes - uint16_t */
    ApiMac_FHAttribute_panVersion = 0x2014,
    /*! Time in min during which the node info considered as valid - uint16_t */
    ApiMac_FHAttribute_neighborValidTime = 0x2019,
    /*! Number of non-sleepy device - uint16_t */
    ApiMac_FHAttribute_numNonSleepDevice = 0x201b,
    /*! Number of sleepy device - uint16_t */
    ApiMac_FHAttribute_numSleepDevice = 0x201c,
    /*! Number of temp table node - uint16_t */
    ApiMac_FHAttribute_numTempTableNode = 0x201d,

} ApiMac_FHAttribute_uint16_t;

/*! Frequency Hopping PIB Get and Set Attributes - size uint32_t */
typedef enum
{
    /*!
     Time between start of two broadcast slots (in milliseconds) - uint32_t
     */
    ApiMac_FHAttribute_BCInterval = 0x2001,
} ApiMac_FHAttribute_uint32_t;

/*! Frequency Hopping PIB Get and Set Attributes - array of bytes */
typedef enum
{
    /*! The parent EUI address - ApiMac_sAddrExt_t */
    ApiMac_FHAttribute_trackParentEUI = 0x2000,
    /*! Unicast excluded channels - APIMAC_FH_MAX_BIT_MAP_SIZE */
    ApiMac_FHAttribute_unicastExcludedChannels = 0x2002,
    /*! Broadcast excluded channels - APIMAC_FH_MAX_BIT_MAP_SIZE */
    ApiMac_FHAttribute_broadcastExcludedChannels = 0x2003,
    /*! Network Name - APIMAC_FH_NET_NAME_SIZE_MAX uint8_t */
    ApiMac_FHAttribute_netName = 0x2013,
    /*!
     Low order 64 bits of SHA256 hash of GTK
     */
    ApiMac_FHAttribute_gtk0Hash = 0x2015,
    /*!
     Next low order 64 bits of SHA256 hash of GTK
     */
    ApiMac_FHAttribute_gtk1Hash = 0x2016,
    /*!
     Next low order 64 bits of SHA256 hash of GTK
     */
    ApiMac_FHAttribute_gtk2Hash = 0x2017,
    /*!
     Next low order 64 bits of SHA256 hash of GTK
     */
    ApiMac_FHAttribute_gtk3Hash = 0x2018,
} ApiMac_FHAttribute_array_t;

/*! FH Frame Types */
typedef enum
{
    /*! WiSUN PAN advertisement */
    ApiMac_fhFrameType_panAdvert = 0x00,
    /*! WiSUN PAN advertisement solicit */
    ApiMac_fhFrameType_panAdvertSolicit = 0x01,
    /*! WiSUN PAN config */
    ApiMac_fhFrameType_config = 0x02,
    /*! WiSUN PAN config solicit */
    ApiMac_fhFrameType_configSolicit = 0x03,
    /*! WiSUN Data frame */
    ApiMac_fhFrameType_data = 0x04,
    /*! WiSUN Ack frame */
    ApiMac_fhFrameType_ack = 0x05,
    /*! WiSUN Ack frame */
    ApiMac_fhFrameType_eapol = 0x06,
    /*! Internal: WiSUN Invalid frame */
    ApiMac_fhFrameType_invalid = 0xFF
} ApiMac_fhFrameType_t;

/*! Payload IE Group IDs */
typedef enum
{
    /*! Payload ESDU IE Group ID */
    ApiMac_payloadIEGroup_ESDU = 0x00,
    /*! Payload MLME IE Group ID */
    ApiMac_payloadIEGroup_MLME = 0x01,
    /*! Payload WiSUN IE Group ID */
    ApiMac_payloadIEGroup_WiSUN = 0x04,
    /*! Payload Termination IE Group ID */
    ApiMac_payloadIEGroup_term = 0x0F
} ApiMac_payloadIEGroup_t;

/*! MLME Sub IEs */
typedef enum
{
    /*! MLME Sub IEs - short format - Coexistence IE */
    ApiMac_MLMESubIE_coexist = 0x21,
    /*! MLME Sub IEs - short format - SUN Device capabilities IE */
    ApiMac_MLMESubIE_sunDevCap = 0x22,
    /*! MLME Sub IEs - short format - SUN FSK Generic PHY IE */
    ApiMac_MLMESubIE_sunFSKGenPhy = 0x23,
} ApiMac_MLMESubIE_t;

/*! WiSUN Sub IEs */
typedef enum
{
    /*! WiSUN Sub IE - Long format - Unicast Schedule IE */
    ApiMac_wisunSubIE_USIE = 1,
    /*! WiSUN Sub IE - Long format - Broadcast Schedule IE */
    ApiMac_wisunSubIE_BSIE = 2,
    /*! WiSUN Sub IE - Short format - PAN IE */
    ApiMac_wisunSubIE_PANIE = 4,
    /*! WiSUN Sub IE - Short format - Network Name IE */
    ApiMac_wisunSubIE_netNameIE = 5,
    /*! WiSUN Sub IE - Short format - PAN Version IE */
    ApiMac_wisunSubIE_PANVersionIE = 6,
    /*! WiSUN Sub IE - Short format - GTK Hash IE */
    ApiMac_wisunSubIE_GTKHashIE = 7
} ApiMac_wisunSubIE_t;

/*! Scan Types */
typedef enum
{
    /*!
     Energy detect scan.  The device will tune to each channel and
     perform and energy measurement.  The list of channels and their
     associated measurements will be returned at the end of the scan
     */
    ApiMac_scantype_energyDetect = 0,
    /*!
     Active scan.  The device tunes to each channel, sends a beacon
     request and listens for beacons.  The PAN descriptors are returned
     at the end of the scan
     */
    ApiMac_scantype_active = 1,
    /*!
     Passive scan.  The device tunes to each channel and listens for
     beacons.  The PAN descriptors are returned at the end of the scan
     */
    ApiMac_scantype_passive = 2,
    /*!
     Orphan scan.  The device tunes to each channel and sends an orphan
     notification to try and find its coordinator.  The status is returned
     at the end of the scan
     */
    ApiMac_scantype_orphan = 3,
    /*!
     Enhanced Active scan. In addition to Active scan, this command is also
     used by a device to locate a subset of all coordinators within its
     POS during an active scan
     */
    ApiMac_scantype_activeEnhanced = 5
} ApiMac_scantype_t;

/*! WiSUN Async Operations */
typedef enum
{
    /*! Start Async */
    ApiMac_wisunAsycnOperation_start = 0,
    /*! Stop Async */
    ApiMac_wisunAsycnOperation_stop = 1
} ApiMac_wisunAsycnOperation_t;

/*! WiSUN Async Frame Types */
typedef enum
{
    /*! WiSUN Async PAN Advertisement Frame type */
    ApiMac_wisunAsyncFrame_advertisement = 0,
    /*! WiSUN Async PAN Advertisement Solicitation Frame type */
    ApiMac_wisunAsyncFrame_advertisementSolicit = 1,
    /*! WiSUN Async PAN Configuration Frame type */
    ApiMac_wisunAsyncFrame_config = 2,
    /*! WiSUN Async PAN Configuration Solicitation Frame type */
    ApiMac_wisunAsyncFrame_configSolicit = 3
} ApiMac_wisunAsyncFrame_t;

/*! Freqency Hopping Dispatch Values */
typedef enum
{
    /*! No protocol dispatch */
    ApiMac_fhDispatchType_none = 0,
    /*! MHD-PDU protocol dispatch */
    ApiMac_fhDispatchType_MHD_PDU = 1,
    /*! 6LowPAN protocol dispatch */
    ApiMac_fhDispatchType_6LowPAN = 2
} ApiMac_fhDispatchType_t;

/******************************************************************************
 Structures - Building blocks for the API for the MAC Module
 *****************************************************************************/

/*! Extended address */
typedef uint8_t ApiMac_sAddrExt_t[APIMAC_SADDR_EXT_LEN];

/*! MAC address type field structure */
typedef struct
{
    /*!
     The address can be either a long address or a short address depending
     the addrMode field.
     */
    union
    {
        /*! 16 bit address */
        uint16_t shortAddr;
        /*! Extended address */
        ApiMac_sAddrExt_t extAddr;
    } addr;

    /*! Address type/mode */
    ApiMac_addrType_t addrMode;
} ApiMac_sAddr_t;

/*! Data buffer structure */
typedef struct _apimac_sdata
{
    /*! pointer to the data buffer */
    uint8_t *p;
    /*! length of the data buffer */
    uint16_t len;
} ApiMac_sData_t;

/*!
 Generic PHY Descriptor.
 We are using this structure for both Channel Page 9 and Channel Page 10.
 */
typedef struct _apimac_mrfskphydesc
{
    /*! First Channel Center frequency */
    uint32_t firstChCentrFreq;
    /*! Number of channels defined for the particular PHY mode */
    uint16_t numChannels;
    /*! Distance between Adjacent center channel frequencies */
    uint32_t channelSpacing;
    /*! 2-FSK/2-GFSK/4-FSK/4-GFSK */
    uint8_t fskModScheme;
    /*! Symbol rate selection */
    uint16_t symbolRate;
    /*!
     Modulation index as a value encoded in
     MR-FSK Generic PHY Descriptor IE
     (IEEE802.15.4g section 5.2.4.20c).
     <BR>
     2FSK MI = 0.25 + Modulation Index * 0.05
     <BR>
     4FSK MI is a third of 2FSK MI
     */
    uint8_t fskModIndex;
    /*! Channel clearance algorithm selection */
    uint8_t ccaType;
} ApiMac_MRFSKPHYDesc_t;

/*! Common security type */
typedef struct _apimac_sec
{
    /*! Key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel;
    /*! Key identifier mode */
    uint8_t keyIdMode;
    /*! Key index */
    uint8_t keyIndex;
} ApiMac_sec_t;

/*! Key ID Lookup Descriptor */
typedef struct _apimac_keyidlookupdescriptor
{
    /*! Data used to identify the key */
    uint8_t lookupData[APIMAC_MAX_KEY_LOOKUP_LEN];
    /*! 0x00 indicates 5 octets; 0x01 indicates 9 octets */
    uint8_t lookupDataSize;
} ApiMac_keyIdLookupDescriptor_t;

/*! Key Device Descriptor */
typedef struct _apimac_keydevicedescriptor
{
    /*! Handle to the DeviceDescriptor */
    uint16_t deviceDescriptorHandle;
    /*! True if the device is unique */
    bool uniqueDevice;
    /*! This key exhausted the frame counter. */
    bool blackListed;
} ApiMac_keyDeviceDescriptor_t;

/*! Key Usage Descriptor */
typedef struct _apimac_keyusagedescriptor
{
    /*! Frame Type */
    uint8_t frameType;
    /*! Command Frame Identifier */
    uint8_t cmdFrameId;
} ApiMac_keyUsageDescriptor_t;

/*! Key Descriptor */
typedef struct _apimac_keydescriptor
{
    /*! A list identifying this KeyDescriptor */
    ApiMac_keyIdLookupDescriptor_t *keyIdLookupList;
    /*! The number of entries in KeyIdLookupList */
    uint8_t keyIdLookupEntries;
    /*!
     A list indicating which devices are currently using this key, including
     their blacklist status.
     */
    ApiMac_keyDeviceDescriptor_t *keyDeviceList;

    /*! The number of entries in KeyDeviceList */
    uint16_t keyDeviceListEntries;

    /*! A list indicating which frame types this key may be used with. */
    ApiMac_keyUsageDescriptor_t *keyUsageList;
    /*! The number of entries in KeyUsageList */
    uint8_t keyUsageListEntries;
    /*! The actual value of the key */
    uint8_t key[APIMAC_KEY_MAX_LEN];
    /*!
     PIB frame counter in 802.15.4 is universal across key,
     but it makes more sense to associate a frame counter
     with a key.
     */
    uint32_t frameCounter;
} ApiMac_keyDescriptor_t;

/*! Device Descriptor */
typedef struct _apimac_devicedescriptor
{
    /*! The 16-bit PAN identifier of the device */
    uint16_t panID;
    /*! The 16-bit short address of the device */
    uint16_t shortAddress;
    /*!
     The 64-bit IEEE extended address of the device. This element is also
     used inunsecuring operations on incoming frames.
     */
    ApiMac_sAddrExt_t extAddress;
} ApiMac_deviceDescriptor_t;

/*! Security Level Descriptor */
typedef struct _apimac_securityleveldescriptor
{
    /*! Frame Type */
    uint8_t frameType;
    /*! Command Frame ID */
    uint8_t commandFrameIdentifier;
    /*!
     The minimal required/expected security level for incoming MAC frames.
     */
    uint8_t securityMinimum;
    /*!
     Indication of whether originating devices for which the Exempt flag is
     set may override the minimum security level indicated by the Security
     Minimum element. If TRUE, this indicates that for originating devices
     with Exempt status, the incoming security level zero is acceptable.
     */
    bool securityOverrideSecurityMinimum;
} ApiMac_securityLevelDescriptor_t;

/*! Frame Counter */
typedef struct
{
    uint32_t             frameCounter;
    uint16_t             keyIdx;
} ApiMac_frameCntr_t;

/*! Security Device Descriptor */
typedef struct _apimac_secdevicedescriptor
{
    /*! Device information */
    ApiMac_deviceDescriptor_t devInfo;
    /*!
     The incoming frame counter of the device. This value is used to
     ensure sequential freshness of frames.
     API_MAX_NODE_KEY_ENTRIES needs to match the value in macCfg.
     */
    ApiMac_frameCntr_t *frameCntr;
    /*! Device may override the minimum security level settings. */
    bool exempt;
} ApiMac_securityDeviceDescriptor_t;

/*! MAC key entry structure */
typedef struct _apimac_securitykeyentry
{
    /*! The 128-bit key */
    uint8_t keyEntry[APIMAC_KEY_MAX_LEN];
    /*! the key's index - unique */
    uint8_t keyIndex;
    /*! The key's frame counter */
    uint32_t frameCounter;
} ApiMac_securityKeyEntry_t;

/*!
 Security PIB Key ID lookup entry for a Get/Set
 ApiMac_securityAttribute_keyIdLookupEntry
 */
typedef struct _apimac_securitypibkeyidlookupentry
{
    /*! index into the macKeyIdLookupList */
    uint16_t keyIndex;
    /*! index into macKeyIdLookupList[keyIndex] */
    uint8_t  keyIdLookupIndex;
    /*! Place to put the requested data */
    ApiMac_keyIdLookupDescriptor_t lookupEntry;
} ApiMac_securityPibKeyIdLookupEntry_t;

/*!
 Security PIB Key ID device entry for a Get/Set
 ApiMac_securityAttribute_keyDeviceEntry
 */
typedef struct _apimac_securitypibkeydeviceentry
{
    /*! index into the macKeyDeviceList */
    uint16_t keyIndex;
    /*! index into macKeyDeviceList[keyIndex] */
    uint16_t keyDeviceIndex;
    /*! Place to put the requested data */
    ApiMac_keyDeviceDescriptor_t deviceEntry;
} ApiMac_securityPibKeyDeviceEntry_t;

/*!
 Security PIB Key ID usage entry for a Get/Set
 ApiMac_securityAttribute_keyUsageEntry
 */
typedef struct _apimac_securitypibkeyusageentry
{
    /*! index into the macKeyUsageList */
    uint16_t keyIndex;
    /*! index into macKeyUsageList[keyIndex] */
    uint8_t keyUsageIndex;
    /*! Place to put the requested data */
    ApiMac_keyUsageDescriptor_t usageEntry;
} ApiMac_securityPibKeyUsageEntry_t;

/*!
 Security PIB Key entry for a Get/Set
 ApiMac_securityAttribute_keyEntry
 */
typedef struct _apimac_securitypibkeyentry
{
    /*! index into the macKeyTable */
    uint16_t keyIndex;
    /*! key entry */
    uint8_t keyEntry[APIMAC_KEY_MAX_LEN];
    /*! frame counter */
    uint32_t frameCounter;
} ApiMac_securityPibKeyEntry_t;

/*!
 Security PIB device entry for a Get/Set
 ApiMac_securityAttribute_deviceEntry
 */
typedef struct _apimac_securitypibdeviceentry
{
    /*! index into the macDeviceTable */
    uint16_t deviceIndex;
    /*! Place to put the requested data */
    ApiMac_securityDeviceDescriptor_t deviceEntry;
} ApiMac_securityPibDeviceEntry_t;


/*!
 Security PIB level entry for a Get/Set
 ApiMac_securityAttribute_securityLevelEntry
 */
typedef struct _apimac_securitypiblevelentry
{
    /*! index into the macSecurityLevelTable */
    uint8_t levelIndex;
    /*! Place to put the requested data */
    ApiMac_securityLevelDescriptor_t levelEntry;
} ApiMac_securityPibSecurityLevelEntry_t;

/*! Structure defines the Capabilities Information bit field. */
typedef struct _apimac_capabilityinfo
{
    /*! True if the device is a PAN Coordinator */
    bool panCoord;
    /*! True if the device is a full function device (FFD) */
    bool ffd;
    /*! True if the device is mains powered */
    bool mainsPower;
    /*! True if the device's RX is on when the device is idle */
    bool rxOnWhenIdle;
    /*! True if the device is capable of sending and receiving secured frames
     */
    bool security;
    /*!
     True if allocation of a short address in the associate procedure
     is needed.
     */
    bool allocAddr;
} ApiMac_capabilityInfo_t;

/*! Data Request Transmit Options */
typedef struct _apimac_txoptions
{
    /*!
     Acknowledged transmission.  The MAC will attempt to retransmit
     the frame until it is acknowledged
     */
    bool ack;
    /*!
     Indirect transmission.  The MAC will queue the data and wait
     for the destination device to poll for it.  This can only be used
     by a coordinator device
     */
    bool indirect;
    /*!
     This proprietary option forces the pending bit set for direct
     transmission
     */
    bool pendingBit;
    /*!
     This proprietary option prevents the frame from being retransmitted
     */
    bool noRetransmits;
    /*!
     This proprietary option prevents a MAC_MCPS_DATA_CNF
     event from being sent for this frame
     */
    bool noConfirm;
    /*!
     Use PIB value MAC_ALT_BE for the minimum backoff exponent
     */
    bool useAltBE;
    /*!
     Use the power and channel values in macDataReq_t
     instead of the PIB values
     */
    bool usePowerAndChannel;
    /*!
     Use option Green Power
     */
    bool useGreenPower;
} ApiMac_txOptions_t;

/*! MCPS data request type */
typedef struct _apimac_mcpsdatareq
{
    /*! The address of the destination device */
    ApiMac_sAddr_t dstAddr;
    /*! The PAN ID of the destination device */
    uint16_t dstPanId;
    /*! The source address mode */
    ApiMac_addrType_t srcAddrMode;
    /*! Application-defined handle value associated with this data request */
    uint8_t msduHandle;
    /*! TX options bit mask */
    ApiMac_txOptions_t txOptions;
    /*! Transmit the data frame on this channel */
    uint8_t channel;
    /*! Transmit the data frame at this power level */
    uint8_t power;
    /*! pointer to the payload IE list, excluding termination IEs */
    uint8_t *pIEList;
    /*! length of the payload IE */
    uint16_t payloadIELen;
    /*!
     Freq hopping Protocol Dispatch - RESERVED for future use, should
     be cleared.
     */
    ApiMac_fhDispatchType_t fhProtoDispatch;
    /*! Bitmap indicates which FH IE's need to be included */
    uint32_t includeFhIEs;
    /*! Data buffer */
    ApiMac_sData_t msdu;
    /*! Security Parameters */
    ApiMac_sec_t sec;
    /*! Transmit Delay for Green Power */
    uint8_t  gpOffset;
    /*! Transmit Window for Green Power */
    uint8_t  gpDuration;
} ApiMac_mcpsDataReq_t;

/*! Structure a Payload information Item */
typedef struct _apimac_payloadieitem
{
    /*! True if payload IE type is long */
    bool ieTypeLong;
    /*! IE ID */
    uint8_t ieId;
    /*! IE Content Length - max size 2047 bytes */
    uint16_t ieContentLen;
    /*! Pointer to the IE's content */
    uint8_t *pIEContent;
} ApiMac_payloadIeItem_t;

/*! A Payload IE Link List record */
typedef struct _apimac_payloadierec
{
    /*! Pointer to the next element in the linked list, NULL if no more */
    void *pNext;
    /*! Payload IE information item */
    ApiMac_payloadIeItem_t item;
} ApiMac_payloadIeRec_t;

/*! MCPS data indication type */
typedef struct _apimac_mcpsdataind
{
    /*! The address of the sending device */
    ApiMac_sAddr_t srcAddr;
    /*! The address of the destination device */
    ApiMac_sAddr_t dstAddr;
    /*! The time, in backoffs, at which the data were received */
    uint32_t timestamp;
    /*!
     The time, in internal MAC timer units, at which the data were received
     */
    uint16_t timestamp2;
    /*! The PAN ID of the sending device */
    uint16_t srcPanId;
    /*! The PAN ID of the destination device */
    uint16_t dstPanId;
    /*! The link quality of the received data frame */
    uint8_t mpduLinkQuality;
    /*! The raw correlation value of the received data frame */
    uint8_t correlation;
    /*! The received RF power in units dBm */
    int8_t rssi;
    /*! The data sequence number of the received frame */
    uint8_t dsn;
    /*! length of the payload IE buffer (pPayloadIE) */
    uint16_t payloadIeLen;
    /*! Pointer to the start of payload IEs */
    uint8_t *pPayloadIE;
    /*! Frequency Hopping Frame Type */
    ApiMac_fhFrameType_t fhFrameType;
    /*! Frequency hopping protocol dispatch - RESERVED for future use. */
    ApiMac_fhDispatchType_t fhProtoDispatch;
    /*! Frame counter value of the received data frame (if used) */
    uint32_t frameCntr;
    /*! Security Parameters */
    ApiMac_sec_t sec;
    /*! Data Buffer */
    ApiMac_sData_t msdu;
} ApiMac_mcpsDataInd_t;

/*! MCPS data confirm type */
typedef struct _apimac_mcpsdatacnf
{
    /*! Contains the status of the data request operation */
    ApiMac_status_t status;
    /*! Application-defined handle value associated with the data request */
    uint8_t msduHandle;
    /*! The time, in backoffs, at which the frame was transmitted */
    uint32_t timestamp;
    /*!
     The time, in internal MAC timer units, at which the frame was
     transmitted
     */
    uint16_t timestamp2;
    /*! The number of retries required to transmit the data frame */
    uint8_t retries;
    /*! The link quality of the received ack frame */
    uint8_t mpduLinkQuality;
    /*! The raw correlation value of the received ack frame */
    uint8_t correlation;
    /*! The RF power of the received ack frame in units dBm */
    int8_t rssi;
    /*! Frame counter value used (if any) for the transmitted frame */
    uint32_t frameCntr;
} ApiMac_mcpsDataCnf_t;

/*! MCPS purge confirm type */
typedef struct _apimac_mcpspurgecnf
{
    /*! The status of the purge request operation */
    ApiMac_status_t status;
    /*! Application-defined handle value associated with the data request */
    uint8_t msduHandle;
} ApiMac_mcpsPurgeCnf_t;

/*! PAN descriptor type */
typedef struct _apimac_pandesc
{
    /*! The address of the coordinator sending the beacon */
    ApiMac_sAddr_t coordAddress;
    /*! The PAN ID of the network */
    uint16_t coordPanId;
    /*!
     The superframe specification of the network, this field contains
     the beacon order, superframe order, final CAP slot, battery life
     extension, PAN coordinator bit, and association permit flag.
     Use the following macros to parse this field: APIMAC_SFS_BEACON_ORDER(),
     APIMAC_SFS_SUPERFRAME_ORDER(), APIMAC_SFS_FINAL_CAP_SLOT()
     APIMAC_SFS_BLE(), APIMAC_SFS_PAN_COORDINATOR(), and
     APIMAC_SFS_ASSOCIATION_PERMIT().
     */
    uint16_t superframeSpec;
    /*! The logical channel of the network */
    uint8_t logicalChannel;
    /*! The current channel page occupied by the network */
    uint8_t channelPage;
    /*!
     TRUE if coordinator accepts GTS requests.  This field is not used for
     enhanced beacons.
     */
    bool gtsPermit;
    /*! The link quality of the received beacon */
    uint8_t linkQuality;
    /*! The time at which the beacon was received, in backoffs */
    uint32_t timestamp;
    /*! TRUE if there was an error in the security processing */
    bool securityFailure;
    /*! The security parameters for the received beacon frame */
    ApiMac_sec_t sec;
} ApiMac_panDesc_t;

/*! MLME associate request type */
typedef struct _apimac_macmlmeassociatereq
{
    /*! The security parameters for this message */
    ApiMac_sec_t sec;
    /*! The channel on which to attempt association */
    uint8_t logicalChannel;
    /*! The channel page on which to attempt association */
    uint8_t channelPage;
    /*! Identifier for the PHY descriptor */
    uint8_t phyID;
    /*! Address of the coordinator with which to associate */
    ApiMac_sAddr_t coordAddress;
    /*! The identifier of the PAN with which to associate */
    uint16_t coordPanId;
    /*! The operational capabilities of this device */
    ApiMac_capabilityInfo_t capabilityInformation;
} ApiMac_mlmeAssociateReq_t;

/*! MLME associate response type */
typedef struct _apimac_mlmeassociatersp
{
    /*! The security parameters for this message */
    ApiMac_sec_t sec;
    /*! The address of the device requesting association */
    ApiMac_sAddrExt_t deviceAddress;
    /*! The short address allocated to the device */
    uint16_t assocShortAddress;
    /*! The status of the association attempt */
    ApiMac_assocStatus_t status;
} ApiMac_mlmeAssociateRsp_t;

/*! MLME disassociate request type */
typedef struct _apimac_mlmedisassociatereq
{
    /*! The security parameters for this message */
    ApiMac_sec_t sec;
    /*! The address of the device with which to disassociate */
    ApiMac_sAddr_t deviceAddress;
    /*! The PAN ID of the device */
    uint16_t devicePanId;
    /*! The disassociate reason */
    ApiMac_disassocateReason_t disassociateReason;
    /*! Transmit Indirect */
    bool txIndirect;
} ApiMac_mlmeDisassociateReq_t;

/*! MLME orphan response type */
typedef struct _apimac_mlmeorphanrsp
{
    /*! The security parameters for this message */
    ApiMac_sec_t sec;
    /*! The extended address of the device sending the orphan notification */
    ApiMac_sAddrExt_t orphanAddress;
    /*! The short address of the orphaned device */
    uint16_t shortAddress;
    /*!
     TRUE if the orphaned device is associated with this coordinator
     */
    bool associatedMember;
} ApiMac_mlmeOrphanRsp_t;

/*! MLME poll request type */
typedef struct _apimac_mlmepollreq
{
    /*! The address of the coordinator device to poll */
    ApiMac_sAddr_t coordAddress;
    /*! The PAN ID of the coordinator */
    uint16_t coordPanId;
    /*! The security parameters for this message */
    ApiMac_sec_t sec;
} ApiMac_mlmePollReq_t;

/*! MLME scan request type */
typedef struct _apimac_mlmescanreq
{
    /*! Bit mask indicating which channels to scan */
    uint8_t scanChannels[APIMAC_154G_CHANNEL_BITMAP_SIZ];
    /*! The type of scan */
    ApiMac_scantype_t scanType;
    /*! The exponent used in the scan duration calculation */
    uint8_t scanDuration;
    /*! The channel page on which to perform the scan */
    uint8_t channelPage;
    /*! PHY ID corresponding to the PHY descriptor to use */
    uint8_t phyID;
    /*!
     The maximum number of PAN descriptor results, these
     results will be returned in the scan confirm.
     */
    uint8_t maxResults;
    /*!
     Only devices with permit joining enabled respond to the enhanced
     beacon request
     */
    bool permitJoining;
    /*!
     The device will respond to the enhanced beacon request
     if mpduLinkQuality is equal or higher than this value
     */
    uint8_t linkQuality;
    /*!
     The device will then randomly determine if it is to
     respond to the enhanced beacon request based on meeting
     this probability (0 to 100%).
     */
    uint8_t percentFilter;
    /*! The security parameters for this message */
    ApiMac_sec_t sec;
    /*!
     When TRUE, scanDuration is ignored. When FALSE, scan duration
     shall be set to scanDuration; MPMScanDuration is ignored
     */
    bool MPMScan;
    /*! BPAN or NBPAN */
    uint8_t MPMScanType;
    /*!
     If MPMScanType is BPAN, MPMScanDuration values are 0-14.
     It is used in determining the max time spent scanning
     for an EB in a beacon enabled PAN on the channel.
     [aBaseSuperframeDuration * 2^n symbols], where
     n is the MPMScanDuration.
     If MPMScanType is NBPAN, valid values are 1 - 16383.
     It is used in determining the max time spent scanning for an
     EB in nonbeacon-enabled PAN on the channel.
     [aBaseSlotDuration * n] symbols, where n
     is MPMScanDuration.
     */
    uint16_t MPMScanDuration;
} ApiMac_mlmeScanReq_t;

/*! MPM(Multi-PHY layer management) parameters */
typedef struct _apimac_mpmParams
{
    /*!
     The exponent used to calculate the enhanced beacon interval.
     A value of 15 indicates no EB in a beacon enabled PAN
     */
    uint8_t eBeaconOrder;
    /*!
     Indicates the time diff between the EB and the preceeding
     periodic Beacon. The valid range for this field is 10 - 15.
     */
    uint8_t offsetTimeSlot;
    /*!
     Indicates how often the EB to tx in a non-beacon enabled PAN. A value of
     16383 indicates no EB in a non-beacon enabled PAN
     */
    uint16_t NBPANEBeaconOrder;
    /*!
     pointer to the buffer containing the Information element IDs which needs
     to be sent in Enhanced Beacon.  This field is reserved for future use
     and should be set to NULL.
     */
    uint8_t *pIEIDs;
    /*!
     The number of Information Elements in the buffer (size of buffer
     at pIEIDs.  This field is reserved for future use and should be set to 0.
     */
    uint8_t numIEs;
} ApiMac_mpmParams_t;

/*! MLME start request type */
typedef struct _apimac_mlmestartreq
{
    /*!
     The time to begin transmitting beacons relative to the received beacon
     */
    uint32_t startTime;
    /*!
     The PAN ID to use.  This parameter is ignored if panCoordinator is FALSE
     */
    uint16_t panId;
    /*!
     The logical channel to use.  This parameter is ignored if panCoordinator
     is FALSE
     */
    uint8_t logicalChannel;
    /*!
     The channel page to use.  This parameter is ignored if panCoordinator
     is FALSE
     */
    uint8_t channelPage;
    /*! PHY ID corresponding to the PHY descriptor to use */
    uint8_t phyID;
    /*! The exponent used to calculate the beacon interval */
    uint8_t beaconOrder;
    /*! The exponent used to calculate the superframe duration */
    uint8_t superframeOrder;
    /*! Set to TRUE to start a network as PAN coordinator */
    bool panCoordinator;
    /*!
     If this value is TRUE, the receiver is disabled after
     MAC_BATT_LIFE_EXT_PERIODS full backoff periods following the interframe
     spacing period of the beacon frame
     */
    bool batteryLifeExt;
    /*!
     Set to TRUE to transmit a coordinator realignment prior to changing
     the superframe configuration
     */
    bool coordRealignment;
    /*! Security parameters for the coordinator realignment frame */
    ApiMac_sec_t realignSec;
    /*! Security parameters for the beacon frame */
    ApiMac_sec_t beaconSec;
    /*! MPM (multi-PHY layer management) parameters */
    ApiMac_mpmParams_t mpmParams;
    /*! Indicates whether frequency hopping needs to be enabled */
    bool startFH;
} ApiMac_mlmeStartReq_t;

/*! MAC_MlmeSyncReq type */
typedef struct _apimac_mlmesyncreq
{
    /*! The logical channel to use */
    uint8_t logicalChannel;
    /*! The channel page to use */
    uint8_t channelPage;
    /*! PHY ID corresponding to the PHY descriptor to use */
    uint8_t phyID;
    /*!
     Set to TRUE to continue tracking beacons after synchronizing with the
     first beacon.  Set to FALSE to only synchronize with the first beacon
     */
    uint8_t trackBeacon;
} ApiMac_mlmeSyncReq_t;

/*! MLME WiSUN Async request type */
typedef struct _apimac_mlmewsasyncreq
{
    /*! The security parameters for this message */
    ApiMac_sec_t sec;
    /*! Start or Stop Async operation */
    ApiMac_wisunAsycnOperation_t operation;
    /*! Async frame type */
    ApiMac_wisunAsyncFrame_t frameType;
    /*!
     Bit Mask indicating which channels to send the Async frames for the
     start operation
     */
    uint8_t channels[APIMAC_154G_CHANNEL_BITMAP_SIZ];
} ApiMac_mlmeWSAsyncReq_t;

/*! Structure to pass information to the ApiMac_secAddDevice(). */
typedef struct _apimac_secadddevice
{
    /*! PAN ID of the new device */
    uint16_t panID;
    /*! short address of the new device */
    uint16_t shortAddr;
    /*! extended address of the new device */
    ApiMac_sAddrExt_t  extAddr;
    /*!
     Device descriptor exempt field value (true or false), setting this
     field to true means that this device can override the minimum
     security level setting.
     */
    bool exempt;
    /*! Frame Counter */
    uint32_t frameCounter;
#ifndef FEATURE_SECURE_COMMISSIONING
    /*!
     key ID lookup data size as it is stored in PIB, (i.e.,
     0 for 5 bytes, 1 for 9 bytes).
     */
    uint8_t keyIdLookupDataSize;
    /*!
     key ID lookup data, to look for the key table entry and create
     proper key device descriptor for this device.
     */
    uint8_t keyIdLookupData[APIMAC_MAX_KEY_LOOKUP_LEN];
    /*! key device descriptor uniqueDevice field value (true or false) */
    bool uniqueDevice;
    /*!
     A flag (true or false) to indicate whether the device entry should be
     duplicated even for the keys that do not match the key ID lookup data.
     The device descriptors that are pointed by the key device descriptors
     that do not match the key ID lookup data shall not update the frame counter
     based on the frameCounter argument to this function or shall set the
     frame counter to zero when the entry is newly created.
     */
    bool  duplicateDevFlag;
#endif
} ApiMac_secAddDevice_t;

/*! Structure to pass information to the ApiMac_secAddKeyInitFrameCounter(). */
typedef struct _apimac_secaddkeyinitframecounter
{
    /*! Key */
    uint8_t key[APIMAC_KEY_MAX_LEN];
    /*! Frame Counter */
    uint32_t frameCounter;
    /*!
     Key index of the mac security key table where the key needs to be
     written
     */

    uint16_t replaceKeyIndex;

    /*!
     If set to true, the function will duplicate the device table enteries
     associated with the previous key, and associate it with the key.
     If set to false, the function will not alter device table entries
     associated with whatever key that was stored in the key table location
     as designated by replaceKeyIndex.
     */
    bool newKeyFlag;
    /*!
     Key ID lookup data size as it is stored in PIB, i.e., 0 for 5 bytes,
     1 for 9 bytes.
     */
    uint8_t lookupDataSize;
    /*!
     Key ID lookup data, to look for the key table entry and create proper
     key device descriptor for this device.
     */
    uint8_t lookupData[APIMAC_MAX_KEY_LOOKUP_LEN];
#ifdef FEATURE_SECURE_COMMISSIONING
    /* device key or network key */
    bool networkKey;
    /* ext. Address*/
    ApiMac_sAddrExt_t ApiMac_extAddr;
#endif
} ApiMac_secAddKeyInitFrameCounter_t;

/*! MAC_MLME_ASSOCIATE_IND type */
typedef struct _apimac_mlmeassociateind
{
    /*! The address of the device requesting association */
    ApiMac_sAddrExt_t deviceAddress;
    /*! The operational capabilities of the device requesting association */
    ApiMac_capabilityInfo_t capabilityInformation;
    /*! The security parameters for this message */
    ApiMac_sec_t sec;
} ApiMac_mlmeAssociateInd_t;

/*! MAC_MLME_ASSOCIATE_CNF type */
typedef struct _apimac_mlmeassociatecnf
{
    /*! Status of associate attempt */
    ApiMac_assocStatus_t status;
    /*! If successful, the short address allocated to this device */
    uint16_t assocShortAddress;
    /*! The security parameters for this message */
    ApiMac_sec_t sec;
} ApiMac_mlmeAssociateCnf_t;

/*! MAC_MLME_DISASSOCIATE_IND type */
typedef struct _apimac_mlmedisassociateind
{
    /*! The address of the device sending the disassociate command */
    ApiMac_sAddrExt_t deviceAddress;
    /*! The disassociate reason */
    ApiMac_disassocateReason_t disassociateReason;
    /*! The security parameters for this message */
    ApiMac_sec_t sec;
} ApiMac_mlmeDisassociateInd_t;

/*! MAC_MLME_DISASSOCIATE_CNF type */
typedef struct _apimac_mlmediassociatecnf
{
    /*! status of the disassociate attempt */
    ApiMac_status_t status;
    /*!
     The address of the device that has either requested disassociation
     or been instructed to disassociate by its coordinator
     */
    ApiMac_sAddr_t deviceAddress;
    /*!
     The pan ID of the device that has either requested disassociation
     or been instructed to disassociate by its coordinator
     */
    uint16_t panId;
} ApiMac_mlmeDisassociateCnf_t;

/*! MAC Beacon data type */
typedef struct _apimac_beacondata
{
    /*! The number of pending short addresses */
    uint8_t numPendShortAddr;
    /*!
     The list of device short addresses for which the sender of the beacon
     has data
     */
    uint16_t *pShortAddrList;
    /*! The number of pending extended addresses */
    uint8_t numPendExtAddr;
    /*!
     The list of device short addresses for which the sender of the beacon
     has data
     */
    uint8_t *pExtAddrList;
    /*! The number of bytes in the beacon payload of the beacon frame */
    uint8_t sduLength;
    /*! The beacon payload */
    uint8_t *pSdu;
} ApiMac_beaconData_t;

/*! Coexistence Information element content type */
typedef struct _apimac_coexist
{
    /*!
     Beacon Order field shall specify the transmission interval of the beacon
     */
    uint8_t beaconOrder;
    /*!
     Superframe Order field shall specify the length of time during which the
     superframe is active (i.e., receiver enabled), including the Beacon
     frametransmission time
     */
    uint8_t superFrameOrder;
    /*! Final CAP slot */
    uint8_t finalCapSlot;
    /*!
     Enhanced Beacon Order field specifies the transmission interval
     of the Enhanced Beacon frames in a beacon enabled network
     */
    uint8_t eBeaconOrder;
    /*!
     Time offset between periodic beacon and the Enhanced Beacon.
     */
    uint8_t offsetTimeSlot;
    /*!
     Actual slot position in which the Enhanced Beacon frame is
     transmitted due to the backoff procedure in the CAP
     */
    uint8_t capBackOff;
    /*!
     NBPAN Enhanced Beacon Order field specifies the transmission interval
     between consecutive Enhanced Beacon frames in the nonbeacon-enabled mode
     */
    uint16_t eBeaconOrderNBPAN;
} ApiMac_coexist_t;

/*! MAC Enhanced beacon data type */
typedef struct _apimac_ebeacondata
{
    /*! Beacon Coexist data */
    ApiMac_coexist_t coexist;
} ApiMac_eBeaconData_t;

/*! MAC_MLME_BEACON_NOTIFY_IND type */
typedef struct
{
    /*! Indicates the beacon type: beacon or enhanced beacon */
    ApiMac_beaconType_t beaconType;
    /*! The beacon sequence number or enhanced beacon sequence number */
    uint8_t bsn;
    /*! The PAN Descriptor for the received beacon */
    ApiMac_panDesc_t panDesc;
    /*!
     Beacon data union depending on beaconType, select beaconData or
     or eBeaconData.
     */
    union
    {
        /*! beacon data */
        ApiMac_beaconData_t beacon;
        /*! enhanced beacon data */
        ApiMac_eBeaconData_t eBeacon;
    } beaconData;
} ApiMac_mlmeBeaconNotifyInd_t;

/*! MAC_MLME_ORPHAN_IND type */
typedef struct _apimac_mlmeorphanind
{
    /*! The address of the orphaned device */
    ApiMac_sAddrExt_t orphanAddress;
    /*! The security parameters for this message */
    ApiMac_sec_t sec;
} ApiMac_mlmeOrphanInd_t;

/*! MAC_MLME_SCAN_CNF type */
typedef struct _apimac_mlmescancnf
{
    /*! status of the  scan request */
    ApiMac_status_t status;
    /*! The type of scan requested */
    ApiMac_scantype_t scanType;
    /*! The channel page of the scan */
    uint8_t channelPage;
    /*! PHY ID corresponding to the PHY descriptor used during scan */
    uint8_t phyId;
    /*! Bit mask of channels that were not scanned */
    uint8_t unscannedChannels[APIMAC_154G_CHANNEL_BITMAP_SIZ];
    /*! The number of PAN descriptors returned in the results list */
    uint8_t resultListSize;
    /*! Depending on the scanType the results are in this union */
    union
    {
        /*! The list of energy measurements, one for each channel scanned */
        uint8_t *pEnergyDetect;
        /*! The list of PAN descriptors, one for each beacon found */
        ApiMac_panDesc_t *pPanDescriptor;
    } result;
} ApiMac_mlmeScanCnf_t;

/*! MAC_MLME_START_CNF type */
typedef struct _apimac_mlmestartcnf
{
    /*! status of the  start request */
    ApiMac_status_t status;
} ApiMac_mlmeStartCnf_t;

/*! MAC_MLME_SYNC_LOSS_IND type */
typedef struct _apimac_mlmesynclossind
{
    /*! Reason that the synchronization was lost */
    ApiMac_status_t reason;
    /*! The PAN ID of the realignment */
    uint16_t panId;
    /*! The logical channel of the realignment */
    uint8_t logicalChannel;
    /*! The channel page of the realignment */
    uint8_t channelPage;
    /*! PHY ID corresponding to the PHY descriptor of the realignment */
    uint8_t phyID;
    /*! The security parameters for this message */
    ApiMac_sec_t sec;
} ApiMac_mlmeSyncLossInd_t;

/*! MAC_MLME_POLL_CNF type */
typedef struct _apimac_mlmepollcnf
{
    /*! status of the  poll request */
    ApiMac_status_t status;
    /*! Set if framePending bit in data packet is set */
    uint8_t framePending;
} ApiMac_mlmePollCnf_t;

/*! MAC_MLME_COMM_STATUS_IND type */
typedef struct _apimac_mlmecommstatusind
{
    /*! status of the event */
    ApiMac_status_t status;
    /*! The source address associated with the event */
    ApiMac_sAddr_t srcAddr;
    /*! The destination address associated with the event */
    ApiMac_sAddr_t dstAddr;
    /*! The PAN ID associated with the event */
    uint16_t panId;
    /*! The reason the event was generated */
    ApiMac_commStatusReason_t reason;
    /*! The security parameters for this message */
    ApiMac_sec_t sec;
} ApiMac_mlmeCommStatusInd_t;

/*! MAC_MLME_POLL_IND type */
typedef struct _apimac_mlmepollind
{
    /*! Address of the device sending the data request */
    ApiMac_sAddr_t srcAddr;
    /*! Pan ID of the device sending the data request */
    uint16_t srcPanId;
    /*!
     indication that no MAC_McpsDataReq() is required. It is set when
     MAC_MLME_POLL_IND is generated, to simply indicate that a received
     data request frame was acked with pending bit cleared.
     */
    bool noRsp;
} ApiMac_mlmePollInd_t;

/*! MAC_MLME_WS_ASYNC_FRAME_IND type */
typedef ApiMac_mcpsDataInd_t ApiMac_mlmeWsAsyncInd_t;

/*! MAC_MLME_WS_ASYNC_FRAME_CNF type */
typedef struct _apimac_mlmewsasynccnf
{
    /*! status of the  Async request */
    ApiMac_status_t status;
} ApiMac_mlmeWsAsyncCnf_t;

/*!
 Associate Indication Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_associateIndFp_t)(ApiMac_mlmeAssociateInd_t *pAssocInd);

/*!
 Assocate Confirmation Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_associateCnfFp_t)(ApiMac_mlmeAssociateCnf_t *pAssocCnf);

/*!
 Disassociate Indication Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_disassociateIndFp_t)(
                ApiMac_mlmeDisassociateInd_t *pDisassociateInd);

/*!
 Disassociate Confirm Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_disassociateCnfFp_t)(
                ApiMac_mlmeDisassociateCnf_t *pDisassociateCnf);

/*!
 Beacon Notify Indication Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_beaconNotifyIndFp_t)(
                ApiMac_mlmeBeaconNotifyInd_t *pBeaconNotifyInd);

/*!
 Orphan Indication Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_orphanIndFp_t)(ApiMac_mlmeOrphanInd_t *pOrphanInd);

/*!
 Scan Confirmation Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_scanCnfFp_t)(ApiMac_mlmeScanCnf_t *pScanCnf);

/*!
 Start Confirmation Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_startCnfFp_t)(ApiMac_mlmeStartCnf_t *pStartCnf);

/*!
 Sync Loss Indication Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_syncLossIndFp_t)(ApiMac_mlmeSyncLossInd_t *pSyncLossInd);

/*!
 Poll Confirm Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_pollCnfFp_t)(ApiMac_mlmePollCnf_t *pPollCnf);

/*!
 Comm Status Indication Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_commStatusIndFp_t)(
                ApiMac_mlmeCommStatusInd_t *pCommStatus);

/*!
 Poll Indication Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_pollIndFp_t)(ApiMac_mlmePollInd_t *pPollInd);

/*!
 Data Confirmation Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_dataCnfFp_t)(ApiMac_mcpsDataCnf_t *pDataCnf);

/*!
 Data Indication Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_dataIndFp_t)(ApiMac_mcpsDataInd_t *pDataInd);

/*!
 Purge Confirmation Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_purgeCnfFp_t)(ApiMac_mcpsPurgeCnf_t *pPurgeCnf);

/*!
 WiSUN Async Indication Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_wsAsyncIndFp_t)(ApiMac_mlmeWsAsyncInd_t *pWsAsyncInd);

/*!
 WiSUN Async Confirmation Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t)
 */
typedef void (*ApiMac_wsAsyncCnfFp_t)(ApiMac_mlmeWsAsyncCnf_t *pWsAsyncCnf);

/*!
 Unprocessed Message Callback function pointer prototype
 for the [callback table](@ref ApiMac_callbacks_t).  This function will
 be called when an unrecognized message is received.
 */
typedef void (*ApiMac_unprocessedFp_t)(uint16_t param1, uint16_t param2,
                                       void *pMsg);

/*!
 Structure containing all the MAC callbacks (indications).
 To receive the confirmation or indication fill in the
 associated callback with a pointer to the function that
 will handle that callback.  To ignore a callback
 set that function pointer to NULL.
 */
typedef struct _apimac_callbacks
{
    /*! Associate Indicated callback */
    ApiMac_associateIndFp_t pAssocIndCb;
    /*! Associate Confirmation callback */
    ApiMac_associateCnfFp_t pAssocCnfCb;
    /*! Disassociate Indication callback */
    ApiMac_disassociateIndFp_t pDisassociateIndCb;
    /*! Disassociate Confirmation callback */
    ApiMac_disassociateCnfFp_t pDisassociateCnfCb;
    /*! Beacon Notify Indication callback */
    ApiMac_beaconNotifyIndFp_t pBeaconNotifyIndCb;
    /*! Orphan Indication callback */
    ApiMac_orphanIndFp_t pOrphanIndCb;
    /*! Scan Confirmation callback */
    ApiMac_scanCnfFp_t pScanCnfCb;
    /*! Start Confirmation callback */
    ApiMac_startCnfFp_t pStartCnfCb;
    /*! Sync Loss Indication callback */
    ApiMac_syncLossIndFp_t pSyncLossIndCb;
    /*! Poll Confirm callback */
    ApiMac_pollCnfFp_t pPollCnfCb;
    /*! Comm Status Indication callback */
    ApiMac_commStatusIndFp_t pCommStatusCb;
    /*! Poll Indication Callback */
    ApiMac_pollIndFp_t pPollIndCb;
    /*! Data Confirmation callback */
    ApiMac_dataCnfFp_t pDataCnfCb;
    /*! Data Indication callback */
    ApiMac_dataIndFp_t pDataIndCb;
    /*! Purge Confirm callback */
    ApiMac_purgeCnfFp_t pPurgeCnfCb;
    /*! WiSUN Async Indication callback */
    ApiMac_wsAsyncIndFp_t pWsAsyncIndCb;
    /*! WiSUN Async Confirmation callback */
    ApiMac_wsAsyncCnfFp_t pWsAsyncCnfCb;
    /*! Unprocessed message callback */
    ApiMac_unprocessedFp_t pUnprocessedCb;
} ApiMac_callbacks_t;

/******************************************************************************
 Function Prototypes
 *****************************************************************************/

/*!
 * @brief       Initialize this module.
 *
 * @param       enableFH - true to enable frequency hopping, false to not.
 *
 * @return      pointer to a wakeup variable (semaphore in some systems)
 */
extern void *ApiMac_init(bool enableFH);

/*!
 * @brief       Register for MAC callbacks.
 *
 * @param       pCallbacks - pointer to callback structure
 */
extern void ApiMac_registerCallbacks(ApiMac_callbacks_t *pCallbacks);

/*!
 * @brief       Process incoming messages from the MAC stack.
 */
extern void ApiMac_processIncoming(void);

/*!
 * @brief       This function sends application data to the MAC for
 *              transmission in a MAC data frame.
 *              <BR>
 *              The MAC can only buffer a certain number of data request
 *              frames.  When the MAC is congested and cannot accept the data
 *              request it will initiate a callback ([ApiMac_dataCnfFp_t]
 *              (@ref ApiMac_dataCnfFp_t)) with
 *              an overflow status ([ApiMac_status_transactionOverflow]
 *              (@ref ApiMac_status_t)) .  Eventually the MAC will become
 *              uncongested and initiate the callback ([ApiMac_dataCnfFp_t]
 *              (@ref ApiMac_dataCnfFp_t)) for
 *              a buffered request.  At this point the application can attempt
 *              another data request.  Using this scheme, the application can
 *              send data whenever it wants but it must queue data to be resent
 *              if it receives an overflow status.
 *
 * @param       pData - pointer to parameter structure
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_noResources]
 *              (@ref ApiMac_status_noResources) - Resources not available
 */
extern ApiMac_status_t ApiMac_mcpsDataReq(ApiMac_mcpsDataReq_t *pData);

/*!
 * @brief       This function purges and discards a data request from the MAC
 *              data queue.  When the operation is complete the MAC sends a
 *              MCPS Purge Confirm which will initiate a callback
 *              ([ApiMac_purgeCnfFp_t](@ref ApiMac_purgeCnfFp_t)).
 *
 * @param       msduHandle - The application-defined handle value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_noResources]
 *              (@ref ApiMac_status_noResources) - Resources not available
 */
extern ApiMac_status_t ApiMac_mcpsPurgeReq(uint8_t msduHandle);

/*!
 * @brief       This function sends an associate request to a coordinator
 *              device.  The application shall attempt to associate only
 *              with a PAN that is currently allowing association, as indicated
 *              in the results of the scanning procedure. In a beacon-enabled
 *              PAN the beacon order must be set by using ApiMac_mlmeSetReq()
 *              before making the call to ApiMac_mlmeAssociateReq().
 *              <BR>
 *              When the associate request is complete the appliction will
 *              receive the [ApiMac_associateCnfFp_t]
 *              (@ref ApiMac_associateCnfFp_t) callback.
 *
 * @param       pData - Pointer to parameters structure.
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_noResources]
 *              (@ref ApiMac_status_noResources) - Resources not available
 */
extern ApiMac_status_t ApiMac_mlmeAssociateReq(
                ApiMac_mlmeAssociateReq_t *pData);

/*!
 * @brief       This function sends an associate response to a device
 *              requesting to associate.  This function must be called after
 *              the [ApiMac_associateIndFp_t]
 *              (@ref ApiMac_associateIndFp_t) callback.  When the associate
 *              response is complete the callback [ApiMac_commStatusIndFp_t]
 *              (@ref ApiMac_commStatusIndFp_t) is
 *              called to indicate the success or failure of the operation.
 *
 * @param       pData - Pointer to parameters structure.
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_noResources]
 *              (@ref ApiMac_status_noResources) - Resources not available
 */
extern ApiMac_status_t ApiMac_mlmeAssociateRsp(
                ApiMac_mlmeAssociateRsp_t *pData);

/*!
 * @brief       This function is used by an associated device to notify the
 *              coordinator of its intent to leave the PAN.  It is also used by
 *              the coordinator to instruct an associated device to leave the
 *              PAN.  When the disassociate procedure is complete the
 *              applications callback [ApiMac_disassociateCnfFp_t]
 *              (@ref ApiMac_disassociateCnfFp_t) is called.
 *
 * @param       pData - Pointer to parameters structure.
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_noResources]
 *              (@ref ApiMac_status_noResources) - Resources not available
 */
extern ApiMac_status_t ApiMac_mlmeDisassociateReq(
                ApiMac_mlmeDisassociateReq_t *pData);

/*!
 * @brief       This direct execute function retrieves an attribute value from
 *              the MAC PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 *
 * @return      The status of the request
 */
extern ApiMac_status_t ApiMac_mlmeGetReqBool(
                ApiMac_attribute_bool_t pibAttribute,
                bool *pValue);

/*!
 * @brief       This direct execute function retrieves an attribute value from
 *              the MAC PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 *
 * @return      The status of the request
 */
extern ApiMac_status_t ApiMac_mlmeGetReqUint8(
                ApiMac_attribute_uint8_t pibAttribute,
                uint8_t *pValue);

/*!
 * @brief       This direct execute function retrieves an attribute value from
 *              the MAC PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 *
 * @return      The status of the request
 */
extern ApiMac_status_t ApiMac_mlmeGetReqUint16(
                ApiMac_attribute_uint16_t pibAttribute,
                uint16_t *pValue);

/*!
 * @brief       This direct execute function retrieves an attribute value from
 *              the MAC PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 *
 * @return      The status of the request
 */
extern ApiMac_status_t ApiMac_mlmeGetReqUint32(
                ApiMac_attribute_uint32_t pibAttribute,
                uint32_t *pValue);

/*!
 * @brief       This direct execute function retrieves an attribute value from
 *              the MAC PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 *
 * @return      The status of the request
 */
extern ApiMac_status_t ApiMac_mlmeGetReqArray(
                ApiMac_attribute_array_t pibAttribute,
                uint8_t *pValue);

/*!
 * @brief       This direct execute function retrieves an attribute value from
 *              the MAC Frequency Hopping PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupportedAttribute]
 *              (@ref ApiMac_status_unsupportedAttribute) - Attribute not found
 */
extern ApiMac_status_t ApiMac_mlmeGetFhReqUint8(
                ApiMac_FHAttribute_uint8_t pibAttribute, uint8_t *pValue);

/*!
 * @brief       This direct execute function retrieves an attribute value from
 *              the MAC Frequency Hopping PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupportedAttribute]
 *              (@ref ApiMac_status_unsupportedAttribute) - Attribute not found
 */
extern ApiMac_status_t ApiMac_mlmeGetFhReqUint16(
                ApiMac_FHAttribute_uint16_t pibAttribute, uint16_t *pValue);

/*!
 * @brief       This direct execute function retrieves an attribute value from
 *              the MAC Frequency Hopping PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupportedAttribute]
 *              (@ref ApiMac_status_unsupportedAttribute) - Attribute not found
 */
extern ApiMac_status_t ApiMac_mlmeGetFhReqUint32(
                ApiMac_FHAttribute_uint32_t pibAttribute, uint32_t *pValue);

/*!
 * @brief       This direct execute function retrieves an attribute value from
 *              the MAC Frequency Hopping PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupportedAttribute]
 *              (@ref ApiMac_status_unsupportedAttribute) - Attribute not found
 */
extern ApiMac_status_t ApiMac_mlmeGetFhReqArray(
                ApiMac_FHAttribute_array_t pibAttribute, uint8_t *pValue);

/*!
 * @brief       This direct execute function retrieves an attribute value from
 *              the MAC Secutity PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupportedAttribute]
 *              (@ref ApiMac_status_unsupportedAttribute) - Attribute not found
 */
extern ApiMac_status_t ApiMac_mlmeGetSecurityReqUint8(
                ApiMac_securityAttribute_uint8_t pibAttribute, uint8_t *pValue);

/*!
 * @brief       This direct execute function retrieves an attribute value from
 *              the MAC Secutity PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupportedAttribute]
 *              (@ref ApiMac_status_unsupportedAttribute) - Attribute not found
 */
extern ApiMac_status_t ApiMac_mlmeGetSecurityReqUint16(
             ApiMac_securityAttribute_uint16_t pibAttribute, uint16_t *pValue);

/*!
 * @brief       This direct execute function retrieves an attribute value from
 *              the MAC Secutity PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupportedAttribute]
 *              (@ref ApiMac_status_unsupportedAttribute) - Attribute not found
 */
extern ApiMac_status_t ApiMac_mlmeGetSecurityReqArray(
             ApiMac_securityAttribute_array_t pibAttribute, uint8_t *pValue);

/*!
 * @brief       This direct execute function retrieves an attribute value from
 *              the MAC Secutity PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupportedAttribute]
 *              (@ref ApiMac_status_unsupportedAttribute) - Attribute not found
 */
extern ApiMac_status_t ApiMac_mlmeGetSecurityReqStruct(
             ApiMac_securityAttribute_struct_t pibAttribute, void *pValue);

/*!
 * @brief       This function is called in response to an orphan notification
 *              from a peer device.  This function must be called after
 *              receiving an [Orphan Indication Callback]
 *              (@ref ApiMac_orphanIndFp_t).
 *              When the orphan response is complete
 *              the [Comm Status Indication Callback]
 *              (@ref ApiMac_commStatusIndFp_t) is called to indicate the success
 *              or failure of the operation.
 *
 *
 * @param       pData - Pointer to parameters structure.
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_noResources]
 *              (@ref ApiMac_status_noResources) - Resources not available
 */
extern ApiMac_status_t ApiMac_mlmeOrphanRsp(ApiMac_mlmeOrphanRsp_t *pData);

/*!
 * @brief       This function is used to request pending data from the
 *              coordinator.  When the poll request is complete the
 *              [Poll Confirm Callback](@ref ApiMac_pollCnfFp_t)
 *              is called.  If a data frame of nonzero length is received from
 *              the coordinator the [Poll Confirm Callback]
 *              (@ref ApiMac_pollCnfFp_t) has a status ApiMac_status_success and
 *              then calls the [Data Indication Callback]
 *              (@ref ApiMac_dataIndFp_t) for the received data.
 *
 *
 * @param       pData - Pointer to parameters structure.
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_noResources]
 *              (@ref ApiMac_status_noResources) - Resources not available
 */
extern ApiMac_status_t ApiMac_mlmePollReq(ApiMac_mlmePollReq_t *pData);

/*!
 * @brief       This direct execute function resets the MAC.  This function
 *              must be called once at system startup before any other
 *              function in the management API is called.
 *
 *
 * @param       setDefaultPib - Set to TRUE to reset the MAC PIB to its
 *                              default values.
 *
 * @return      always [ApiMac_status_success](@ref ApiMac_status_success)
 */
extern ApiMac_status_t ApiMac_mlmeResetReq(bool setDefaultPib);

/*!
 * @brief       This function initiates an energy detect, active, passive, or
 *              orphan scan on one or more channels.  An energy detect scan
 *              measures the peak energy on each requested channel.  An active
 *              scan sends a beacon request on each channel and then listening
 *              for beacons.  A passive scan is a receive-only operation that
 *              listens for beacons on each channel.  An orphan scan is used to
 *              locate the coordinator with which the scanning device had
 *              previously associated.  When a scan operation is complete the
 *              [Scan Confirm callback](@ref ApiMac_scanCnfFp_t) is called.
 *              <BR>
 *              For active or passive scans the application sets the maxResults
 *              parameter the maximum number of PAN descriptors to return.  If
 *              maxResults is greater than zero then the application must also
 *              set result.panDescriptor to point to a buffer of size
 *              maxResults * sizeof([ApiMac_panDesc_t](@ref ApiMac_panDesc_t))
 *              to store the results of
 *              the scan.  The application must not access or deallocate this
 *              buffer until the [Scan Confirm Callback]
 *              (@ref ApiMac_scanCnfFp_t) is called.  The MAC will store up to
 *              maxResults PAN descriptors and ignore duplicate beacons.
 *              <BR>
 *              An alternative way to get results for an active or passive scan
 *              is to set maxResults to zero or set PIB attribute
 *              ApiMac_attribute_autoRequest to FALSE.  Then the MAC will not
 *              store results but rather call the [Beacon Notify Indication
 *              Callback](@ref ApiMac_beaconNotifyIndFp_t) for each beacon
 *              received.  The application will not need to supply any memory
 *              to store the scan results but the MAC will not filter out
 *              duplicate beacons.
 *              <BR>
 *              For energy detect scans the application must set
 *              result.energyDetect to point to a buffer of size 18 bytes to
 *              store the results of the scan.  The application must not
 *              access or deallocate this buffer until the
 *              [Scan Confirm Callback](@ref ApiMac_scanCnfFp_t) is called.
 *              <BR>
 *              An energy detect, active or passive scan may be performed
 *              at any time if a scan is not already in progress.  However a
 *              device cannot perform any other MAC management operation or
 *              send or receive MAC data until the scan is complete.
 *
 * @param       pData - Pointer to parameters structure.
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_scanInProgress]
 *              (@ref ApiMac_status_scanInProgress) - already scanning<BR>
 *              [ApiMac_status_noResources]
 *              (@ref ApiMac_status_noResources) - memory allocation error
 *
 */
extern ApiMac_status_t ApiMac_mlmeScanReq(ApiMac_mlmeScanReq_t *pData);

/*!
 * @brief       This direct execute function sets an attribute value
 *              in the MAC PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       value - the attribute value
 *
 * @return      The status of the request
 */
extern ApiMac_status_t ApiMac_mlmeSetReqBool(
                ApiMac_attribute_bool_t pibAttribute,
                bool value);

/*!
 * @brief       This direct execute function sets an attribute value
 *              in the MAC PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       value - the attribute value
 *
 * @return      The status of the request
 */
extern ApiMac_status_t ApiMac_mlmeSetReqUint8(
                ApiMac_attribute_uint8_t pibAttribute,
                uint8_t value);

/*!
 * @brief       This direct execute function sets an attribute value
 *              in the MAC PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       value - the attribute value
 *
 * @return      The status of the request
 */
extern ApiMac_status_t ApiMac_mlmeSetReqUint16(
                ApiMac_attribute_uint16_t pibAttribute,
                uint16_t value);

/*!
 * @brief       This direct execute function sets an attribute value
 *              in the MAC PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       value - the attribute value
 *
 * @return      The status of the request
 */
extern ApiMac_status_t ApiMac_mlmeSetReqUint32(
                ApiMac_attribute_uint32_t pibAttribute,
                uint32_t value);

/*!
 * @brief       This direct execute function sets an attribute value
 *              in the MAC PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - the attribute value
 *
 * @return      The status of the request
 */
extern ApiMac_status_t ApiMac_mlmeSetReqArray(
                ApiMac_attribute_array_t pibAttribute,
                uint8_t *pValue);

/*!
 * @brief       This direct execute function sets an attribute value
 *              in the MAC Frequency Hopping PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       value - the attribute value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupportedAttribute]
 *              (@ref ApiMac_status_unsupportedAttribute) - Attribute not found
 */
extern ApiMac_status_t ApiMac_mlmeSetFhReqUint8(
                ApiMac_FHAttribute_uint8_t pibAttribute, uint8_t value);

/*!
 * @brief       This direct execute function sets an attribute value
 *              in the MAC Frequency Hopping PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       value - the attribute value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupportedAttribute]
 *              (@ref ApiMac_status_unsupportedAttribute) - Attribute not found
 */
extern ApiMac_status_t ApiMac_mlmeSetFhReqUint16(
                ApiMac_FHAttribute_uint16_t pibAttribute, uint16_t value);

/*!
 * @brief       This direct execute function sets an attribute value
 *              in the MAC Frequency Hopping PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       value - the attribute value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupportedAttribute]
 *              (@ref ApiMac_status_unsupportedAttribute) - Attribute not found
 */
extern ApiMac_status_t ApiMac_mlmeSetFhReqUint32(
                ApiMac_FHAttribute_uint32_t pibAttribute, uint32_t value);

/*!
 * @brief       This direct execute function sets an attribute value
 *              in the MAC Frequency Hopping PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupportedAttribute]
 *              (@ref ApiMac_status_unsupportedAttribute) - Attribute not found
 */
extern ApiMac_status_t ApiMac_mlmeSetFhReqArray(
                ApiMac_FHAttribute_array_t pibAttribute, uint8_t *pValue);

/*!
 * @brief       This direct execute function sets an attribute value
 *              in the MAC Security PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       value - the attribute value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupportedAttribute]
 *              (@ref ApiMac_status_unsupportedAttribute) - Attribute not found
 */
extern ApiMac_status_t ApiMac_mlmeSetSecurityReqUint8(
                ApiMac_securityAttribute_uint8_t pibAttribute, uint8_t value);

/*!
 * @brief       This direct execute function sets an attribute value
 *              in the MAC Security PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       value - the attribute value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupportedAttribute]
 *              (@ref ApiMac_status_unsupportedAttribute) - Attribute not found
 */
extern ApiMac_status_t ApiMac_mlmeSetSecurityReqUint16(
                ApiMac_securityAttribute_uint16_t pibAttribute, uint16_t value);

/*!
 * @brief       This direct execute function sets an attribute value
 *              in the MAC Security PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupportedAttribute]
 *              (@ref ApiMac_status_unsupportedAttribute) - Attribute not found
 */
extern ApiMac_status_t ApiMac_mlmeSetSecurityReqArray(
                ApiMac_securityAttribute_array_t pibAttribute, uint8_t *pValue);

/*!
 * @brief       This direct execute function sets an attribute value
 *              in the MAC Security PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupportedAttribute]
 *              (@ref ApiMac_status_unsupportedAttribute) - Attribute not found
 */
extern ApiMac_status_t ApiMac_mlmeSetSecurityReqStruct(
                ApiMac_securityAttribute_struct_t pibAttribute, void *pValue);

/*!
 * @brief       This function is called by a coordinator or PAN coordinator
 *              to start or reconfigure a network.  Before starting a network
 *              the device must have set its short address.  A PAN coordinator
 *              sets the short address by setting the attribute
 *              [ApiMac_attribute_shortAddress]
 *              (@ref ApiMac_attribute_shortAddress).  A coordinator sets the
 *              short address through association.
 *              <BR>
 *              When parameter panCoordinator is TRUE, the MAC automatically
 *              sets attributes ApiMac_attribute_panID and
 *              [ApiMac_attribute_logicalChannel]
 *              (@ref ApiMac_attribute_logicalChannel) to the panId and
 *              logicalChannel
 *              parameters.  If panCoordinator is FALSE, these parameters are
 *              ignored (they would already be set through association).
 *              <BR>
 *              The parameter beaconOrder controls whether the network is
 *              beacon-enabled or non beacon-enabled.  For a beacon-enabled
 *              network this parameter also controls the beacon transmission
 *              interval.
 *              <BR>
 *              When the operation is complete the [Start Confirm Callback]
 *              (@ref ApiMac_startCnfFp_t) is called.
 *
 * @param       pData - Pointer to parameters structure.
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_noResources]
 *              (@ref ApiMac_status_noResources) - Resources not available
 */
extern ApiMac_status_t ApiMac_mlmeStartReq(ApiMac_mlmeStartReq_t *pData);

/*!
 * @brief       This function requests the MAC to synchronize with the
 *              coordinator by acquiring and optionally tracking its beacons.
 *              Synchronizing with the coordinator is recommended before
 *              associating in a beacon-enabled network.  If the beacon could
 *              not be located on its initial search or during tracking, the MAC
 *              calls the [Sync Loss Indication Callback]
 *              (@ref ApiMac_syncLossIndFp_t) with [ApiMac_status_beaconLoss]
 *              (@ref ApiMac_status_beaconLoss) as the reason.
 *              <BR>
 *              Before calling this function the application must set PIB
 *              attributes [ApiMac_attribute_beaconOrder]
 *              (@ref ApiMac_attribute_beaconOrder), [ApiMac_attribute_panId]
 *              (@ref ApiMac_attribute_panId)
 *              and either [ApiMac_attribute_coordShortAddress]
 *              (@ref ApiMac_attribute_coordShortAddress) or
 *              [ApiMac_attribute_coordExtendedAddress]
 *              (@ref ApiMac_attribute_coordExtendedAddress) to the address
 *              of the coordinator with which to synchronize.
 *              <BR>
 *              The application may wish to set PIB attribute
 *              [ApiMac_attribute_autoRequest]
 *              (@ref ApiMac_attribute_autoRequest) to FALSE before calling this
 *              function.  Then when the MAC successfully synchronizes with
 *              the coordinator it will call the [Beacon Notify Indication
 *              Callback] (@ref ApiMac_beaconNotifyIndFp_t).  After receiving
 *              the callback the application may set
 *              ApiMac_attribute_autoRequest to TRUE to stop receiving
 *              beacon notifications.
 *              <BR>
 *              This function is only applicable to beacon-enabled networks.
 *
 * @param       pData - Pointer to parameters structure.
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_noResources]
 *              (@ref ApiMac_status_noResources) - Resources not available
 */
extern ApiMac_status_t ApiMac_mlmeSyncReq(ApiMac_mlmeSyncReq_t *pData);

/*!
 * @brief       This function returns a random byte from the MAC random number
 *              generator.
 *
 * @return      A random byte.
 */
extern uint8_t ApiMac_randomByte(void);

/*!
 * @brief       Update Device Table entry and PIB with new Pan Id.
 *
 * @param       panId - the new Pan ID
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_noResources]
 *              (@ref ApiMac_status_noResources) - Resources not available
 */
extern ApiMac_status_t ApiMac_updatePanId(uint16_t panId);

#if defined(COMBO_MAC) || defined(FREQ_2_4G)
/*!
 * @brief       Enable source match for auto ack and pending.
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_noResources]
 *              (@ref ApiMac_status_noResources) - Resources not available
 */
extern ApiMac_status_t ApiMac_srcMatchEnable(void);
#endif

/*!
 * @brief       This functions handles a WiSUN async request.
 *              The possible operation is Async Start or Async Stop.
 *              For the async start operation, the caller of this function
 *              can indicate which WiSUN async frame type to be
 *              sent on the specified channels.
 *
 * @param       pData pointer to the asynchronous parameters structure
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_noResources]
 *              (@ref ApiMac_status_noResources) - Resources not available
 */
extern ApiMac_status_t ApiMac_mlmeWSAsyncReq(ApiMac_mlmeWSAsyncReq_t* pData);

/*!
 * @brief       This function starts the frequency hopping. Frequency hopping
 *              operation should have been enabled using ApiMac_enableFH()
 *              before calling this API.  No need to call this API if you
 *              have called ApiMac_mlmeStartReq() with the startFH field set to
 *              true.
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_noResources]
 *              (@ref ApiMac_status_noResources) - Resources not available
 */
extern ApiMac_status_t ApiMac_startFH(void);

/*!
 * @brief Parses the Group payload information element.
 *        This function creates a linked list (plist) from the Payload IE
 *        (pPayload). Each item in the linked list
 *        is a seperate Group IE with its own content.
 *        <BR>
 *        If no IEs are found pList will be set to NULL.
 *        <BR>
 *        The caller is responsible to release the memory for
 *        the linked list by calling ApiMac_freeIEList().
 *        <BR>
 *        Call this function to create the list of Group IEs, then
 *        call ApiMac_parsePayloadSubIEs() to parse each of the group IE's
 *        content into sub IEs.
 *
 * @param pPayload - pointer to the buffer with the payload IEs.
 * @param payloadLen - length of the buffer with the payload IEs.
 * @param pList - pointer to link list pointer.
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_noData](@ref ApiMac_status_noData)
 *              - pPayload or payloadLen is NULL,<BR>
 *              [ApiMac_status_unsupported](@ref ApiMac_status_unsupported)
 *              - invalid field found,<BR>
 *              [ApiMac_status_noResources](@ref ApiMac_status_noResources)
 *              - if memory allocation fails.
 */
extern ApiMac_status_t ApiMac_parsePayloadGroupIEs(
                uint8_t *pPayload, uint16_t payloadLen,
                ApiMac_payloadIeRec_t **pList);

/*!
 * @brief Parses the payload sub information element.
 *        This function creates a linked list (pList) of sub IEs from
 *        the Group IE content (pContent).   Each item in the linked list
 *        is a seperate sub IE with its own content.
 *        <BR>
 *        If no IEs are found pList will be set to NULL.
 *        <BR>
 *        The caller is responsible to release the memory for
 *        the linked list by calling ApiMac_freeIEList().
 *        <BR>
 *        Call this function after calling ApiMac_parsePayloadGroupIEs().
 *
 * @param pContent - pointer to the buffer with the sub IEs.
 * @param contentLen - length of the buffer with the payload IEs.
 * @param pList - pointer to link list pointer.
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_noData](@ref ApiMac_status_noData)
 *              - pPayload or payloadLen is NULL,<BR>
 *              [ApiMac_status_unsupported](@ref ApiMac_status_unsupported)
 *              - invalid field found,<BR>
 *              [ApiMac_status_noResources](@ref ApiMac_status_noResources)
 *              - if memory allocation fails.
 */
extern ApiMac_status_t ApiMac_parsePayloadSubIEs(uint8_t *pContent,
                                                 uint16_t contentLen,
                                                 ApiMac_payloadIeRec_t **pList);

/*!
 * @brief Free the linked list allocated by ApiMac_parsePayloadGroupIEs()
 *        or ApiMac_parsePayloadSubIEs().
 *
 * @param pList - pointer to linked list
 */
extern void ApiMac_freeIEList(ApiMac_payloadIeRec_t *pList);

/*!
 * @brief       Enables the Frequency hopping operation.  Make sure you call
 *              this function before setting any FH parameters or before
 *              calling ApiMac_mlmeStartReq() or ApiMac_startFH(), if you're
 *              using FH.
 *
 * @return      The status of the request, as follows:<BR>
 *              [ApiMac_status_success](@ref ApiMac_status_success)
 *               - Operation successful<BR>
 *              [ApiMac_status_unsupported](@ref ApiMac_status_unsupported)
 *              - feature not available.
 */
extern ApiMac_status_t ApiMac_enableFH(void);

/*!
 * @brief       Convert ApiMac_capabilityInfo_t data type to uint8 capInfo
 *
 * @param       pMsgcapInfo - CapabilityInfo pointer
 *
 * @return      capInfo bit mask byte
 */
extern uint8_t ApiMac_convertCapabilityInfo(
                ApiMac_capabilityInfo_t *pMsgcapInfo);

/*!
 * @brief      Convert from bitmask byte to API MAC capInfo
 *
 * @param       cInfo - source
 * @param       pPBcapInfo - destination
 */
extern void ApiMac_buildMsgCapInfo(uint8_t cInfo,
                                   ApiMac_capabilityInfo_t *pPBcapInfo);

/*!
 * @brief      Adds a new MAC device table entry.
 *
 * @param      pAddDevice - Add device information
 *
 * @return     [ApiMac_status_success](@ref ApiMac_status_success) if
 *             successful, other status value if not.
 */
extern ApiMac_status_t ApiMac_secAddDevice(ApiMac_secAddDevice_t *pAddDevice);

/*!
 * @brief      Removes MAC device table entries.
 *
 * @param      pExtAddr - extended address of the device table entries
 *                        that shall be removed
 *
 * @return     [ApiMac_status_success](@ref ApiMac_status_success) if
 *             successful, other status value if not.
 */
extern ApiMac_status_t ApiMac_secDeleteDevice(ApiMac_sAddrExt_t *pExtAddr);

/*!
 * @brief      Removes the key at the specified key Index and removes all
 *             MAC device table enteries associated with this key. Also
 *             removes(initializes) the key lookup list associated with
 *             this key.
 *
 * @param      keyIndex - mac secuirty key table index of the key
 *                        to be removed.
 *
 * @return     [ApiMac_status_success](@ref ApiMac_status_success) if
 *             successful, other status value if not.
 */
extern ApiMac_status_t ApiMac_secDeleteKeyAndAssocDevices(uint8_t keyIndex);

/*!
 * @brief      Removes all MAC device table entries.
 *
 * @return     [ApiMac_status_success](@ref ApiMac_status_success) if
 *             successful, other status value if not.
 */
extern ApiMac_status_t ApiMac_secDeleteAllDevices(void);

/*!
 * @brief      Reads the frame counter value associated with a MAC security key
 *             indexed by the designated key identifier and the default key
 *             source.
 *
 * @param      keyId - Key ID.
 * @param      pFrameCounter - pointer to a buffer to store the outgoing
 *                             frame counter of the key.
 *
 * @return     [ApiMac_status_success](@ref ApiMac_status_success) if
 *             successful, other status value if not.
 */
extern ApiMac_status_t ApiMac_secGetDefaultSourceKey(uint8_t keyId,
                                                     uint32_t *pFrameCounter);

/*!
 * @brief      Adds the MAC security key, adds the associated lookup list
 *             for the key, initializes the frame counter to the value
 *             provided. It also duplicates the device table enteries
 *             (associated with the previous key if any) if available based
 *             on the flag dupDevFlag value and associates the device
 *             descriptor with this key.
 *
 * @param      pInfo - structure need to perform this function.
 *
 * @return     [ApiMac_status_success](@ref ApiMac_status_success) if
 *             successful, other status value if not.
 */
extern ApiMac_status_t ApiMac_secAddKeyInitFrameCounter(
                ApiMac_secAddKeyInitFrameCounter_t *pInfo);

#ifdef __cplusplus
}
#endif

#endif /* API_MAC_H */

