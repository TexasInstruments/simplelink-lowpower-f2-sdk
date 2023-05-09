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

#ifndef SID_NW_SYNC_H
#define SID_NW_SYNC_H

#include "rnet_transact_store.h"
#include "rnet_time.h"

#define SID_NW_SYNC_PERIODICAL_SYNC_NO_RETRIES 0
#define SID_NW_SYNC_PERIODICAL_SYNC_MAX_RETRIES 3

#define SID_NW_SYNC_INTERVAL_SECS_HDR_DEFAULT 840 // in sec
#define SID_NW_SYNC_INTERVAL_SECS_LDR_DEFAULT 300 // in sec
#define SID_NW_SYNC_NOTIFY_WAITING_SECS_DEFAULT 25 // in sec

/* Network synchronization event */
enum sid_nw_sync_event {
    SID_NW_SYNC_EVENT_NONE = 0x00,
    SID_NW_SYNC_EVENT_RESPONSE = 0x01,
    SID_NW_SYNC_EVENT_TERMINATE = 0x02,
    SID_NW_SYNC_EVENT_NOTIFY = 0x03,
    SID_NW_SYNC_EVENT_SYNC_STATE_TIMEOUT = 0x04,
    SID_NW_SYNC_EVENT_PIGGYBACKING_STATE_TIMEOUT = 0x05,
    SID_NW_SYNC_EVENT_WAITING_PIGGYBACK_NOTIFY_TIMEOUT = 0x06,
    SID_NW_SYNC_EVENT_UPLINK_DATA = 0x07
};

/* Network sychronization timer configuration*/
struct sid_nw_sync_timer_cfg {
    uint32_t sync_interval_secs;
    uint16_t sync_notify_waiting_secs;
};

/* Network sychronization active sync configuration*/
struct sid_nw_periodical_sync_cfg {
    bool enabled;
    uint8_t num_retries;
};

/*!
 * @brief Network synchronization activate
 * @param periodical_sync_cfg: .enabled -
 *                             True: Support not only passive synchronization in piggybacking state, when
 *                             end device is not synced, activate JOIN process as synchronization
 *                             False: Support only passive synchronization in piggybacking state, no JOIN
 *                             process activated as synchronization
 *                             .num_retries - Number of periodical sync retry
 * @param timer_cfg: Timer configuration
 */
void sid_nw_sync_activate(struct sid_nw_periodical_sync_cfg *periodical_sync_cfg, struct sid_nw_sync_timer_cfg *timer_cfg);

/*!
 * @brief Network synchronization deactivate
 */
void sid_nw_sync_deactivate(void);

/*!
 * @brief Network synchronization timer process
 * @param curr_tm: Current time in time_def, assertion for NULLP
 */
time_def sid_nw_sync_timer_process(const time_def *curr_tm);

/*!
 * @brief Processing outgoing packets, set network sync packet according to condition
 * @param[in/out] tr: Transaction store
 */
void sid_nw_sync_process_outgoing(rnet_transact_store_t *tr);

/*!
 * @brief Notify network layer synchronization event
 * @param event: Network synchronization event
 */
void sid_nw_sync_event_handler(enum sid_nw_sync_event event);

#endif // SID_NW_SYNC_H
