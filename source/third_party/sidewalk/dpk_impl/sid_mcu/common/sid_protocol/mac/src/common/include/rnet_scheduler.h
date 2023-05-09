/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNET_SCHEDULER_H
#define RNET_SCHEDULER_H

#include <sid_protocol_defs.h>
#include <sid_protocol_opts.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    EVENT_NONE=0,
    EVENT_START_CAD,
    EVENT_START_RX,
    EVENT_TX_PROTO_FRAME,
    EVENT_RADIO_TMOUT
} SchedEventTypes_t;

typedef struct {
    SchedEventTypes_t event;
    volatile U32 start_tm_tu;
    U8 phy_mode_idx;        // phy mode the event should use
} RnetSchedEvent_t;

void RnetCheckSchedEvent();

void RnetSchedEventTicksFromNow(U32 tu_from_now, SchedEventTypes_t event);

void RnetSchedEventMsFromNow(U32 ms_from_now, SchedEventTypes_t event);

void RnetSchedNextInterrupt();

void RnetSchedEventHandled();

void P2PModeSchedEvents();

void CancelScheduledEvents();

S16  p2p_schedule_next_tx();

void schedule_packet_sending();

bool is_packet_sending_scheduled();

#ifdef COMPILE_RX_DUTY_CYCLE
void p2p_start_tx();
#endif

#if defined(GW_SUPPORT) && GW_SUPPORT
U8 get_zwave_ed_enable_flag(uint8_t *buf);

U8 get_zwave_ed_threshold(uint8_t *buf);

void set_zwave_ed_enable_flag(U8 zwave_enable_flag_value);

void set_zwave_ed_threshold(S8 zwave_threshold_value, U8 zwave_timeout_value);
#endif

#ifdef __cplusplus
}
#endif

#endif
