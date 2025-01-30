/******************************************************************************

 @file  mac_api.h

 @brief Public interface file for 802.15.4 MAC.

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

#ifndef MAC_API_H
#define MAC_API_H

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_types.h"
#include "saddr.h"
#include "sdata.h"

#include "api_mac.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* Status */
#define MAC_SUCCESS                 0x00  /* Operation successful */
#define MAC_AUTOACK_PENDING_ALL_ON  0xFE  /* The AUTOPEND pending all is turned on */
#define MAC_AUTOACK_PENDING_ALL_OFF 0xFF  /* The AUTOPEND pending all is turned off */
#define MAC_BEACON_LOSS             0xE0  /* The beacon was lost following a synchronization request */
#define MAC_CHANNEL_ACCESS_FAILURE  0xE1  /* The operation or data request failed because of
                                             activity on the channel */
#define MAC_COUNTER_ERROR           0xDB  /* The frame counter puportedly applied by the originator of
                                             the received frame is invalid */
#define MAC_DENIED                  0xE2  /* The MAC was not able to enter low power mode. */
#define MAC_DISABLE_TRX_FAILURE     0xE3  /* Unused */
#define MAC_FRAME_TOO_LONG          0xE5  /* The received frame or frame resulting from an operation
                                             or data request is too long to be processed by the MAC */
#define MAC_IMPROPER_KEY_TYPE       0xDC  /* The key purportedly applied by the originator of the
                                             received frame is not allowed */
#define MAC_IMPROPER_SECURITY_LEVEL 0xDD  /* The security level purportedly applied by the originator of
                                             the received frame does not meet the minimum security level */
#define MAC_INVALID_ADDRESS         0xF5  /* The data request failed because neither the source address nor
                                             destination address parameters were present */
#define MAC_INVALID_GTS             0xE6  /* Unused */
#define MAC_INVALID_HANDLE          0xE7  /* The purge request contained an invalid handle */
#define MAC_INVALID_INDEX           0xF9  /* Unused */
#define MAC_INVALID_PARAMETER       0xE8  /* The API function parameter is out of range */
#define MAC_LIMIT_REACHED           0xFA  /* The scan terminated because the PAN descriptor storage limit
                                             was reached */
#define MAC_NO_ACK                  0xE9  /* The operation or data request failed because no
                                             acknowledgement was received */
#define MAC_NO_BEACON               0xEA  /* The scan request failed because no beacons were received or the
                                             orphan scan failed because no coordinator realignment was received */
#define MAC_NO_DATA                 0xEB  /* The associate request failed because no associate response was received
                                             or the poll request did not return any data */
#define MAC_NO_SHORT_ADDRESS        0xEC  /* The short address parameter of the start request was invalid */
#define MAC_ON_TIME_TOO_LONG        0xF6  /* Unused */
#define MAC_OUT_OF_CAP              0xED  /* Unused */
#define MAC_PAN_ID_CONFLICT         0xEE  /* A PAN identifier conflict has been detected and
                                             communicated to the PAN coordinator */
#define MAC_PAST_TIME               0xF7  /* Unused */
#define MAC_READ_ONLY               0xFB  /* A set request was issued with a read-only identifier */
#define MAC_REALIGNMENT             0xEF  /* A coordinator realignment command has been received */
#define MAC_SCAN_IN_PROGRESS        0xFC  /* The scan request failed because a scan is already in progress */
#define MAC_SECURITY_ERROR          0xE4  /* Cryptographic processing of the secure frame failed */
#define MAC_SUPERFRAME_OVERLAP      0xFD  /* The beacon start time overlapped the coordinator transmission time */
#define MAC_TRACKING_OFF            0xF8  /* The start request failed because the device is not tracking
                                             the beacon of its coordinator */
#define MAC_TRANSACTION_EXPIRED     0xF0  /* The associate response, disassociate request, or indirect
                                             data transmission failed because the peer device did not respond
                                             before the transaction expired or was purged */
#define MAC_TRANSACTION_OVERFLOW    0xF1  /* The request failed because MAC data buffers are full */
#define MAC_TX_ACTIVE               0xF2  /* Unused */
#define MAC_UNAVAILABLE_KEY         0xF3  /* The operation or data request failed because the
                                             security key is not available */
#define MAC_UNSUPPORTED_ATTRIBUTE   0xF4  /* The set or get request failed because the attribute is not supported */
#define MAC_UNSUPPORTED_LEGACY      0xDE  /* The received frame was secured with legacy security which is
                                             not supported */
#define MAC_UNSUPPORTED_SECURITY    0xDF  /* The security of the received frame is not supported */
#define MAC_UNSUPPORTED             0x18  /* The operation is not supported in the current configuration */
#define MAC_BAD_STATE               0x19  /* The operation could not be performed in the current state */
#define MAC_NO_RESOURCES            0x1A  /* The operation could not be completed because no
                                             memory resources were available */
#define MAC_ACK_PENDING             0x1B  /* For internal use only */
#define MAC_NO_TIME                 0x1C  /* For internal use only */
#define MAC_TX_ABORTED              0x1D  /* The transmission was aborted */
#define MAC_DUPLICATED_ENTRY        0x1E  /* For internal use only - A duplicated entry is added to the source matching table */

/* MAC HL Patches */
#define MAC_PIB_PATCH_DONE          0xC0  /* Patch is done */
#define MAC_PIB_PATCH_MORE          0xC1  /* Patch is needed more at the next step */
#define MAC_PIB_PATCH_NONE          0xC2  /* Patch is not needed since this is existing pib attribute */

/* MAC Security Level */
#define MAC_SEC_LEVEL_NONE          0x00  /* No security is used */
#define MAC_SEC_LEVEL_MIC_32        0x01  /* MIC-32 authentication is used */
#define MAC_SEC_LEVEL_MIC_64        0x02  /* MIC-64 authentication is used */
#define MAC_SEC_LEVEL_MIC_128       0x03  /* MIC-128 authentication is used */
#define MAC_SEC_LEVEL_ENC           0x04  /* AES encryption is used */
#define MAC_SEC_LEVEL_ENC_MIC_32    0x05  /* AES encryption and MIC-32 authentication are used */
#define MAC_SEC_LEVEL_ENC_MIC_64    0x06  /* AES encryption and MIC-64 authentication are used */
#define MAC_SEC_LEVEL_ENC_MIC_128   0x07  /* AES encryption and MIC-128 authentication are used */

/* Key Identifier Mode */
#define MAC_KEY_ID_MODE_NONE        0x00  /* Key is is not used */
#define MAC_KEY_ID_MODE_IMPLICIT    0x00  /* Key is determined implicitly */
#define MAC_KEY_ID_MODE_1           0x01  /* Key is determined from the 1-byte key index */
#define MAC_KEY_ID_MODE_4           0x02  /* Key is determined from the 4-byte key index */
#define MAC_KEY_ID_MODE_8           0x03  /* Key is determined from the 8-byte key index */

/* Key identifier field length in bytes */
#define MAC_KEY_ID_IMPLICIT_LEN     0
#define MAC_KEY_ID_1_LEN            1
#define MAC_KEY_ID_4_LEN            5
#define MAC_KEY_ID_8_LEN            9

/* Key source maximum length in bytes */
#define MAC_KEY_SOURCE_MAX_LEN      8

/* Key index length in bytes */
#define MAC_KEY_INDEX_LEN           1

/* Frame counter length in bytes */
#define MAC_FRAME_COUNTER_LEN       4

/* Key length in bytes */
#define MAC_KEY_MAX_LEN             16

/* Key lookup data length in bytes */
#define MAC_KEY_LOOKUP_SHORT_LEN    5
#define MAC_KEY_LOOKUP_LONG_LEN     9
#define MAC_MAX_KEY_LOOKUP_LEN      MAC_KEY_LOOKUP_LONG_LEN


/* Data constants */
#if !defined ( MAC_MAX_FRAME_SIZE )
#ifndef FREQ_2_4G
  #define MAC_MAX_FRAME_SIZE        1000   /* Maximum application data length without security */
#else
  #define MAC_MAX_FRAME_SIZE        125   /* Maximum application data length without security */
#endif
#endif

//#if !defined ( MAX_KEY_TABLE_ENTRIES )
//  #define MAX_KEY_TABLE_ENTRIES     2     /* Maximum number of keys in the keytable */
//#endif
#define MAC_DATA_OFFSET             24    /* Bytes required for MAC header in data frame */
#define MAC_MIC_32_LEN              4     /* Length required for MIC-32 authentication */
#define MAC_MIC_64_LEN              8     /* Length required for MIC-64 authentication */
#define MAC_MIC_128_LEN             16    /* Length required for MIC-128 authentication */

/* MHR length for received frame */
#define MAC_MHR_LEN                 37    /* FCF (2) + Seq (1) + Addr Fields (20) + Security HDR (14) */

/* TX Options */
#define MAC_TXOPTION_ACK            0x0001  /* Acknowledged transmission.  The MAC will attempt to retransmit
                                               the frame until it is acknowledged */
#define MAC_TXOPTION_GTS            0x0002  /* GTS transmission (unused) */
#define MAC_TXOPTION_INDIRECT       0x0004  /* Indirect transmission.  The MAC will queue the data and wait
                                               for the destination device to poll for it.  This can only be used
                                               by a coordinator device */
#define MAC_TXOPTION_PEND_BIT       0x0008  /* This proprietary option forces the pending bit set for direct
                                               transmission */
#define MAC_TXOPTION_NO_RETRANS     0x0010  /* This proprietary option prevents the frame from being retransmitted */
#define MAC_TXOPTION_NO_CNF         0x0020  /* This proprietary option prevents a MAC_MCPS_DATA_CNF
                                               event from being sent for this frame */
#define MAC_TXOPTION_ALT_BE         0x0040  /* Use PIB value MAC_ALT_BE for the minimum backoff exponent */
#define MAC_TXOPTION_PWR_CHAN       0x0080  /* Use the power and channel values in macDataReq_t
                                               instead of the PIB values */
#define MAC_TXOPTION_GREEN_PWR      0x0100  /* Special transmit for Green Power. Must be direct */

/* Green Power limitations */
#define MAC_MIN_GREEN_PWR_DELAY     1       /* GP delay must be greater than ~1.6ms or 1600/320 */

#ifndef MAC_CHAN_LOWEST
#define MAC_CHAN_LOWEST             0
#endif

#ifndef MAC_CHAN_HIGHEST
#define MAC_CHAN_HIGHEST            128
#endif

/* Maximum number of channels */
#define MAC_154G_MAX_NUM_CHANNEL      (MAC_CHAN_HIGHEST - MAC_CHAN_LOWEST + 1)

/* Bitmap size to hold the channel list */
#define MAC_154G_CHANNEL_BITMAP_SIZ \
        ((MAC_154G_MAX_NUM_CHANNEL + 7) / 8)

/* Supported Channel Pages */
#define MAC_CHANNEL_PAGE_9          9     /* standard-defined SUN PHY operating modes */
#define MAC_CHANNEL_PAGE_10         10    /* MR-FSK Generic-PHY-defined PHY modes */

/* Maximum number of PHY descriptor entries */
#define MAC_STANDARD_PHY_DESCRIPTOR_ENTRIES  5
#define MAC_GENERIC_PHY_DESCRIPTOR_ENTRIES   10

/* PHY IDs */
/* 915MHz Frequency band operating mode #1 */
#define MAC_50KBPS_915MHZ_PHY_1         1
/* 915MHz Frequency band operating mode #2 */
#define MAC_150KBPS_915MHZ_PHY_2        2
/* 868MHz Frequency band operating mode #1 */
#define MAC_50KBPS_868MHZ_PHY_3         3
/* 433MHz Frequency band operating mode #1 */
#define MAC_50KBPS_433MHZ_PHY_128       128
/* 915MHz LRM Frequency band operating mode #1 */
#define MAC_5KBPS_915MHZ_PHY_129        129
/* 433MHz LRM Frequency band operating mode #1 */
#define MAC_5KBPS_433MHZ_PHY_130        130
/* 868MHz LRM Frequency band operating mode #1 */
#define MAC_5KBPS_868MHZ_PHY_131        131
/* 915MHz Frequency band operating mode #3 */
#define MAC_200KBPS_915MHZ_PHY_132      132
/* 868MHz Frequency band operating mode #3 */
#define MAC_200KBPS_868MHZ_PHY_133      133
/* Configurable custom PHY*/
#define MAC_CUSTOM_PHY_ID               134
/* 920MHz Frequency band operating mode #4a */
#define MAC_200KBPS_920MHZ_PHY_136      136

/* MRFSK Standard Phy ID start */
#define MAC_MRFSK_STD_PHY_ID_BEGIN           MAC_50KBPS_915MHZ_PHY_1
/* MRFSK Standard Phy ID end */
#define MAC_MRFSK_STD_PHY_ID_END             (MAC_MRFSK_STD_PHY_ID_BEGIN + MAC_STANDARD_PHY_DESCRIPTOR_ENTRIES - 1)

