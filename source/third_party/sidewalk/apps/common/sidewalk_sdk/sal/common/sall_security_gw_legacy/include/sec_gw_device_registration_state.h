/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef SEC_GW_DEVICE_REGISTRATION_STATE_H
#define SEC_GW_DEVICE_REGISTRATION_STATE_H

#include <sid_event_queue_ifc.h>

#include <sid_error.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @section device registration state handler
 *
 * This module primarily implements are simple state machine that triggers a
 * timeout error state if the device registration is not completed within a set
 * time frame. In addition it also handles possible state transitions that
 * could occur during device registration and will prevent invalid state
 * transitions.
 *
 * The state change for successful device registration is
 * DR_GW_STATE_NOT_STARTED -> DR_GW_STATE_STARTED -> DR_GW_STATE_COMPLETED
 *
 * If during device registration, any component encounters an error
 * condition
 * DR_GW_STATE_NOT_STARTED -> DR_GW_STATE_STARTED -> DR_GW_STATE_ERRORED
 *
 * If device registration needs to be cancelled, say ble disconnected
 * DR_GW_STATE_NOT_STARTED -> DR_GW_STATE_STARTED -> DR_GW_STATE_CANCELLED
 *
 * If device registration times out, then
 * DR_GW_STATE_NOT_STARTED -> DR_GW_STATE_STARTED -> DR_GW_STATE_TIMED_OUT
 *
 * note - That the DR_GW_STATE_TIMED_OUT transition happens automatically and does
 * not need an explicit state transition
 *
 * All error states can transition to DR_GW_STATE_STARTED
 * DR_GW_STATE_ERRORED | DR_GW_STATE_CANCELLED | DR_GW_STATE_TIMED_OUT -> DR_GW_STATE_STARTED
 *
 * Invalid state transitions are
 * DR_GW_STATE_NOT_STARTED !-> DR_GW_STATE_COMPLETED
 * DR_GW_STATE_ERRORED | DR_GW_STATE_CANCELLED | DR_GW_STATE_COMPLETED | DR_GW_STATE_TIMED_OUT !-> DR_GW_STATE_STARTED
 */

enum dr_gw_state {
    DR_GW_STATE_NOT_STARTED = 1,
    DR_GW_STATE_STARTED = 2,
    DR_GW_STATE_COMPLETED = 3,
    DR_GW_STATE_CANCELLED = 4,
    DR_GW_STATE_TIMED_OUT = 5,
    DR_GW_STATE_ERRORED = 6,
};

enum dr_gw_cb_state {
    DR_CB_STATE_STARTED = 1,
    DR_CB_STATE_COMPLETED = 2,
    DR_CB_STATE_ERROR = 3,
};

typedef sid_error_t (*dr_gw_handle_state_t)(enum dr_gw_cb_state state);

enum dr_gw_state dr_gw_get_current_state(void);
void dr_gw_set_current_state(enum dr_gw_state new_state);
sid_error_t dr_gw_handle_state(enum dr_gw_state state);

void dr_gw_reset(void);
sid_error_t dr_gw_init(const struct sid_event_queue *const event_queue,
                       enum dr_gw_state initial_state,
                       uint32_t timeout_seconds,
                       dr_gw_handle_state_t handle_cb);
void dr_gw_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /*! SEC_GW_DEVICE_REGISTRATION_STATE_H */
