/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_PAL_TEMPERATURE_IFC_H
#define SID_PAL_TEMPERATURE_IFC_H

/** @file
 *
 * @defgroup sid_pal_lib_temperature sid temperature interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details     Provides temperature interface to be implemented by platform
 */
#include <sid_error.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Init temperature detection
 *
 * @retval true if support, else false
 */
sid_error_t sid_pal_temperature_init(void);

/**
 * Get temperature
 *
 * @retval temperature in celsius degree
 */
int16_t sid_pal_temperature_get(void);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
