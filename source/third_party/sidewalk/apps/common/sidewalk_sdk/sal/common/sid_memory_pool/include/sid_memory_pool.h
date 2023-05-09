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

#ifndef SID_MEMORY_POOL_H
#define SID_MEMORY_POOL_H

#include <sid_error.h>

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque memory pool handle returns by sid_memory_pool_init().
 */
struct sid_memory_pool;

/**
 * Configuration parameter used to create memory pool
 */
struct sid_memory_pool_config {
    uint8_t *buffer; /**< Address must be aligned to pointer size! */
    size_t size;     /**< Buffer size */
};

/**
 * Initializes sid_memory_pool
 *
 * @note If the user does not provide buffer in sid_memory_pool_config, sid_memory_pool_init
 * will call malloc to allocate the memory indicated by size variable.
 *
 * @param[out] memory_pool Pointer where the initialized sid_memory_pool type will stored
 * @param[in] config Required configuration needed to initialize sid_memory_pool type
 *
 * @return #SID_ERROR_NONE on success or error code on failure
 */
sid_error_t sid_memory_pool_init(struct sid_memory_pool **memory_pool,
                                 const struct sid_memory_pool_config *const config);

/**
 * De-Initializes sid_memory_pool
 *
 *  @param[in] memory_pool A pointer to the handle returned by sid_memory_pool_init()
 *
 */
void sid_memory_pool_deinit(struct sid_memory_pool *const memory_pool);

/**
 * Function to allocate buffer of desired size
 *
 * @param[in] size        Size of memory in bytes that needs to be allocated
 *
 * @return pointer to allocated buffer
 * @retval NULL if allocation of buffer with given size is not possible
 *
 * @attention Buffer returned to user should be initialized with 0(ZERO) and aligned by pointer size
 */
void *sid_memory_pool_allocate(struct sid_memory_pool *const memory_pool, size_t size);

/**
 * Function to release previously allocated buffer
 *
 *  @param[in] memory_pool A pointer to the handle returned by sid_memory_pool_init()
 * @param[in] buffer   Pointer to buffer as returned by sid_memory_pool_allocate()
 */
void sid_memory_pool_free(struct sid_memory_pool *const memory_pool, void *const buffer);

/**
 * Function returns the minimum size that user has to provide to sid_memory_pool_allocate()
 *
 *  @return size of the minimum memory that user should allocate
 *
 *  @attention Use only for debug builds
 */
size_t sid_memory_pool_get_min_impl_size(void);

/**
 * Function returns the remaining size (in bytes) of memory that can be allocated
 * using sid_memory_pool_allocate()
 *
 *  @param[in] memory_pool A pointer to the handle returned by sid_memory_pool_init()
 *
 *  @return size of the remaining memory that can be allocated
 *
 *  @attention Use only for debug builds, will return 0 for release builds
 */
size_t sid_memory_pool_get_free_size(struct sid_memory_pool *const memory_pool);

/**
 * Function returns the initial size (in bytes) of memory that could have been alloated by
 * sid_memory_pool_allocate()
 *
 * @param[in] memory_pool A pointer to the handle returned by sid_memory_pool_init()
 *
 * @return size of the initial size of  memory that can be allocated
 *
 * @attention Use only for debug builds, will return 0 for release builds
 */
size_t sid_memory_pool_get_free_bytes_initial_size(struct sid_memory_pool *const memory_pool);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif /* SID_MEMORY_POOL_H */
