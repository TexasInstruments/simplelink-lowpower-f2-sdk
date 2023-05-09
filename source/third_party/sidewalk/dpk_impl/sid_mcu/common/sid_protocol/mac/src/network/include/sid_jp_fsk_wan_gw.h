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

#ifndef RNET_FSK_WAN_JOIN_PROCESSOR_H_
#define RNET_FSK_WAN_JOIN_PROCESSOR_H_
#include <halo/lib/hall/appv1/mgm/types.h>
#include <rnet_frames.h>
#include <stdint.h>

struct sid_jp_generic_data_holder {
    uint32_t join_proc_keys_mask;
    uint8_t join_response_code;
    uint8_t fsk_wan_ul_offset;
    uint8_t fsk_wan_ul_periodicity;
    uint8_t fsk_wan_dl_offset;
    uint8_t fsk_wan_dl_periodicity;
    uint8_t subslot_contention_window_size;
    uint8_t slot_contention_window_size;
    uint8_t link_duration;
    uint8_t node_address[HALO_DEVICE_ID_SIZE];
    uint8_t fsk_wan_ul_latency;
    uint8_t fsk_wan_ul_load;
    uint8_t fsk_wan_dl_latency;
    uint8_t fsk_wan_dl_load;
    uint8_t fsk_wan_sec_key[SEC_PROCESS_TYPE_FSK_WAN_JOIN_KEY_SIZE];
};

#define JOIN_RESP_PARAM_FULL_MASK ((1 << HALO_HALL_APPV1_MGM_FSK_WAN_JOIN_RESPONSE_CODE) | \
    (1 << HALO_HALL_APPV1_MGM_UL_SCHED_PROF_3) | (1 << HALO_HALL_APPV1_MGM_DL_SCHED_PROF_3) | \
    (1 << HALO_HALL_APPV1_MGM_UL_CONTENTION_CHAR) | (1 << HALO_HALL_APPV1_MGM_LINK_DURATION))

#define JOIN_RESP_REJECT_PARAM_FULL_MASK (1 << HALO_HALL_APPV1_MGM_FSK_WAN_JOIN_RESPONSE_CODE)

#define JOIN_RESP_PARAM_FOR_TERM_FULL_MASK JOIN_RESP_REJECT_PARAM_FULL_MASK

#define SLOT_ASSIGNMENT_REQ_PARAM_FULL_MASK (\
    (1 << HALO_HALL_APPV1_MGM_UL_CHAR_PROF_3) | (1 << HALO_HALL_APPV1_MGM_DL_CHAR_PROF_3) | \
    (1 << HALO_HALL_APPV1_MGM_SYNCED_GW) | (1 << HALO_HALL_APPV1_MGM_LINK_DURATION))

#define SLOT_ASSIGNMENT_RESP_PARAM_FULL_MASK (\
    (1 << HALO_HALL_APPV1_MGM_FSK_WAN_JOIN_RESPONSE_CODE) | (1 << HALO_HALL_APPV1_MGM_SYNCED_GW) | \
    (1 << HALO_HALL_APPV1_MGM_UL_SCHED_PROF_3) | (1 << HALO_HALL_APPV1_MGM_DL_SCHED_PROF_3) | \
    (1 << HALO_HALL_APPV1_MGM_UL_CONTENTION_CHAR) | (1 << HALO_HALL_APPV1_MGM_LINK_DURATION))

#define SLOT_ASSIGNMENT_RESP_REJECT_PARAM_FULL_MASK (\
    (1 << HALO_HALL_APPV1_MGM_FSK_WAN_JOIN_RESPONSE_CODE) | (1 << HALO_HALL_APPV1_MGM_SYNCED_GW))

#define SLOT_ASSIGNMENT_RESP_PARAM_MIN_MASK SLOT_ASSIGNMENT_RESP_REJECT_PARAM_FULL_MASK

#define AUTH_RESP_PARAM_FULL_MASK (\
        (1 << HALO_HALL_APPV1_MGM_FSK_WAN_JOIN_RESPONSE_CODE) | (1 << HALO_HALL_APPV1_MGM_SYNCED_GW) | \
        (1 << HALO_HALL_APPV1_MGM_UL_CHAR_PROF_3) | (1 << HALO_HALL_APPV1_MGM_DL_CHAR_PROF_3) | \
        (1 << HALO_HALL_APPV1_MGM_LINK_DURATION) | (1 << HALO_HALL_APPV1_MGM_SEC_KEY))

#define TERM_RESP_PARAM_FULL_MASK (\
        (1 << HALO_HALL_APPV1_MGM_FSK_WAN_JOIN_RESPONSE_CODE) | (1 << HALO_HALL_APPV1_MGM_SYNCED_GW) | \
        (1 << HALO_HALL_APPV1_MGM_SEC_KEY))

#define SLOT_LOOKUP_REQ_FULL_MASK (1 << HALO_HALL_APPV1_MGM_SYNCED_GW)
#define SLOT_EXT_REQ_FULL_MASK (1 << HALO_HALL_APPV1_MGM_SYNCED_GW)
#define TERM_RESP_FULL_MASK (1 << HALO_HALL_APPV1_MGM_SYNCED_GW)

bool sid_fsk_wan_jp_is_active(uint8_t *node_address);
sid_error_t sid_fsk_wan_jp_consume_sch_assign_resp(struct sid_jp_generic_data_holder *fsk_wan_info);
sid_error_t sid_fsk_wan_jp_consume_join_wan_req(struct sid_jp_generic_data_holder *fsk_wan_info);

void reset_join_proc_state(void);

#endif /* RNET_FSK_WAN_JOIN_PROCESSOR_H_ */
