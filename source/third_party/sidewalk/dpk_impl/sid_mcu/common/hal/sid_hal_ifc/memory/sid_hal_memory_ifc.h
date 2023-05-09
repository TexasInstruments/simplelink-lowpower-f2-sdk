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

#ifndef SID_HAL_MEMORY_IFC_H
#define SID_HAL_MEMORY_IFC_H

/** @file
 *
 * @defgroup sid_hal_lib_memory sid Memory interface
 * @{
 * @ingroup sid_hal_ifc
 *
 * @details     Provides timer interface to be implemented by platform
 */
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate a block of memory
 *
 * @param[in]   size_t          Size in bytes of memory to allocate
 *
 * @retval Pointer to block of memory allocated
 */
void * sid_hal_malloc(size_t size);

/**
 * Free a block memory
 *
 * @param[in]   ptr             Pointer to block of memory to be freed
 *
 * @retval none
 */
void sid_hal_free(void * ptr);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
