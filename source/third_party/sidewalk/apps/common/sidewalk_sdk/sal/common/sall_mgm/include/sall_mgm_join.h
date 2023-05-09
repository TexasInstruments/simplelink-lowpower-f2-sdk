/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SALL_MGM_JOIN_INTERFACE_H
#define SALL_MGM_JOIN_INTERFACE_H

#include <sall_mgm_lib.h>

/* @brief : Function called upon connection state change
 * @param [in] : pointer to the sall_mgm_lib_impl_t
 * @param [in] : pointer to network_interface
 * @param [in] : The Join command will be triggered if the connection state is
 *                   NETWORK_INTERFACE_CONTROL_CONNECTION_STATE_SYNCED,
 *                   NETWORK_INTERFACE_CONTROL_CONNECTION_STATE_NW_SYNC_REQUEST or
 *                   NETWORK_INTERFACE_CONTROL_CONNECTION_STATE_DEVICE_PARAM_CHANGE
 */
void sall_mgm_join_request_trigger(void *context,
                                   const network_interface_control_ifc *ni_control,
                                   const network_interface_control_connection_state connection_state);

/* @brief : Callback function for time event for Join
 */
sid_error_t sid_sall_mgm_join_request_event_handle(struct sid_event *event, void *data);

/* @brief : Function called upon receiving the join request from a device. To be executed by the
 * GW in case of FSK PAN
 */
sid_error_t sall_mgm_ifc_on_join_request(const halo_hall_appv1_mgm_ifc * ifc,
    halo_hall_message_descriptor_t * mdesc, const halo_hall_appv1_mgm_join_proc_key_t* param);

/* @brief : Function called upon receiving the join response from the GW or the Cloud
 */
sid_error_t sall_mgm_ifc_on_join_response(const halo_hall_appv1_mgm_ifc * ifc,
    halo_hall_message_descriptor_t * mdesc, const halo_hall_appv1_mgm_join_proc_key_t* param);

/* @brief : Function called upon receiving the join notify from the GW or the Cloud
 */
sid_error_t sall_mgm_ifc_on_join_notify(const halo_hall_appv1_mgm_ifc *ifc,
    halo_hall_message_descriptor_t *mdesc);
#endif //SALL_MGM_JOIN_INTERFACE_H
