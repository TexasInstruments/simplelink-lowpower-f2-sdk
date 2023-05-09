/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_RADIO_FSK_DEFS_H
#define SID_PAL_RADIO_FSK_DEFS_H

/** @file sid_pal_radio_fsk_defs.h
 *
 * @defgroup sid_pal_lib_radio sid Radio interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details     FSK modulation defines for Sidewalk
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// Radio Mod Shaping parameter
#define SID_PAL_RADIO_FSK_MOD_SHAPING_OFF 0x00
#define SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_03 0x08
#define SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_05 0x09
#define SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_07 0x0A
#define SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_1 0x0B

// Bandwidth
#define SID_PAL_RADIO_FSK_BW_4800 0x1F
#define SID_PAL_RADIO_FSK_BW_5800 0x07
#define SID_PAL_RADIO_FSK_BW_7300 0x0F
#define SID_PAL_RADIO_FSK_BW_9700 0x1E
#define SID_PAL_RADIO_FSK_BW_11700 0x16
#define SID_PAL_RADIO_FSK_BW_14600 0x0E
#define SID_PAL_RADIO_FSK_BW_19500 0x1D
#define SID_PAL_RADIO_FSK_BW_23400 0x15
#define SID_PAL_RADIO_FSK_BW_29300 0x0D
#define SID_PAL_RADIO_FSK_BW_39000 0x1C
#define SID_PAL_RADIO_FSK_BW_46900 0x14
#define SID_PAL_RADIO_FSK_BW_58600 0x0C
#define SID_PAL_RADIO_FSK_BW_78200 0x1B
#define SID_PAL_RADIO_FSK_BW_93800 0x13
#define SID_PAL_RADIO_FSK_BW_117300 0x0B
#define SID_PAL_RADIO_FSK_BW_156200 0x1A
#define SID_PAL_RADIO_FSK_BW_187200 0x12
#define SID_PAL_RADIO_FSK_BW_234300 0x0A
#define SID_PAL_RADIO_FSK_BW_312000 0x19
#define SID_PAL_RADIO_FSK_BW_373600 0x11
#define SID_PAL_RADIO_FSK_BW_467000 0x09

#define SID_PAL_RADIO_FSK_BW_100KHZ SID_PAL_RADIO_FSK_BW_93800
#define SID_PAL_RADIO_FSK_BW_117KHZ SID_PAL_RADIO_FSK_BW_117300
#define SID_PAL_RADIO_FSK_BW_125KHZ SID_PAL_RADIO_FSK_BW_156200
#define SID_PAL_RADIO_FSK_BW_150KHZ SID_PAL_RADIO_FSK_BW_156200
#define SID_PAL_RADIO_FSK_BW_250KHZ SID_PAL_RADIO_FSK_BW_312000
#define SID_PAL_RADIO_FSK_BW_500KHZ SID_PAL_RADIO_FSK_BW_467000


// Radio Preamble detection
#define SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_OFF 0x00
#define SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_08_BITS 0x04
#define SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_16_BITS 0x05
#define SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_24_BITS 0x06
#define SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_32_BITS 0x07

// Radio sync word correlators activated
#define SID_PAL_RADIO_FSK_ADDRESSCOMP_FILT_OFF 0x00
#define SID_PAL_RADIO_FSK_ADDRESSCOMP_FILT_NODE 0x01
#define SID_PAL_RADIO_FSK_ADDRESSCOMP_FILT_NODE_BROAD 0x02

// Radio packet length modes
#define SID_PAL_RADIO_FSK_RADIO_PACKET_FIXED_LENGTH 0x00
#define SID_PAL_RADIO_FSK_RADIO_PACKET_VARIABLE_LENGTH 0x01

// packet params crc types
#define SID_PAL_RADIO_FSK_CRC_OFF 0x01
#define SID_PAL_RADIO_FSK_CRC_1_BYTES 0x00
#define SID_PAL_RADIO_FSK_CRC_2_BYTES 0x02
#define SID_PAL_RADIO_FSK_CRC_1_BYTES_INV 0x04
#define SID_PAL_RADIO_FSK_CRC_2_BYTES_INV 0x06
#define SID_PAL_RADIO_FSK_CRC_2_BYTES_IBM 0xF1
#define SID_PAL_RADIO_FSK_CRC_2_BYTES_CCIT 0xF2

// packet params Radio whitening mode
#define SID_PAL_RADIO_FSK_DC_FREE_OFF 0x00
#define SID_PAL_RADIO_FSK_DC_FREEWHITENING 0x01

#define SID_PAL_RADIO_FSK_WHITENING_SEED 0x01FF

#define SID_PAL_RADIO_FSK_SYNC_WORD_LENGTH   8

/* timeout duration in usec. the conversion to semtec ticks is done by start_rx() start_tx() */
#define SECS_TO_MUS(X)                         (X * 1000000UL)
/* set max radio timeout to 5sec */
#define SID_PAL_RADIO_FSK_DEFAULT_TX_TIMEOUT            SECS_TO_MUS(5)
/* 1 sec timeout used by diagnostics code */
#define SID_PAL_RADIO_FSK_TIMEOUT_DURATION_1_SEC        SECS_TO_MUS(1)