/* MRFSK Generic Phy ID start */
#define MAC_MRFSK_GENERIC_PHY_ID_BEGIN       MAC_50KBPS_433MHZ_PHY_128
/* MRFSK Generic Phy ID end */
#define MAC_MRFSK_GENERIC_PHY_ID_END         (MAC_MRFSK_GENERIC_PHY_ID_BEGIN + MAC_GENERIC_PHY_DESCRIPTOR_ENTRIES - 1)

/* MAC PHY Descriptor parameters */
#define MAC_PHY_DESCRIPTOR                0x01  /* PHY descriptor table entry */

/* Symbol rate in kilo symbols per second */
#define MAC_MRFSK_SYMBOL_RATE_20_K    20     /* 20K symbol rate */
#define MAC_MRFSK_SYMBOL_RATE_50_K    50     /* 50K symbol rate */
#define MAC_MRFSK_SYMBOL_RATE_100_K  100     /* 100k symbol rate */
#define MAC_MRFSK_SYMBOL_RATE_150_K  150     /* 150k symbol rate */
#define MAC_MRFSK_SYMBOL_RATE_200_K  200     /* 200K symbol rate */

/* Clear channel access type */
#define MAC_CCA_TYPE_NO_BACKOFF      0   /* No back off and CSMA/CA */
#define MAC_CCA_TYPE_CSMA_CA         1   /* CSMA/CA */
#define MAC_CCA_TYPE_LBT             2   /* ETSI EN 300 220 LBT */
#define MAC_CCA_TYPE_CSMA_ONLY       3   /* CSMA only */

/* Modulation scheme */
#define MAC_2_FSK_MODULATION         0   /* 2-FSK */
#define MAC_4_FSK_MODULATION         1   /* 4-FSK */

/* Preamble length. The number of 1-octet patterns */
#define MAC_MIN_PREAMBLE_LEN         4     /* Minimum number of preamble repetition */
#define MAC_MAX_PREAMBLE_LEN         31    /* Maximum number of preamble repetition in byte.
                                              Note: 31 means 4 bits */
#define MAC_PREAMBLE_LEN_8           8     /* WiSUN preamble length for 50 and 100 kbps */
#define MAC_PREAMBLE_LEN_12          12    /* WiSUN preamble length for 150 kbps */
#define MAC_PREAMBLE_LEN_24          24    /* WiSUN preamble length for 300 kbps */

/* Capability Information */
#define MAC_CAPABLE_PAN_COORD       0x01  /* Device is capable of becoming a PAN coordinator */
#define MAC_CAPABLE_FFD             0x02  /* Device is an FFD */
#define MAC_CAPABLE_MAINS_POWER     0x04  /* Device is mains powered rather than battery powered */
#define MAC_CAPABLE_RX_ON_IDLE      0x08  /* Device has its receiver on when idle */
#define MAC_CAPABLE_SECURITY        0x40  /* Device is capable of sending and receiving secured frames */
#define MAC_CAPABLE_ALLOC_ADDR      0x80  /* Request allocation of a short address in the associate procedure */

/* Standard PIB Get and Set Attributes */
#define MAC_ACK_WAIT_DURATION             0x40  /* The maximum number of symbols to wait for an acknowledgment frame */
#define MAC_ASSOCIATION_PERMIT            0x41  /* TRUE if a coordinator is currently allowing association */
#define MAC_AUTO_REQUEST                  0x42  /* TRUE if a device automatically sends a data request if its address
                                                   is listed in the beacon frame */
#define MAC_BATT_LIFE_EXT                 0x43  /* TRUE if battery life extension is enabled */
#define MAC_BATT_LIFE_EXT_PERIODS         0x44  /* The number of backoff periods during which the receiver is
                                                   enabled following a beacon in battery life extension mode */
#define MAC_BEACON_PAYLOAD                0x45  /* The contents of the beacon payload */
#define MAC_BEACON_PAYLOAD_LENGTH         0x46  /* The length in bytes of the beacon payload */
#define MAC_BEACON_ORDER                  0x47  /* How often the coordinator transmits a beacon */
#define MAC_BEACON_TX_TIME                0x48  /* The time the device transmitted its last beacon frame,
                                                   in backoff period units */
#define MAC_BSN                           0x49  /* The beacon sequence number */
#define MAC_COORD_EXTENDED_ADDRESS        0x4A  /* The extended address of the coordinator with which the device
                                                   is associated */
#define MAC_COORD_SHORT_ADDRESS           0x4B  /* The short address assigned to the coordinator with which the
                                                   device is associated.  A value of MAC_ADDR_USE_EXT indicates
                                                   that the coordinator is using its extended address */
#define MAC_DSN                           0x4C  /* The data or MAC command frame sequence number */
#define MAC_GTS_PERMIT                    0x4D  /* TRUE if the PAN coordinator accepts GTS requests */
#define MAC_MAX_CSMA_BACKOFFS             0x4E  /* The maximum number of backoffs the CSMA-CA algorithm will attempt
                                                   before declaring a channel failure */
#define MAC_MIN_BE                        0x4F  /* The minimum value of the backoff exponent in the CSMA-CA algorithm.
                                                   If this value is set to 0, collision avoidance is disabled during
                                                   the first iteration of the algorithm. Also for the slotted version
                                                   of the CSMA-CA algorithm with the battery life extension enabled,
                                                   the minimum value of the backoff exponent will be at least 2 */
#define MAC_PAN_ID                        0x50  /* The PAN identifier.  If this value is 0xffff, the device is not
                                                   associated */
#define MAC_PROMISCUOUS_MODE              0x51  /* TRUE if the MAC is in promiscuous mode */
#define MAC_RX_ON_WHEN_IDLE               0x52  /* TRUE if the MAC enables its receiver during idle periods */
#define MAC_SHORT_ADDRESS                 0x53  /* The short address that the device uses to communicate in the PAN.
                                                   If the device is a PAN coordinator, this value shall be set before
                                                   calling MAC_StartReq().  Otherwise the value is allocated during
                                                   association.  Value MAC_ADDR_USE_EXT indicates that the device is
                                                   associated but not using a short address */
#define MAC_SUPERFRAME_ORDER              0x54  /* This specifies the length of the active portion of the superframe */
#define MAC_TRANSACTION_PERSISTENCE_TIME  0x55  /* The maximum time in beacon intervals that a transaction is stored by
                                                   a coordinator and indicated in the beacon */
#define MAC_ASSOCIATED_PAN_COORD          0x56  /* TRUE if the device is associated to the PAN coordinator */
#define MAC_MAX_BE                        0x57  /* The maximum value of the backoff exponent in the CSMA-CA algorithm */
#define MAC_MAX_FRAME_TOTAL_WAIT_TIME     0x58  /* The maximum number of CAP symbols in a beacon-enabled PAN, or
                                                   symbols in a non beacon-enabled PAN, to wait for a frame intended
                                                   as a response to a data request frame */
#define MAC_MAX_FRAME_RETRIES             0x59  /* The maximum number of retries allowed after a transmission failure */
#define MAC_RESPONSE_WAIT_TIME            0x5A  /* The maximum number of symbols a device shall wait for a response
                                                   command to be available following a request command in multiples
                                                   of aBaseSuperframeDuration */
#define MAC_SYNC_SYMBOL_OFFSET            0x5B  /* The timestamp offset from SFD in symbols */
#define MAC_TIMESTAMP_SUPPORTED           0x5C  /* TRUE if the MAC supports RX and TX timestamps */
#define MAC_SECURITY_ENABLED              0x5D  /* TRUE if security is enabled */

#define MAC_EBSN                          0x5E  /* Enhanced beacon sequence number */
#define MAC_EBEACON_ORDER                 0x5F  /* Enhanced beacon order in a beacon enabled network */
#define MAC_EBEACON_ORDER_NBPAN           0x60  /* Enhanced beacon order in a non-beacon enabled network */
#define MAC_OFFSET_TIMESLOT               0x61  /* Offset time slot from the beacon */
#define MAC_INCLUDE_MPMIE                 0x62  /* TRUE if MPM IE needs to be included */

#define MAC_PHY_FSK_PREAMBLE_LEN          0x63  /* The number of 1-octet patterns. vslue of 2 is for 2FSK and 4 for 4FSK */
#define MAC_PHY_MRFSKSFD                  0x64  /* which group of SFD to use. valid values are 0 and 1 */

/* Security PIB Get and Set Attributes */
#define MAC_KEY_TABLE                     0x71  /* A table of KeyDescriptor, entries, each containing keys and related
                                                   information required for secured communications */
#define MAC_DEVICE_TABLE                  0x72  /* A table of Device-Descriptor entries, each indicating a remote device
                                                   with which this device securely communicates */
#define MAC_SECURITY_LEVEL_TABLE          0x73  /* A table of SecurityLevel-Descriptor entries, each with information
                                                   about the minimum security level expected depending on incoming frame
                                                   type and subtype. */
#define MAC_KEY_TABLE_ENTRIES             0x81  /* The number of entries in macKeyTable */
#define MAC_DEVICE_TABLE_ENTRIES          0x82  /* The number of entries in macDeviceTable. */

#define MAC_SECURITY_LEVEL_TABLE_ENTRIES  0x83  /* The number of entries in macSecurityLevelTable. */
#define MAC_FRAME_COUNTER                 0x84  /* The outgoing frame counter for this device */
#define MAC_AUTO_REQUEST_SECURITY_LEVEL   0x85  /* The security level used for automatic data requests. */
#define MAC_AUTO_REQUEST_KEY_ID_MODE      0x86  /* The key identifier mode used for automatic data requests */
#define MAC_AUTO_REQUEST_KEY_SOURCE       0x87  /* The originator of the key used for automatic data requests. */
#define MAC_AUTO_REQUEST_KEY_INDEX        0x88  /* The index of the key used for automatic data requests. */
#define MAC_DEFAULT_KEY_SOURCE            0x89  /* The originator of the default key used for key ID mode 0x01 */
#define MAC_PAN_COORD_EXTENDED_ADDRESS    0x8A  /* The 64-bit address of the PAN coordinator. */
#define MAC_PAN_COORD_SHORT_ADDRESS       0x8B  /* The 16-bit short address assigned to the PAN coordinator. */

/* Proprietary Security PIB Get and Set Attributes */
#define MAC_KEY_ID_LOOKUP_ENTRY           0xD0  /* The key lookup table entry, part of an entry of the key table */
#define MAC_KEY_DEVICE_ENTRY              0xD1  /* The key device entry, part of an entry of the key table */
#define MAC_KEY_USAGE_ENTRY               0xD2  /* The key usage entry, part of an entry of the key table */
#define MAC_KEY_ENTRY                     0xD3  /* The MAC key entry, an entry of the key table */
#define MAC_DEVICE_ENTRY                  0xD4  /* The MAC device entry, an entry of the device table */
#define MAC_SECURITY_LEVEL_ENTRY          0xD5  /* The MAC security level entry, an entry of the security level table */


