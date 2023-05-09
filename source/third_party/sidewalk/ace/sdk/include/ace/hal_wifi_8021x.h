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
 * @file hal_wifi_8021x.h
 *
 * @brief HAL WiFi API implemented per platform to support
 * IEEE802.1X Enterprise security.
 *
 * @addtogroup ACE_HAL_WIFI
 * @{
 */

#ifndef __ACE_HAL_WIFI_1X_H__
#define __ACE_HAL_WIFI_1X_H__

#include <ace/hal_wifi.h>

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief Verifies that any wifi keystore credential has a corresponding
 *        network configuration in the wpa_supplicant. Removes any unused
 *        credentials.
 *
 * @return aceWifiHal_error_t: Return aceWifiHal_ERROR_SUCCESS on backup success,
 *                             aceWifiHal_ERROR_FAILURE on failure.
 */
aceWifiHal_error_t aceWifiHal_synchronizeConfig(void);

#ifdef  __cplusplus
}
#endif

#endif // __ACE_HAL_WIFI_1X_H__
/** @} */
