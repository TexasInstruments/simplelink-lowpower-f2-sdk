/******************************************************************************

 @file  rcp_mac_common.h

 @brief Public constants for MAC used for RCP interface

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2025-2026, Texas Instruments Incorporated

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

#ifndef RCP_MAC_CONSTS_H_
#define RCP_MAC_CONSTS_H_
#include <stdint.h>
#include "saddr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAC_ADDR_LEN 8

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

/* Number of keys */
#define MAC_NUM_KEY_DESCRIPTORS     4

/* Supported Channel Pages */
#define MAC_CHANNEL_PAGE_9          9     /* standard-defined SUN PHY operating modes */
#define MAC_CHANNEL_PAGE_10         10    /* MR-FSK Generic-PHY-defined PHY modes */

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

/* PHY IDs */
/* These constants should not be used in Wisun, instead use the constant values from api_mac.h */
/* 915MHz Frequency band operating mode #2 */
#define MAC_50KBPS_915MHZ_PHY_1         1
/* 915MHz Frequency band operating mode #4 */
#define MAC_150KBPS_915MHZ_PHY_2        2
/* 868MHz Frequency band operating mode #5 */
#define MAC_50KBPS_868MHZ_PHY_3         3
/*! 866MHz Frequency band operating mode #1 */
#define MAC_50KBPS_866MHZ_PHY_4           4
/*! 866MHz Frequency band operating mode #2 */
#define MAC_100KBPS_866MHZ_PHY_5          5
/* 915MHz Frequency band operating mode #3 */
#define MAC_200KBPS_915MHZ_PHY_6      6
/* 915MHz Frequency band operating mode #8 */
#define MAC_300KBPS_915MHZ_PHY_7        7
// TODO: add 300 kbps here
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

/* Symbol rate in kilo symbols per second */
#define MAC_MRFSK_SYMBOL_RATE_20_K    20     /* 20K symbol rate */
#define MAC_MRFSK_SYMBOL_RATE_50_K    50     /* 50K symbol rate */
#define MAC_MRFSK_SYMBOL_RATE_100_K  100     /* 100k symbol rate */
#define MAC_MRFSK_SYMBOL_RATE_150_K  150     /* 150k symbol rate */
#define MAC_MRFSK_SYMBOL_RATE_200_K  200     /* 200K symbol rate */
#define MAC_MRFSK_SYMBOL_RATE_300_K  300     /* 300K symbol rate */

/* Clear channel access type */
#define MAC_CCA_TYPE_NO_BACKOFF      0   /* No back off and CSMA/CA */
#define MAC_CCA_TYPE_CSMA_CA         1   /* CSMA/CA */
#define MAC_CCA_TYPE_LBT             2   /* ETSI EN 300 220 LBT */
#define MAC_CCA_TYPE_CSMA_ONLY       3   /* CSMA only */

/* Modulation scheme */
#define MAC_2_FSK_MODULATION         0   /* 2-FSK */
#define MAC_4_FSK_MODULATION         1   /* 4-FSK */

/* Modulation Index */
#define MAC_MODULATION_INDEX_2FSK_50K_1     15 // Modulation index 1
#define MAC_MODULATION_INDEX_2FSK_50K_0_5   5  // Modulation index 0.5
#define MAC_MODULATION_INDEX_2FSK_100K_1    15
#define MAC_MODULATION_INDEX_2FSK_100K_0_5  5
#define MAC_MODULATION_INDEX_2FSK_150K_1    15
#define MAC_MODULATION_INDEX_2FSK_150K_0_5  5
#define MAC_MODULATION_INDEX_2FSK_150K_1    15
#define MAC_MODULATION_INDEX_2FSK_5K        5
#define MAC_MODULATION_INDEX_2FSK_200K_0_5  5
#define MAC_MODULATION_INDEX_2FSK_200K_1    15
#define MAC_MODULATION_INDEX_2FSK_300K_0_5  5
#define MAC_MODULATION_INDEX_2FSK_300K_1    15
#define MAC_MODULATION_INDEX_2FSK_500K      9

/* Maximum number of PHY descriptor entries */
#define MAC_STANDARD_PHY_DESCRIPTOR_ENTRIES  10
#define MAC_GENERIC_PHY_DESCRIPTOR_ENTRIES   10

/* MRFSK Standard Phy ID start */
#define MAC_MRFSK_STD_PHY_ID_BEGIN           MAC_50KBPS_915MHZ_PHY_1
/* MRFSK Standard Phy ID end */
#define MAC_MRFSK_STD_PHY_ID_END             (MAC_MRFSK_STD_PHY_ID_BEGIN + MAC_STANDARD_PHY_DESCRIPTOR_ENTRIES - 1)

/* MRFSK Generic Phy ID start */
#define MAC_MRFSK_GENERIC_PHY_ID_BEGIN       MAC_50KBPS_433MHZ_PHY_128
/* MRFSK Generic Phy ID end */
#define MAC_MRFSK_GENERIC_PHY_ID_END         (MAC_MRFSK_GENERIC_PHY_ID_BEGIN + MAC_GENERIC_PHY_DESCRIPTOR_ENTRIES - 1)

#define RCP_PHY_MODE_UNDEF          (0)
#define RCP_PHY_MODE_STD            (1)
#define RCP_PHY_MODE_GEN            (2)

typedef struct __attribute__((__packed__)) {
    uint32_t ufsi;
    uint32_t ref_timeStamp;
    uint8_t dwellInterval;
    uint8_t channelFunc; // fixed or list
    uint8_t fixedChannel; // fixed channel from nanoStack
    uint8_t bitMap[MAC_154G_CHANNEL_BITMAP_SIZ];   // excluded channel
    uint8_t numChannels; // numChannels = max. chans - # of excl chans
    sAddrExt_t extAddr;
} fhnt_entry_t;

#ifdef __cplusplus
};
#endif
#endif  // RCP_MAC_CONSTS_H_