/* Proprietary PIB Get and Set Attributes */
#define MAC_PHY_TRANSMIT_POWER_SIGNED     0xE0  /* Duplicate transmit power attribute in signed
                                                    (2's complement) dBm unit */
#define MAC_LOGICAL_CHANNEL               0xE1  /* The logical channel */
#define MAC_EXTENDED_ADDRESS              0xE2  /* The extended address of the device */
#define MAC_ALT_BE                        0xE3  /* alternate minimum backoff exponent */
#define MAC_DEVICE_BEACON_ORDER           0xE4  /* Device beacon order */
#define MAC_RF4CE_POWER_SAVINGS           0xE5  /* valid values are true and false */
#define MAC_FRAME_VERSION_SUPPORT         0xE6  /* Currently supports 0 and 1.
                                                   If 0, frame Version is always 0 and set to 1 only for secure frames.
                                                   If 1, frame version will be set to 1 only if packet len > 102 or
                                                   for secure frames */
#define MAC_CHANNEL_PAGE                  0xE7
#define MAC_PHY_CURRENT_DESCRIPTOR_ID     0xE8  /* PHY Descriptor ID, used to support channel page number and index
                                                   into descriptor table */
#define MAC_FCS_TYPE                      0xE9  /* FCS type */

/* Diagnostic PIB Get and Set Attributes */
#define MAC_DIAGS_RX_CRC_PASS             0xEA  /* Received CRC pass counter */
#define MAC_DIAGS_RX_CRC_FAIL             0xEB  /* Received CRC fail counter */
#define MAC_DIAGS_RX_BCAST                0xEC  /* Received broadcast counter */
#define MAC_DIAGS_TX_BCAST                0xED  /* Transmitted broadcast counter */
#define MAC_DIAGS_RX_UCAST                0xEE  /* Received unicast counter */
#define MAC_DIAGS_TX_UCAST                0xEF  /* Transmitted unicast counter */
#define MAC_DIAGS_TX_UCAST_RETRY          0xF0  /* Transmitted unicast retry counter */
#define MAC_DIAGS_TX_UCAST_FAIL           0xF1  /* Transmitted unicast fail counter */
#define MAC_DIAGS_RX_SEC_FAIL             0xF2  /* Received Security fail counter */
#define MAC_DIAGS_TX_SEC_FAIL             0xF3  /* Transmitted Security fail counter */

#ifdef MAC_DUTY_CYCLE_CHECKING
/* Duty Cycling */
#define MAC_DC_ENABLED                    0xEB
#define MAC_DC_LIMITED                    0xEC
#define MAC_DC_CRITICAL                   0xED
#define MAC_DC_REGULATED                  0xEE
#define MAC_DC_USED                       0xEF
#define MAC_DC_PTR                        0xF1
#define MAC_DC_BUCKET                     0xF2
#define MAC_DC_STATUS                     0xF3
#endif

/* LBT RSSI Threshold Attributes */
#define MAC_RSSI_THRESHOLD                0xF4  /* RSSI Threshold */

/* Range Extender */
#define MAC_RANGE_EXTENDER                0xF5  /* Range Extender Mode */
/* Enhanced Data Pending Ack */
#define MAC_EN_DATA_ACK_PENDING           0xF6

/* RF Band */
#define MAC_RF_FREQ                       0xF7  /* RF Freq Selection */

/* PA Type */
#define MAC_RF_PA_TYPE                    0xF8  /* RF PA Type */

/* Coex Debug Struct */
#define MAC_COEX_METRICS                  0xF9  /* Coex Debug struct */

#define MAC_STATISTICS                    0xFA  /* MAC Statistics struct */

/* Frequency hopping PIB Get and Set Attributes */
#define MAC_FHPIB_TRACK_PARENT_EUI        0x2000  /* The parent EUI address */
#define MAC_FHPIB_BC_INTERVAL             0x2001  /* Time between start of two broadcast slots in msec */
#define MAC_FHPIB_UC_EXCLUDED_CHANNELS    0x2002  /* Unicast excluded channels */
#define MAC_FHPIB_BC_EXCLUDED_CHANNELS    0x2003  /* Broadcast excluded channels */
#define MAC_FHPIB_UC_DWELL_INTERVAL       0x2004  /* Duration of node's unicast slot */
#define MAC_FHPIB_BC_DWELL_INTERVAL       0x2005  /* Duration of broadcast slot */
#define MAC_FHPIB_CLOCK_DRIFT             0x2006  /* Clock drift in PPM */
#define MAC_FHPIB_TIMING_ACCURACY         0x2007  /* Timing accuracy in 10usec resolution */
#define MAC_FHPIB_UC_CHANNEL_FUNCTION     0x2008  /* Unicast channel hopping function */
#define MAC_FHPIB_BC_CHANNEL_FUNCTION     0x2009  /* Broadcast channel hopping function */
#define MAC_FHPIB_USE_PARENT_BS_IE        0x200A  /* Node is propagating parent's BS-IE */
#define MAC_FHPIB_BROCAST_SCHED_ID        0x200B  /* Broadcast schedule ID for broadcast channel hopping sequence */
#define MAC_FHPIB_UC_FIXED_CHANNEL        0x200C  /* Unicast channel number when no hopping */
#define MAC_FHPIB_BC_FIXED_CHANNEL        0x200D  /* Broadcast channel number when no hopping */
#define MAC_FHPIB_PAN_SIZE                0x200E  /* Number of nodes in the PAN */
#define MAC_FHPIB_ROUTING_COST            0x200F  /* Estimate of routing path ETX to the PAN coordinator */
#define MAC_FHPIB_ROUTING_METHOD          0x2010  /* RPL(1), MHDS(0)*/
#define MAC_FHPIB_EAPOL_READY             0x2011  /* Node can accept EAPOL message */
#define MAC_FHPIB_FAN_TPS_VERSION         0x2012  /* Wi-SUN FAN version */
#define MAC_FHPIB_NET_NAME                0x2013  /* Network Name */
#define MAC_FHPIB_PAN_VERSION             0x2014  /* PAN version to notify PAN configuration changes */
#define MAC_FHPIB_GTK_0_HASH              0x2015  /* Low order 64 bits of SHA256 hash of GTK */
#define MAC_FHPIB_GTK_1_HASH              0x2016  /* Low order 64 bits of SHA256 hash of GTK */
#define MAC_FHPIB_GTK_2_HASH              0x2017  /* Low order 64 bits of SHA256 hash of GTK */
#define MAC_FHPIB_GTK_3_HASH              0x2018  /* Low order 64 bits of SHA256 hash of GTK */
#define MAC_FHPIB_NEIGHBOR_VALID_TIME     0x2019  /* Time in min during which the node info considered as valid */
#define MAC_FHPIB_CSMA_BASE_BACKOFF       0x201A  /* Additional base wait time to sense target channel */
#define MAC_FHPIB_NUM_MAX_NON_SLEEP_NODES 0x201B  /* Number of non-sleepy neighbors that can be supported */
#define MAC_FHPIB_NUM_MAX_SLEEP_NODES     0x201C  /* Number of sleepy neighbors that can be supported */

/* Disassociate Reason */
#define MAC_DISASSOC_COORD          1     /* The coordinator wishes the device to disassociate */
#define MAC_DISASSOC_DEVICE         2     /* The device itself wishes to disassociate */


/* Scan Type */
#define MAC_SCAN_ED                 0     /* Energy detect scan.  The device will tune to each channel and
                                             perform and energy measurement.  The list of channels and their
                                             associated measurements will be returned at the end of the scan */
#define MAC_SCAN_ACTIVE             1     /* Active scan.  The device tunes to each channel, sends a beacon
                                             request and listens for beacons.  The PAN descriptors are returned
                                             at the end of the scan */
#define MAC_SCAN_PASSIVE            2     /* Passive scan.  The device tunes to each channel and listens for
                                             beacons.  The PAN descriptors are returned at the end of the scan */
#define MAC_SCAN_ORPHAN             3     /* Orphan scan.  The device tunes to each channel and sends an orphan
                                             notification to try and find its coordinator.  The status is returned
                                             at the end of the scan */
#define MAC_SCAN_ACTIVE_ENHANCED    5     /* Enhanced Active scan. In addition to Active scan, this command is also
                                             used by a device to locate a subset of all coordinators within its
                                             POS during an active scan. */

/* MPM (Multi-Phy Mode) Scan Types */
#define MAC_MPM_SCAN_BPAN           1     /* passive scan for the enhanced beacons with co-existence IE
                                             in a beacon enabled PAN*/

#define MAC_MPM_SCAN_NBPAN          2     /* perform scan for the enhanced beacons with co-existence IE
                                             in a non-beacon PAN. The scan can be either passive or active */

/* enhanced beacon order for Non-Beacon enabled PAN values */
#define MAC_EBEACON_ORDER_NBPAN_MIN  10     /* Minimum value of enhanced beacon order supported for NBPAN */
#define MAC_EBEACON_ORDER_NBPAN_MAX  16383  /* Maximum value of enhanced beacon order supported for NBPAN */

/* minBe and maxBe */
#define MAC_DEFAULT_MIN_BE           3       /* default minBe for regular modes */
#define MAC_DEFAULT_MAX_BE           5       /* default maxBe for regular modes */
#define MAC_LRM_DEFAULT_MIN_BE       5       /* default minBe for LRM modes */
#define MAC_LRM_DEFAULT_MAX_BE       8       /* default maxBe for LRM modes */

/* responseWaitTime and maxFrameTotalWaitTime */
/* response wait time = 48sym = 48*20*960 = approx. 1 sec */
#define MAC_DEFAULT_RESPONSE_WAIT_TIME          48      /* default responseWaitTime for 50kbps and 5kbps */
#define MAC_LRM_DEFAULT_RESPONSE_WAIT_TIME      20      /* default responseWaitTime for 50kbps and 5kbps */
#define MAC_DEFAULT_FRAME_WAIT_TIME             8000    /* default maxFrameTotalWaitTime for 50kbps and 5kbps */
#define MAC_200K_DEFAULT_RESPONSE_WAIT_TIME     192     /* default responseWaitTime for 50kbps and 5kbps */
#define MAC_200K_DEFAULT_FRAME_WAIT_TIME        32000   /* default maxFrameTotalWaitTime for 50kbps and 5kbps */
#define MAC_2P4G_RESPONSE_WAIT_TIME             16      /* default responseWaitTime for 2.4G */

#define MAC_CUSTOM_FRAME_WAIT_TIME ((symbol_rate_custom < 200) ? ((symbol_rate_custom/200) * 32000) : (32001))

/* The response time cannot be bigger than 255, so all symbol rates over 266 will have 255 in response wait time */  
#define MAC_RESPONSE_WAIT_TIME_CUSTOM ((symbol_rate_custom < 266) ? ((symbol_rate_custom * 960)/1000) : (255))

/* rf band */
#define MAC_RF_FREQ_SUBG            1        /* subG */
#define MAC_RF_FREQ_2_4G            2        /* 2.4G */

/* phy transmit power */
#define MAC_DEFAULT_TX_POWER        14       /* default tx power for regular modes */
#define MAC_RE_DEFAULT_TX_POWER     27       /* default tx power for range extender modes */
#define MAC_HPA_DEFAULT_TX_POWER    20       /* default tx power for high PA type */

/* Special address values */
#define MAC_ADDR_USE_EXT            0xFFFE  /* Short address value indicating extended address is used */
#define MAC_SHORT_ADDR_BROADCAST    0xFFFF  /* Broadcast short address */
#define MAC_SHORT_ADDR_NONE         0xFFFF  /* Short address when there is no short address */

/* Comm status indication reasons */
#define MAC_COMM_ASSOCIATE_RSP      0     /* Event sent in response to MAC_AssociateRsp() */
#define MAC_COMM_ORPHAN_RSP         1     /* Event sent in response to MAC_OrphanRsp() */
#define MAC_COMM_RX_SECURE          2     /* Event sent as a result of receiving a secure frame */

/* Power Mode */
#define MAC_PWR_ON                  0     /* MAC and radio hardware is powered on */
#define MAC_PWR_SLEEP_LITE          1     /* MAC and radio hardware are partially powered off */
#define MAC_PWR_SLEEP_DEEP          2     /* MAC and radio hardware are fully powered off */

/* MAC Callback Events */
#define MAC_MLME_ASSOCIATE_IND      1     /* Associate indication */
#define MAC_MLME_ASSOCIATE_CNF      2     /* Associate confirm */
#define MAC_MLME_DISASSOCIATE_IND   3     /* Disassociate indication */
#define MAC_MLME_DISASSOCIATE_CNF   4     /* Disassociate confirm */
#define MAC_MLME_BEACON_NOTIFY_IND  5     /* Beacon notify indication */
#define MAC_MLME_ORPHAN_IND         6     /* Orphan indication */
#define MAC_MLME_SCAN_CNF           7     /* Scan confirm */
#define MAC_MLME_START_CNF          8     /* Start confirm */
#define MAC_MLME_SYNC_LOSS_IND      9     /* Sync loss indication */
#define MAC_MLME_POLL_CNF           10    /* Poll confirm */
#define MAC_MLME_COMM_STATUS_IND    11    /* Communication status indication */
#define MAC_MCPS_DATA_CNF           12    /* Data confirm */
#define MAC_MCPS_DATA_IND           13    /* Data indication */
#define MAC_MCPS_PURGE_CNF          14    /* Purge confirm */
#define MAC_PWR_ON_CNF              15    /* Power on confirm */
#define MAC_MLME_POLL_IND           16    /* Poll indication */
#define MAC_MLME_WS_ASYNC_CNF       17    /* WiSUN Async frame confirm */
#define MAC_MLME_WS_ASYNC_IND       18    /* WiSUN Async frame indication */
#define MAC_NCP_MT_MSG              19    /* NPI MP message */

/* The length of the random seed is set for maximum requirement which is
 * 32 for ZigBeeIP
 */
#define MAC_RANDOM_SEED_LEN         32

/* Frequency hopping header IE's */
#define MAC_FH_UT_IE            0x00000002 /* Bit to include UTT IE */
#define MAC_FH_RSL_IE           0x00000004 /* Bit to include RSL IE */
#define MAC_FH_BT_IE            0x00000008 /* Bit to include BT IE */

/* Frequency hopping header IE's mask */
#define MAC_FH_HEADER_IE_MASK   0x000000FF

/* Frequency hopping payload IE's */
#define MAC_FH_US_IE            0x00010000 /* Bit to include US IE */
#define MAC_FH_BS_IE            0x00020000 /* Bit to include BS IE */
/* Frequency hopping payload IE's -- internal use */
#define MAC_FH_PAN_IE           0x00040000
#define MAC_FH_NETNAME_IE       0x00080000
#define MAC_FH_PANVER_IE        0x00100000
#define MAC_FH_GTKHASH_IE       0x00200000
/* Frequency hopping payload IE's mask */
#define MAC_FH_PAYLOAD_IE_MASK  0x00FF0000


/* Frequency hopping Protocol dispatch values */
#define MAC_FH_PROTO_DISPATCH_NONE    0x00  /* Protocol dispatch none */
#define MAC_FH_PROTO_DISPATCH_MHD_PDU 0x01  /* Protocol dispatch MHD-PDU */
#define MAC_FH_PROTO_DISPATCH_6LOWPAN 0x02  /* Protocol dispatch 6LOWPAN */

/* Maximum number of header ie's */
#define MAC_HEADER_IE_MAX                 2
/* Maximum number of payload-ie's */
#define MAC_PAYLOAD_IE_MAX                2
/* Maximum number of sub-ie's */
#define MAC_PAYLOAD_SUB_IE_MAX            4

/* WiSUN Async Start operation */
#define MAC_WS_OPER_ASYNC_START           0
/* WiSUN Async Stop operation */
#define MAC_WS_OPER_ASYNC_STOP            1

/* WiSUN Async PAN Advertisement Frame type */
#define MAC_WS_ASYNC_PAN_ADVERT           0x00
/* WiSUN Async PAN Advertisement Solicitation Frame type */
#define MAC_WS_ASYNC_PAN_ADVERT_SOL       0x01
/* WiSUN Async PAN Configuration Frame type */
#define MAC_WS_ASYNC_PAN_CONFIG           0x02
/* WiSUN Async PAN Configuration Solicitation Frame type */
#define MAC_WS_ASYNC_PAN_CONFIG_SOL       0x03

/* Invalid index of device table
 * The EUI entry can be found in the mace device table
 */
#define MAC_EUI_INVALID_INDEX             (0xFFFF)

/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

/* Returns the number of short addresses in the pending address specification */
#define MAC_PEND_NUM_SHORT(pendAddrSpec)  ((pendAddrSpec) & 0x07)

/* Returns the number of extended addresses in the pending address specification */
#define MAC_PEND_NUM_EXT(pendAddrSpec)    (((pendAddrSpec) & 0x70) >> 4)

/* Returns the length in bytes of the pending address fields in the beacon */
#define MAC_PEND_FIELDS_LEN(pendAddrSpec) ((MAC_PEND_NUM_SHORT(pendAddrSpec) * 2) + \
                                           (MAC_PEND_NUM_EXT(pendAddrSpec) * 8))

