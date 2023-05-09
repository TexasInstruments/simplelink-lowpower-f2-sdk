/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef HALO_LIB_RINGNET_WAN_SRC_INCLUDE_HALO_LIB_RINGNET_WAN_RNET_OBSERVER_H
#define HALO_LIB_RINGNET_WAN_SRC_INCLUDE_HALO_LIB_RINGNET_WAN_RNET_OBSERVER_H

#include <rnet_transact_store.h>

/**
 * Possible packet event types
 */
typedef enum {
    RNET_OBSERVER_PKT_EVENT_TX_ERROR,   /**< TX failed  */
    RNET_OBSERVER_PKT_EVENT_TX_SUCCESS, /**< TX success */
    RNET_OBSERVER_PKT_EVENT_RX,         /**< RX         */
    RNET_OBSERVER_PKT_EVENT_TX_START,   /**< TX start   */
} rnet_observer_pkt_event_t;

/**
 * Observer operations that will be called in case of events
 */
typedef struct {
    void (*pkt_event)(void* ctx, rnet_observer_pkt_event_t event, const rnet_transact_store_t *pkt);
    void (*message_start_event)(void* ctx, uint32_t duration);
    void (*pairing_state_event)(void* ctx, bool prev_state, bool current_state);
    void (*dfu_req_event)(void *ctx);
} rnet_observer_ops_t;

/**
 * Observer declarations
 */
typedef struct {
    rnet_observer_ops_t ops;
    void               *ctx;
} rnet_observer_t;

/**
 * Should be called by rnet in case of packet event
 *
 *  TODO: HALO-4767
 *
 * @param[in] event  Event type. @sa rnet_timestamp_gps_t
 * @param[in] pkt    Packet event data
 */
void rnet_observer_pkt_event(rnet_observer_pkt_event_t event, const rnet_transact_store_t *pkt);

/**
 * Should be called by rnet in case of packet is start transfering
 *
 * @param[in] duration    Packet transfer duration
 */

void rnet_observer_message_start_event(uint32_t duration);

/**
 * Should be called by rnet in case of pairing event
 *
 * @param[in] prev_state     Previous state
 * @param[in] current_state  Current state
 */
void rnet_observer_pairing_state_event(bool prev_state, bool current_state);

/**
 * Should be called by rnet in case of dfu event
 */
void rnet_observer_dfu_req_event();

/**
 * Public API to register observer
 *
 * @param[in] observer      Observer to use in rnet
 * @retval true     Success
 * @retval false    Error
 */
bool rnet_observer_set(const rnet_observer_t *observer);

#endif /* HALO_LIB_RINGNET_WAN_SRC_INCLUDE_HALO_LIB_RINGNET_WAN_RNET_OBSERVER_H */
