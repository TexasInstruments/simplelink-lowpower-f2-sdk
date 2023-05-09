/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNET_EVENTS_H
#define RNET_EVENTS_H

/**
 * RingNet Events
 *
 * Functions are defined as weak to allow for the application layer to easily create callbacks for when things happen in
 * the ringnet layer.
 */

/**
 * LoRa Communication Events
 */

#if defined(__GNUC__)
// LoRa Communication Events

void rnet_event_on_before_transmit(const uint8_t *buffer, const uint8_t size) __attribute__((weak));

// Called immediately after a LoRa transmission has started
void rnet_event_on_transmit_started(const uint8_t *buffer, const uint8_t size) __attribute__ ((weak));

// Called immediately after a LoRa transmission has finished
void rnet_event_on_transmit_finished(void) __attribute__ ((weak));

// BLE Communication Events
void rnet_event_on_ble_connection(const uint16_t m_connection_handle) __attribute__((weak));

void rnet_event_on_ble_disconnect(const uint16_t m_connection_handle) __attribute__((weak)) ;

// Ringnet Setting Events
 void rnet_event_on_group_id_changed(const uint8_t new_group_id) __attribute__((weak)) ;
#elif defined(__ICCARM__)
// LoRa Communication Events

__weak void rnet_event_on_before_transmit(const uint8_t *buffer, const uint8_t size);

// BLE Communication Events

__weak void rnet_event_on_ble_connection(const uint16_t m_connection_handle);

__weak void rnet_event_on_ble_disconnect(const uint16_t m_connection_handle);

// Ringnet Setting Events

__weak void rnet_event_on_group_id_changed(const uint8_t new_group_id);
#endif

#endif // RNET_EVENTS_H