typedef struct sid_pal_radio_fsk_cad_params {
    int16_t fsk_ed_rssi_threshold;
    uint16_t fsk_ed_duration_mus;
    uint8_t fsk_cs_min_prm_det;
    uint16_t fsk_cs_duration;
} sid_pal_radio_fsk_cad_params_t;


/** Sidewalk phy fsk modulation parameters*/
typedef struct sid_pal_radio_fsk_modulation_params {
    uint32_t bit_rate;
    uint32_t freq_dev;
    uint8_t mod_shaping;
    uint8_t bandwidth;
} sid_pal_radio_fsk_modulation_params_t;

/** Sidewalk phy fsk packet parameters*/
typedef struct sid_pal_radio_fsk_packet_params {
    uint16_t preamble_length;
    uint8_t preamble_min_detect;
    uint8_t sync_word_length;
    uint8_t addr_comp;
    uint8_t header_type;
    uint8_t payload_length;
    uint8_t *payload;
    uint8_t crc_type;
    uint8_t radio_whitening_mode;
} sid_pal_radio_fsk_packet_params_t;

/**
 * @brief Radio FSK FCS enumeration definition
 */
typedef enum {
    RADIO_FSK_FCS_TYPE_0 = 0,  // 4-octet FCS
    RADIO_FSK_FCS_TYPE_1 = 1,  // 2-octet FCS
} radio_fsk_fcs_t;

/**
 * @brief Radio FSK PHY HDR structure definition
 */
typedef struct {
    radio_fsk_fcs_t   fcs_type;
    bool              is_data_whitening_enabled;
    bool              is_fec_enabled;
} sid_pal_radio_fsk_phy_hdr_t;

typedef struct {
    sid_pal_radio_fsk_phy_hdr_t  *phy_hdr;
    sid_pal_radio_fsk_packet_params_t  *packet_params;
    uint32_t                  packet_timeout;
    uint8_t                   *sync_word;
    uint8_t                   *payload;
} sid_pal_radio_fsk_pkt_cfg_t;

/** Sidewalk Phy received FSK packet status*/
typedef struct sid_pal_radio_fsk_rx_packet_status {
    uint8_t rx_status;
    int8_t rssi_avg;
    int8_t rssi_sync;
} sid_pal_radio_fsk_rx_packet_status_t;


/** Sidewalk phy fsk configuation handle*/
typedef struct sid_pal_radio_fsk_phy_settings {
    uint32_t freq;
    int8_t power;
    uint8_t sync_word[SID_PAL_RADIO_FSK_SYNC_WORD_LENGTH];
    uint16_t whitening_seed;
    uint32_t tx_timeout;
    uint32_t symbol_timeout;
    sid_pal_radio_fsk_modulation_params_t fsk_modulation_params;
    sid_pal_radio_fsk_packet_params_t fsk_packet_params;
    sid_pal_radio_fsk_cad_params_t fsk_cad_params;
    sid_pal_radio_fsk_phy_hdr_t fsk_phy_hdr;
} sid_pal_radio_fsk_phy_settings_t;

#ifdef __cplusplus
}
#endif

/** @} */

#endif