/* The following macros are provided to help parse the superframe specification */
#define MAC_SFS_BEACON_ORDER(s)           ((s) & 0x0F)          /* returns the beacon order */
#define MAC_SFS_SUPERFRAME_ORDER(s)       (((s) >> 4) & 0x0F)   /* returns the beacon order */
#define MAC_SFS_FINAL_CAP_SLOT(s)         (((s) >> 8) & 0x0F)   /* returns the final CAP slot */
#define MAC_SFS_BLE(s)                    (((s) >> 12) & 0x01)  /* returns the battery life extension bit */
#define MAC_SFS_PAN_COORDINATOR(s)        (((s) >> 14) & 0x01)  /* returns the PAN coordinator bit */
#define MAC_SFS_ASSOCIATION_PERMIT(s)     ((s) >> 15)           /* returns the association permit bit */

/* Sets or clears a specific bit in the scan channel map array */
#define MAC_SET_CHANNEL_MASK(a, c)     *((uint8*)(a) + ((c) - MAC_CHAN_LOWEST) / 8) |= \
                                        ((uint8) 1 << (((c) - MAC_CHAN_LOWEST) % 8))
#define MAC_CLR_CHANNEL_MASK(a, c)     *((uint8*)(a) + ((c) - MAC_CHAN_LOWEST) / 8) &= \
                                         ~((uint8) 1 << (((c) - MAC_CHAN_LOWEST) % 8))

/* Returns if the specific bit in the scan channel map array is set */
#define MAC_IS_CHANNEL_MASK_SET(a, c)  (*((uint8*)(a) + ((c) - MAC_CHAN_LOWEST) / 8) & \
                                        ((uint8) 1 << (((c) - MAC_CHAN_LOWEST) % 8)))

/* Type of Beacon */
#define MAC_BEACON_STANDARD     0 /* Beacon with frame version < 2 */
#define MAC_BEACON_ENHANCED     1 /* Beacon with frame version 2 */
/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* Stack version types */
typedef struct {
    uint8_t iv_major;
    uint8_t iv_minor;
    uint16_t iv_revision;
    uint32_t iv_build_num;
} mcuboot_image_version_t;

typedef struct __attribute__((__packed__)) {
    uint8_t major;
    uint8_t minor;
    uint8_t revision;
    uint8_t reserved;
} ti154stack_version_t;

typedef struct __attribute__((__packed__)) {
    uint8_t major;
    uint8_t minor;
} ti154stack_core_version_t;

/* Generic PHY Descriptor.
 * We are using this structure for both Channel Page 9 and Channel Pg 10.
 */
typedef struct
{
  uint32 firstChCentrFreq;            /* First Channel Center frequency */
  uint32 channelSpacing;              /* Distance between Adjacent center channel frequencies */
  uint16 noOfChannels;                /* Number of channels defined for the particular PHY mode */

  uint8 fskModScheme;                 /* 0: 2-FSK/2-GFSK; 1: 4-FSK/4-GFSK */

  uint16 symbolRate;                   /* Symbol rate selection */
  uint8 fskModIndex;                  /* Modulation index as a value encoded in
                                       * MR-FSK Generic PHY Descriptor IE
                                       * (IEEE802.15.4g section 5.2.4.20c).
                                       *
                                       * 2FSK MI = 0.25 + Modulation Index * 0.05
                                       * 4FSK MI is a third of 2FSK MI
                                       */

  uint8 ccaType;                      /* Channel clearance algorithm selection */
} macMRFSKPHYDesc_t;

#if defined(COMBO_MAC) || defined(FREQ_2_4G)
typedef macMRFSKPHYDesc_t macIEEEPHYDesc_t;
#endif

typedef macMRFSKPHYDesc_t macPHYDesc_t;

/* MAC event header type */
typedef struct
{
  uint8   event;              /* MAC event */
  uint8   status;             /* MAC status */
} macEventHdr_t;

/* Common security type */
typedef struct
{
  uint8   keySource[MAC_KEY_SOURCE_MAX_LEN];  /* Key source */
  uint8   securityLevel;                      /* Security level */
  uint8   keyIdMode;                          /* Key identifier mode */
  uint16  keyIndex;                           /* Key index */
} macSec_t;

/* Key ID Lookup Descriptor */
typedef struct
{
  uint8              lookupData[MAC_MAX_KEY_LOOKUP_LEN];  /* Data used to identify the key */
  uint8              lookupDataSize;  /* 0x00 indicates 5 octets; 0x01 indicates 9 octets. */
} keyIdLookupDescriptor_t;

/* Key Device Descriptor */
typedef struct
{
  uint16             deviceDescriptorHandle;  /* Handle to the DeviceDescriptor */
  bool               uniqueDevice;            /* Is it a link key or a group key? */
  bool               blackListed;             /* This key exhausted the frame counter. */
} keyDeviceDescriptor_t;

/* Key Usage Descriptor */
typedef struct
{
  uint8              frameType;               /* Frame Type */
  uint8              cmdFrameId;              /* Command Frame Identifier */
} keyUsageDescriptor_t;

/* Key Descriptor */
typedef struct
{
  keyIdLookupDescriptor_t  *keyIdLookupList;   /* A list identifying this KeyDescriptor */
  uint8                    keyIdLookupEntries; /* The number of entries in KeyIdLookupList */

  keyDeviceDescriptor_t    *keyDeviceList;        /* A list indicating which devices are
                                                     currently using this key, including
                                                     their blacklist status. */
  uint16                    keyDeviceListEntries;  /* The number of entries in KeyDeviceList */

  keyUsageDescriptor_t     *keyUsageList;         /* A list indicating which frame types
                                                   * this key may be used with. */
  uint8                    keyUsageListEntries;   /* The number of entries in KeyUsageList */

  uint8                    key[MAC_KEY_MAX_LEN];  /* The actual value of the key */
  uint32                   frameCounter;  /* PIB frame counter in 802.15.4 is universal across key,
                                           * but it makes more sense to associate a frame counter
                                           * with a key. */
} keyDescriptor_t;

/* Frame Counter */
typedef struct
{
    uint32             frameCounter;
    uint16             frameCntrIdx;
} frameCntr_t;

/* Device Descriptor */
typedef struct
{
  uint16             panID;          /* The 16-bit PAN identifier of the device */
  uint16             shortAddress;   /* The 16-bit short address of the device */
  sAddrExt_t         extAddress;     /* The 64-bit IEEE extended address of the
                                        device. This element is also used in
                                        unsecuring operations on incoming frames. */
  frameCntr_t          *frameCtr;
  //uint32             *frameCounter;
  //uint32             frameCounter[MAX_NODE_KEY_TABLE_ENTRIES];
                                     /* The incoming frame counter of the device
                                        This value is used to ensure sequential
                                        freshness of frames. */
  bool               exempt;         /* Device may override the minimum security
                                        level settings. */
} deviceDescriptor_t;

/* Security Level Descriptor */
typedef struct
{
  uint8              frameType;              /* Frame Type */
  uint8              commandFrameIdentifier; /* Command Frame ID */
  uint8              securityMinimum;        /* The minimal required/expected security
                                                level for incoming MAC frames. */
  bool               securityOverrideSecurityMinimum;
                                             /* Indication of whether originating devices
                                                for which the Exempt flag is set may
                                                override the minimum security level
                                                indicated by the SecurityMinimum
                                                element. If TRUE, this indicates that for
                                                originating devices with Exempt status,
                                                the incoming security level zero is
                                                acceptable. */
} securityLevelDescriptor_t;

/* For internal use only */
typedef struct
{
  uint16                  key_index;
  uint8                   key_id_lookup_index;
  keyIdLookupDescriptor_t macKeyIdLookupEntry;
} macSecurityPibKeyIdLookupEntry_t;

typedef struct
{
  uint16                  key_index;
  uint16                  key_device_index;
  keyDeviceDescriptor_t   macKeyDeviceEntry;
} macSecurityPibKeyDeviceEntry_t;

typedef struct
{
  uint16                  key_index;
  uint8                   key_key_usage_index;
  keyUsageDescriptor_t    macKeyUsageEntry;
} macSecurityPibKeyUsageEntry_t;

typedef struct
{
  uint16                  key_index;
  uint8                   keyEntry[MAC_KEY_MAX_LEN];
  uint32                  frameCounter;
} macSecurityPibKeyEntry_t;

typedef struct
{
  uint16                  device_index;
  deviceDescriptor_t      macDeviceEntry;
} macSecurityPibDeviceEntry_t;

typedef struct
{
  uint8                       security_level_index;
  securityLevelDescriptor_t   macSecurityLevelEntry;
} macSecurityPibSecurityLevelEntry_t;

typedef struct _payloadIeInfo
{
  uint8                 gId;
  union
  {
    uint8              id[MAC_PAYLOAD_SUB_IE_MAX];
    uint32             subIdBMap;
  } subId;
} payloadIeInfo_t;

typedef struct _headerIeInfo
{
  uint8                 elementId;
  uint32                subIdBitMap;

} headerIeInfo_t;


typedef struct
{
  uint32                timestamp;
  uint16                timestamp2;
  uint16                timeToLive;
  uint8                 frameType;
  uint16                txOptions;
  uint8                 txMode;
  uint8                 txSched;
  uint8                 retries;
  uint8                 channel;
  uint8                 power;
  uint8                 mpduLinkQuality;
  uint8                 correlation;
  int8                  rssi;
#if defined (TIMAC_ROM_IMAGE_BUILD) || defined(FEATURE_MAC_SECURITY)
  uint8*                ptrMData;       /* pointer to MData */
  keyDescriptor_t*      pKeyDesc;       /* pointer to the key descriptor */
  uint32                frameCntr;      /* frame counter */
#endif
  payloadIeInfo_t       payloadIeInfo[MAC_PAYLOAD_IE_MAX];
  headerIeInfo_t        headerIeInfo[MAC_HEADER_IE_MAX];
  uint16                totalLen;       /* total length of the allocated msdu buffer */
  uint8*                ptrHeaderIE;    /* pointer to the start of header IE field */
  uint16                headerIesLen;   /* length of header ie's */
  uint8*                ptrPayloadIE;   /* pointer to the start of Payload IE field */
  uint8*                ptrUTIE;        /* pointer to the UT IE Content */
  uint8*                ptrBTIE;        /* pointer to the BT IE Content */
  uint8*                ptrFrameCnt;    /* pointer to the frameCnt */
  uint8                 rsl;            /* update with the information from RSL-IE */
  uint8                 nb;             /* number of backoffs */
  uint8                 be;             /* backoff exponent */
  uint8                 fhFrameType;    /* Type of WiSun frame */
  uint8                 gpDuration;
  uint8                 gpNumOfTx;
  uint8                 gpInterframeDelay;
  union
  {
    sAddrExt_t          destAddr;       /* Destination address, extended */
    sAddr_t             dstAddr;       /* Destination address, extended */
    uint8               chList[MAC_154G_CHANNEL_BITMAP_SIZ]; /* Channel List as a bitmap */
  } dest;
} macTxIntData_t;

/* For internal use only */
typedef struct
{
  uint16            fcf;
  uint8             fhFrameType;
  uint8             cmdFrameId;
  uint8             flags;
#if defined (TIMAC_ROM_IMAGE_BUILD) || defined(FEATURE_MAC_SECURITY)
  uint8*            ptrCData;
#endif
  uint32            frameCntr;
} macRxIntData_t;

