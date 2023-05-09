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

#ifndef SID_CLOCK_IFC_H
#define SID_CLOCK_IFC_H

/** @file
 *
 * @defgroup sid_clock sid clock implementation
 * @details Provides common clock implementation for sidewalk
 */

#include <sid_error.h>
#include <sid_time_types.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SID_CLOCK_SOURCE_UPTIME = 0,
    SID_CLOCK_SOURCE_NETWORK = 1,
    SID_CLOCK_SOURCE_GPS = 2,
    SID_CLOCK_SOURCE_LAST = 3,
} sid_clock_t;

/**
 * Get the current time of specified clock source
 *
 * @param[in]   source          clock source
 * @param[out]  time            current time
 * @param[out]  drift           drift of clock source
 *
 * @retval SID_ERROR_NONE in case of success
 *
 * NOTE: drift may be NULL. In this case time should be set and drift ignored. Success should be returned.
 *
 */
sid_error_t sid_clock_now(sid_clock_t source, struct sid_timespec * time, struct sid_timespec * drift);

/**
 * Set the current time and drift of specified clock source
 *
 * @param[in]  source          clock source
 * @param[in]  time            time to set
 * @param[in]  drift           drift to set
 *
 * @retval SID_ERROR_NONE in case of success
 *
 * NOTE: drift may be NULL. In this case time should be set and drift ignored. Success should be returned.
 */
sid_error_t sid_clock_set(sid_clock_t source, const struct sid_timespec * time, const struct sid_timespec * drift);

/**
 * Set the uncertainty of the clock source. This will be linked to the crystal oscillator error in parts per million.
 *
 * @param[in]  ppm           uncertainty in parts per million (PPM)
 *
 * @retval none
 */
void sid_clock_set_uncertainty(int16_t ppm);

/**
 * Get the uncertainty of the clock source. This will be linked to the crystal oscillator error in parts per million.
 *
 * @retval Current uncertainty in PPM.
 */
int16_t sid_clock_get_uncertainty(void);

#ifdef __cplusplus
}
#endif

#endif
