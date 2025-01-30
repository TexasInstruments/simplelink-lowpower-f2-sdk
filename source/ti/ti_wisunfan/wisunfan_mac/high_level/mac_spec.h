/******************************************************************************

 @file  mac_spec.h

 @brief This file contains constants and other data defined by the 802.15.4
        spec.

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

#ifndef MAC_SPEC_H
#define MAC_SPEC_H

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* PHY packet fields lengths in bytes */
#define MAC_PHY_SHR_LEN                 5       /* preamble bytes plus SFD byte */
#define MAC_PHY_SHR_LEN_2FSK            10      /* preamble bytes plus SFD bytes for 2FSK SUN */
#define MAC_PHY_SHR_LEN_2FSK_LRM        (2 * 64 + 64) /* sync words in symbols */
#define MAC_PHY_TERM_LEN_LRM            (6 * 2) /* termination symbols */
#define MAC_PHY_PHR_LEN                 2       /* PHR length byte, FCS, and DW */
#define MAC_PHY_PHR_LEN_MSB_MASK        0x07    /* PHR length MSb mask */
#define MAC_PHY_PHR_DW_MASK             0x08    /* PHR Data Whitener bit mask */
#define MAC_PHY_PHR_FCS_TYPE_MASK       0x10    /* PHR FCS type bit mask */
#define MAC_PHY_PHR_MS_MASK             0x80    /* PHR Mode switch bit mask */
#if defined(COMBO_MAC) || defined(FREQ_2_4G)
#define MAC_PHY_PHR_IEEE_LEN            1
#define MAC_RSSI_LEN                    1
#define MAC_TS_LEN                      4
#endif

/* MAC frame field lengths in bytes */
#define MAC_FCF_FIELD_LEN               2       /* frame control field */
#define MAC_SEQ_NUM_FIELD_LEN           1       /* sequence number  */
#define MAC_PAN_ID_FIELD_LEN            2       /* PAN ID  */
#define MAC_EXT_ADDR_FIELD_LEN          8       /* Extended address */
#define MAC_SHORT_ADDR_FIELD_LEN        2       /* Short address */
#define MAC_FCS2_FIELD_LEN              2       /* 2-byte FCS field */
#define MAC_FCS4_FIELD_LEN              4       /* 4-byte FCS field */
#define MAC_SEC_CONTROL_FIELD_LEN       1       /* Security control field */

/* Frame offsets in bytes */
#define MAC_FCF_OFFSET                  0       /* offset to frame control field */
#define MAC_SEQ_NUM_OFFSET              2       /* offset to sequence number */
#define MAC_DEST_PAN_ID_OFFSET          3       /* offset to destination PAN ID */
#define MAC_DEST_ADDR_OFFSET            5       /* offset to destination address */

/* Frame control field bit masks */
#define MAC_FCF_FRAME_TYPE_MASK         0x0007
#define MAC_FCF_SEC_ENABLED_MASK        0x0008
#define MAC_FCF_FRAME_PENDING_MASK      0x0010
#define MAC_FCF_ACK_REQUEST_MASK        0x0020
#define MAC_FCF_INTRA_PAN_MASK          0x0040
#define MAC_FCF_SEQ_NO_SUPPRESS_MASK    0x0100
#define MAC_FCF_IE_PRESENT_MASK         0x0200
#define MAC_FCF_DST_ADDR_MODE_MASK      0x0C00
#define MAC_FCF_FRAME_VERSION_MASK      0x3000
#define MAC_FCF_SRC_ADDR_MODE_MASK      0xC000

/* Frame control field bit positions */
#define MAC_FCF_FRAME_TYPE_POS          0
#define MAC_FCF_SEC_ENABLED_POS         3
#define MAC_FCF_FRAME_PENDING_POS       4
#define MAC_FCF_ACK_REQUEST_POS         5
#define MAC_FCF_INTRA_PAN_POS           6
/* Bit positions 8,9 for FCF as per new spec */
#define MAC_FCF_SEQ_NO_SUPPRESS_POS     8
#define MAC_FCF_IE_LIST_PRESENT_POS     9
#define MAC_FCF_DST_ADDR_MODE_POS       10
#define MAC_FCF_FRAME_VERSION_POS       12
#define MAC_FCF_SRC_ADDR_MODE_POS       14

/* Security control field bit masks */
#define MAC_SCF_SECURITY_LEVEL_MASK     0x07
#define MAC_SCF_KEY_IDENTIFIER_MASK     0x18

