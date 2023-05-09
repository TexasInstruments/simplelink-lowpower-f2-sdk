/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_SDK_GW_CONFIG_H
#define SID_SDK_GW_CONFIG_H

#if defined(SID_SDK_USE_GW_APP_CONFIG) && SID_SDK_USE_GW_APP_CONFIG
#include <sid_sdk_app_gw_config.h>
#endif //defined(SID_SDK_USE_GW_APP_CONFIG) && SID_SDK_USE_GW_APP_CONFIG

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contains external exposed configurations
 */

// Internal Application Memory pool size
#ifndef SID_SDK_CONFIG_APPICATION_MEMORY_POOL_SIZE
#define SID_SDK_CONFIG_APPICATION_MEMORY_POOL_SIZE 1536
#endif

// Enable Link type 2 by default (FSK)
#ifndef SID_SDK_CONFIG_ENABLE_LINK_TYPE_2
#define SID_SDK_CONFIG_ENABLE_LINK_TYPE_2 1
#endif

// Enable Link type 3 by default (LORA)
#ifndef SID_SDK_CONFIG_ENABLE_LINK_TYPE_3
#define SID_SDK_CONFIG_ENABLE_LINK_TYPE_3 1
#endif

/*
 * @brief: Configuration flag to control the periodic time synchronization
 * interval with cloud. Optimal value should be choosen
 * based on clock drift on the product/platform.
 */
#if defined(SID_SDK_CONFIG_PERIODIC_TIMESYNC_INTERVAL_SEC) && (SID_SDK_CONFIG_PERIODIC_TIMESYNC_INTERVAL_SEC < 7200)
#undef SID_SDK_CONFIG_PERIODIC_TIMESYNC_INTERVAL_SEC
#endif

#ifndef SID_SDK_CONFIG_PERIODIC_TIMESYNC_INTERVAL_SEC
#define SID_SDK_CONFIG_PERIODIC_TIMESYNC_INTERVAL_SEC 7200
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_SDK_GW_CONFIG_H */
