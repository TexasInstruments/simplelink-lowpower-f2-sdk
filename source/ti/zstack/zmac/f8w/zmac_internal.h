/******************************************************************************

 @file  zmac_internal.h

 @brief This file contains the ZStack MAC Porting Layer.

 Group: CMCU, LPRF
 Target Device: CC2652

 ******************************************************************************

 Copyright (c) 2005-2017, Texas Instruments Incorporated

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

 ******************************************************************************
 Release Name: simplelink_zigbee_sdk_plugin_0_95_00_18_s
 Release Date: 2017-11-10 13:43:40
 *****************************************************************************/

#ifndef ZMAC_INTERNAL_H
#define ZMAC_INTERNAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/********************************************************************************************************
 *                                               INCLUDES
 ********************************************************************************************************/

#include "mac_api.h"
#include "ti_zstack_config.h"

/********************************************************************************************************
 *                                                DEFINES
 ********************************************************************************************************/

// MAC Type Indication
#define ZMAC_F8W

// PHY transiver output power values
#define OUTPUT_POWER_0DBM       0x00
#define OUTPUT_POWER_N1DBM      0x21
#define OUTPUT_POWER_N3DBM      0x23
#define OUTPUT_POWER_N5DBM      0x25
#define OUTPUT_POWER_N7DBM      0x27
#define OUTPUT_POWER_N10DBM     0x2A
#define OUTPUT_POWER_N15DBM     0x2F
#define OUTPUT_POWER_N25DBM     0x39

// MAC PIB Attributes
enum
{
  ZMacAckWaitDuration                   = MAC_ACK_WAIT_DURATION,
  ZMacAssociationPermit                 = MAC_ASSOCIATION_PERMIT,
  ZMacAutoRequest                       = MAC_AUTO_REQUEST,
  ZMacBattLifeExt                       = MAC_BATT_LIFE_EXT,
  ZMacBattLeftExtPeriods                = MAC_BATT_LIFE_EXT_PERIODS,

  ZMacBeaconMSDU                        = MAC_BEACON_PAYLOAD,
  ZMacBeaconMSDULength                  = MAC_BEACON_PAYLOAD_LENGTH,
  ZMacBeaconOrder                       = MAC_BEACON_ORDER,
  ZMacBeaconTxTime                      = MAC_BEACON_TX_TIME,
  ZMacBSN                               = MAC_BSN,

  ZMacCoordExtendedAddress              = MAC_COORD_EXTENDED_ADDRESS,
  ZMacCoordShortAddress                 = MAC_COORD_SHORT_ADDRESS,
  ZMacDSN                               = MAC_DSN,
  ZMacGTSPermit                         = MAC_GTS_PERMIT,
  ZMacMaxCSMABackoffs                   = MAC_MAX_CSMA_BACKOFFS,

  ZMacMinBE                             = MAC_MIN_BE,
  ZMacPanId                             = MAC_PAN_ID,
  ZMacPromiscuousMode                   = MAC_PROMISCUOUS_MODE,
  ZMacRxOnIdle                          = MAC_RX_ON_WHEN_IDLE,
  ZMacShortAddress                      = MAC_SHORT_ADDRESS,

  ZMacSuperframeOrder                   = MAC_SUPERFRAME_ORDER,
  ZMacTransactionPersistenceTime        = MAC_TRANSACTION_PERSISTENCE_TIME,
  ZMacAssociatedPanCoord                = MAC_ASSOCIATED_PAN_COORD,
  ZMacMaxBE                             = MAC_MAX_BE,
  ZMacMaxFrameTotalWaitTime             = MAC_MAX_FRAME_TOTAL_WAIT_TIME,

  ZMacMaxFrameRetries                   = MAC_MAX_FRAME_RETRIES,
  ZMacResponseWaitTime                  = MAC_RESPONSE_WAIT_TIME,
  ZMacSyncSymbolOffset                  = MAC_SYNC_SYMBOL_OFFSET,
  ZMacTimestampSupported                = MAC_TIMESTAMP_SUPPORTED,
  ZMacSecurityEnabled                   = MAC_SECURITY_ENABLED,

