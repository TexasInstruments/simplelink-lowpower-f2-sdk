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

#ifndef SID_PAL_RADIO_LORA_DEFS_H
#define SID_PAL_RADIO_LORA_DEFS_H

/** @file sid_pal_radio_lora_defs.h
 *
 * @defgroup sid_pal_lib_radio sid Radio interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details     LORA modulation defines for Sidewalk
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Spreading Factor
#define SID_PAL_RADIO_LORA_SF5 0x05
#define SID_PAL_RADIO_LORA_SF6 0x06
#define SID_PAL_RADIO_LORA_SF7 0x07
#define SID_PAL_RADIO_LORA_SF8 0x08
#define SID_PAL_RADIO_LORA_SF9 0x09
#define SID_PAL_RADIO_LORA_SF10 0x0A
#define SID_PAL_RADIO_LORA_SF11 0x0B
#define SID_PAL_RADIO_LORA_SF12 0x0C

// Bandwidth
#define SID_PAL_RADIO_LORA_BW_7KHZ 0x00
#define SID_PAL_RADIO_LORA_BW_10KHZ 0x08
#define SID_PAL_RADIO_LORA_BW_15KHZ 0x01
#define SID_PAL_RADIO_LORA_BW_20KHZ 0x09
#define SID_PAL_RADIO_LORA_BW_31KHZ 0x02
#define SID_PAL_RADIO_LORA_BW_41KHZ 0x0A
#define SID_PAL_RADIO_LORA_BW_62KHZ 0x03
#define SID_PAL_RADIO_LORA_BW_125KHZ 0x04
#define SID_PAL_RADIO_LORA_BW_250KHZ 0x05
#define SID_PAL_RADIO_LORA_BW_500KHZ 0x06


// Coding Rate
#define SID_PAL_RADIO_LORA_CODING_RATE_4_5 0x01
#define SID_PAL_RADIO_LORA_CODING_RATE_4_6 0x02
#define SID_PAL_RADIO_LORA_CODING_RATE_4_7 0x03
#define SID_PAL_RADIO_LORA_CODING_RATE_4_8 0x04
#define SID_PAL_RADIO_LORA_CODING_RATE_4_5_LI 0x05
#define SID_PAL_RADIO_LORA_CODING_RATE_4_6_LI 0x06
#define SID_PAL_RADIO_LORA_CODING_RATE_4_8_LI 0x07

// packet params header type
#define SID_PAL_RADIO_LORA_HEADER_TYPE_VARIABLE_LENGTH 0x00
#define SID_PAL_RADIO_LORA_HEADER_TYPE_FIXED_LENGTH 0x01

// packet params crc modes
#define SID_PAL_RADIO_LORA_CRC_OFF 0x00
#define SID_PAL_RADIO_LORA_CRC_ON 0x01

// packet params IQ modes
#define SID_PAL_RADIO_LORA_IQ_NORMAL 0x00
#define SID_PAL_RADIO_LORA_IQ_INVERTED 0x01

// packet params LI modes
#define SID_PAL_RADIO_LORA_LDR_LONG_INTERLEAVER_OFF 0x00
#define SID_PAL_RADIO_LORA_LDR_LONG_INTERLEAVER_ON  0x01

// cad params
#define SID_PAL_RADIO_LORA_CAD_01_SYMBOL 0x00
#define SID_PAL_RADIO_LORA_CAD_02_SYMBOL 0x01
#define SID_PAL_RADIO_LORA_CAD_04_SYMBOL 0x02
#define SID_PAL_RADIO_LORA_CAD_08_SYMBOL 0x03
#define SID_PAL_RADIO_LORA_CAD_16_SYMBOL 0x04

#define SID_PAL_RADIO_LORA_CAD_EXIT_MODE_CAD_ONLY 0x00
#define SID_PAL_RADIO_LORA_CAD_EXIT_MODE_CAD_RX 0x01
#define SID_PAL_RADIO_LORA_CAD_EXIT_MODE_CAD_LBT 0x10

#define SID_PAL_RADIO_LORA_SF5_SF6_MIN_PREAMBLE_LEN 12

/* timeout duration in usec. the conversion to semtech ticks is done by start_rx() start_tx() */
#define SECS_TO_MUS(X)                         (X * 1000000UL)
/* set max radio timeout to 5sec */
#define SID_PAL_RADIO_LORA_CAD_DEFAULT_TX_TIMEOUT       SECS_TO_MUS(5)
#define SID_PAL_RADIO_LORA_DEFAULT_TX_TIMEOUT           SID_PAL_RADIO_LORA_CAD_DEFAULT_TX_TIMEOUT
/* 1 sec timeout used by diagnostics code */
#define SID_PAL_RADIO_LORA_TIMEOUT_DURATION_1_SEC       SECS_TO_MUS(1)

