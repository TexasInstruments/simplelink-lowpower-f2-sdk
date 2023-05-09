/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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
 * @file osal_mp.h
 * @addtogroup ACE_OSAL_MP
 * @{
 */
#ifndef OSAL_MP_H
#define OSAL_MP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ace/ace_status.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @cond DEPRECATED
 * @deprecated Please use the new symbols.
 * @{
 */
#define ACE_FLAG_MP_OVERFLOW_CHECK ACE_MP_FLAG_OVERFLOW_CHECK
#define ACE_FLAG_MP_ALIGNMENT ACE_MP_FLAG_ALIGNMENT

#define aceMP_handle_t aceMp_handle_t
#define aceMP_allocator aceMp_allocator

#define aceMP_init aceMp_init
#define aceMP_initWithAlignment aceMp_initWithAlignment
#define aceMP_initWithAllocatorAlignment aceMp_initWithAllocatorAlignment
#define aceMP_initWithAllocatorAlignmentFlags \
    aceMp_initWithAllocatorAlignmentFlags
#define aceMP_destroy aceMp_destroy
#define aceMP_destroyForce aceMp_destroyForce
#define aceMP_alloc aceMp_alloc
#define aceMP_allocSize aceMp_allocSize
#define aceMP_free aceMp_free
/**
 * @}
 * @endcond
 */

/**
 * @name Memory pool flags
 * @brief Bitmask flags for use with @ref aceMp_initWithAllocatorAlignmentFlags.
 * @{
 */
/** @brief Perform fixed-size allocations. */
#define ACE_MP_FLAG_FIXED_ALLOC (0)

/**
 * @brief   Perform variable-size allocations.
 * @details Enables support for variable-size allocations via @ref
 *          aceMp_allocSize.
 */
#define ACE_MP_FLAG_VAR_ALLOC (1 << 0)

/**
 * @brief Perform buffer overflow checks.
 * @note  Overflows will be reported via log messages.
 */
#define ACE_MP_FLAG_OVERFLOW_CHECK (1 << 1)

/** @brief Force alignment of individual buffers. */
#define ACE_MP_FLAG_ALIGNMENT (1 << 2)
/** @} */

/**
 * @brief Defines the memory pool handle type. Used to identify a particular
 *        memory pool.
 */
typedef void* aceMp_handle_t;

/**
 * @brief   Defines the memory pool allocator structure.
 * @details The allocator structure is used to control where the memory for a
 *          given memory pool comes from. This is useful when a system has
 *          multiple heaps, for example, and the caller does not want aceMP to
 *          use the default system heap.
 */
struct aceMp_allocator {
    void* (*alloc)(size_t size);
    void (*free)(void* p);
    void* (*memset)(void* s, int c, size_t n);
};

/**
 * @brief   Creates a memory pool.
 * @details Creates a fixed-size memory pool, using the default block alignment,
 *          and using the system heap for the pool's memory.
 *
 * @param[in] size  The size of each block in the pool.
 * @param[in] count The number of blocks in the pool.
 *
 * @return Handle to the memory pool on success, or `NULL` on error.
 */
aceMp_handle_t aceMp_init(size_t size, size_t count);

/**
 * @brief   Creates a memory pool with the specified block alignment.
 * @details Creates a fixed-size memory pool, using the specified block
 *          alignment, and using the system heap for the pool's memory.
 *
 * @param[in] size  The size of each block in the pool.
 * @param[in] count The number of blocks in the pool.
 * @param[in] align The alignment of blocks in the pool.
 *
 * @return Handle to the memory pool on success, or `NULL` on error.
 */
aceMp_handle_t aceMp_initWithAlignment(size_t size, size_t count, size_t align);

/**
 * @brief   Creates a memory pool with the specified block alignment using the
 *          provided allocator.
 * @details Creates a fixed-size memory pool, using the specified block
 *          alignment, and the provided allocator for the the pool's memory.
 *
 * @param[in] allocator Pointer to a memory pool allocator to be used for this
 *                      memory pool.
 * @param[in] size      The size of each block in the pool.
 * @param[in] count     The number of blocks in the pool.
 * @param[in] align     The alignment of blocks in the pool.
 *
 * @return Handle to the memory pool on success, or `NULL` on error.
 */

aceMp_handle_t aceMp_initWithAllocatorAlignment(
    struct aceMp_allocator* allocator, size_t size, size_t count, size_t align);
/**
 * @brief   Creates a memory pool with the specified block alignment, using the
 *          provided allocator, and with the specified flags.
 * @details Creates a fixed-size memory pool, using the specified block
 *          alignment, and the provided allocator for the the pool's memory.
 *
 * @param[in] allocator Pointer to a memory pool allocator to be used for this
 *                      memory pool.
 * @param[in] size      The size of each block in the pool.
 * @param[in] count     The number of blocks in the pool.
 * @param[in] align     The alignment of blocks in the pool. This is only used
 *                      if `ACE_MP_FLAG_ALIGNMENT` is set in `flags`.
 * @param[in] flags     A bitmask of `ACE_MP_FLAG` flags.
 *
 * @return Handle to the memory pool on success, or `NULL` on error.
 */
aceMp_handle_t aceMp_initWithAllocatorAlignmentFlags(
    struct aceMp_allocator* allocator, size_t size, size_t count, size_t align,
    uint32_t flags);

/**
 * @brief   Destroys a memory pool.
 * @details Destroys the memory pool associated with the provided handle.
 * @warning Destroying a memory pool that is currently in use results in
 *          undefined behavior.
 *
 * @param[in] mp Handle to the memory pool to destroy.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceMp_destroy(aceMp_handle_t mp);

/**
 * @deprecated
 * @brief   Destroys a memory pool, even if it in use.
 * @details Destroys the memory pool associated with the provided handle even if
 *          it is still in use.
 * @warning Destroying a memory pool that is currently in use results in
 *          undefined behavior.
 *
 * @param[in] mp    Handle to the memory pool to destroy.
 * @param[in] force Indicates whether or not to forcibly delete the memory pool
 *                  if it is in use.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceMp_destroyForce(aceMp_handle_t mp, bool force);

/**
 * @brief Allocates a block from the specified memory pool.
 *
 * @param[in] mp Handle to the memory pool to allocate a block from.
 *
 * @return Address to the allocated block on success, or `NULL` on error.
 */
void* aceMp_alloc(aceMp_handle_t mp);

/**
 * @brief Allocates a buffer of given size from the specified memory pool.
 * @note  Only usable on pools that were created with the @ref
 *        ACE_MP_FLAG_VAR_ALLOC flag.
 *
 * @param[in] mp   Handle to the memory pool to allocate a block from.
 * @param[in] size Size of the buffer to allocate.
 *
 * @return Address to the allocated block on success, or `NULL` on error.
 */
void* aceMp_allocSize(aceMp_handle_t mp, size_t size);

/**
 * @brief Returns a block of memory back to the memory pool.

 * @param[in] mp Handle to the memory pool to allocate a block from.
 * @param[in] p  Pointer to the block of memory to free.
 */
void aceMp_free(aceMp_handle_t mp, void* p);

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* OSAL_MP_H */
