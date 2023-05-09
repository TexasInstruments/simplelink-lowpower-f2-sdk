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

#ifndef SID_STATE_IFC_H
#define SID_STATE_IFC_H

#include <sid_network_address.h>
#include <sid_time_ops.h>
#include <stdbool.h>

#include <lk/list.h>

#ifdef __cplusplus
extern "C" {
#endif


enum sid_state_event_type {
    SID_STATE_EVENT_REGISTRATION = 1,
    SID_STATE_EVENT_TIME_SYNC = 2,
    SID_STATE_EVENT_LINK_CHANGE = 3,
    SID_STATE_EVENT_ADDRESS_CHANGE = 4,
    SID_STATE_EVENT_FACTORY_RESET = 5,
};

enum sid_state_link_type {
    SID_STATE_LINK_TYPE_1 = 1 << 0,
    SID_STATE_LINK_TYPE_2 = 1 << 1,
    SID_STATE_LINK_TYPE_3 = 1 << 2,
};

enum sid_state_registration_status {
    SID_STATE_REGISTRATION_STATUS_REGISTERED = 0,
    SID_STATE_REGISTRATION_STATUS_NOT_REGISTERED = 1,
    SID_STATE_REGISTRATION_STATUS_STARTED = 2,
    SID_STATE_REGISTRATION_STATUS_FAILED = 3,
};

enum sid_state_time_sync_status {
    SID_STATE_TIME_SYNC_DONE = 0,
    SID_STATE_TIME_SYNC_FAILED = 1,
};

struct sid_state_event_data {
    enum sid_state_event_type type;
    union {
        struct {
            enum sid_state_registration_status status;
        } registration;
        struct {
            enum sid_state_time_sync_status status;
            enum sid_state_link_type recieved_link;
            struct sid_timespec time;
            struct sid_timespec drift;
            struct sid_address remote;
        } timesync;
        struct {
            enum sid_state_link_type link_type;
            uint32_t link_mask;
        } link_change;
        struct {
            struct sid_address address;
            bool flush;
        } address_change;
    };
};

enum sid_state_event_priority {
    SID_STATE_EVENT_PRIORITY_LIBS = 1,    // Libs such as sid_registration, sid_state get the highest priority
    SID_STATE_EVENT_PRIORITY_LINKS = 2,   // Links get the next priority
    SID_STATE_EVENT_PRIORITY_API = 3,     // sid_api get the least priority since it need to indicate to users
    SID_STATE_EVENT_PRIORITY_MAX = SID_STATE_EVENT_PRIORITY_API,
};

struct sid_state_callback {
    list_node_t node;
    void (*state_change_callback)(const struct sid_state_event_data *data, void *context);
    void *context;
    uint8_t prio;
};

/**
 * Initializes the sidewalk state library
 */
void sid_state_init(void);

/**
 * De-initializes the sidewalk state library
 */
void sid_state_deinit(void);

/**
 * Register event callback to sidewalk state library
 *
 * Registered components get a callback whenever a state change
 * occurs. The data payload contains the new value of the state
 * and any relevant additional data.
 *
 */
void sid_state_register_state_change_callback(struct sid_state_callback *callback);

/**
 * Checks if link is connected
 *
 * @param[in] link_type The link that needs to be checked
 *
 * @returns true if link_type is connected else false
 */
bool sid_state_is_link_connected(enum sid_state_link_type link_type);

/**
 * Returns the connected link mask
 *
 * @returns uint8_t value where each set bit represents if that
 *           link is active
 */
uint8_t sid_state_get_connected_mask(void);

/**
 * Set link has connected
 *
 * Triggers state change event SID_STATE_EVENT_LINK_CHANGE
 *
 * @param[in] link_type The link that has connected
 */
void sid_state_set_link_connected(enum sid_state_link_type link_type);

/**
 * Set link has disconnected
 *
 * Triggers state change event SID_STATE_EVENT_LINK_CHANGE
 *
 * @param[in] link_type The link that has disconnected
 */
void sid_state_set_link_disconnected(enum sid_state_link_type link_type);

/**
 * Checks if device is sidewalk registered
 *
 * @returns true if device is registered else false
 */
bool sid_state_is_device_registered(void);

/**
 * Check if registration has started
 *
 * @returns true is registration has started false otherwise
 */
bool sid_state_has_device_registration_started(void);

/**
 * Set device has been registered
 *
 * Additionally triggers state change event #SID_STATE_EVENT_REGISTRATION
 * and registration state #SID_STATE_REGISTRATION_STATUS_REGISTERED
 *
 * @return #SID_ERROR_NONE on success, error_code otherwise
 */
sid_error_t sid_state_set_device_registered(void);

/**
 * Set device has been unregistered
 *
 * Additionally triggers state change event #SID_STATE_EVENT_REGISTRATION
 * and registration state #SID_STATE_REGISTRATION_STATUS_NOT_REGISTERED
 *
 * @return #SID_ERROR_NONE on success, error_code otherwise
 */
sid_error_t sid_state_set_device_unregistered(void);

/**
 * Trigger registration started state change
 *
 * Triggers state change event #SID_STATE_EVENT_REGISTRATION
 * and registration state #SID_STATE_REGISTRATION_STATUS_STARTED_REGISTRETION
 */
void sid_state_trigger_device_started_registration(void);

/**
 * Trigger registration failed state change
 *
 * Triggers state change event #SID_STATE_EVENT_REGISTRATION
 * and registration state #SID_STATE_REGISTRATION_STATUS_FAILED_REGISTRATION
 */
void sid_state_trigger_device_failed_registration(void);

/**
 * Checks if network time sync is done
 *
 * @return true if network time sync is done else false
 */
bool sid_state_is_network_timesync_done(void);

/**
 * Sets network gcs timesync  is done
 *
 * Triggers state change event #SID_STATE_EVENT_TIME_SYNC and
 * time sync status #SID_STATE_TIME_SYNC_DONE
 *
 * @param[in] timespec New received network gcs time
 * @param[in] drift Received network gcs time drift
 * @param[in] remote Remote address from where time-sync was received from
 * @param[in] recieved_link The link over which time-sync was received from
 *
 * @return true if network time sync is done else false
 */
void sid_state_set_network_timesync(const struct sid_timespec *const timespec,
                                    const struct sid_timespec *const drift,
                                    const struct sid_address *const remote,
                                    enum sid_state_link_type recieved_link);

/**
 * Unsets network gcs time
 *
 * Triggers state change event #SID_STATE_EVENT_TIME_SYNC and
 * time sync status #SID_STATE_TIME_SYNC_FAILED
 */
void sid_state_unset_network_timesync(void);

/**
 * Triggers a factory reset state change
 *
 * Triggers state change event #SID_STATE_EVENT_FACTORY_RESET
 */
void sid_state_trigger_factory_reset(void);

/**
 * Trigger an address chnage state change
 *
 * @param[in] address New address
 * @param[in] flush True address is stored in flash, False it's not stored in flash
 */
void sid_state_trigger_address_change(const struct sid_address *const address, bool flush);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_STATE_IFC_H */