#define SID_PAL_RADIO_LORA_PRIVATE_NETWORK_SYNC_WORD    0x1424
#define SID_PAL_RADIO_LORA_PUBLIC_NETWORK_SYNC_WORD LORA_MAC_PUBLIC_SYNCWORD
#define SID_PAL_RADIO_LORA_MAX_PAYLOAD_LENGTH       250

#define SID_PAL_RADIO_LORA_ED_PREAMBLE_LENGTH_DEFAULT (250 << 3)
#define SID_PAL_RADIO_LORA_ED_MOD_SHAPING MOD_SHAPING_G_BT_1
#define SID_PAL_RADIO_LORA_ED_PREAMBLE_MIN_DETECT RADIO_PREAMBLE_DETECTOR_08_BITS
#define SID_PAL_RADIO_LORA_ED_SYNCWORD_LENGTH_DEFAULT (3 << 3)
#define SID_PAL_RADIO_LORA_ED_ADDRCOMP_DEFAULT RADIO_ADDRESSCOMP_FILT_OFF
#define SID_PAL_RADIO_LORA_ED_HEADER_TYPE_DEFAULT RADIO_PACKET_VARIABLE_LENGTH
#define SID_PAL_RADIO_LORA_ED_CRC_LENGTH_DEFAULT  RADIO_CRC_2_BYTES_CCIT
#define SID_PAL_RADIO_LORA_ED_RADIO_WHITENING_MODE_DEFAULT RADIO_DC_FREEWHITENING
#define SID_PAL_RADIO_LORA_ED_PAYLOAD_LENGTH_DEFAULT 0
#define SID_PAL_RADIO_LORA_ED_DEFAULT_WHITENING_SEED 0x01FF

/** Sidewalk phy lora crc present*/
typedef enum sid_pal_radio_lora_crc_present {
    SID_PAL_RADIO_CRC_PRESENT_INVALID = 0,
    SID_PAL_RADIO_CRC_PRESENT_OFF = 1,
    SID_PAL_RADIO_CRC_PRESENT_ON = 2,
    SID_PAL_RADIO_CRC_PRESENT_MAX_NUM = SID_PAL_RADIO_CRC_PRESENT_ON,
} sid_pal_radio_lora_crc_present_t;

/** Sidewalk phy lora modulation parameters*/
typedef struct sid_pal_radio_lora_modulation_params {
    uint8_t spreading_factor;
    uint8_t bandwidth;
    uint8_t coding_rate;
} sid_pal_radio_lora_modulation_params_t;

/** Sidewalk phy lora packet parameters*/
typedef struct sid_pal_radio_lora_packet_params {
    uint16_t preamble_length;
    uint8_t header_type;
    uint8_t payload_length;
    uint8_t crc_mode;
    uint8_t invert_IQ;
} sid_pal_radio_lora_packet_params_t;

/** Sidewalk Phy received LORA packet status*/
typedef struct sid_pal_radio_lora_rx_packet_status {
    int16_t rssi;
    int8_t snr;
    int8_t signal_rssi;
    sid_pal_radio_lora_crc_present_t is_crc_present;
} sid_pal_radio_lora_rx_packet_status_t;

/** Sidewalk phy lora cad parameters*/
typedef struct sid_pal_radio_lora_cad_params {
    uint8_t cad_symbol_num;
    uint8_t cad_detect_peak;
    uint8_t cad_detect_min;
    uint8_t cad_exit_mode;
    uint32_t cad_timeout;
} sid_pal_radio_lora_cad_params_t;

/** Sidewalk phy lora configuation handle*/
typedef struct sid_pal_radio_lora_phy_settings {
    uint32_t freq;
    int8_t power;
    uint16_t sync_word;
    uint8_t symbol_timeout;
    uint32_t tx_timeout;
    uint8_t lora_ldr_long_interleaved_enable;
    sid_pal_radio_lora_modulation_params_t lora_modulation_params;
    sid_pal_radio_lora_packet_params_t lora_packet_params;
    sid_pal_radio_lora_cad_params_t lora_cad_params;
} sid_pal_radio_lora_phy_settings_t;

#ifdef __cplusplus
}
#endif

/** @} */

#endif
