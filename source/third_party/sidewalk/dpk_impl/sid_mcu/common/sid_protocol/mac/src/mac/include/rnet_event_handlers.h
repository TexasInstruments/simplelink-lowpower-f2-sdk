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

#ifndef RNET_EVENT_HANDLERS_H
#define RNET_EVENT_HANDLERS_H

#include <sid_protocol_defs.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t dio1;
    uint8_t rtc;
    uint8_t tx_pkt;
    uint8_t nwl_event;
    uint8_t phy;
} swi_pending_events_t;

/**
 * @brief SWI initialization. Must be called before an event can be processed.
 */
void event_handler_init(void);

/**
 * @brief Trigger the SWI to process a pending event. event_handler_init() must be
 *        called before this function.
 */
void event_handler_process(void);

/**
 * @brief Trigger the SWI to process a radio event.
 */
void event_handler_radio_process(void);

/**
 * @brief Trigger the SWI to process a phy event.
 */
void event_handler_phy_process(void);

/**
 * @brief Pending event query
*  Returns True if there is a pending dio, rtc, tx_pckt, nwl or phy event. False Otherwise.
 */
bool is_event_pending(void);

/**
 * @brief Pending event flags
 *      TODO: Make this a non-global variable
 */
extern volatile swi_pending_events_t pending_events;

#ifdef __cplusplus
}
#endif

#endif
