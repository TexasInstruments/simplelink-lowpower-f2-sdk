/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_FLEX_TYPES_H
#define SID_FLEX_TYPES_H

#include <sid_network_address.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The largest possible size for FLEX header. */
#define FLEX_MAX_SIZE 141

#define FLEX_META_AUTH_TAG_MAX_SIZE 16
#define FLEX_META_BCN_TIME_SIZE 6
#define FLEX_META_INNER_SRC_SIZE 5
#define FLEX_DEBUG_INFO_CRID_SIZE_VERSION_0 9

enum sid_flex_sequence_max {
    SID_LL_SEQUENCE_BYTE_MAX = (1 << 6) - 1,
    SID_LL_SEQUENCE_2BYTE_MAX = (1 << 14) - 1,
    SID_LL_SEQUENCE_3BYTE_MAX = (1 << 22) - 1,
    SID_LL_SEQUENCE_UNDEFINED = 0xFFFFFFFF
};

enum sid_flex_key_optional_field_ind {
    SID_FLEX_KEY_RSSI_INDICATOR = (1 << 0),
    SID_FLEX_KEY_SNR_INDICATOR = (1 << 1),
    SID_FLEX_KEY_DESPREAD_RSSI_INDICATOR = (1 << 2),
    SID_FLEX_KEY_LDR_TX_BIN_INDICATOR = (1 << 3)
};

struct sid_debug_info_s {
    uint8_t msg_crid[FLEX_DEBUG_INFO_CRID_SIZE_VERSION_0];
};

/**
 * definition of FLEX level
 *
 * @link https://wiki.labcollab.net/confluence/display/HALO/Halo+Flex+Format
 *
 * General structure and assumptions:
 *  -# all data are stored in network byte order
 *  -# package format
 *          - | 1B : size | 1..N Entries |
 *          - max package size 256 bytes
 *  -# Entry format
 *          - | 1B: HEADER | NB : PAYLOAD |
 *  -# HEADER format:
 *          - | 2b : size  | 6b : key     |
 *          - 2b size comes from @see FLEX_META_SIZE
 *          - 6b key - @see FLEX_META_KEY
 *  -# PAYLOAD: have size from 1 to N bytes
 */

struct sid_flex_properties {
    uint8_t version;

    uint8_t ack_req;
    uint8_t auth_tag_size;
    uint8_t avg_noise;
    uint8_t channel_number;
    uint8_t device_class;
    uint8_t enc_enabled;
    uint8_t is_pass_through_pld;
    uint8_t link_for_tx;
    uint8_t nack_msg;
    uint8_t phy_mode;
    uint8_t resp_required;
    uint8_t retries;
    uint8_t rf_frame_proto_version;
    uint8_t sba_flag;
    uint8_t sec_enabled;
    uint8_t sec_ref;
    uint8_t sec_topology;
    uint8_t ldr_tx_bin_idx;

    uint16_t rx_window_interval;
    uint16_t sleepy_device;

    uint32_t ll_seqn;
    uint32_t processing_delay_ms;
    uint32_t rx_epoch_nsec;
    uint32_t rx_epoch_sec;
    uint32_t sec_ref_counter;
    uint8_t is_data_unified_asl;
    uint64_t rx_tstamp;

    uint8_t auth_tag[FLEX_META_AUTH_TAG_MAX_SIZE];
    uint8_t bcn_time[FLEX_META_BCN_TIME_SIZE];
    struct sid_address inner_src;
    struct sid_address inner_dst;

    struct sid_debug_info_s debug;
};

struct sid_flex_stats {
    int8_t snr;
    int16_t rssi;
    int16_t signal_rssi;
    int32_t carrier_offset;
};

struct sid_flex {
    struct sid_address src;
    struct sid_address dst;

    uint32_t sequence;
    struct sid_flex_properties props;
    struct sid_flex_stats tr_stats;

    uint32_t optional_key_indicator;
    //! @note - this is not copy, just reference to buffer provided onto rnet_flex_parse()
    //!         not used in case of build flex
    size_t raw_flex_len;
    const uint8_t *raw_flex;
};

#ifdef __cplusplus
}   // extern "C"
#endif

#endif /* SID_FLEX_TYPES_H */
