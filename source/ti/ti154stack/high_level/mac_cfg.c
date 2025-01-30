/******************************************************************************

 @file  mac_cfg.c

 @brief Compile-time parameters which are configurable by the user.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2005-2025, Texas Instruments Incorporated

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
 
 
 *****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#ifndef LIBRARY
#include "ti_154stack_features.h"
#ifndef COPROCESSOR
#include "ti_154stack_config.h"
#endif
#endif


#include <stddef.h>
#include <stdint.h>
#include "mac_spec.h"
#include "mac_security_pib.h"
#include "mac_pib.h"
#include "fh_pib.h"
#include "mac_radio.h"
#include "ti_radio_config.h"

/* Use 2_4G default settings for CC2652P build */
#ifdef COPROCESSOR_CC2652P
#ifndef FREQ_2_4G
#define FREQ_2_4G
#endif
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

// Coprocessor is not supported with the custom PHY feature yet. This is a temp solution.
// If coprocessor is not defined, use the sysconfig values from ti_154stack_config.h
// If coprocessor is defined, set the PHY settings to WB-DSSS PHY
#if !defined(COPROCESSOR) && defined(ENABLE_CUSTOM_PHY)
#define MAC_CUSTOM_BAND_MODE_2_CENTER_FREQ_KHZ          CENTER_FREQUENCY_CUSTOM
#define MAC_CUSTOM_BAND_MODE_2_CHAN_SPACING_KHZ         CHANNEL_SPACING_CUSTOM
#define MAC_CUSTOM_BAND_MODE_2_TOTAL_CHANNELS           NUM_CHANNELS_CUSTOM
#define MAC_CCA_TYPE_CUSTOM                             CCA_TYPE_CUSTOM
#else 
#define MAC_CUSTOM_BAND_MODE_2_CENTER_FREQ_KHZ          903000
#define MAC_CUSTOM_BAND_MODE_2_CHAN_SPACING_KHZ         1600
#define MAC_CUSTOM_BAND_MODE_2_TOTAL_CHANNELS           17
#define MAC_MODULATION_INDEX_CUSTOM                     1
#define MAC_CCA_TYPE_CUSTOM                             1
#define SYMBOL_RATE_CUSTOM                              480
#endif
uint16_t symbol_rate_custom = SYMBOL_RATE_CUSTOM;
#define MAC_MODULATION_INDEX_CUSTOM                     1

/* maximum number of data frames in transmit queue */
#ifndef MAC_CFG_TX_DATA_MAX
#define MAC_CFG_TX_DATA_MAX         2
#endif

/* maximum number of frames of all types in transmit queue */
#ifndef MAC_CFG_TX_MAX
#define MAC_CFG_TX_MAX              5
#endif

/* maximum number of frames in receive queue */
#ifndef MAC_CFG_RX_MAX
#define MAC_CFG_RX_MAX              2
#endif

/* allocate additional bytes in the data indication for application-defined headers */
#ifndef MAC_CFG_DATA_IND_OFFSET
#define MAC_CFG_DATA_IND_OFFSET     0
#endif

/* determine whether MAC_MLME_POLL_IND will be sent to the application */
#ifndef MAC_CFG_APP_PENDING_QUEUE
#define MAC_CFG_APP_PENDING_QUEUE   FALSE
#endif

/* Maximum PHY packet size */
#ifndef MAC_MAX_FRAME_SIZE
#ifndef FREQ_2_4G
#define MAC_MAX_FRAME_SIZE          500
#else
#define MAC_MAX_FRAME_SIZE          125
#endif
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* configurable parameters */
macCfg_t macCfg =
{
  MAC_CFG_TX_DATA_MAX,
  MAC_CFG_TX_MAX,
  MAC_CFG_RX_MAX,
  MAC_CFG_DATA_IND_OFFSET,
  MAX_DEVICE_TABLE_ENTRIES,
  MAX_KEY_DEVICE_TABLE_ENTRIES,
  MAX_KEY_TABLE_ENTRIES,
  MAX_NODE_KEY_ENTRIES,
  MAX_KEY_ID_LOOKUP_ENTRIES,
  MAC_CFG_APP_PENDING_QUEUE,
  MAC_MAX_FRAME_SIZE
};