  // Proprietary Items
  ZMacPhyTransmitPowerSigned            = MAC_PHY_TRANSMIT_POWER_SIGNED,
  ZMacChannel                           = MAC_LOGICAL_CHANNEL,
  ZMacExtAddr                           = MAC_EXTENDED_ADDRESS,
  ZMacAltBE                             = MAC_ALT_BE,
  ZMacDeviceBeaconOrder                 = MAC_DEVICE_BEACON_ORDER,
  ZMacRf4cePowerSavings                 = MAC_RF4CE_POWER_SAVINGS,
  ZMacFrameVersionSupport               = MAC_FRAME_VERSION_SUPPORT,

  // Diagnostics Items
  ZMacDiagsRxCrcPass                    = MAC_DIAGS_RX_CRC_PASS,
  ZMacDiagsRxCrcFail                    = MAC_DIAGS_RX_CRC_FAIL,
  ZMacDiagsRxBcast                      = MAC_DIAGS_RX_BCAST,
  ZMacDiagsTxBcast                      = MAC_DIAGS_TX_BCAST,
  ZMacDiagsRxUcast                      = MAC_DIAGS_RX_UCAST,
  ZMacDiagsTxUcast                      = MAC_DIAGS_TX_UCAST,
  ZMacDiagsTxUcastRetry                 = MAC_DIAGS_TX_UCAST_RETRY,
  ZMacDiagsTxUcastFail                  = MAC_DIAGS_TX_UCAST_FAIL,

#ifdef FEATURE_MAC_SECURITY
  ZMacKeyTable                          = MAC_KEY_TABLE,
  ZMacKeyTableEntries                   = MAC_KEY_TABLE_ENTRIES,
  ZMacDeviceTable                       = MAC_DEVICE_TABLE,
  ZMacDeviceTableEntries                = MAC_DEVICE_TABLE_ENTRIES,
  ZMacSecurityLevelTable                = MAC_SECURITY_LEVEL_TABLE,

  ZMacSecurityLevelTableEntries         = MAC_SECURITY_LEVEL_TABLE_ENTRIES,
  ZMacFrameCounter                      = MAC_FRAME_COUNTER,
  ZMacAutoRequestSecurityLevel          = MAC_AUTO_REQUEST_SECURITY_LEVEL,
  ZMacAutoRequestKeyIdMode              = MAC_AUTO_REQUEST_KEY_ID_MODE,
  ZMacAutoRequestKeySource              = MAC_AUTO_REQUEST_KEY_SOURCE,

  ZMacAutoRequestKeyIndex               = MAC_AUTO_REQUEST_KEY_INDEX,
  ZMacDefaultKeySource                  = MAC_DEFAULT_KEY_SOURCE,
  ZMacPanCoordExtendedAddress           = MAC_PAN_COORD_EXTENDED_ADDRESS,
  ZMacPanCoordShortAddress              = MAC_PAN_COORD_SHORT_ADDRESS,

  ZMacKeyIdLookupEntry                  = MAC_KEY_ID_LOOKUP_ENTRY,
  ZMacKeyDeviceEntry                    = MAC_KEY_DEVICE_ENTRY,
  ZMacKeyUsageEntry                     = MAC_KEY_USAGE_ENTRY,
  ZMacKeyEntry                          = MAC_KEY_ENTRY,
  ZMacDeviceEntry                       = MAC_DEVICE_ENTRY,
  ZMacSecurityLevelEntry                = MAC_SECURITY_LEVEL_ENTRY,
#endif /* FEATURE_MAC_SECURITY */

#ifdef IEEE_COEX_ENABLED
  ZMacCoexPIBMetrics                    = MAC_COEX_METRICS,
#endif /* IEEE_COEX_ENABLED */

  // Junk
  ZMacACLDefaultSecurityMaterialLength  = 0,     // not implemented
  ZMacTxGTSId                           = 1,     // not implemented
  ZMacUpperLayerType                    = 2,     // not implemented
  ZMacRxGTSId                           = 3,     // not implemented
  ZMacSnoozePermit                      = 4      // not implemented
};

