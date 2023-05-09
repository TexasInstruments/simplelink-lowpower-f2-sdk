/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_MAC_LINK_LAYER_FRAME_H
#define SID_MAC_LINK_LAYER_FRAME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Describes the type of frame data
 */
enum sid_link_layer_frame_type {
    /** Indicates the frame type is invalid or unknown */
    SID_LINK_LAYER_FRAME_TYPE_UNKNOWN = -1,

    /** Indicates a Beacon frame - TODO: add more explanation */
    SID_LINK_LAYER_FRAME_TYPE_BEACON = 0,
    /** Indicates a frame contains an application layer data frame */
    SID_LINK_LAYER_FRAME_TYPE_DATA = 1,
    /** Indicates a network data frame */
    SID_LINK_LAYER_FRAME_TYPE_NETWORK_DATA = 2,
    /** Indicates an ACK frame */
    SID_LINK_LAYER_FRAME_TYPE_ACK = 3,
    /** Indicates a probe request frame */
    SID_LINK_LAYER_FRAME_TYPE_PROBE_REQ = 4,
    /** Indicates a probe request response */
    SID_LINK_LAYER_FRAME_TYPE_PROBE_RESP = 5,

    SID_LINK_LAYER_FRAME_TYPE_MAX_SUPPORTED,
};

/**
 * Used to signal if a frame is used for synchronization. Only applies to LDR uplink.
 */
enum sid_link_layer_epoch_type {
    /** The default for For HDR and LDR downlink */
    SID_LINK_LAYER_EPOCH_TYPE_DEFAULT = 0,
    /** For LDR uplink indicates the frame is not used for synchronization */
    SID_LINK_LAYER_EPOCH_TYPE_NO_SYNC = 0,
    /** For LDR uplink indicates the frame IS used for synchronization */
    SID_LINK_LAYER_EPOCH_TYPE_FRAME_SYNC = 1,
};

/**
 * Describes the format of the link layer source field
 */
enum sid_link_layer_src_format {
    /** Indicates the source of the frame is the cloud; the source field identifies the cloud address/ID */
    SID_LINK_LAYER_SRC_FORMAT_CLOUD = 0,
    /** Indicates the source field contains a device ID */
    SID_LINK_LAYER_SRC_FORMAT_DEV_ID = 1,
    /** Indicates the source field contains handle (TODO: what is this used for?) */
    SID_LINK_LAYER_SRC_FORMAT_HANDLE = 2,
};

/**
 * Describes the format of the link layer destination field
 */
enum sid_link_layer_dst_format {
    /** Indicates the destination for the frame is the cloud; the destination field identifies the cloud address/ID */
    SID_LINK_LAYER_DST_FORMAT_CLOUD = 0,
    /** Indicates the destination field contains a device ID */
    SID_LINK_LAYER_DST_FORMAT_DEV_ID = 1,
    /** Indicates the destination field contains handle (TODO: what is this used for?) */
    SID_LINK_LAYER_DST_FORMAT_HANDLE = 2,
    /** Indicates the destination field contains a group ID */
    SID_LINK_LAYER_DST_FORMAT_GROUP_ID = 3,
};

struct sid_link_layer_data {
    enum sid_link_layer_frame_type frame_type;
    enum sid_link_layer_epoch_type epoch_bit;
    uint8_t proto_ver;
    enum sid_link_layer_src_format src_format;
    enum sid_link_layer_dst_format dst_format;
    uint8_t nw_frame_ctrl_inc;
    uint8_t ack_req;
    uint8_t suppress_broadcast_ack;
    uint8_t src_sz;
    uint8_t dst_sz;
    uint8_t src[5];
    uint8_t dst[5];
    uint32_t seq_num;
};

#ifdef __cplusplus
}
#endif

#endif // SID_MAC_LINK_LAYER_FRAME_H