/* Data request parameters type */
typedef struct
{
  sAddr_t         dstAddr;           /* The address of the destination device */
  uint16          dstPanId;          /* The PAN ID of the destination device */
  uint8           srcAddrMode;       /* The source address mode */
  uint8           msduHandle;        /* Application-defined handle value associated with this data request */
  uint16          txOptions;         /* TX options bit mask */
  uint8           channel;           /* Transmit the data frame on this channel */
  uint8           power;             /* Transmit the data frame at this power level */
  uint8*          pIEList;           /* pointer to the payload IE list, excluding termination IEs */
  uint16          payloadIELen;      /* length of the payload IE�s */
  uint8           fhProtoDispatch;   /* Not Used, RESERVED for future. Shall be set to zero */
  uint32          includeFhIEs;      /* Bitmap indicates which FH IE's need to be included */
} macDataReq_t;

/* MCPS data request type */
typedef struct
{
  macEventHdr_t   hdr;        /* Internal use only */
  sData_t         msdu;       /* Data pointer and length */
  macTxIntData_t  internal;   /* Internal use only */
  ApiMac_sec_t    sec;        /* Security parameters */
  macDataReq_t    mac;        /* Data request parameters */
} macMcpsDataReq_t;

/* Data indication parameters type */
typedef struct
{
  sAddr_t   srcAddr;          /* The address of the sending device */
  sAddr_t   dstAddr;          /* The address of the destination device */
  uint32    timestamp;        /* The time, in backoffs, at which the data were received */
  uint16    timestamp2;       /* The time, in internal MAC timer units, at which the
                                 data were received */
  uint16    srcPanId;         /* The PAN ID of the sending device */
  uint16    dstPanId;         /* The PAN ID of the destination device */
  uint8     mpduLinkQuality;  /* The link quality of the received data frame */
  uint8     correlation;      /* The raw correlation value of the received data frame */
  int8      rssi;             /* The received RF power in units dBm */
  uint8     dsn;              /* The data sequence number of the received frame */
  uint8*    pPayloadIE;       /* Pointer to the start of payload IE's */
  uint16    payloadIeLen;     /* length of payload ie if any */
  uint8     fhProtoDispatch;  /* Not Used, RESERVED for future. */
  uint32    frameCntr;        /* Frame counter value of the received data frame (if used) */
} macDataInd_t;


/* MCPS data indication type */
typedef struct
{
  macEventHdr_t  hdr;         /* Internal use only */
  sData_t        msdu;        /* Data pointer and length */
  macRxIntData_t internal;    /* Internal use only */
  ApiMac_sec_t   sec;         /* Security parameters */
  macDataInd_t   mac;         /* Data indication parameters */
} macMcpsDataInd_t;

/* MCPS data confirm type */
typedef struct
{
  macEventHdr_t      hdr;              /* Contains the status of the data request operation */
  uint8              msduHandle;       /* Application-defined handle value associated with the data request */
  macMcpsDataReq_t   *pDataReq;        /* Pointer to the data request buffer for this data confirm */
  uint32             timestamp;        /* The time, in backoffs, at which the frame was transmitted */
  uint16             timestamp2;       /* The time, in internal MAC timer units, at which the
                                          frame was transmitted */
  uint8              retries;          /* The number of retries required to transmit the data frame */
  uint8              mpduLinkQuality;  /* The link quality of the received ack frame */
  uint8              correlation;      /* The raw correlation value of the received ack frame */
  int8               rssi;             /* The RF power of the received ack frame in units dBm */
  uint32             frameCntr;        /* Frame counter value used (if any) for the transmitted frame */
} macMcpsDataCnf_t;


/* MCPS purge confirm type */
typedef struct
{
  macEventHdr_t      hdr;         /* Contains the status of the purge request operation */
  uint8              msduHandle;  /* Application-defined handle value associated with the data request */
} macMcpsPurgeCnf_t;

/* PAN descriptor type */
typedef struct
{
  sAddr_t       coordAddress;     /* The address of the coordinator sending the beacon */
  uint16        coordPanId;       /* The PAN ID of the network */
  uint16        superframeSpec;   /* The superframe specification of the network */
  uint8         logicalChannel;   /* The logical channel of the network */
  uint8         channelPage;      /* The current channel page occupied by the network */
  bool          gtsPermit;        /* TRUE if coordinator accepts GTS requests. Don't care for enhanced beacon */
  uint8         linkQuality;      /* The link quality of the received beacon */
  uint32        timestamp;        /* The time at which the beacon was received, in backoffs */
  bool          securityFailure;  /* Set to TRUE if there was an error in the security processing */
  ApiMac_sec_t  sec;              /* The security parameters for the received beacon frame */
} macPanDesc_t;

/* MLME associate request type */
typedef struct
{
  ApiMac_sec_t  sec;                    /* The security parameters for this message */
  uint8         logicalChannel;         /* The channel on which to attempt association */
  uint8         channelPage;            /* The channel page on which to attempt association */
  uint8         phyID;                  /* Identifier for the PHY descriptor */
  sAddr_t       coordAddress;           /* Address of the coordinator with which to associate */
  uint16        coordPanId;             /* The identifier of the PAN with which to associate */
  uint8         capabilityInformation;  /* The operational capabilities of this device */
} macMlmeAssociateReq_t;

/* MLME associate response type */
typedef struct
{
  ApiMac_sec_t  sec;                 /* The security parameters for this message */
  sAddrExt_t    deviceAddress;       /* The address of the device requesting association */
  uint16        assocShortAddress;   /* The short address allocated to the device */
  uint8         status;              /* The status of the association attempt */
} macMlmeAssociateRsp_t;

/* MLME disassociate request type */
typedef struct
{
  ApiMac_sec_t  sec;                 /* The security parameters for this message */
  sAddr_t       deviceAddress;       /* The address of the device with which to disassociate */
  uint16        devicePanId;         /* The PAN ID of the device */
  uint8         disassociateReason;  /* The disassociate reason */
  bool          txIndirect;          /* Transmit Indirect */
} macMlmeDisassociateReq_t;


/* MLME orphan response type */
typedef struct
{
  ApiMac_sec_t  sec;                /* The security parameters for this message */
  sAddrExt_t    orphanAddress;      /* The extended address of the device sending the orphan notification */
  uint16        shortAddress;       /* The short address of the orphaned device */
  bool          associatedMember;   /* Set to TRUE if the orphaned device is associated with this coordinator */
} macMlmeOrphanRsp_t;

/* MLME poll request type */
typedef struct
{
  sAddr_t       coordAddress;       /* The address of the coordinator device to poll */
  uint16        coordPanId;         /* The PAN ID of the coordinator */
  ApiMac_sec_t  sec;                /* The security parameters for this message */
} macMlmePollReq_t;

/* MLME scan request type */
typedef struct
{
  uint8            scanChannels[MAC_154G_CHANNEL_BITMAP_SIZ];    /* Bit mask indicating which channels to scan */
  uint8            scanType;        /* The type of scan */
  uint8            scanDuration;    /* The exponent used in the scan duration calculation */
  uint8            channelPage;     /* The channel page on which to perform the scan */
  uint8            phyID;           /* PHY ID corresponding to the PHY descriptor to use */
  uint8            maxResults;      /* The maximum number of PAN descriptor results */
  bool             permitJoining;   /* Only devices with permit joining on respond to the
                                       enahnced beacon request */
  uint8            linkQuality;     /* The device will respond to the enhanced beacon request
                                       if mpduLinkQuality is equal or higher than this value */
  uint8            percentFilter;   /* The device will then randomly determine if it is to
                                       respond to the enhanced beacon request based on meeting
                                       this probability (0 to 100%). */
  ApiMac_sec_t     sec;             /* The security parameters for orphan scan */
  bool             MPMScan;         /* When TRUE, scanDuration is ignored. When FALSE, scan duration
                                      shall be set to scanDuration; MPMScanDuration is ignored */
  uint8            MPMScanType;     /* BPAN or NBPAN */
  uint16           MPMScanDuration; /* If MPMScanType is BPAN, MPMScanDuration values are 0-14.
                                      It is used in determining the max time spent scanning
                                      for an EB in a beacon enabled PAN on the channel.
                                      [aBaseSuperframeDuration * 2^n symbols], where
                                       n is the MPMScanDuration.
                                      If MPMScanType is NBPAN, valid values are 1 - 16383.
                                      It is used in determining the max time spent scanning for an
                                      EB in nonbeacon-enabled PAN on the channel.
                                      [aBaseSlotDuration * n] symbols, where n
                                      is MPMScanDuration. */

  union {
    uint8         *pEnergyDetect;   /* Pointer to a buffer to store energy detect measurements */
    macPanDesc_t  *pPanDescriptor;  /* Pointer to a buffer to store PAN descriptors */
  } result;
} macMlmeScanReq_t;

/* MPM(Multi-PHY layer management) parameters */
typedef struct _mpmParams
{
  uint8       eBeaconOrder;       /* The exponent used to calculate the enhanced beacon interval.
                                     A value of 15 indicates no EB in a beacon enabled PAN */
  uint8       offsetTimeSlot;     /* The offset between the start of the periodic beacon
                                     transmission and the start of the following EB transmission
                                     expressed in superframe time slots. Supported values are from 10-15 */
  uint16      NBPANEBeaconOrder;  /* Indicates how often the EB to tx in a non-beacon enabled PAN. A value of
                                     16383 indicates no EB in a non-beacon enabled PAN */
  uint8*      pIEIDs;             /* RESERVED for future, shall be set to NULL */
  uint8       numIEs;             /* RESERVED for future, shall be set to zero */

} macMpmParams_t;


/* MLME start request type */
typedef struct
{
  uint32          startTime;          /* The time to begin transmitting beacons relative to the received beacon */
  uint16          panId;              /* The PAN ID to use.  This parameter is ignored if panCoordinator is FALSE */
  uint8           logicalChannel;     /* The logical channel to use.  This parameter is ignored if panCoordinator is FALSE */
  uint8           channelPage;        /* The channel page to use.  This parameter is ignored if panCoordinator is FALSE */
  uint8           phyID;              /* PHY ID corresponding to the PHY descriptor to use */
  uint8           beaconOrder;        /* The exponent used to calculate the beacon interval */
  uint8           superframeOrder;    /* The exponent used to calculate the superframe duration */
  bool            panCoordinator;     /* Set to TRUE to start a network as PAN coordinator */
  bool            batteryLifeExt;     /* If this value is TRUE, the receiver is disabled after MAC_BATT_LIFE_EXT_PERIODS
                                         full backoff periods following the interframe spacing period of the beacon frame */
  bool            coordRealignment;   /* Set to TRUE to transmit a coordinator realignment prior to changing
                                         the superframe configuration */
  ApiMac_sec_t    realignSec;         /* Security parameters for the coordinator realignment frame */
  ApiMac_sec_t    beaconSec;          /* Security parameters for the beacon frame */
  macMpmParams_t  mpmParams;          /* MPM (multi-PHY layer management) parameters */
  bool            startFH;            /* Indicates whether frequency hopping needs to be started */
} macMlmeStartReq_t;

/* MAC_MlmeSyncReq type */
typedef struct
{
  uint8       logicalChannel;     /* The logical channel to use */
  uint8       channelPage;        /* The channel page to use */
  uint8       phyID;              /* PHY ID corresponding to the PHY descriptor to use */
  uint8       trackBeacon;        /* Set to TRUE to continue tracking beacons after synchronizing with the
                                     first beacon.  Set to FALSE to only synchronize with the first beacon */
} macMlmeSyncReq_t;

/* MLME WiSUN Async request type */
typedef struct _macMlmeWSAsyncReq
{
  ApiMac_sec_t sec;    /* The security parameters */
  uint8        operation; /* Start or Stop Async operation */
  uint8        frameType; /* Async frame type */
  uint8        channels[MAC_154G_CHANNEL_BITMAP_SIZ]; /* Bit Mask indicating which channels to send
                                                      the Async frames for the start operation */

} macMlmeWSAsyncReq_t;

/* MAC_MLME_ASSOCIATE_IND type */
typedef struct
{
  macEventHdr_t       hdr;                    /* The event header */
  sAddrExt_t          deviceAddress;          /* The address of the device requesting association */
  uint8               capabilityInformation;  /* The operational capabilities of the device requesting association */
  ApiMac_sec_t        sec;                    /* The security parameters for this message */
} macMlmeAssociateInd_t;

/* MAC_MLME_ASSOCIATE_CNF type */
typedef struct
{
  macEventHdr_t       hdr;                    /* Event header contains the status of the associate attempt */
  uint16              assocShortAddress;      /* If successful, the short address allocated to this device */
  ApiMac_sec_t        sec;                    /* The security parameters for this message */
} macMlmeAssociateCnf_t;

/* MAC_MLME_DISASSOCIATE_IND type */
typedef struct
{
  macEventHdr_t       hdr;                    /* The event header */
  sAddrExt_t          deviceAddress;          /* The address of the device sending the disassociate command */
  uint8               disassociateReason;     /* The disassociate reason */
  ApiMac_sec_t        sec;                    /* The security parameters for this message */
} macMlmeDisassociateInd_t;

/* MAC_MLME_DISASSOCIATE_CNF type */
typedef struct
{
  macEventHdr_t   hdr;                    /* Event header contains the status of the disassociate attempt */
  sAddr_t         deviceAddress;          /* The address of the device that has either requested disassociation
                                             or been instructed to disassociate by its coordinator */
  uint16          panId;                  /* The pan ID of the device that has either requested disassociation
                                             or been instructed to disassociate by its coordinator */
} macMlmeDisassociateCnf_t;

