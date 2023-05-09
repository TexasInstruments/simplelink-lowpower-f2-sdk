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

#ifndef SID_HAL_RESET_IFC_H
#define SID_HAL_RESET_IFC_H

/** @file
 *
 * @defgroup sid_hal_lib_reset sid reset interface
 * @{
 * @ingroup sid_hal_ifc
 *
 * @details Provides a way for protocol to reset the device
 */

#include <sid_error.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SID_HAL_RESET_NORMAL,  //< Perform a reset
    SID_HAL_RESET_DFU,     //< Reset and boot into firmware update mode
} sid_hal_reset_type_t;

/**
 * Reset the device with a specific boot mode.
 *
 * @param[in] type reset type
 *
 * @retval SID_ERROR_NOSUPPORT if reset type not supported otherwise
 *         will not return.
 */
sid_error_t sid_hal_reset(sid_hal_reset_type_t type);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