/* Security control field bit positions */
#define MAC_SCF_SECURITY_LEVEL_POS      0
#define MAC_SCF_KEY_IDENTIFIER_POS      3

/* MAC Payload offsets in bytes */
#define MAC_SFS_OFFSET                  0
#define MAC_PENDING_ADDR_OFFSET         3       /* if GTS is not in use */

/* Beacon superframe spec bit positions, low byte */
#define MAC_SFS_BEACON_ORDER_POS        0
#define MAC_SFS_SUPERFRAME_ORDER_POS    4

/* Beacon superframe spec bit positions, high byte */
#define MAC_SFS_FINAL_CAP_SLOT_POS      0
#define MAC_SFS_BATT_LIFE_EXT_POS       4
#define MAC_SFS_PAN_COORD_POS           6
#define MAC_SFS_ASSOC_PERMIT_POS        7


/* Frame type */
#define MAC_FRAME_TYPE_BEACON           0
#define MAC_FRAME_TYPE_DATA             1
#define MAC_FRAME_TYPE_ACK              2
#define MAC_FRAME_TYPE_COMMAND          3
#define MAC_FRAME_TYPE_MP               5
#define MAC_FRAME_TYPE_FRAGMENT         6
#define MAC_FRAME_TYPE_EXTENDED         7

#define MAC_FRAME_TYPE_MAX_VALID        MAC_FRAME_TYPE_COMMAND

/* Internal Frame Type for frame version bits (13,12) set to (1,0) */
#define MAC_FRAME_TYPE_INTERNAL_MAC_VERSION_E   8

/* MAC Frame version field values */
#define MAC_FRAME_VERSION_0             0
#define MAC_FRAME_VERSION_1             1
#define MAC_FRAME_VERSION_2             2
#define MAC_FRAME_VERSION_3             3

/* Command frame identifiers */
#define MAC_ASSOC_REQ_FRAME             1
#define MAC_ASSOC_RSP_FRAME             2
#define MAC_DISASSOC_NOTIF_FRAME        3
#define MAC_DATA_REQ_FRAME              4
#define MAC_PAN_CONFLICT_FRAME          5
#define MAC_ORPHAN_NOTIF_FRAME          6
#define MAC_BEACON_REQ_FRAME            7
#define MAC_COORD_REALIGN_FRAME         8
#define MAC_GTS_REQ_FRAME               9
#define MAC_ENHANCED_BEACON_REQ_FRAME   10

/* Length of command frame payload (includes command ID byte) */
#define MAC_ZERO_DATA_PAYLOAD           0
#define MAC_ASSOC_REQ_PAYLOAD           2
#define MAC_ASSOC_RSP_PAYLOAD           4
#define MAC_DISASSOC_NOTIF_PAYLOAD      2
#define MAC_DATA_REQ_PAYLOAD            1
#define MAC_PAN_CONFLICT_PAYLOAD        1
#define MAC_ORPHAN_NOTIF_PAYLOAD        1
#define MAC_BEACON_REQ_PAYLOAD          1
#define MAC_COORD_REALIGN_PAYLOAD       9
#define MAC_GTS_REQ_PAYLOAD             2
#define MAC_ENHANCED_BEACON_REQ_PAYLOAD 30
#define MAC_WS_ASYNC_REQ_MAX_PAYLOAD    120
#define MAC_COEXIST_EB_REQ_PAYLOAD      20
#define MAC_ENH_ACK_PAYLOAD             0
#define MAC_EDFE_EFRAME_PAYLOAD         0

