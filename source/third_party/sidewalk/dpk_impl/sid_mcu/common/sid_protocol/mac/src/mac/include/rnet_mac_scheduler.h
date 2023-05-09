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

#ifndef _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_SCHEDULER_EVENTS_H_
#define _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_SCHEDULER_EVENTS_H_

#include <stdbool.h>
#include <sid_pal_timer_types.h>
#include <sid_event_queue_ifc.h>

/*!
 * @brief Initialize MAC Scheduler and Submodules
 * @param void
 * @return void
*/
void rnet_mac_scheduler_init( void );

/*!
 * @brief Mac Scheduler process called from SWI upon RTC, pending Tx in transaction buffer and
 * Radio interrupt
 * @param void
 * @return void
*/
void rnet_mac_process_events( void );

/*!
 * @brief Check the idle status of MAC Scheduler
 * @param void
 * @return true if no event is running or pending to be scheduled
*/
bool rnet_mac_scheduler_is_idle(void);

/*!
 * @brief TBD if this function is required
*/
void RnetMacCheckSchedEvent( void );

#endif