/* MAC Beacon data type */
typedef struct
{
  macPanDesc_t   *pPanDesc;       /* The PAN descriptor for the received beacon */
  uint8          pendAddrSpec;    /* The beacon pending address specification */
  uint8          *pAddrList;      /* The list of device addresses for which the sender of the beacon has data */
  uint8          sduLength;       /* The number of bytes in the beacon payload of the beacon frame */
  uint8          *pSdu;           /* The beacon payload */
} macBeaconData_t;

/* Coexistence Information element content type */
typedef struct
{
  uint8         beaconOrder;       /* Beacon Order field shall specify the transmission interval of the beacon */
  uint8         superFrameOrder;   /* Superframe Order field shall specify the length of time during which the superframe
                                      is active (i.e., receiver enabled), including the Beacon frame transmission time*/
  uint8         finalCapSlot;
  uint8         eBeaconOrder;      /* Enhanced Beacon Order field specifies the transmission interval
                                      of the Enhanced Beacon frames in a beacon enabled network */
  uint8         offsetTimeSlot;    /* time offset between periodic beacon and the Enhanced Beacon */
  uint8         capBackOff;        /* actual slot position in which the Enhanced Beacon frame is
                                      transmitted due to the backoff procedure in the CAP.*/
  uint16        eBeaconOrderNBPAN; /* NBPAN Enhanced Beacon Order field specifies the transmission interval between consecutive
                                      Enhanced Beacon frames in the nonbeacon-enabled mode */
} macCoexist_t;

/* MAC Enhanced beacon data type */
typedef struct
{
  macPanDesc_t   *pPanDesc;       /* The PAN descriptor for the received beacon */
  macCoexist_t   coexist;         /* coexistence information */

} macEBeaconData_t;

/* MAC_MLME_BEACON_NOTIFY_IND type */
typedef struct
{
  macEventHdr_t  hdr;             /* The event header */
  uint8          beaconType;      /* Indicates the beacon type: beacon or enhanced beacon */
  uint8          bsn;             /* The beacon sequence number or enhanced beacon sequence number */
  union
  {
    macBeaconData_t beaconData;
    macEBeaconData_t eBeaconData;
  } info;
} macMlmeBeaconNotifyInd_t;


/* MAC_MLME_ORPHAN_IND type */
typedef struct
{
  macEventHdr_t       hdr;            /* The event header */
  sAddrExt_t          orphanAddress;  /* The address of the orphaned device */
  ApiMac_sec_t        sec;            /* The security parameters for this message */
} macMlmeOrphanInd_t;

/* MAC_MLME_SCAN_CNF type */
typedef struct
{
  macEventHdr_t   hdr;                /* Event header contains the status of the scan request */
  uint8           scanType;           /* The type of scan requested */
  uint8           channelPage;        /* The channel page of the scan */
  uint8           phyID;              /* PHY ID corresponding to the PHY descriptor used during scan */
  uint8           unscannedChannels[MAC_154G_CHANNEL_BITMAP_SIZ]; /* Bit mask of channels that were not scanned */
  uint8           resultListSize;     /* The number of PAN descriptors returned in the results list */
  union
  {
    uint8         *pEnergyDetect;     /* The list of energy measurements, one for each channel scanned */
    macPanDesc_t  *pPanDescriptor;    /* The list of PAN descriptors, one for each beacon found */
  } result;
} macMlmeScanCnf_t;

/* MAC_MLME_START_CNF type */
typedef struct
{
  macEventHdr_t   hdr;            /* Event header contains the status of the start request */
} macMlmeStartCnf_t;

/* MAC_MLME_SYNC_LOSS_IND type */
typedef struct
{
  macEventHdr_t   hdr;            /* Event header contains the reason that synchronization was lost */
  uint16          panId;          /* The PAN ID of the realignment */
  uint8           logicalChannel; /* The logical channel of the realignment */
  uint8           channelPage;    /* The channel page of the realignment */
  uint8           phyID;          /* PHY ID corresponding to the PHY descriptor of the realignment */
  ApiMac_sec_t    sec;            /* The security parameters for this message */
} macMlmeSyncLossInd_t;

/* MAC_MLME_POLL_CNF type */
typedef struct
{
  macEventHdr_t   hdr;            /* Event header contains the status of the poll request */
  uint8           framePending;   /* Set if framePending bit in data packet is set */
} macMlmePollCnf_t;

/* MAC_MLME_COMM_STATUS_IND type */
typedef struct
{
  macEventHdr_t   hdr;            /* Event header contains the status for this event */
  sAddr_t         srcAddr;        /* The source address associated with the event */
  sAddr_t         dstAddr;        /* The destination address associated with the event */
  uint16          panId;          /* The PAN ID associated with the event */
  uint8           reason;         /* The reason the event was generated */
  ApiMac_sec_t    sec;            /* The security parameters for this message */
} macMlmeCommStatusInd_t;

/* MAC_MLME_POLL_IND type */
typedef struct
{
  macEventHdr_t   hdr;
  sAddr_t         srcAddr;        /* Short address of the device sending the data request */
  uint16          srcPanId;       /* Pan ID of the device sending the data request */
  uint8           noRsp;          /* indication that no MAC_McpsDataReq() is required.
                                   * It is set when MAC_MLME_POLL_IND is generated,
                                   * to simply indicate that a received data request frame
                                   * was acked with pending bit cleared. */
} macMlmePollInd_t;

/* MAC_MLME_WS_ASYNC_FRAME_IND type */
typedef macMcpsDataInd_t macMlmeWSAsyncInd_t;

/* MAC_MLME_WS_ASYNC_FRAME_CNF type */
typedef struct _macMlmeWSAsyncCnf
{
  macEventHdr_t   hdr;

} macMlmeWSAsyncCnf_t;

/* MAC_NPI_MT_MSG type  */
typedef struct _macNpiMtMsg
{
  macEventHdr_t       hdr;                    /* Event header contains the status of the associate attempt */
                                              /* message pointer to NPI MAC message */
} macNpiMtMsg_t;

/* Union of callback structures */
typedef union
{
  macEventHdr_t            hdr;
  macMlmeAssociateInd_t    associateInd;      /* MAC_MLME_ASSOCIATE_IND */
  macMlmeAssociateCnf_t    associateCnf;      /* MAC_MLME_ASSOCIATE_CNF */
  macMlmeDisassociateInd_t disassociateInd;   /* MAC_MLME_DISASSOCIATE_IND */
  macMlmeDisassociateCnf_t disassociateCnf;   /* MAC_MLME_DISASSOCIATE_CNF */
  macMlmeBeaconNotifyInd_t beaconNotifyInd;   /* MAC_MLME_BEACON_NOTIFY_IND */
  macMlmeOrphanInd_t       orphanInd;         /* MAC_MLME_ORPHAN_IND */
  macMlmeScanCnf_t         scanCnf;           /* MAC_MLME_SCAN_CNF */
  macMlmeStartCnf_t        startCnf;          /* MAC_MLME_START_CNF */
  macMlmeSyncLossInd_t     syncLossInd;       /* MAC_MLME_SYNC_LOSS_IND */
  macMlmePollCnf_t         pollCnf;           /* MAC_MLME_POLL_CNF */
  macMlmeCommStatusInd_t   commStatusInd;     /* MAC_MLME_COMM_STATUS_IND */
  macMlmePollInd_t         pollInd;           /* MAC_MLME_POLL_IND */
  macMcpsDataCnf_t         dataCnf;           /* MAC_MCPS_DATA_CNF */
  macMcpsDataInd_t         dataInd;           /* MAC_MCPS_DATA_IND */
  macMcpsPurgeCnf_t        purgeCnf;          /* MAC_MCPS_PURGE_CNF */
  macMlmeWSAsyncInd_t      asyncInd;          /* MAC_MLME_WS_ASYNC_FRAME_IND */
  macMlmeWSAsyncCnf_t      asyncCnf;          /* MAC_MLME_WS_ASYNC_FRAME_CNF */
  macNpiMtMsg_t            npiMtMsg;          /* NPI MT message */
} macCbackEvent_t;

/* Configurable parameters */
typedef struct
{
  const uint8   txDataMax;                /* maximum number of data frames in transmit queue */
  const uint8   txMax;                    /* maximum number of frames of all types in transmit queue */
  const uint8   rxMax;                    /* maximum number of command and data frames in receive queue */
  const uint8   dataIndOffset;            /* allocate additional bytes in the data indication for
                                             application-defined headers */
  const uint16  maxDeviceTableEntries;    /* max Number of Devices */
  const uint16  maxKeyDeviceTableEntries; /* max Number of Devices per Key */
  const uint16  maxKeyTableEntries;       /* max Number of Keys */
  const uint8   maxNodeKeyTableEntries;   /* max Number of Keys per Node */
  const uint8   maxKeyIdLookUpEntries;    /* max Number of LookUp descriptors per Key */
  const bool    appPendingQueue;          /* determine whether MAC_MLME_POLL_IND will be sent to the application or not
                                             when data request is received and no pending frame is found in the MAC */
  uint16        macMaxFrameSize;          /* Maximum application data length without security */
} macCfg_t;

#ifdef MAC_DUTY_CYCLE_CHECKING
typedef enum
{
    dcModeNormal,
    dcModeLimited,
    dcModeCritical,
    dcModeRegulated
}macDutyCycleMode_t;
#endif

/* ------------------------------------------------------------------------------------------------
 *                                        Global Variables
 * ------------------------------------------------------------------------------------------------
 */

extern const ti154stack_version_t ti154stack_version;
extern const ti154stack_core_version_t ti154stack_core_version;
extern uint16_t symbol_rate_custom;

/* ------------------------------------------------------------------------------------------------
 *                                        Internal Functions
 * ------------------------------------------------------------------------------------------------
 */

/* These functions are used when creating the OSAL MAC task.  They must not be used for any
 * other purpose.
 */
extern void macMainSetTaskId(uint8 taskId);
extern uint16 macEventLoop(uint8 taskId, uint16 events);

/* ------------------------------------------------------------------------------------------------
 *                                           Functions
 * ------------------------------------------------------------------------------------------------
 */

/**************************************************************************************************
 * @fn          MAC_Init
 *
 * @brief       This function initializes the MAC subsystem.  It must be called once when the
 *              software system is started and before any other function in the MAC API
 *              is called.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
extern void MAC_Init(void);

/**************************************************************************************************
 * @fn          MAC_InitDevice
 *
 * @brief       This function initializes the MAC to associate with a non
 *              beacon-enabled network.  This function would be used to
 *              initialize a device as an RFD.  If this function is used it
 *              must be called during application initialization before any
 *              other function in the data or management API is called.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
extern void MAC_InitDevice(void);

/**************************************************************************************************
 * @fn          MAC_InitCoord
 *
 * @brief       This function initializes the MAC for operation as a
 *              coordinator.  A coordinator can start a network, accept
 *              associate requests from other devices, send beacons, send
 *              indirect data, and other operations.  This function would
 *              be used to initialize a device as an FFD.  If this function
 *              is used it must be called during application initialization
 *              before any other function in the data or management API
 *              is called.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
extern void MAC_InitCoord(void);

/**************************************************************************************************
 * @fn          MAC_InitBeaconCoord
 *
 * @brief       This function initializes the MAC for operation as a coordinator in a
 *              beacon-enabled network.  If this function is used it must
 *              be called during application initialization before any other
 *              function in the data or management API is called.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
extern void MAC_InitBeaconCoord(void);

/**************************************************************************************************
 * @fn          MAC_InitBeaconTrack
 *
 * @brief       This function initializes the MAC to allow it to associate
 *              with and track a beacon-enabled network.  If this function is
 *              used it must be called during application initialization
 *              before any other function in the data or management API
 *              is called.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
extern void MAC_InitBeaconDevice(void);

/**
 * @brief This function initializes the MAC to allow Frequency
 *        hopping operation.
 *
 * @return None
 */
extern void MAC_InitFH(void);

/**
 * @brief This function start the Frequency hopping operation.
 *        Frequency hopping operation should have been enabled
 *        before calling this API.
 *
 * @return MAC_SUCCESS or MAC_UNSUPPORTED.
 */
extern uint8 MAC_StartFH(void);

/**
 * @brief Enables the Frequency hopping operation.
 *
 * @return MAC_SUCCESS if successful in enabling the frequency
 *         hopping, otherwise MAC_UNSUPPORTED.
 */
extern uint8 MAC_EnableFH(void);

/**************************************************************************************************
 * @fn          MAC_McpsDataReq
 *
 * @brief       This function sends application data to the MAC for
 *              transmission in a MAC data frame.
 *
 * input parameters
 *
 * @param       pData - Pointer to parameters structure.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
extern uint8 MAC_McpsDataReq(ApiMac_mcpsDataReq_t *pData);

/**************************************************************************************************
 * @fn          MAC_McpsPurgeReq
 *
 * @brief       This function purges and discards a data request from the
 *              MAC data queue.  When the operation is complete the MAC sends
 *              a MAC_MCPS_PURGE_CNF.
 *
 * input parameters
 *
 * @param       msduHandle - The application-defined handle value
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SUCCESS or MAC error code.
 **************************************************************************************************
 */