/* PIB default values */
const macPib_t macPibDefaults =
{
  54,                                         /* ackWaitDuration */
  FALSE,                                      /* associationPermit */
  TRUE,                                       /* autoRequest */
  FALSE,                                      /* battLifeExt */
  6,                                          /* battLifeExtPeriods */

  NULL,                                       /* *pMacBeaconPayload */
  0,                                          /* beaconPayloadLength */
  MAC_BO_NON_BEACON,                          /* beaconOrder */
  0,                                          /* beaconTxTime */
  0,                                          /* bsn */

  {0, SADDR_MODE_EXT},                        /* coordExtendedAddress */
  MAC_SHORT_ADDR_NONE,                        /* coordShortAddress */
  0,                                          /* dsn */
  FALSE,                                      /* gtsPermit */
  4,                                          /* maxCsmaBackoffs */

  MAC_DEFAULT_MIN_BE,                         /* minBe */
  0xFFFF,                                     /* panId */
  FALSE,                                      /* promiscuousMode */
  FALSE,                                      /* rxOnWhenIdle */
  MAC_SHORT_ADDR_NONE,                        /* shortAddress */

  MAC_SO_NONE,                                /* superframeOrder */
  1000,                                       /* transactionPersistenceTime */
  FALSE,                                      /* assocciatedPanCoord */
  MAC_DEFAULT_MAX_BE,                         /* maxBe */
  MAC_DEFAULT_FRAME_WAIT_TIME,                /* maxFrameTotalWaitTime */
  3,                                          /* maxFrameRetries */
  MAC_2P4G_RESPONSE_WAIT_TIME,                /* responseWaitTime */
  0,                                          /* syncSymbolOffset */
  TRUE,                                       /* timeStampSupported */
#ifdef FEATURE_MAC_SECURITY
  TRUE,                                      /* securityEnabled */
#else
  FALSE,                                      /* securityEnabled */
#endif
  0,                                          /* ebsn */
  0,                                          /* Enhanced beacon order */
  16383,                                      /* Non Beacon PAN enhanced beacon order */
  15,                                         /* ehanced beacon offset time slot */
  FALSE,                                      /* MPM IE to be included */

  MAC_PREAMBLE_LEN_8,                         /* FSK presamble len, 4 to 31 */
  0,                                          /* MR-FSK PHY SFD, valid values 0, 1 */

#ifndef FREQ_2_4G
  14,                                         /* phyTransmitPower */
#else
  5,                                          /* phyTransmitPower */
#endif
#ifndef FREQ_2_4G
  MAC_CHAN_LOWEST,                            /* logicalChannel */
#else
  11,                                         /* logicalChannel */
#endif
  {0, SADDR_MODE_EXT},                        /* extendedAddress */
  1,                                          /* altBe */
  MAC_BO_NON_BEACON,                          /* deviceBeaconOrder */
  0,                                          /* power savings for rf4ce */
  0,                                          /* default is frame Ver is always 0,
                                                 except for security enabled packets */
#ifndef FREQ_2_4G
  9,                                          /* default channel page */
  MAC_50KBPS_915MHZ_PHY_1,                    /* current PHY Descriptor ID */
#else
  0,                                          /* default channel page */
  0,                                          /* current PHY Descriptor ID */
#endif
  0,                                          /* fcsType */

  0,                                          /* diagsRxCrcPass */
  0,                                          /* diagsRxCrcFail */
  0,                                          /* diagsRxBcast */
  0,                                          /* diagsTxBcast */
  0,                                          /* diagsRxUcast */
  0,                                          /* diagsTxUcast */
  0,                                          /* diagsTxUcastRetry */
  0,                                          /* diagsTxUcastFail */
  0,                                          /* diagsRxSecFail */
  0,                                          /* diagsTxSecFail */
  -83,                                        /* LBT RSSI Threshold */
  0,                                          /* range extender mode */
  0,                                          /* enable Ack Pending for Data Pkts */
#ifndef FREQ_2_4G
  MAC_RF_FREQ_SUBG,                           /* rf freq */
#else
  MAC_RF_FREQ_2_4G,                           /* rf freq */
#endif
  0                                           /* PA type */
};


/* PIB access and min/max table.  min/max of 0/0 means not checked; if min/max are
 * equal, element is read-only
 */
