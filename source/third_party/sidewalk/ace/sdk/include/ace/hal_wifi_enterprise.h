/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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
/**
 * @file hal_wifi_enterprise.h
 *
 * @brief HAL WiFi API implemented per platform to support
 * Enterprise Credential Management (ECM)
 *
 * @addtogroup ACE_HAL_WIFI
 * @{
 */

#ifndef __ACE_HAL_WIFI_ECM_H__
#define __ACE_HAL_WIFI_ECM_H__

#include <ace/hal_wifi.h>

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief Update existing wifi network profile.
 * Existing profile can be restored by calling 'aceWifiHal_restoreConfig'
 *
 * @param[in] config: Wifi configuration to be added. Must be allocated and populated by the caller.
 * @return aceWifiHal_error_t: aceWifiHal_ERROR_SUCCESS on success, aceWifiHal_ERROR_FAILURE on failure.
 */
aceWifiHal_error_t aceWifiHal_updateNetwork(const aceWifiHal_config_t* config);

/**
 * @brief Restore original Wifi configuration that was previously updated.
 * After calling 'aceWifiHal_updateNetwork', the caller can call this API to restore the Wifi
 * configuration back to the original configuration prior to calling 'aceWifiHal_restoreConfig'
 *
 * @return aceWifiHal_error_t: Return aceWifiHal_ERROR_SUCCESS on backup success,
 *                             aceWifiHal_ERROR_FAILURE on failure.
 */
aceWifiHal_error_t aceWifiHal_restoreConfig(void);

#ifdef  __cplusplus
}
#endif

#endif // __ACE_HAL_WIFI_ECM_H__
/** @} */