extern uint8 MAC_McpsPurgeReq(uint8 msduHandle);

/**
 * @brief       This direct-execute function simplifies the allocation and
 *              preparation of the data buffer MAC_McpsDataReq().  The
 *              function allocates a buffer and prepares the data pointer.
 *
 * @param       len - Length of the application data.
 * @param       securityLevel - security suire used for this frame.
 * @param       keyIdMode - Key Id mode used for this frame.
 *                  Possible values are:
 *                  MAC_KEY_ID_MODE_NONE        0x00
 *                  MAC_KEY_ID_MODE_IMPLICIT    0x00
 *                  MAC_KEY_ID_MODE_1           0x01
 *                  MAC_KEY_ID_MODE_4           0x02
 *                  MAC_KEY_ID_MODE_8           0x03
 *                  Set to MAC_KEY_ID_MODE_NONE if security is
 *                  not used.
 * @param       includeFhIEs - bitmap indicating which FH IE's
 *                           need to be included.
 * @param       payloadIeLen - length of the application payload IE's if any.
 *                             This function will allocate the buffer for the
 *                             payload IE's and set the data element pIEList
 *                             of the macDataReq_t in macMcpsDataReq_t
 *                             appropriately.
 *
 * @return      Returns a pointer to the allocated buffer. If
 *              the function fails for any reason it returns
 *              NULL.
 */
extern macMcpsDataReq_t*
MAC_McpsDataAlloc(uint16 len, uint8 securityLevel, uint8 keyIdMode,
                  uint32 includeFhIEs, uint16 payloadIeLen);

/**************************************************************************************************
 * @fn          MAC_MlmeAssociateReq
 *
 * @brief       This function sends an associate request to a coordinator
 *              device.  The application shall attempt to associate only with
 *              a PAN that is currently allowing association, as indicated
 *              in the results of the scanning procedure. In a beacon-enabled
 *              PAN the beacon order and superframe order must be set by using
 *              MAC_MlmeSetReq() before making the call to MAC_MlmeAssociateReq().
 *              If not, the associate request frame is likely to be transmitted
 *              outside the superframe.  When the associate request is complete
 *              the MAC sends a MAC_MLME_ASSOCIATE_CNF to the application.
 *
 * input parameters
 *
 * @param       pData - Pointer to parameters structure.
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SUCCESS or MAC error code.
 **************************************************************************************************
 */
extern uint8 MAC_MlmeAssociateReq(ApiMac_mlmeAssociateReq_t *pData);

/**************************************************************************************************
 * @fn          MAC_MlmeAssociateRsp
 *
 * @brief       This function sends an associate response to a device
 *              requesting to associate.  This function must be called after
 *              receiving a MAC_MLME_ASSOCIATE_IND.  When the associate response is
 *              complete the MAC sends a MAC_MLME_COMM_STATUS_IND to the application
 *              to indicate the success or failure of the operation.
 *
 * input parameters
 *
 * @param       pData - Pointer to parameters structure.
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SUCCESS or MAC error code.
 **************************************************************************************************
 */
extern uint8 MAC_MlmeAssociateRsp(ApiMac_mlmeAssociateRsp_t *pData);

/**************************************************************************************************
 * @fn          MAC_MlmeDisassociateReq
 *
 * @brief       This function is used by an associated device to notify the
 *              coordinator of its intent to leave the PAN.  It is also used
 *              by the coordinator to instruct an associated device to leave
 *              the PAN.  When the disassociate is complete the MAC sends a
 *              MAC_MLME_DISASSOCIATE_CNF to the application.
 *
 * input parameters
 *
 * @param       pData - Pointer to parameters structure.
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SUCCESS or MAC error code.
 **************************************************************************************************
 */
extern uint8 MAC_MlmeDisassociateReq(ApiMac_mlmeDisassociateReq_t *pData);

/**************************************************************************************************
 * @fn          MAC_MlmeGetReq
 *
 * @brief       This direct execute function retrieves an attribute value
 *              from the MAC PIB.
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 * @param       pValue - pointer to the attribute value.
 *
 * output parameters
 *
 * @param       pValue - pointer to the attribute value.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *
 **************************************************************************************************
 */
extern uint8 MAC_MlmeGetReq(uint8 pibAttribute, void *pValue);

/**************************************************************************************************
 * @fn          MAC_MlmeGetSecurityReq
 *
 * @brief       This direct execute function retrieves an attribute value
 *              from the MAC Secutity PIB. This function only exists when
 *              FEATURE_MAC_SECURITY is defined.
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 * @param       pValue - pointer to the attribute value.
 *
 * output parameters
 *
 * @param       pValue - pointer to the attribute value.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *
 **************************************************************************************************
 */
extern uint8 MAC_MlmeGetSecurityReq(uint8 pibAttribute, void *pValue);

/**************************************************************************************************
 * @fn          MAC_MlmeGetSecurityReqSize
 *
 * @brief       This direct execute function gets the MAC security PIB attribute size
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 *
 * output parameters
 *
 * None.
 *
 * @return      size in bytes
 *
 **************************************************************************************************
 */
extern uint8 MAC_MlmeGetSecurityReqSize(uint8 pibAttribute);

/**************************************************************************************************
 * @fn          MAC_MlmeOrphanRsp
 *
 * @brief       This function is called in response to an orphan notification
 *              from a peer device.  This function must be called after
 *              receiving a MAC_MLME_ORPHAN_IND.  When the orphan response is
 *              complete the MAC sends a MAC_MLME_COMM_STATUS_IND to the
 *              application to indicate the success or failure of the operation.
 *
 * input parameters
 *
 * @param       pData - Pointer to parameters structure.
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SUCCESS or MAC error code.
 **************************************************************************************************
 */
extern uint8 MAC_MlmeOrphanRsp(ApiMac_mlmeOrphanRsp_t *pData);

/**************************************************************************************************
 * @fn          MAC_MlmePollReq
 *
 * @brief       This function is used to request pending data from the
 *              coordinator.  When the poll request is complete the MAC sends
 *              a MAC_MLME_POLL_CNF to the application.  If a data frame of
 *              nonzero length is received from the coordinator the MAC sends
 *              a MAC_MLME_POLL_CNF with status MAC_SUCCESS and then sends a
 *              MAC_MCPS_DATA_IND with the data.
 *
 * input parameters
 *
 * @param       pData - Pointer to parameters structure.
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SUCCESS or MAC error code.
 **************************************************************************************************
 */
extern uint8 MAC_MlmePollReq(ApiMac_mlmePollReq_t *pData);

/**************************************************************************************************
 * @fn          MAC_MlmeResetReq
 *
 * @brief       This direct execute function resets the MAC.  This function
 *              must be called once at system startup before any other
 *              function in the management API is called.
 *
 * input parameters
 *
 * @param       setDefaultPib - Set to TRUE to reset the MAC PIB to its
 *                              default values.
 *
 * output parameters
 *
 * None.
 *
 * @return      Returns MAC_SUCCESS always.
 *
 **************************************************************************************************
 */
extern uint8 MAC_MlmeResetReq(bool setDefaultPib);

/**************************************************************************************************
 * @fn          MAC_MlmeScanReq
 *
 * @brief       This function initiates an energy detect, active, passive,
 *              or orphan scan on one or more channels.  An energy detect
 *              scan measures the peak energy on each requested channel.
 *              An active scan sends a beacon request on each channel and
 *              then listening for beacons.  A passive scan is a receive-only
 *              operation that listens for beacons on each channel.  An orphan
 *              scan is used to locate the coordinator with which the scanning
 *              device had previously associated.  When a scan operation is
 *              complete the MAC sends a MAC_MLME_SCAN_CNF to the application.
 *              Note: For energy detect scan a large enough buffer equal to the
 *              maximum number of channels should be provided.
 *
 * input parameters
 *
 * @param       pData - Pointer to parameters structure.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
extern ApiMac_status_t MAC_MlmeScanReq(ApiMac_mlmeScanReq_t *pData);

/**************************************************************************************************
 * @fn          MAC_MlmeSetReq
 *
 * @brief       This direct execute function sets an attribute value
 *              in the MAC PIB.
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 * @param       pValue - pointer to the attribute value.
 *
 * output parameters
 *
 * None.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *
 **************************************************************************************************
 */
extern uint8 MAC_MlmeSetReq(uint8 pibAttribute, void *pValue);

/**************************************************************************************************
 * @fn          MAC_MlmeSetSecurityReq
 *
 * @brief       This direct execute function sets an attribute value
 *              in the MAC Security PIB. This function only exists when
 *              FEATURE_MAC_SECURITY is defined.
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 * @param       pValue - pointer to the attribute value.
 *
 * output parameters
 *
 * None.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *
 **************************************************************************************************
 */
extern uint8 MAC_MlmeSetSecurityReq(uint8 pibAttribute, void *pValue);

/**************************************************************************************************
 * @fn          MAC_MlmeStartReq
 *
 * @brief       This function is called by a coordinator or PAN coordinator
 *              to start or reconfigure a network.  Before starting a
 *              network the device must have set its short address.  A PAN
 *              coordinator sets the short address by setting the attribute
 *              MAC_SHORT_ADDRESS.  A coordinator sets the short address
 *              through association.  When the operation is complete the
 *              MAC sends a MAC_MLME_START_CNF to the application.
 *
 * input parameters
 *
 * @param       pData - Pointer to parameters structure.
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SUCCESS or MAC error code.
 **************************************************************************************************
 */
extern uint8 MAC_MlmeStartReq(ApiMac_mlmeStartReq_t *pData);

/**************************************************************************************************
 * @fn          MAC_MlmeSyncReq
 *
 * @brief       This function requests the MAC to synchronize with the
 *              coordinator by acquiring and optionally tracking its beacons.
 *              Synchronizing with the coordinator is recommended before
 *              associating in a beacon-enabled network.  If the beacon could
 *              not be located on its initial search or during tracking, the
 *              MAC sends a MAC_MLME_SYNC_LOSS_IND to the application with
 *              status MAC_BEACON_LOSS.
 *
 * input parameters
 *
 * @param       pData - Pointer to parameters structure.
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SUCCESS or MAC error code.
 **************************************************************************************************
 */
extern uint8 MAC_MlmeSyncReq(ApiMac_mlmeSyncReq_t *pData);

/**************************************************************************************************
 * @fn          MAC_PwrOffReq
 *
 * @brief       This direct execute function requests the MAC to power off
 *              the radio hardware and go to sleep.  If the MAC is able to
 *              power off it will execute its power off procedure and return
 *              MAC_SUCCESS.  If the MAC is unable to sleep it will return
 *              MAC_DENIED.  The MAC is unable to sleep when it is executing
 *              certain procedures, such as a scan, data request, or association.
 *              If this function is called when the MAC is already in sleep mode
 *              it will return MAC_SUCCESS but do nothing.
 *
 * input parameters
 *
 * @param       mode - The desired low power mode.
 *
 * output parameters
 *
 * None.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS  Operation successful; the MAC is powered off.
 *              MAC_DENIED  The MAC was not able to power off.
 **************************************************************************************************
 */
extern uint8 MAC_PwrOffReq(uint8 mode);

/**************************************************************************************************
 * @fn          MAC_PwrOnReq
 *
 * @brief       This function requests the MAC to power on the radio hardware
 *              and wake up.  When the power on procedure is complete the MAC
 *              will send a MAC_PWR_ON_CNF to the application.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
extern void MAC_PwrOnReq(void);

/**************************************************************************************************
 * @fn          MAC_PwrMode
 *
 * @brief       This function returns the current power mode of the MAC.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      The current power mode of the MAC.
 **************************************************************************************************
 */
extern uint8 MAC_PwrMode(void);

/**************************************************************************************************
 * @fn          MAC_PwrNextTimeout
 *
 * @brief       This function returns the next MAC timer expiration in 320 usec units.  If no
 *              timer is running it returns zero.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      The next MAC timer expiration or zero.
 **************************************************************************************************
*/
extern uint32 MAC_PwrNextTimeout(void);

/**************************************************************************************************
 * @fn          MAC_RandomByte
 *
 * @brief       This function returns a random byte from the MAC random number generator.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      A random byte.
 **************************************************************************************************
 */
extern uint8 MAC_RandomByte(void);

/**************************************************************************************************
 * @fn          MAC_ResumeReq
 *
 * @brief       This direct execute function resumes the MAC after a successful return from
 *              MAC_YieldReq().
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
extern void MAC_ResumeReq(void);

/**************************************************************************************************
 * @fn          MAC_MlmeSetActivePib
 *
 * @brief       This direct execute function sets the active MAC PIB.
 *
 * input parameters
 *
 * @param       pPib - pointer to the PIB structure.
 *
 * output parameters
 *
 * @return      None.
 *
 **************************************************************************************************
 */
void MAC_MlmeSetActivePib( void* pPib );