const macPibTbl_t macPibTbl[] =
{
  {offsetof(macPib_t, ackWaitDuration), sizeof(uint8), 54, 54},                      /* MAC_ACK_WAIT_DURATION */
  {offsetof(macPib_t, associationPermit), sizeof(bool), FALSE, TRUE},                /* MAC_ASSOCIATION_PERMIT */
  {offsetof(macPib_t, autoRequest), sizeof(bool), FALSE, TRUE},                      /* MAC_AUTO_REQUEST */
  {offsetof(macPib_t, battLifeExt), sizeof(bool), FALSE, TRUE},                      /* MAC_BATT_LIFE_EXT */
  {offsetof(macPib_t, battLifeExtPeriods), sizeof(uint8), 6, 6},                     /* MAC_BATT_LIFE_EXT_PERIODS */

  {offsetof(macPib_t, pBeaconPayload), sizeof(uint8 *), 0, 0},                       /* MAC_BEACON_PAYLOAD */
  {offsetof(macPib_t, beaconPayloadLength), sizeof(uint8), 0, MAC_PIB_MAX_BEACON_PAYLOAD_LEN}, /* MAC_BEACON_PAYLOAD_LENGTH */
  {offsetof(macPib_t, beaconOrder), sizeof(uint8), 0, 15},                           /* MAC_BEACON_ORDER */
  {offsetof(macPib_t, beaconTxTime), sizeof(uint32), 1, 1},                          /* MAC_BEACON_TX_TIME */
  {offsetof(macPib_t, bsn), sizeof(uint8), 0x00, 0xFF},                              /* MAC_BSN */

  {offsetof(macPib_t, coordExtendedAddress.addr.extAddr), sizeof(sAddrExt_t), 0, 0}, /* MAC_COORD_EXTENDED_ADDRESS */
  {offsetof(macPib_t, coordShortAddress), sizeof(uint16), 0, 0},                     /* MAC_COORD_SHORT_ADDRESS */
  {offsetof(macPib_t, dsn), sizeof(uint8), 0x00, 0xFF},                              /* MAC_DSN */
  {offsetof(macPib_t, gtsPermit), sizeof(bool), FALSE, TRUE},                        /* MAC_GTS_PERMIT */

  /* Range of maxCsmaBackoffs is between 0 and 5 in IEEE 802.15.4.
   * Such restriction is removed here to allow 802.15.4 non-compliant experimental
   * applications.
   * To be compliant with 802.15.4, application or upper layer must not set this
   * PIB attribute out of range of 802.15.4.
   * The range is still restricted to 254 since value 255 would cause backoff
   * counter to overflow, wrap around and loop infinitely. */
  {offsetof(macPib_t, maxCsmaBackoffs), sizeof(uint8), 0, 254},                      /* MAC_MAX_CSMA_BACKOFFS */

  {offsetof(macPib_t, minBe), sizeof(uint8), 0, 8},                                  /* MAC_MIN_BE */
  {offsetof(macPib_t, panId), sizeof(uint16), 0, 0},                                 /* MAC_PAN_ID */
  {offsetof(macPib_t, promiscuousMode), sizeof(bool), FALSE, TRUE},                  /* MAC_PROMISCUOUS_MODE */
  {offsetof(macPib_t, rxOnWhenIdle), sizeof(bool), FALSE, TRUE},                     /* MAC_RX_ON_WHEN_IDLE */
  {offsetof(macPib_t, shortAddress), sizeof(uint16), 0, 0},                          /* MAC_SHORT_ADDRESS */

  {offsetof(macPib_t, superframeOrder), sizeof(uint8), 0, 15},                       /* MAC_SUPERFRAME_ORDER */
  {offsetof(macPib_t, transactionPersistenceTime), sizeof(uint16), 0, 0},            /* MAC_TRANSACTION_PERSISTENCE_TIME */
  {offsetof(macPib_t, associatedPanCoord), sizeof(bool), FALSE, TRUE},               /* MAC_ASSOCIATED_PAN_COORD */

  /* Range of maxBe is between 3 and 8 in IEEE 802.15.4.
   * Such restriction is removed here to allow 802.15.4 non-compliant experimental
   * applications.
   * To be compliant with 802.15.4, application or upper layer must not set this
   * PIB attribute out of range of 802.15.4. */
  {offsetof(macPib_t, maxBe), sizeof(uint8), 0, 8},                                  /* MAC_MAX_BE */
  {offsetof(macPib_t, maxFrameTotalWaitTime), sizeof(uint16), 0x00, 0xFF},           /* MAC_MAX_FRAME_RESPONSE_TIME */

  {offsetof(macPib_t, maxFrameRetries), sizeof(uint8), 0, 7},                        /* MAC_MAX_FRAME_RETRIES */
  {offsetof(macPib_t, responseWaitTime), sizeof(uint8), 2, 255},                     /* MAC_RESPONSE_WAIT_TIME */
  {offsetof(macPib_t, syncSymbolOffset), sizeof(uint8), 0, 0},                       /* MAC_SYNC_SYMBOL_OFFSET */
  {offsetof(macPib_t, timeStampSupported), sizeof(bool), FALSE, TRUE},               /* MAC_TIMESTAMP_SUPPORTED */
  {offsetof(macPib_t, securityEnabled), sizeof(bool), FALSE, TRUE},                  /* MAC_SECURITY_ENABLED */

  {offsetof(macPib_t, ebsn), sizeof(uint8), 0x00, 0xFF},                             /* MAC_EBSN                */
  {offsetof(macPib_t, eBeaconOrder), sizeof(uint8), 0, 15 },                         /* MAC_EBEACON_ORDER       */
  {offsetof(macPib_t, eBeaconOrderNBPAN), sizeof(uint16), 0x00, 0xFF},               /* MAC_EBEACON_ORDER_NBPAN */
  {offsetof(macPib_t, offsetTimeSlot), sizeof(uint8), 1, 15},                        /* MAC_OFFSET_TIMESLOT     */
  {offsetof(macPib_t, includeMPMIE), sizeof(bool), FALSE, TRUE},                     /* MAC_INCLUDE_MPMIE       */

  {offsetof(macPib_t, fskPreambleLen), sizeof(uint8), MAC_MIN_PREAMBLE_LEN, MAC_MAX_PREAMBLE_LEN},              /* MAC_PHY_FSK_PREAMBLE_LEN */
  {offsetof(macPib_t, mrFSKSFD), sizeof(uint8), 0, 1},                               /* MAC_PHY_MRFSKSFD        */

  /* Proprietary PIBs */
  {offsetof(macPib_t, phyTransmitPower), sizeof(uint8), 0, 0xFF},                   /* MAC_PHY_TRANSMIT_POWER_SIGNED */
  {offsetof(macPib_t, logicalChannel), sizeof(uint8), MAC_CHAN_LOWEST, MAC_CHAN_HIGHEST},     /* MAC_LOGICAL_CHANNEL */
  {offsetof(macPib_t, extendedAddress.addr.extAddr), sizeof(sAddrExt_t), 0, 0},      /* MAC_EXTENDED_ADDRESS */
  {offsetof(macPib_t, altBe), sizeof(uint8), 0, 8},                                  /* MAC_ALT_BE */
  {offsetof(macPib_t, deviceBeaconOrder), sizeof(uint8), 0, 15},                     /* MAC_DEVICE_BEACON_ORDER */
  {offsetof(macPib_t, rf4cepowerSavings), sizeof(uint8), 0, 1},                      /* MAC_RF4CE_POWER_SAVINGS */
  {offsetof(macPib_t, frameVersionSupport), sizeof(uint8), 0, 0xFF},                 /* MAC_FRAME_VERSION_SUPPORT */
#ifndef FREQ_2_4G
  {offsetof(macPib_t, channelPage), sizeof(uint8), 0, 10},                            /* MAC_CHANNEL_PAGE */
  {offsetof(macPib_t, curPhyID), sizeof(uint8), 0, MAC_MRFSK_GENERIC_PHY_ID_END}, /* MAC_PHY_CURRENT_DESCRIPTOR_ID */
#else
  {offsetof(macPib_t, channelPage), sizeof(uint8), 0, 0},                            /* MAC_CHANNEL_PAGE */
  {offsetof(macPib_t, curPhyID), sizeof(uint8), 0, 0},                               /* MAC_PHY_CURRENT_DESCRIPTOR_ID */
#endif
  {offsetof(macPib_t, fcsType), sizeof(bool), FALSE, TRUE },                         /* MAC_FCS_TYPE */

  {offsetof(macPib_t, diagsRxCrcPass), sizeof(uint32), 0, 0},                        /* MAC_DIAGS_RX_CRC_PASS */
  {offsetof(macPib_t, diagsRxCrcFail), sizeof(uint32), 0, 0},                        /* MAC_DIAGS_RX_CRC_FAIL */
  {offsetof(macPib_t, diagsRxBcast), sizeof(uint32), 0, 0},                          /* MAC_DIAGS_RX_BCAST */
  {offsetof(macPib_t, diagsTxBcast), sizeof(uint32), 0, 0},                          /* MAC_DIAGS_TX_BCAST */
  {offsetof(macPib_t, diagsRxUcast), sizeof(uint32), 0, 0},                          /* MAC_DIAGS_RX_UCAST */
  {offsetof(macPib_t, diagsTxUcast), sizeof(uint32), 0, 0},                          /* MAC_DIAGS_TX_UCAST */
  {offsetof(macPib_t, diagsTxUcastRetry), sizeof(uint32), 0, 0},                     /* MAC_DIAGS_TX_UCAST_RETRY */
  {offsetof(macPib_t, diagsTxUcastFail), sizeof(uint32), 0, 0},                      /* MAC_DIAGS_TX_UCAST_FAIL */
  {offsetof(macPib_t, diagsRxSecFail), sizeof(uint32), 0, 0},                        /* MAC_DIAGS_RX_SEC_FAIL */
  {offsetof(macPib_t, diagsTxSecFail), sizeof(uint32), 0, 0},                        /* MAC_DIAGS_TX_SEC_FAIL */
  {offsetof(macPib_t, rssiThreshold), sizeof(int8), 0, 0xFF},                        /* MAC_RSSI_THRESHOLD */
  {offsetof(macPib_t, rangeExt), sizeof(uint8), 0, 1},                               /* MAC_RANGE_EXTENDER */
  {offsetof(macPib_t, enDataAckPending), sizeof(uint8), 0, 1},                       /* DATA ACK PENDING */
#ifndef FREQ_2_4G
  {offsetof(macPib_t, rfFreq), sizeof(uint8), MAC_RF_FREQ_SUBG, MAC_RF_FREQ_2_4G},   /* MAC_RF_BAND */
#else
  {offsetof(macPib_t, rfFreq), sizeof(uint8), MAC_RF_FREQ_2_4G, MAC_RF_FREQ_2_4G},   /* MAC_RF_BAND */
#endif
  {offsetof(macPib_t, paType), sizeof(uint8), 0, 1},                                 /* MAC_RF_PA_TYPE */
};

