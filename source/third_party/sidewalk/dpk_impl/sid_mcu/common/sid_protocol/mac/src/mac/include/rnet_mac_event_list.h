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

#ifndef _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_EVENT_LIST_H_
#define _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_EVENT_LIST_H_

#include "rnet_mac_submodule_common.h"

/*!
 * @brief Get the event that needs to go next after resolving any conflicts in time, priority
 * and/or token
 * @param void
 * @return pointer to the Event which is the head after conflict resolution
*/
rnet_mac_event_t* rnet_get_mac_event( void );

/*!
 * @brief Cancel and reschedule event in the list
 * @param event pointer to the Event which needs to be removed from the Event list and
 * rescheduled by the corresponding submodule at a later time
 * @param event_future_time pointer to timedef containing a time after which the slot time must
 *  be returned. If NULL then slot immediately after current time is selected
 * @return SUCCESS or FAIL
*/
rnet_mac_cmd_status_t rnet_reschedule_mac_event( rnet_mac_event_t *event, time_def *event_future_time);

/*!
 * @brief Get the address of head of the event list without resolving any conflicts
 * @param void
 * @return Pointer to the head of the Event list without removing it from the list
*/
rnet_mac_event_t* rnet_peek_mac_event( void );

/*!
 * @brief Pop the head of the event list without resolving any conflicts
 * @param void
 * @return Pointer to the Event after removing from the head of the Event list
*/
rnet_mac_event_t* rnet_pop_mac_event( void );

/*!
 * @brief Print the events is the list
*/
void rnet_mac_event_list_print();

#endif /* _SRC_MAC_HALO_INCLUDE_RNET_MAC_EVENT_LIST_H_ */
