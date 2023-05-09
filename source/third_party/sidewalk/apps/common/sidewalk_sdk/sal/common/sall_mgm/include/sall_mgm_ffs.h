/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SALL_MGM_FFS_INTERFACE_H
#define SALL_MGM_FFS_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Function called upon connection state change
 *
 * @param[in]  context           pointer to the sall_mgm_lib_impl_t
 * @param[in]  ni_control        pointer to network_interface
 * @param[in]  connection_state  Connection state change, registration will handle
 *                               NETWORK_INTERFACE_CONTROL_CONNECTION_STATE_SYNCED
 *                               and NETWORK_INTERFACE_CONTROL_CONNECTION_STATE_PROV_SYNCED_FAILED
 *                               to start or to retry registration process respectively
 */
void sall_mgm_ffs_ni_connection_state_change(void* context,
                                             const network_interface_control_ifc * ni_control,
                                             const network_interface_control_connection_state connection_state);

/**
 * Initialize registration state
 *
 * @param[in]  impl              pointer to the sall_mgm_lib_impl_t
 *
 * @returns #SID_ERROR_NONE on success
 */
sid_error_t sall_mgm_ffs_init(sall_mgm_lib_impl_t *impl);


/**
 * Deinitialize registration state
 *
 * @param[in]  impl              pointer to the sall_mgm_lib_impl_t
 *
 */
void sall_mgm_ffs_deinit(sall_mgm_lib_impl_t *impl);

/**
 * Starts registration process
 *
 * @param[in]  ifc               pointer to sall mgm interface
 *
 * @returns #SID_ERROR_NONE on success
 */

sid_error_t sall_mgm_ffs_start(const sall_mgm_ifc * ifc);

/**
 * Stops registration process
 *
 * @param[in]  ifc               pointer to sall mgm interface
 *
 */
void sall_mgm_ffs_stop(const sall_mgm_ifc *ifc);

/**
 * Function called upon SET_CONFIGURATION_COMPLETE. Handles configuration
 * complete setting, on success it will stop registration process,
 * on failure it will evaluate whether to retry registration.
 *
 * @param[in]  ifc               pointer to sall mgm interface
 * @param[in]  complete          indicates sucess/failure of
 *                               received SET_CONFIGURATION_COMPLETE command
 */
void sall_mgm_ffs_handle_config_complete(const sall_mgm_ifc *ifc, bool complete);

/**
 * Handles error during registration process, it evaluates if
 * retry is allowed or stops registration due failure
 *
 * @param[in]  ifc               pointer to sall mgm interface
 */
void sall_mgm_ffs_handle_error(const sall_mgm_ifc *ifc);

/**
 * Starts network key refresh
 *
 * @param[in]  ifc               pointer to sall mgm interface
 */
void sall_mgm_ffs_key_refresh_start(const sall_mgm_ifc *ifc);

/**
 * Stops network key refresh
 *
 * @param[in]  ifc               pointer to sall mgm interface
 */
void sall_mgm_ffs_key_refresh_stop(const sall_mgm_ifc *ifc);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //SALL_MGM_FFS_INTERFACE_H