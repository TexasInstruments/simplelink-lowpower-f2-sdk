/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_ASL_TYPES_H
#define SID_ASL_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NW Data format ID, the format IDs have a fixed value
 * */
enum nw_data_format_id {
    NW_DATA_FORMAT_0 = 0,
    NW_DATA_FORMAT_1 = 1,
    NW_DATA_FORMAT_2 = 2,
    NW_DATA_FORMAT_3 = 3,
    NW_DATA_FORMAT_4 = 4,
    NW_DATA_FORMAT_5 = 5,
    NW_DATA_FORMAT_6 = 6,
    NW_DATA_FORMAT_7 = 7,
    NW_DATA_FORMAT_8 = 8,
    NW_DATA_FORMAT_9 = 9,
    NW_DATA_FORMAT_10 = 10,
    NW_DATA_FORMAT_11 = 11,
    NW_DATA_FORMAT_12 = 12,
    NW_DATA_FORMAT_13 = 13,
    NW_DATA_FORMAT_14 = 14,
    NW_DATA_FORMAT_15 = 15,
    NW_DATA_FORMAT_MAX = 16
};

/**
 * Format 3 TLV tag ID.
 */
enum format_3_tag_id {
    NW_DATA_FORMAT_3_TAG_ID_SYNC_REQ = 1,
    NW_DATA_FORMAT_3_TAG_ID_LL_RETRY_CNT = 2,
};

/**
 * Format 3 TLV bytes and TLV delimiter length.
 */
enum format_3_tlv_byte_len {
    NW_DATA_FORMAT_3_SYNC_REQ_BYTE_LEN = 2,
    NW_DATA_FORMAT_3_LL_RETRY_CNT_BYTE_LEN = 2
};

/**
 * NW format ID 3.
 */
struct nw_data_format_3 {
    uint32_t tag_mask;
    uint8_t sync_req;
    uint8_t ll_retry_cnt;
};

/**
 * Timing info type.
 */
enum timing_info_type {
    NW_DATA_FORMAT_1_TIMING_INFO_TYPE_0 = 0,
    NW_DATA_FORMAT_1_TIMING_INFO_TYPE_1 = 1,
    NW_DATA_FORMAT_1_TIMING_INFO_TYPE_2 = 2,
    NW_DATA_FORMAT_1_TIMING_INFO_TYPE_3 = 3,
};

/**
 * Format of timing info of types 1, 2 and 3.
 */
struct timing_info_type_1_to_3 {
    uint16_t ticks;
    uint32_t secs_from_gps_epoch;
};

/**
 * Format of timing info.
 * Type 0 is latency in ms of type uint16_t.
 */
union timing_info {
    uint16_t latency_in_ms;
    struct timing_info_type_1_to_3 info_1_to_3;
};

/**
 * NW format ID 2.
 */
struct nw_data_format_2 {
    int8_t down_rssi;
    int8_t down_snr;
    int8_t tx_power;
    uint8_t timing_info_type;
    union timing_info info;
};

/**
 * NW format ID 1.
 */
struct nw_data_format_1 {
    int8_t down_rssi;
    int8_t down_snr;
};

/**
 * NW data formats.
 */
union nw_data {
    struct nw_data_format_1 one;
    struct nw_data_format_2 two;
    struct nw_data_format_3 three;
};

/**
 * NW data format structure.
 */
struct app_cmd_nw_data {
    enum nw_data_format_id id;
    union nw_data format;
    uint16_t pad_buffer_length;
};

/**
 * Decoded application command address.
 */
struct app_cmd_addr {
    uint16_t cmd_class_id;
    uint16_t cmd_id;
    uint8_t type;
};

/**
 * Application command format.
 */
struct app_cmd_frame {
    uint8_t version;
    bool status_code_inc;
    bool additional_app_data_inc;
    uint8_t status_code;
    uint8_t *additional_app_data;
    uint8_t additional_app_data_length;
    struct app_cmd_addr addr;
    uint8_t *app_payload;
    uint16_t app_payload_length;
};

/**
 * Presentation/Application support Layer format.
 */
struct app_pl_frame {
    bool version_inc;
    bool resp_req;
    bool seqn_inc;
    bool app_enc_ena;
    bool status_hdr_inc;
    bool nw_data_inc;
    uint8_t asl_version;
    uint16_t seqn;
    struct app_cmd_nw_data nw_data;
};

/**
 * Serialized buffer and meta information
 */
struct app_pl_frame_serialized_buffer {
    /** serialized buffer, passed by user and filled by serializer. */
    uint8_t *buffer;
    /** Length of the buffer the user has allocated. */
    uint16_t allocated_size;
    /** Length of the allocated buffer the serialization has used. */
    uint16_t total_used_size;
    /**
     * Offset at which the application command starts in the serialized frame
     * The user has to encrypt the message with the application server
     * key starting at this offset to the end of the buffer.
     */
    uint16_t app_cmd_frame_start_offset;
};

struct app_pl_frame_deserialized_buffer {
    /**
     * Offset at which the application command starts in the serialized buffer
     * The user has to decrypt the message with the application server
     * key starting at this offset to the end of the buffer.
     */
    uint16_t app_cmd_frame_offset;
    /**
     * Length of the application command frame i.e length of struct app_cmd_frame
     * and includes application payload length.
     */
    uint16_t app_cmd_frame_length;
    /** Pl frame deserialized. */
    bool pl_frame_deserialized;
    /** Presentation/Application support layer frame. */
    struct app_pl_frame pl_frame;
    /** Application command frame. */
    struct app_cmd_frame cmd_frame;
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* SID_ASL_TYPES_H */
