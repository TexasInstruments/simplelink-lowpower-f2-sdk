/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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
 * @file osal_deps_freertos.h
 * @brief A list of statically-verifiable dependencies that the OSAL has on
 * FreeRTOS configurations.
 *
 * @note This file must, at least once, be included in a file that's actually
 *       being built.
 */
#ifndef OSAL_DEPS_FREERTOS_H
#define OSAL_DEPS_FREERTOS_H

#include "FreeRTOS.h"

#include "ace/ace_config.h"
#include "static_assert.h"

ACE_STATIC_ASSERT((INCLUDE_vTaskSuspend) == 1,
                  "INCLUDE_vTaskSuspend must be set to enable support for "
                  "indefinite waits in APIs such as xSemaphoreTake.",
                  osal_deps_freertos_h);

ACE_STATIC_ASSERT((configSUPPORT_STATIC_ALLOCATION) == 1,
                  "configSUPPORT_STATIC_ALLOCATION must be set to enable the "
                  "static variants of kernel APIs.",
                  osal_deps_freertos_h);

#ifdef SIDEWALK_WORKAROUND_ACS
// See HALO-7613, we think that the Nordic SDK needs to set a configTICK_RATE_HZ of 1024
// and that portTICK_PERIOD_MS shouldn't be used on the Nordic platform
ACE_STATIC_ASSERT((portTICK_PERIOD_MS) >= 1 && (configTICK_RATE_HZ) <= 1000,
                  "portTICK_PERIOD_MS must be a positive non-zero integer "
                  "value for our (integer) math to give correct results.",
                  osal_deps_freertos_h);
#endif // SIDEWALK_WORKAROUND_ACS

#ifdef ACE_OSAL_DETACHED_THREAD_SUPPORT
ACE_STATIC_ASSERT((configNUM_THREAD_LOCAL_STORAGE_POINTERS) >= 1,
                  "configNUM_THREAD_LOCAL_STORAGE_POINTERS must be set to "
                  "enable support for OSAL threads.",
                  osal_deps_freertos_h);
#endif

#endif /* OSAL_DEPS_FREERTOS_H */
