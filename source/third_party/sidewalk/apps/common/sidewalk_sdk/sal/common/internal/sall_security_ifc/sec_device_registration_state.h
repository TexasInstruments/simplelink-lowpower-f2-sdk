/*
 * Copyright 2021-2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef SEC_DEVICE_REGISTRATION_STATE_H
#define SEC_DEVICE_REGISTRATION_STATE_H

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
 * DR_STATE_NOT_STARTED -> DR_STATE_STARTED -> DR_STATE_COMPLETED
 *
 * If during device registration, any component encounters an error
 * condition
 * DR_STATE_NOT_STARTED -> DR_STATE_STARTED -> DR_STATE_ERRORED
 *
 * If device registration needs to be cancelled, say ble disconnected
 * DR_STATE_NOT_STARTED -> DR_STATE_STARTED -> DR_STATE_CANCELLED
 *
 * If device registration times out, then
 * DR_STATE_NOT_STARTED -> DR_STATE_STARTED -> DR_STATE_TIMED_OUT
 *
 * note - That the DR_STATE_TIMED_OUT transition happens automatically and does
 * not need an explicit state transition
 *
 * All error states can transition to DR_STATE_STARTED
 * DR_STATE_ERRORED | DR_STATE_CANCELLED | DR_STATE_TIMED_OUT -> DR_STATE_STARTED
 *
 * Invalid state transitions are
 * DR_STATE_NOT_STARTED !-> DR_STATE_COMPLETED
 * DR_STATE_ERRORED | DR_STATE_CANCELLED | DR_STATE_COMPLETED | DR_STATE_TIMED_OUT !-> DR_STATE_STARTED
 */

enum dr_state {
    DR_STATE_NOT_STARTED = 1,
    DR_STATE_STARTED = 2,
    DR_STATE_COMPLETED = 3,
    DR_STATE_CANCELLED = 4,
    DR_STATE_TIMED_OUT = 5,
    DR_STATE_ERRORED = 6,
};

enum dr_cb_state {
    DR_CB_STATE_STARTED = 1,
    DR_CB_STATE_COMPLETED = 2,
    DR_CB_STATE_ERROR = 3,
};

enum dr_ss_state {
    DR_STATE_SS_NOT_STARTED = 1,
    DR_STATE_SS_STARTED = 2,
    DR_STATE_SS_COMPLETED = 3,
    DR_STATE_SS_CANCELLED = 4,
    DR_STATE_SS_TIMED_OUT = 5,
    DR_STATE_SS_ERRORED = 6,
};

enum dr_ss_cb_state {
    DR_CB_SS_STATE_STARTED = 1,
    DR_CB_SS_STATE_COMPLETED = 2,
    DR_CB_SS_STATE_ERROR = 3,
};

typedef sid_error_t (*dr_handle_state_t)(enum dr_cb_state state);
typedef sid_error_t (*dr_handle_ss_state_t)(enum dr_ss_cb_state state);

enum dr_state dr_get_current_state(void);
void dr_set_current_state(enum dr_state new_state);
sid_error_t dr_handle_state(enum dr_state state);
enum dr_ss_state dr_get_ss_current_state(void);
void dr_set_ss_current_state(enum dr_ss_state new_state);
sid_error_t dr_ss_handle_state(enum dr_ss_state state);

void dr_reset(void);
sid_error_t dr_init(const struct sid_event_queue *const event_queue,
                    enum dr_state initial_state,
                    enum dr_ss_state initial_ss_state,
                    uint32_t timeout_seconds,
                    dr_handle_state_t handle_cb,
                    dr_handle_ss_state_t handle_ss_cb);
void dr_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /*! SEC_DEVICE_REGISTRATION_STATE_H */