/* Length of command frames (max header plus payload) */
#define MAC_ZERO_DATA_FRAME_LEN         (21 + MAC_ZERO_DATA_PAYLOAD)
#define MAC_ASSOC_REQ_FRAME_LEN         (23 + MAC_ASSOC_REQ_PAYLOAD)
#define MAC_ASSOC_RSP_FRAME_LEN         (23 + MAC_ASSOC_RSP_PAYLOAD)
#define MAC_DISASSOC_NOTIF_FRAME_LEN    (17 + MAC_DISASSOC_NOTIF_PAYLOAD)
#define MAC_DATA_REQ_FRAME_LEN          (23 + MAC_DATA_REQ_PAYLOAD)
#define MAC_PAN_CONFLICT_FRAME_LEN      (23 + MAC_PAN_CONFLICT_PAYLOAD)
#define MAC_ORPHAN_NOTIF_FRAME_LEN      (17 + MAC_ORPHAN_NOTIF_PAYLOAD)
#define MAC_BEACON_REQ_FRAME_LEN        (7 + MAC_BEACON_REQ_PAYLOAD)
#define MAC_COORD_REALIGN_FRAME_LEN     (23 + MAC_COORD_REALIGN_PAYLOAD)
#define MAC_GTS_REQ_FRAME_LEN           (7 + MAC_GTS_REQ_PAYLOAD)
#define MAC_COEXIST_EB_REQ_FRAME_LEN    (23 + MAC_COEXIST_EB_REQ_PAYLOAD)
#define MAC_ENHANCED_BEACON_REQ_FRAME_LEN  (7 + MAC_ENHANCED_BEACON_REQ_PAYLOAD)
#define MAC_WS_ASYNC_REQ_FRAME_LEN      (30 + MAC_WS_ASYNC_REQ_MAX_PAYLOAD)
#define MAC_ENH_ACK_FRAME_LEN           (23 + MAC_ENH_ACK_PAYLOAD)
#define MAC_EDFE_EFRAME_LEN             (23 + MAC_EDFE_EFRAME_PAYLOAD)

/* Beacon frame base length (max header plus minimum payload) */
#define MAC_BEACON_FRAME_BASE_LEN       (13 + 4)

/* Maximum number of pending addresses in a beacon */
#define MAC_PEND_ADDR_MAX               7

/* Associate response command frame status values */
#define MAC_ASSOC_SUCCESS               0             /* association successful */
#define MAC_ASSOC_CAPACITY              1             /* PAN at capacity */
#define MAC_ASSOC_DENIED                2             /* PAN access denied */

/* Beacon order and superframe order maximum values */
#define MAC_BO_NON_BEACON               15
#define MAC_SO_NONE                     15

/* Enhanced Beacon order maximum values */
#define MAC_ENHANCED_BEACON_ORDER_MAX   15

/* Non-Beacon enabled PAN Enhanced beacon order maximum value */
#define MAC_NBPAN_ENHANCED_BEACON_ORDER_MAX   16383

/* Broadcast PAN ID */
#define MAC_PAN_ID_BROADCAST            0xFFFF

/* Number of symbols per octet */
#define MAC_SYMBOLS_PER_OCTET           2

/* Number of symbols per octet for 2FSK modulation scheme */
#define MAC_SYMBOLS_PER_OCTET_2FSK      8

/* Number of symbole per octet for 2FSK modulation scheme in long range mode */
#define MAC_SYMBOLS_PER_OCTET_2FSK_LRM  32

/* ToDo: Add definition for symbols per octet for 4FSK modulation scheme,
   when it is supported */

/* Maximum phy frame size in bytes */
#define MAC_A_MAX_PHY_PACKET_SIZE       500

/* Phy RX <--> TX turnaround time in symbols */
#define MAC_A_TURNAROUND_TIME           (1000/macSpecUsecsPerSymbol)

/* Number of symbols forming a superframe slot when the superframe order is equal to 0 */
#define MAC_A_BASE_SLOT_DURATION        60

/* Number of symbols forming a superframe when the superframe order is equal to 0 */
#define MAC_A_BASE_SUPERFRAME_DURATION  (MAC_A_BASE_SLOT_DURATION * MAC_A_NUM_SUPERFRAME_SLOTS)

/* Maximum number of bytes added by the MAC sublayer to the payload of its beacon frame */
#define MAC_A_MAX_BEACON_OVERHEAD       75

/* Maximum size in bytes of a beacon payload */
#define MAC_A_MAX_BEACON_PAYLOAD_LENGTH (MAC_A_MAX_PHY_PACKET_SIZE - MAC_A_MAX_BEACON_OVERHEAD)

/* The number of superframes a GTS descriptor exists for in the beacon frame */
#define MAC_A_GTS_DESC_PERSISTENCE_TIME 4

/* Maximum size in bytes of a MAC frame header */
#define MAC_A_MAX_FRAME_OVERHEAD        25

/* The number of consecutive lost beacons that will case the MAC to declare a sync loss */
#define MAC_A_MAX_LOST_BEACONS          4

/* The maximum number of bytes that can be transmitted in the MAC frame payload */
#define MAC_A_MAX_FRAME_SIZE            (MAC_A_MAX_PHY_PACKET_SIZE - MAC_A_MAX_FRAME_OVERHEAD)

/* The maximum frame size in bytes that can be followed by a short interframe spacing period */
#define MAC_A_MAX_SIFS_FRAME_SIZE       18