/**************************************************************************************************
 * @fn          MAC_MlmeSetActivePib
 *
 * @brief       This direct execute function sets the active MAC security PIB.
 *
 * input parameters
 *
 * @param       pSecPib - pointer to the security PIB structure.
 *
 * output parameters
 *
 * @return      None.
 *
 **************************************************************************************************
 */
void MAC_MlmeSetActiveSecurityPib( void* pSecPib );

/**************************************************************************************************
 * @fn         MAC_SrcMatchEnable
 *
 * @brief      Deprecated! Enabled AUTOPEND and source address matching. This function shall not
 *             be called from ISR. It is not thread safe.
 *
 * @return     None
 **************************************************************************************************
 */
extern void MAC_SrcMatchEnable ( void );

/**************************************************************************************************
 * @fn          MAC_SrcMatchAddEntry
 *
 * @brief       Deprecated! Add a short or extended address to source address table. This
 *              function shall not be called from ISR. It is not thread safe.
 *
 * @param       addr - a pointer to sAddr_t which contains addrMode
 *                     and a union of a short 16-bit MAC address or an extended
 *                     64-bit MAC address to be added to the source address table.
 * @param       panID - the device PAN ID. It is only used when the addr is
 *                      using short address
 *
 * @return      MAC_SUCCESS or MAC_NO_RESOURCES (source address
 *              table full) or MAC_DUPLICATED_ENTRY (the entry added is duplicated),
 *              or MAC_INVALID_PARAMETER if the input parameters are invalid.
 **************************************************************************************************
 */
extern uint8 MAC_SrcMatchAddEntry ( sAddr_t *addr, uint16 panID );

/*********************************************************************
 * @fn         MAC_SrcMatchSetPend
 *
 * @brief      Set or unset the pending bit of a short or extended address
 *             from the source address table. This function shall be not be
 *             called from ISR. It is not thread safe.
 *
 * @param      addr - a pointer to sAddr_t which contains addrMode
 *                    and a union of a short 16-bit MAC address or an extended
 *                    64-bit MAC address to have the pending bit changed.
 * @param      panID - the device PAN ID. It is only used when the addr is
 *                     using short address.
 * @param      option - TRUE (Set the pending bit for the address)
 *                      FALSE (Unset the pending bit for the address)
 *
 *
 * @return     MAC_SUCCESS or MAC_INVALID_PARAMETER (address to be modified
 *                  cannot be found in the source address table).
 **************************************************************************************************
 */
extern uint8 MAC_SrcMatchSetPend ( sAddr_t *addr, uint16 panID, uint8_t option );

/**************************************************************************************************
 * @fn         MAC_SrcMatchDeleteEntry
 *
 * @brief      Deprecated! Delete a short or extended address from source address table.
 *             This function shall not be called from ISR. It is not thread safe.
 *
 * @param      addr - a pointer to sAddr_t which contains addrMode
 *                    and a union of a short 16-bit MAC address or an extended
 *                    64-bit MAC address to be deleted from the source address table.
 * @param      panID - the device PAN ID. It is only used when the addr is
 *                     using short address
 *
 * @return     MAC_SUCCESS or MAC_INVALID_PARAMETER (address to be deleted
 *                  cannot be found in the source address table).
 **************************************************************************************************
 */
extern uint8 MAC_SrcMatchDeleteEntry ( sAddr_t *addr, uint16 panID  );

/**************************************************************************************************
 * @fn          MAC_SrcMatchAckAllPending
 *
 * @brief       Deprecated! Enabled/disable acknowledging all packets with pending bit set
 *              The application normally enables it when adding new entries to
 *              the source address table fails due to the table is full, or
 *              disables it when more entries are deleted and the table has
 *              empty slots.
 *
 * @param       option - TRUE (acknowledging all packets with pending field set)
 *                       FALSE (acknowledging all packets with pending field cleared)
 *
 * @return      none
 **************************************************************************************************
 */
extern void MAC_SrcMatchAckAllPending ( uint8 option  );

/**************************************************************************************************
 * @fn          MAC_SrcMatchCheckAllPending
 *
 * @brief       Deprecated! Check if acknowledging all packets with pending bit set
 *              is enabled.
 *
 * @param       none
 *
 * @return      MAC_AUTOACK_PENDING_ALL_ON or MAC_AUTOACK_PENDING_ALL_OFF
 **************************************************************************************************
 */
extern uint8 MAC_SrcMatchCheckAllPending ( void );

/**************************************************************************************************
 * @fn          MAC_SrcMatchSetManualAddressTracking
 *
 * @brief       Controls whether the stack assumes the upper layers will manually call
 *              MAC_SrcMatchAddEntry and MAC_SrcMatchDeleteEntry to track device addresses in the
 *              network. Required to be enabled if MAC_SrcMatchSetDefaultPend is set to 1.
 *
 * @param       option - TRUE (Enable manual address tracking)
 *                       FALSE (Disable manual address tracking)
 *
 * @return      none
 **************************************************************************************************
 */
void MAC_SrcMatchSetManualAddressTracking ( uint8 option );

/**************************************************************************************************
 * @fn          MAC_SrcMatchCheckManualAddressTracking
 *
 * @brief       Returns whether manual address tracking is enabled or not. See
 *              MAC_SrcMatchSetManualAddressTracking for details.
 *
 * @param       none
 *
 * @return      FALSE if disabled, TRUE if enabled.
 **************************************************************************************************
 */
bool MAC_SrcMatchCheckManualAddressTracking( void );

/**************************************************************************************************
 * @fn          MAC_SrcMatchSetDefaultPend
 *
 * @brief       Modify the default value for the pending bit. Note that this default value
 *              is only used when the RX frame address is not found in the source address
 *              table.
 *
 * @param       option - TRUE (Set default pending bit to 1)
 *                       FALSE (Set default pending bit to 0)
 *
 * @return      none
 **************************************************************************************************
 */
extern void MAC_SrcMatchSetDefaultPend ( uint8 option  );

/**************************************************************************************************
 * @fn          MAC_SrcMatchCheckDefaultPend
 *
 * @brief       Return the value of the default pending bit. Note that this default value
 *              is only used when the RX frame address is not found in the source address
 *              table.
 *
 * @param       none
 *
 * @return      Value of default pending bit (0 or 1)
 **************************************************************************************************
 */
extern uint8 MAC_SrcMatchCheckDefaultPend ( void );

/**************************************************************************************************
 * @fn          MAC_SelectRadioRegTable
 *
 * @brief       Select radio register table in case multiple register tables are included
 *              in the build
 *
 * @param       txPwrTblIdx - TX power register value table index
 * @param       rssiAdjIdx - RSSI adjustment value index
 *
 * @return      none
 **************************************************************************************************
 */
extern void MAC_SetRadioRegTable ( uint8 txPwrTblIdx, uint8 rssiAdjIdx );

/**************************************************************************************************
 * @fn          MAC_CbackEvent
 *
 * @brief       This callback function sends MAC events to the application.
 *              The application must implement this function.  A typical
 *              implementation of this function would allocate an OSAL message,
 *              copy the event parameters to the message, and send the message
 *              to the application's OSAL event handler.  This function may be
 *              executed from task or interrupt context and therefore must
 *              be reentrant.
 *
 * input parameters
 *
 * @param       pData - Pointer to parameters structure.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
extern void MAC_CbackEvent(macCbackEvent_t *pData);

/**************************************************************************************************
 * @fn          MAC_CbackCheckPending
 *
 * @brief       This callback function returns the number of pending indirect messages queued in
 *              the application. Most applications do not queue indirect data and can simply
 *              always return zero. The number of pending indirect messages only needs to be
 *              returned if macCfg.appPendingQueue to TRUE.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      The number of indirect messages queued in the application or zero.
 **************************************************************************************************
*/
extern uint8 MAC_CbackCheckPending(void);

/**************************************************************************************************
 * @fn          MAC_CbackQueryRetransmit
 *
 * @brief       This function callback function returns whether or not to continue MAC
 *              retransmission.
 *              A return value '0x00' will indicate no continuation of retry and a return value
 *              '0x01' will indicate to continue retransmission. This callback function shall be
 *              used to stop continuing retransmission for RF4CE.
 *              MAC shall call this callback function whenever it finishes transmitting a packet
 *              for macMaxFrameRetries times.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      0x00 to stop retransmission, 0x01 to continue retransmission.
 **************************************************************************************************
*/
extern uint8 MAC_CbackQueryRetransmit(void);

/**************************************************************************************************
 * @fn          MAC_YieldReq
 *
 * @brief       Checks if the mac is in idle or polling state by calling macStateIdleOrPolling().
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS  The MAC is ready to yield.
 *              MAC_DENIED  The MAC cannot yield now.
 **************************************************************************************************
 */
extern uint8 MAC_YieldReq(void);

/**************************************************************************************************
 * @fn          macUpdatePanId
 *
 * @brief       Update Device Table entry and PIB with new Pan Id
 *
 *
 * input parameters
 *
 * @param       panId - the new Pan ID
 *
 * output parameters
 *
 * @return      None
 **************************************************************************************************/
extern void macUpdatePanId(uint16 panId);

/**************************************************************************************************
 * @fn          MAC_SetPHYParamReq
 *
 * @brief       This direct execute function sets an attribute value
 *              in MAC PHY items. Only generic phy descriptor can be set.
 *              Note: this function will cause a radio restart.
 *
 * input parameters
 *
 * @param       phyAttribute - phy attribute identifier.
 * @param       phyID corresponding to the attribute
 * @param       pValue - pointer to the attribute value.
 *
 * output parameters
 *
 * None.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *
 **************************************************************************************************
*/
extern uint8 MAC_SetPHYParamReq(uint8 phyAttribute, uint8 phyID, void *pValue);

/**************************************************************************************************
 * @fn          MAC_GetPHYParamReq
 *
 * @brief       This direct execute function sets an attribute value
 *              in the MAC PHY items.
 *
 * input parameters
 *
 * @param       phyAttribute - phy attribute identifier.
 * @param       phyID corresponding to the attribute
 * @param       pValue - pointer to the attribute value.
 * output parameters
 *
 * None.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *
 **************************************************************************************************
 */
extern uint8 MAC_GetPHYParamReq(uint8 phyAttribute, uint8 phyID, void *pValue);

/**
 * @brief       This functions handles the WiSUN async request.
 *              The possible operation is Async Start or Async Stop.
 *              For the async start operation, the caller of this function
 *              can indicate which WiSUN async frame type to be
 *              sent on the specified channels.
 *
 * @param pData pointer to the asynchronous parameters structure
 * @return MAC_SUCCESS if successful otherwise MAC error codes.
 */
extern uint8 MAC_MlmeWSAsyncReq( ApiMac_mlmeWSAsyncReq_t* pData );

/**************************************************************************************************
 * @fn          MAC_MlmeGetReqSize
 *
 * @brief       This direct execute function gets the MAC PIB attribute size
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 *
 * output parameters
 *
 * None.
 *
 * @return      size in bytes
 *
 **************************************************************************************************
 */
extern uint8 MAC_MlmeGetReqSize( uint8 pibAttribute );

/**
 * @brief This direct excute function gets the MAC FH PIB
 *        attribute size.
 *
 * @param pibAttribute - The attribute identifier.
 *
 * @return size in bytes.
 */
extern uint8 MAC_MlmeFHGetReqSize( uint16 pibAttribute );


/**************************************************************************************************
 * @fn          MAC_MlmeFHSetReq
 *
 * @brief       This direct execute function sets an attribute value
 *              in the MAC FH(Frequency hopping) PIB.
 *
 * input parameters
 *
 * @param       pibAttribute - The FH attribute identifier.
 * @param       pValue - pointer to the attribute value.
 *
 * output parameters
 *
 * None.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *              MAC_UNSUPPORTED if frequency hopping is not enabled.
 *
 **************************************************************************************************
 */
extern uint8 MAC_MlmeFHSetReq(uint16 pibAttribute, void *pValue);

/**************************************************************************************************
 * @fn          MAC_MlmeFHGetReq
 *
 * @brief       This direct execute function retrieves an attribute value
 *              from the MAC FH(Frequency hopping) PIB.
 *
 * input parameters
 *
 * @param       pibAttribute - The FH attribute identifier.
 * @param       pValue - pointer to the attribute value.
 *
 * output parameters
 *
 * @param       pValue - pointer to the attribute value.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *              MAC_UNSUPPORTED if frequency hopping is not enabled.
 *
 **************************************************************************************************
 */
extern uint8 MAC_MlmeFHGetReq(uint16 pibAttribute, void *pValue);

/**************************************************************************************************
*/



#ifdef MAC_DUTY_CYCLE_CHECKING
/*
 *
 *
 * @brief  Callback to application layer to notify when the MAC Duty Cycling
 * mode changes.
 *
 * @param mode  - mode of duty cycle that the device should change to and trigger an
 * event to notify the application
 *
 * @return        None
 */
void MAC_DutyCycleNotifyCB(uint8_t mode);
#endif // MAC_DUTY_CYCLE_CHECKING


/**************************************************************************************************
 * @fn          MAC_AbortScan
 *
 * @brief       Abort currenting running scan operation by calling macScanComplete.
 *
 * @return      None.
 **************************************************************************************************
 */
void MAC_AbortScan();

#ifdef __cplusplus
};
#endif

#endif /* MAC_API_H */
