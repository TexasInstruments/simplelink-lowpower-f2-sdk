/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNET_FSK_WAN_SCH_LOOKUP_TAGS_H
#define RNET_FSK_WAN_SCH_LOOKUP_TAGS_H

#include <rnet_nw_layer.h>
#include "rnet_dev_addr_def.h"

// List of TLV TAGs used in FSK-WAN slot lookup request/response
enum rnet_fsk_wan_sch_query_tlv_tag_list{
    RNET_FSK_WAN_SCH_QUERY_TLV_TAG_FSK_WAN_JOIN_RESPONSE_CODE = 0x08,
    RNET_FSK_WAN_SCH_QUERY_TLV_TAG_DL_SCHED_PROF_3 = 0x01,
    RNET_FSK_WAN_SCH_QUERY_TLV_TAG_UL_SCHED_PROF_3 = 0x06,
    RNET_FSK_WAN_SCH_QUERY_TLV_TAG_UL_CONTENTION_CHAR = 0x07,
    RNET_FSK_WAN_SCH_QUERY_TLV_TAG_LINK_DURATION = 0x09,
    RNET_FSK_WAN_SCH_QUERY_TLV_TAG_SYNCED_GW = 0x03,
};

// Expected size of TLV-values in bytes used in FSK-WAN slot lookup request/response
enum rnet_fsk_wan_sch_query_tlv_tag_exp_size_list{
    RNET_FSK_WAN_SCH_QUERY_TLV_TAG_FSK_WAN_JOIN_RESPONSE_CODE_SIZE = 1,
    RNET_FSK_WAN_SCH_QUERY_TLV_TAG_DL_SCHED_PROF_3_SIZE = 2,
    RNET_FSK_WAN_SCH_QUERY_TLV_TAG_UL_SCHED_PROF_3_SIZE = 2,
    RNET_FSK_WAN_SCH_QUERY_TLV_TAG_UL_CONTENTION_CHAR_SIZE = 2,
    RNET_FSK_WAN_SCH_QUERY_TLV_TAG_LINK_DURATION_SIZE = 1,
    RNET_FSK_WAN_SCH_QUERY_TLV_TAG_SYNCED_GW_SIZE = RNET_DEV_ID_SZ,
};

// Structure to holding deserialized sch_response data.
struct slot_sch_resp_data {
    uint32_t join_proc_keys_mask; // Keymask for valid entries.
    uint8_t join_response_code;
    uint8_t node_address[RNET_DEV_ID_SZ];
    uint8_t fsk_wan_ul_offset;
    uint8_t fsk_wan_ul_period;
    uint8_t fsk_wan_dl_offset;
    uint8_t fsk_wan_dl_period;
    uint8_t fsk_wan_ul_cont_subslot;
    uint8_t fsk_wan_ul_cont_slot;
    uint8_t link_duration;
};
// Expected keymask with all struct members valid for slot_sch_resp_data.
#define RNET_SCH_RESP_DATA_FULL_MASK (\
        (1 << RNET_FSK_WAN_SCH_QUERY_TLV_TAG_FSK_WAN_JOIN_RESPONSE_CODE) | (1 << RNET_FSK_WAN_SCH_QUERY_TLV_TAG_SYNCED_GW) | \
        (1 << RNET_FSK_WAN_SCH_QUERY_TLV_TAG_UL_SCHED_PROF_3) | (1 << RNET_FSK_WAN_SCH_QUERY_TLV_TAG_DL_SCHED_PROF_3) | \
        (1 << RNET_FSK_WAN_SCH_QUERY_TLV_TAG_UL_CONTENTION_CHAR) | (1 << RNET_FSK_WAN_SCH_QUERY_TLV_TAG_LINK_DURATION))

// Expected size of seralized sch-lookup request message
#define SERIAL_SCH_LOOKUP_REQ_DATA_SIZE 7

#endif /* RNET_FSK_WAN_SCH_LOOKUP_TAGS_H */