/* The minimum number of symbols forming the CAP */
#define MAC_A_MIN_CAP_LENGTH            440

/* The minimum number of symbols forming a long interframe spacing period */
#define MAC_A_MIN_LIFS_PERIOD           40

/* The minimum number of symbols forming a short interframe spacing period */
#define MAC_A_MIN_SIFS_PERIOD           12

/* The number of slots contained in any superframe */
#define MAC_A_NUM_SUPERFRAME_SLOTS      16

/* The number of symbols of CCA assesment time */
#define MAC_A_CCA_TIME                  8

/* Maximum value for energy detect */
#define MAC_SPEC_ED_MAX                 0xFF

/* Threshold above receiver sensitivity for minimum energy detect in dBm (see 6.9.7) */
#define MAC_SPEC_ED_MIN_DBM_ABOVE_RECEIVER_SENSITIVITY    10


/* ----- values specific to subG PHY ----- */

/* minimum receiver sensitivity in dBm (see 6.5.3.3) */
#define MAC_SPEC_MIN_RECEIVER_SENSITIVITY   -85

/* Length of preamble field in symbols */
#define MAC_SPEC_PREAMBLE_FIELD_LENGTH      8

/* Length of SFD field in symbols */
#define MAC_SPEC_SFD_FIELD_LENGTH           2

/* octets (or bytes) per symbol */
#define MAC_SPEC_OCTETS_PER_SYMBOL          2


/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

/* Length of GTS fields in a beacon, not including one byte for GTS specification */
#define MAC_GTS_FIELDS_LEN(gtsSpec)     ((uint8)((((gtsSpec) & 0x07) * 3) + (((gtsSpec) & 0x07) ? 1 : 0)))

/*
 *  Macros to decode FCF and sequence number.  For efficiency, the FCF is accessed as separate
 *  bytes instead of as a 16-bit word.  The first byte (+0) is the lowest significant byte.
 *  The second byte (+1) is the most signficant byte.  NOTE!  Any change to the specification
 *  involving FCF would require updating these macros.
 *
 *  Macro parameter is pointer to start of frame.
 */

#define MAC_FRAME_TYPE(p)       ((p)[MAC_FCF_OFFSET+0] & 0x07)
#define MAC_SEC_ENABLED(p)      ((p)[MAC_FCF_OFFSET+0] & 0x08)  /* note: value is *non-zero* if true */
#define MAC_FRAME_PENDING(p)    ((p)[MAC_FCF_OFFSET+0] & 0x10)  /* note: value is *non-zero* if true */
#define MAC_FRAME_PENDING_BIT(p)             ((p)[MAC_FCF_OFFSET+0] & 0x10)  /* note: use this macro if the bits are not reversed */
#define MAC_FRAME_PENDING_BIT_REVERSED(p)    ((p)[MAC_FCF_OFFSET+0] & 0x08)  /* note: use this macro if the bits are reversed.
                                                                                      the bit reversal is employed before tx */
#define MAC_ACK_REQUEST(p)      ((p)[MAC_FCF_OFFSET+0] & 0x20)  /* note: value is *non-zero* if true */
#define MAC_INTRA_PAN(p)        ((p)[MAC_FCF_OFFSET+0] & 0x40)  /* note: value is *non-zero* if true */
#define MAC_SEQ_NO_SUPPRESSED(p)  ((p)[MAC_FCF_OFFSET+1] & 0x01)
#define MAC_DEST_ADDR_MODE(p)   (((p)[MAC_FCF_OFFSET+1] >> 2) & 0x3)
#define MAC_FRAME_VERSION(p)    (((p)[MAC_FCF_OFFSET+1] >> 4) & 0x3)
#define MAC_SRC_ADDR_MODE(p)    (((p)[MAC_FCF_OFFSET+1] >> 6) & 0x3)
#define MAC_SEQ_NUMBER(p)       ((p)[MAC_SEQ_NUM_OFFSET])
#define MAC_IE_LIST_PRESENT(p)  (((p)[MAC_FCF_OFFSET+1]) & 0x2)

/**************** TI WISUN FAN SPECIFIC **********************/
#ifdef TI_WISUN_FAN_DEBUG
/* Macros for address filtering */
#define SIZE_OF_EUI_LIST    75
#define EXT_FILTER_DISABLED  0
#define ALLOWLIST_ENABLED   1
#define DENYLIST_ENABLED    2
#endif


/**************************************************************************************************
*/

#endif /* MAC_SPEC_H */