#ifdef FEATURE_MAC_SECURITY
#ifdef STATIC_MAC_SECURITY_DEFAULTS
/* Security related PIB default values */
const macSecurityPib_t macSecurityPibDefaults =
{
  0,                                                 /* keyTableEntries */
  0,                                                 /* deviceTableEntries */
  0,                                                 /* securityLevelTableEntries */
  0x00,                                              /* autoRequestSecurityLevel */
  0,                                                 /* autoRequestKeyIdMode */
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* autoRequestKeySource */
  0xFF,                                              /* autoRequestKeyIndex */
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* defaultKeySource */
  {0, SADDR_MODE_EXT},                               /* panCoordExtendedAddress */
  MAC_SHORT_ADDR_NONE,                               /* panCoordShortAddress */
  {                                                  /* macKeyTable */
    {NULL, 0, NULL, 0, NULL, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
    {NULL, 0, NULL, 0, NULL, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}
  },
  {                                                  /* macKeyIdLookupList */
    {{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00}},
    {{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00}}
  },
  {                                                  /* macKeyDeviceList */
    {{0x00, false, false}, {0x00, false, false}, {0x00, false, false}  /* The rest, if any, will be initialized to zeros */},
    {{0x00, false, false}, {0x00, false, false}, {0x00, false, false}  /* The rest, if any, will be initialized to zeros */}
  },
  {
    {{MAC_FRAME_TYPE_DATA, MAC_DATA_REQ_FRAME},{MAC_FRAME_TYPE_COMMAND, MAC_DATA_REQ_FRAME}}, /* macKeyUsageList */
    {{MAC_FRAME_TYPE_DATA, MAC_DATA_REQ_FRAME},{MAC_FRAME_TYPE_COMMAND, MAC_DATA_REQ_FRAME}}
  },
  {                                                  /* macDeviceTable */
    {0x0000, 0xFFFF, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, 0, 0, FALSE},
    {0x0000, 0xFFFF, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, 0, 0, FALSE},
    {0x0000, 0xFFFF, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, 0, 0, FALSE}
    /* The rest, if any, will be initialized to zeros */
  },
  {                                                   /* macSecurityLevelTable */
    {MAC_FRAME_TYPE_DATA, MAC_DATA_REQ_FRAME, MAC_SEC_LEVEL_ENC_MIC_32, TRUE},
    {MAC_FRAME_TYPE_COMMAND, MAC_DATA_REQ_FRAME, MAC_SEC_LEVEL_ENC_MIC_32, TRUE}
  }
};
#endif

/* PIB access and min/max table.  min/max of 0/0 means not checked; if min/max are
 * equal, element is read-only
 */
const macSecurityPibTbl_t macSecurityPibTbl[] =
{
  {offsetof(macSecurityPib_t, keyTableEntries), sizeof(uint16), 0, MAX_KEY_TABLE_ENTRIES},         /* MAC_KEY_TABLE_ENTRIES */
  {offsetof(macSecurityPib_t, deviceTableEntries), sizeof(uint16), 0, MAX_DEVICE_TABLE_ENTRIES},   /* MAC_DEVICE_TABLE_ENTRIES */
  {offsetof(macSecurityPib_t, securityLevelTableEntries), sizeof(uint8), 0, MAX_SECURITY_LEVEL_TABLE_ENTRIES},
                                                                                                  /* MAC_SECURITY_LEVEL_TABLE_ENTRIES */
  {0, 0, 0, 0},                                                                                   /* MAC_FRAME_COUNTER */
  {offsetof(macSecurityPib_t, autoRequestSecurityLevel), sizeof(uint8), 0, 7},                    /* MAC_AUTO_REQUEST_SECURITY_LEVEL */
  {offsetof(macSecurityPib_t, autoRequestKeyIdMode), sizeof(uint8), 0, 3},                        /* MAC_AUTO_REQUEST_KEY_ID_MODE */
  {offsetof(macSecurityPib_t, autoRequestKeySource), MAC_KEY_SOURCE_MAX_LEN*sizeof(uint8), 0, 0}, /* MAC_AUTO_REQUEST_KEY_SOURCE*/
  {offsetof(macSecurityPib_t, autoRequestKeyIndex), sizeof(uint8), 0x01, 0xFF},                   /* MAC_AUTO_REQUEST_KEY_INDEX */
  {offsetof(macSecurityPib_t, defaultKeySource), MAC_KEY_SOURCE_MAX_LEN*sizeof(uint8), 0, 0},     /* MAC_DEFAULT_KEY_SOURCE */
  {offsetof(macSecurityPib_t, panCoordExtendedAddress), sizeof(sAddrExt_t), 0, 0},                /* MAC_PAN_COORD_EXTENDED_ADDRESS */
  {offsetof(macSecurityPib_t, panCoordShortAddress), sizeof(uint16), 0, 0},                       /* MAC_PAN_COORD_SHORT_ADDRESS */
};
#endif


#ifdef FEATURE_FREQ_HOP_MODE
/*!
 FH PIB default
 */
const FHPIB_DB_t FHPIB_defaults =
{
    /*! macTrackParentEUI */
    FHPIB_DEFAULT_EUI,
    /*! macBcInterval : 4.25s */
    4250,
    /*! macUcExcludedChannels */
    {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00,
    },
    /*! macBcExcludedChannels */
    {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00,
    },
    /*! macUcDwellInterval */
    250,
    /*! macBcDwellInterval */
    250,
    /*! macClockDrift */
    20,
    /*! macTimingAccuracy */
    0x00,
    /*! macUcChannelFunction */
    0x00,
    /*! macBcChannelFunction */
    0x00,
    /*! macUseParentBSIE */
    0x00,
    /*! macBroadcastSchedId */
    0x0000,
    /*! macUcFixedChannel */
    0x0000,
    /*! macBcFixedChannel */
    0x0000,
    /*! macPanSize */
    0x0001,
    /*! macRoutingCost */
    0x00,
    /*! macRoutingMethod : 1(L3 RPL), 0(L2 MHDS)*/
    0x01,
    /*! macEapolReady : 1(ready to accept EAPOL message)*/
    0x01,
    /*! macFanTpsVersion : 1(WiSUN FAN version 1.0) */
    0x00,
    /*! macNetName */
    {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    /*! macPanVersion */
    0x0000,
    /*! macGtk0Hash */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /*! macGtk1Hash */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /*! macGtk2Hash */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /*! macGtk3Hash */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /*! macNeighborValidTime : 120 mins*/
    120,
    /*! macBaseBackOff : 0 */
    0x08,
    /*! maximum non-sleep nodes */
    2,
    /*! maximum sleep node */
    (FHPIB_MAX_NUM_DEVICE -2),
    /*! maximum temp node */
#ifndef FEATURE_MAC_SECURITY
/*! Maximum number of temp Nodes */
    MAX_DEVICE_TABLE_ENTRIES,
#else
    /*! Maximum number of fixed Nodes */
    10,
#endif

};

/*!
 FH PIB table
 */
const FH_PibTbl_t FH_PibTbl[] =
{
    /*!< FHPIB_TRACK_PARENT_EUI */
    {offsetof(FHPIB_DB_t, macTrackParentEUI), sizeof(sAddrExt_t), 0, 0},
    /*!< FHPIB_BC_INTERVAL */
    {offsetof(FHPIB_DB_t, macBcInterval), sizeof(uint32_t), 250, 16777215},
    /*!< FHPIB_UC_EXCLUDED_CHANNELS */
    {offsetof(FHPIB_DB_t, macUcExcludedChannels),
              FHPIB_MAX_BIT_MAP_SIZE*sizeof(uint8_t), 0, 0},
    /*!< FHPIB_BC_EXCLUDED_CHANNELS */
    {offsetof(FHPIB_DB_t, macBcExcludedChannels),
              FHPIB_MAX_BIT_MAP_SIZE*sizeof(uint8_t), 0, 0},
    /*!< FHPIB_UC_DWELL_INTERVAL */
    {offsetof(FHPIB_DB_t, macUcDwellInterval), sizeof(uint8_t), 15, 250},
    /*!< FHPIB_BC_DWELL_INTERVAL */
    {offsetof(FHPIB_DB_t, macBcDwellInterval), sizeof(uint8_t), 0, 250},
    /*!< FHPIB_CLOCK_DRIFT */
    {offsetof(FHPIB_DB_t, macClockDrift), sizeof(uint8_t), 0, 255},
    /*!< FHPIB_TIMING_ACCURACY */
    {offsetof(FHPIB_DB_t, macTimingAccuracy), sizeof(uint8_t), 0, 255},
    /*!< FHPIB_UC_CHANNEL_FUNCTION */
    {offsetof(FHPIB_DB_t, macUcChannelFunction), sizeof(uint8_t), 0, 3},
    /*!< FHPIB_BC_CHANNEL_FUNCTION */
    {offsetof(FHPIB_DB_t, macBcChannelFunction), sizeof(uint8_t), 0, 3},
    /*!< FHPIB_USE_PARENT_BS_IE */
    {offsetof(FHPIB_DB_t, macUseParentBSIE), sizeof(uint8_t), 0, 1},
    /*!< FHPIB_BROCAST_SCHED_ID */
    {offsetof(FHPIB_DB_t, macBroadcastSchedId), sizeof(uint16_t), 0, 65535},
    /*!< FHPIB_UC_FIXED_CHANNEL */
    {offsetof(FHPIB_DB_t, macUcFixedChannel), sizeof(uint16_t), 0, 255},
    /*!< FHPIB_BC_FIXED_CHANNEL */
    {offsetof(FHPIB_DB_t, macBcFixedChannel), sizeof(uint16_t), 0, 255},
    /*!< FHPIB_PAN_SIZE */
    {offsetof(FHPIB_DB_t, macPanSize), sizeof(uint16_t), 0, 65535},
    /*!< FHPIB_ROUTING_COST */
    {offsetof(FHPIB_DB_t, macRoutingCost), sizeof(uint8_t), 0, 255},
    /*!< FHPIB_ROUTING_METHOD */
    {offsetof(FHPIB_DB_t, macRoutingMethod), sizeof(uint8_t), 0, 1},
    /*!< FHPIB_EAPOL_READY */
    {offsetof(FHPIB_DB_t, macEapolReady), sizeof(uint8_t), 0, 1},
    /*!< FHPIB_FAN_TPS_VERSION */
    {offsetof(FHPIB_DB_t, macFanTpsVersion), sizeof(uint8_t), 0, 7},
    /*!< FHPIB_NET_NAME */
    {offsetof(FHPIB_DB_t, macNetName),
              FHPIB_NET_NAME_SIZE_MAX*sizeof(uint8_t), 0, 0},
    /*!< FHPIB_PAN_VERSION */
    {offsetof(FHPIB_DB_t, macPanVersion), sizeof(uint16_t), 0, 65535},
    /*!< FHPIB_GTK_0_HASH */
    {offsetof(FHPIB_DB_t, macGtk0Hash),
              FHPIB_GTK_HASH_SIZE*sizeof(uint8_t), 0, 0},
    /*!< FHPIB_GTK_1_HASH */
    {offsetof(FHPIB_DB_t, macGtk1Hash),
              FHPIB_GTK_HASH_SIZE*sizeof(uint8_t), 0, 0},
    /*!< FHPIB_GTK_2_HASH */
    {offsetof(FHPIB_DB_t, macGtk2Hash),
              FHPIB_GTK_HASH_SIZE*sizeof(uint8_t), 0, 0},
    /*!< FHPIB_GTK_3_HASH */
    {offsetof(FHPIB_DB_t, macGtk3Hash),
              FHPIB_GTK_HASH_SIZE*sizeof(uint8_t), 0, 0},
    /*!< FHPIB_HOPPING_NEIGHBOR_VALID_TIME */
    {offsetof(FHPIB_DB_t, macNeighborValidTime), sizeof(uint16_t), 5, 600},
    /*!< CSMA Base Backoff */
    {offsetof(FHPIB_DB_t, macBaseBackOff), sizeof(uint8_t), 0, 16},
    /*!< maximum non-sleep nodes */
    {offsetof(FHPIB_DB_t, macMaxNonSleepNodes), sizeof(uint16_t), 0, 50},
    /*!< maximum sleep node */
    {offsetof(FHPIB_DB_t, macMaxSleepNodes), sizeof(uint16_t), 0, 50},
    /*!< maximum temp table node */
    {offsetof(FHPIB_DB_t, macMaxTempNodes), sizeof(uint16_t), 0, 10},

};
#else
/*!
 FH PIB default
 */
const FHPIB_DB_t FHPIB_defaults = {0};

/*!
 FH PIB table
 */
const FH_PibTbl_t FH_PibTbl[1];
#endif

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)

/* MAC 15.4g Standard PHY Descriptor Table */
CODE const macMRFSKPHYDesc_t macMRFSKStdPhyTable[MAC_STANDARD_PHY_DESCRIPTOR_ENTRIES] =
{
  { MAC_915MHZ_BAND_MODE_1_CENTER_FREQ_KHZ, MAC_915MHZ_BAND_MODE_1_CHAN_SPACING_KHZ,
    MAC_915MHZ_BAND_MODE_1_TOTAL_CHANNELS, MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_50_K,
    MAC_MODULATION_INDEX_2FSK_50K,
    MAC_CCA_TYPE_CSMA_CA },
  { MAC_915MHZ_BAND_MODE_2_CENTER_FREQ_KHZ, MAC_915MHZ_BAND_MODE_2_CHAN_SPACING_KHZ,
    MAC_915MHZ_BAND_MODE_2_TOTAL_CHANNELS, MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_150_K,
    MAC_MODULATION_INDEX_2FSK_150K,
    MAC_CCA_TYPE_CSMA_CA },
  { MAC_868MHZ_BAND_MODE_1_CENTER_FREQ_KHZ, MAC_868MHZ_BAND_MODE_1_CHAN_SPACING_KHZ,
    MAC_868MHZ_BAND_MODE_1_TOTAL_CHANNELS, MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_50_K,
    MAC_MODULATION_INDEX_2FSK_50K,
    MAC_CCA_TYPE_LBT },
};

/* MAC 15.4g Generic PHY Descriptor Table */
macMRFSKPHYDesc_t macMRFSKGenPhyTable[MAC_GENERIC_PHY_DESCRIPTOR_ENTRIES] =
{
 { MAC_433MHZ_BAND_MODE_1_CENTER_FREQ_KHZ, MAC_433MHZ_BAND_MODE_1_CHAN_SPACING_KHZ,
   MAC_433MHZ_BAND_MODE_1_TOTAL_CHANNELS, MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_50_K,
   MAC_MODULATION_INDEX_2FSK_50K,
   MAC_CCA_TYPE_CSMA_CA },
 { MAC_5KBPS_915MHZ_BAND_MODE_1_CENTER_FREQ_KHZ, MAC_5KBPS_915MHZ_BAND_MODE_1_CHAN_SPACING_KHZ,
   MAC_5KBPS_915MHZ_BAND_MODE_1_TOTAL_CHANNELS, MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_20_K,
   MAC_MODULATION_INDEX_2FSK_5K,
   MAC_CCA_TYPE_CSMA_CA },
 { MAC_5KBPS_433MHZ_BAND_MODE_1_CENTER_FREQ_KHZ, MAC_5KBPS_433MHZ_BAND_MODE_1_CHAN_SPACING_KHZ,
   MAC_5KBPS_433MHZ_BAND_MODE_1_TOTAL_CHANNELS, MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_20_K,
   MAC_MODULATION_INDEX_2FSK_5K,
   MAC_CCA_TYPE_CSMA_CA },
 { MAC_5KBPS_868MHZ_BAND_MODE_1_CENTER_FREQ_KHZ, MAC_5KBPS_868MHZ_BAND_MODE_1_CHAN_SPACING_KHZ,
   MAC_5KBPS_868MHZ_BAND_MODE_1_TOTAL_CHANNELS, MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_20_K,
   MAC_MODULATION_INDEX_2FSK_5K,
   MAC_CCA_TYPE_LBT },
 { MAC_915MHZ_BAND_MODE_3_CENTER_FREQ_KHZ, MAC_915MHZ_BAND_MODE_3_CHAN_SPACING_KHZ,
   MAC_915MHZ_BAND_MODE_3_TOTAL_CHANNELS, MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_200_K,
   MAC_MODULATION_INDEX_2FSK_200K,
   MAC_CCA_TYPE_CSMA_CA },
 { MAC_868MHZ_BAND_MODE_2_CENTER_FREQ_KHZ, MAC_868MHZ_BAND_MODE_2_CHAN_SPACING_KHZ,
   MAC_868MHZ_BAND_MODE_2_TOTAL_CHANNELS, MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_200_K,
   MAC_MODULATION_INDEX_2FSK_200K,
   MAC_CCA_TYPE_LBT },
 // Dummy placeholder PHY
 { MAC_CUSTOM_BAND_MODE_2_CENTER_FREQ_KHZ, MAC_CUSTOM_BAND_MODE_2_CHAN_SPACING_KHZ,
   MAC_CUSTOM_BAND_MODE_2_TOTAL_CHANNELS, MAC_2_FSK_MODULATION, SYMBOL_RATE_CUSTOM,
   MAC_MODULATION_INDEX_CUSTOM,
   MAC_CCA_TYPE_CUSTOM },
 // Dummy placeholder PHY
 { MAC_915MHZ_BAND_MODE_1_CENTER_FREQ_KHZ, MAC_915MHZ_BAND_MODE_1_CHAN_SPACING_KHZ,
   MAC_915MHZ_BAND_MODE_1_TOTAL_CHANNELS, MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_50_K,
   MAC_MODULATION_INDEX_2FSK_50K,
   MAC_CCA_TYPE_CSMA_CA },
 { MAC_920MHZ_BAND_MODE_4_CENTER_FREQ_KHZ, MAC_920MHZ_BAND_MODE_4_CHAN_SPACING_KHZ,
   MAC_920MHZ_BAND_MODE_4_TOTAL_CHANNELS, MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_200_K,
   MAC_MODULATION_INDEX_2FSK_200K,
   MAC_CCA_TYPE_CSMA_CA }
};
#endif
#if defined(FEATURE_BEACON_MODE) || defined(FEATURE_NON_BEACON_MODE)
#if defined(LP_CC2652RB)
uint16 macBeaconMargin[] =
{
  33,
  33,
  33,
  33,
  33,
  33,
  33,
  64,
  125,
  250,
  500,
  1000,
  2000,
  4000,
  8000,
  0
};
#else
uint16 macBeaconMargin[] =
{
 12,
 12,
 12,
 12,
 12,
 12,
 12,
 24,
 48,
 96,
 192,
 384,
 768,
 1324,
 2588,
 0
};
#endif
#else
uint16 macBeaconMargin[1];
#endif
