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
 * @file osal_utils_freertos.h
 */
#ifndef OSAL_UTILS_FREERTOS_H
#define OSAL_UTILS_FREERTOS_H

#include <stdint.h>

#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CEEDLING_TEST
/**
 * @brief      Wrappers for the various platform-defined macros; this extra
 *             level of indirection is needed to enable unit testing.
 *
 * @return     The relevant macro, as defined by the platform.
 * @{
 */
static inline TickType_t aceOsal_getportMAX_DELAY(void) {
    return portMAX_DELAY;
}
static inline TickType_t aceOsal_getportTICK_PERIOD_MS(void) {
    return portTICK_PERIOD_MS;
}
static inline UBaseType_t aceOsal_getportPRIVILEGE_BIT(void) {
    return portPRIVILEGE_BIT;
}
static inline TickType_t aceOsal_getpdMS_TO_TICKS(TickType_t ms) {
    return pdMS_TO_TICKS(ms);
}
/** @} */
#else
TickType_t aceOsal_getportMAX_DELAY(void);
TickType_t aceOsal_getportTICK_PERIOD_MS(void);
UBaseType_t aceOsal_getportPRIVILEGE_BIT(void);
TickType_t aceOsal_getpdMS_TO_TICKS(TickType_t ms);
#endif  // CEEDLING_TEST

/**
 * @brief      Gets the maximum possible "wait" time, in milliseconds, that can
 *             be (converted and then) used as the xTicksToWait argument for
 *             various FreeRTOS APIs.
 *
 * @note       This necessarily varies from product to product, based on
 *             portMAX_DELAY and portTICK_PERIOD_MS, which is why we hide the
 *             details behind a function.
 *
 * @return     The maximum allowable wait time for the platform. This can be
 *             converted to ticks by multiplying with
 *             aceOsal_getportTICK_PERIOD_MS().
 */
uint32_t aceOsal_getMaxWaitMs(void);

#ifdef __cplusplus
}
#endif

#endif  // OSAL_UTILS_FREERTOS_H
