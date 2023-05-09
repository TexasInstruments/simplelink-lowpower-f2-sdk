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

#ifndef _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_DEFICIT_MANAGER_H_
#define _H_SRC_MAC_HALO_INCLUDE_RNET_MAC_DEFICIT_MANAGER_H_

#include "rnet_mac_submodule_common.h"


/*!
 * @brief Enable/Disable Deficit logic.
 * @param enable if True then enable deficit logic and reset token values. if False then disable
 * deficit logic
 * @return SUCCESS or FAIL
 */
rnet_mac_cmd_status_t rnet_mac_set_deficit_logic( bool enable );

/*!
 * @brief Get status of deficit logic
 * @param void
 * @return true: deficit logic enabled, false: deficit logic disabled
 */
bool rnet_mac_get_deficit_logic( void );

/*!
 * @brief Get the quantum value which is currently set in the requested submodule
 * @param submodule Submodule for which the quantum is requested
 * @return quantum submodule quantum. SUBMODULE_QUANTUM_INVALID if invalid or No Submodule
 */
int32_t rnet_mac_get_deficit_quantum(rnet_mac_submodule_t submodule);

/*!
 * @brief Set/Update quantum values to be added to submodule. API for Upper Layer
 * @param submodule submodule whose quantum must be updated
 * @param quantum new quantum value to be set
 * @return FAIL if invalid, No Submodule or invalid quantum value; else SUCCESS
 */
rnet_mac_cmd_status_t rnet_mac_set_deficit_quantum(rnet_mac_submodule_t submodule,
        int32_t quantum);

/*!
 * @brief Get the load factor value corresponding to a submodule
 * @param submodule Submodule for which the quantum is requested
 * @return load factor submodule load factor. SUBMODULE_LOAD_FACTOR_INVALID if invalid or No Submodule
 */
float rnet_mac_get_deficit_load_factor(rnet_mac_submodule_t submodule);

/*!
 * @brief Set/Update load factor values for submodule. API for Upper Layer
 * @param submodule submodule whose quantum must be updated
 * @param load_factor new quantum value to be set
 * @return FAIL if invalid, No Submodule or invalid quantum value; else SUCCESS
 */
rnet_mac_cmd_status_t rnet_mac_set_deficit_load_factor(rnet_mac_submodule_t submodule,
        float load_factor);

/*!
 * @brief Reset token values corresponding to all active submodules to SUBMODULE_TOKEN_LIMIT
 * @return Success or Fail
 */
rnet_mac_cmd_status_t rnet_mac_deficit_reset_token(void);

/*!
 * @brief Get token value for corresponding submodule
 * @param submodule Submodule for which the token is requested
 * @return token Submodule token. SUBMODULE_TOKEN_INVALID if invalid or No Submodule
 */
int32_t rnet_mac_get_deficit_token(rnet_mac_submodule_t submodule);

/*!
 * @brief Get token value for corresponding submodule at a future time
 * @param submodule Submodule for which the token is requested
 * @param time_in_ms time in msec
 * @return token Submodule token. SUBMODULE_TOKEN_INVALID if invalid or No Submodule
 */
int32_t rnet_mac_deficit_token_at_future_time(rnet_mac_submodule_t submodule, uint32_t time_in_ms);

/*!
 * @brief Compensate token for each submodules with IDLE time between end of one submodule process and
 *  the start of next submodule process
 * @param time_in_ms time in msec during which submodules were inactive
 * @return FAIL if invalid or No Submodule; else SUCCESS
 */
rnet_mac_cmd_status_t rnet_mac_deficit_idle_time_compensate(uint32_t time_in_ms);

/*!
 * @brief Subtract processing time from the active submodule and compensate this time for other submodules
 * @param time_in_ms time in msec during which submodules were inactive
 * @param active_submodule the submodule which is currently active
 * @return FAIL if invalid or No Submodule; else SUCCESS
 */
rnet_mac_cmd_status_t rnet_mac_deficit_process_time_compensate(int32_t time_in_ms,
        rnet_mac_submodule_t active_submodule);

/*!
 * @brief Add Quantum Value to the Submodule Token
 * @param submodule submodule to which Quantum must be added
 * @return FAIL if invalid or No Submodule; else SUCCESS
 */
rnet_mac_cmd_status_t rnet_mac_deficit_add_quantum(rnet_mac_submodule_t submodule);

#endif /* LIB_AMAZON_RINGNET_SRC_MAC_HALO_INCLUDE_RNET_MAC_DEFICIT_MANAGER_H_ */
