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

#ifndef SID_PAL_UPTIME_IFC_H
#define SID_PAL_UPTIME_IFC_H

/** @file
 *
 * @defgroup sid_pal_uptime_ifc sid clock interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details     Provides clock interface to be implemented by platform
 */

#include <sid_error.h>
#include <sid_time_types.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the current time of specified clock source
 *
 * @param[out]  time            current time
 *
 * @retval SID_ERROR_NONE in case of success
 *
 * NOTE: drift may be NULL. In this case time should be set and drift ignored. Success should be returned.
 *
 */
sid_error_t sid_pal_uptime_now(struct sid_timespec *time);

/**
 * Set crystal offset for RTC compensation
 *
 * @param[in]   ppm          offset in PPM
 *
 */
void sid_pal_uptime_set_xtal_ppm(int16_t ppm);

/**
 * Get current crystal offset
 *
 * @retval offset in PPM
 */
int16_t sid_pal_uptime_get_xtal_ppm(void);

#ifdef __cplusplus
}
#endif

#endif