typedef uint8_t ZMacAttributes_t;

// Status type
typedef uint8_t ZMacStatus_t;

/* Definition of scan type */
#define ZMAC_ED_SCAN                   MAC_SCAN_ED
#define ZMAC_ACTIVE_SCAN               MAC_SCAN_ACTIVE
#define ZMAC_PASSIVE_SCAN              MAC_SCAN_PASSIVE
#define ZMAC_ORPHAN_SCAN               MAC_SCAN_ORPHAN

/* Adding Enhanced Active Scan request/ Enhanced beacon request */
#define ZMAC_ENHANCED_ACTIVE_SCAN      MAC_SCAN_ACTIVE_ENHANCED
// Association Status Field Values
#define ZMAC_SUCCESSFUL_ASSOCIATION    0x00
#define ZMAC_PAN_AT_CAPACITY           0x01
#define ZMAC_PAN_ACCESS_DENIED         0x02

// Disassociation Reason Codes
#define ZMAC_COORD_INITIATE            MAC_DISASSOC_COORD
#define ZMAC_DEVICE_INITIATE           MAC_DISASSOC_DEVICE

#define ZMAC_SECURITY_USE              0x01
#define ZMAC_ACL_ENTRY                 0x02
#define ZMAC_SECURITY_FAILURE          0x04

// TX Option flags
#define ZMAC_TXOPTION_ACK              MAC_TXOPTION_ACK
#define ZMAC_TXOPTION_GTS              MAC_TXOPTION_GTS
#define ZMAC_TXOPTION_INDIRECT         MAC_TXOPTION_INDIRECT
#define ZMAC_TXOPTION_SECURITY_ENABLE  MAC_TXOPTION_SECURITY
#define ZMAC_TXOPTION_NO_RETRANS       MAC_TXOPTION_NO_RETRANS
#define ZMAC_TXOPTION_NO_CNF           MAC_TXOPTION_NO_CNF
#define ZMAC_TXOPTION_GREEN_PWR        MAC_TXOPTION_GREEN_PWR

#define ZMAC_ASSOC_CAPINFO_FFD_TYPE    MAC_CAPABLE_FFD
#define ZMAC_ASSOC_CAPINFO_RX_ON_IDLE  MAC_CAPABLE_RX_ON_IDLE
#define ZMAC_ASSOC_CAPINFO_SECURITY    MAC_CAPABLE_SECURITY

// LQI to Cost mapping
#define MIN_LQI_COST_1  12     //24
#define MIN_LQI_COST_2  9      //20
#define MIN_LQI_COST_3  6      //16
#define MIN_LQI_COST_4  4      //12
#define MIN_LQI_COST_5  2      //8
#define MIN_LQI_COST_6  1      //4
#define MIN_LQI_COST_7  0      //0

/* Number of bytes to allocate for ED scan; matches ED_SCAN_MAXCHANNELS in nwk.h */
#define ZMAC_ED_SCAN_MAXCHANNELS       27

#define ZMAC_SUCCESS                   MAC_SUCCESS
#define ZMAC_TRANSACTION_OVERFLOW      MAC_TRANSACTION_OVERFLOW
#define ZMAC_TRANSACTION_EXPIRED       MAC_TRANSACTION_EXPIRED
#define ZMAC_CHANNEL_ACCESS_FAILURE    MAC_CHANNEL_ACCESS_FAILURE
#define ZMAC_NO_RESOURCES              MAC_NO_RESOURCES
#define ZMAC_NO_ACK                    MAC_NO_ACK
#define ZMAC_COUNTER_ERROR             MAC_COUNTER_ERROR
#define ZMAC_INVALID_PARAMETER         MAC_INVALID_PARAMETER

#define ZMAC_KEY_SOURCE_MAX_LEN        MAC_KEY_SOURCE_MAX_LEN

/********************************************************************************************************
 *                                            TYPE DEFINITIONS
 ********************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZMAC_INTERNAL_H */
