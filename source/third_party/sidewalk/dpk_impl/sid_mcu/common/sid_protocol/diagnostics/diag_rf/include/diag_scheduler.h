/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.  This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef DIAG_SCHEDULER_H
#define DIAG_SCHEDULER_H

#include <sid_error.h>
#include <sid_pal_timer_ifc.h>
#include <sid_time_types.h>

#include <diag_test_mode.h>

#ifdef __cplusplus
extern "C" {
#endif

enum sched_event_types {
    EVENT_NONE = 0,
    EVENT_START_RX,
    EVENT_RADIO_TMOUT
};

struct diag_sched_events {
    enum sched_event_types event;
    sid_pal_timer_t mac_timer;
    sid_pal_timer_t pp_timer;
    sid_pal_timer_t tx_timer;
};

/**
 *  @brief Get schedule events and timers information sturcture
 *
 *  @return diag_sched_events which point to schedule events and timers information sturcture
 */
struct diag_sched_events *diag_sched_evnets_get(void);

/**
 *  @brief To init mac behavior timer, pp test mode timer, tx back-off timer
 *
 *  @param sid_error_t timers init status
 */
sid_error_t diag_sched_init_timers(void);

/**
 *  @brief To clear the event state
 */
void diag_sched_clear_event(void);

/**
 *  @brief To schedule an MAC event
 *
 *  @param delta the delay time to fire a MAC event
 *  @param event the MAC events include start continuous RX event and TX timeout event
 */
void diag_sched_mac_event(const struct sid_timespec *delta, enum sched_event_types event);

/**
 *  @brief To schedule a ping-pong event
 *
 *  @param msecs the delay time to fire a ping-pong event
 *  @param pp_state the transition state of pinger or ponger
 */
void diag_sched_pp_event(uint32_t msecs, enum diag_test_ping_pong_state pp_state);

/**
 *  @brief To setup continuous RX MAC event if previous RX action is finished
 */
void diag_sched_set_continuous_rx(void);

/**
 *  @brief To schedule a TX packet event
 *
 *  @param msecs the delay time to fire a TX packet event which can
 *               control the inter-packet interval time
 */
void diag_sched_tx_event(uint32_t msecs);

/**
 *  @brief To check and run PP and TX timer events when SWI is triggered
 */
void diag_sched_process_timer_events(void);

#ifdef __cplusplus
}
#endif

#endif