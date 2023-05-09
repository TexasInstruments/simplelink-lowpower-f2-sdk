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
 * @file osal_shmem.h
 * @brief ACE OSAL shared memory
 * @addtogroup ACE_OSAL_SHMEM
 * @{
 */

#ifndef OSAL_SHMEM_H
#define OSAL_SHMEM_H

#include <ace/ace_status.h>
#include <ace/osal_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @cond DEPRECATED
 * @deprecated Please use the new symbols.
 * @{
 */
#define aceShmem_getsize aceShmem_getSize
/**
 * @}
 * @endcond
 */

/**
 * @brief Initialize ACE shared memory resources.
 *
 * @returns ACE_STATUS_OK on success or an error status from ace_status_t
 *
 * @remark Not thread-safe. May be called only once per process
 */
ace_status_t aceShmem_init(void);

/**
 * @brief Create a shared memory region with the given name and size.
 *
 * @param [in] name Pointer to name of the shared memory region
 * @param [in] size Size of the share memory region. Must be >0.
 * @param [in] mode Access modes of shared memory region.
 *             It is only used in Linux-like system.
 *  - 0700 user (shmem owner) has read, write, and execute permission
 *  - 0400 user has read permission
 *  - 0200 user has write permission
 *  - 0100 user has execute permission
 *  - 0070 group has read, write, and execute permission
 *  - 0040 group has read permission
 *  - 0020 group has write permission
 *  - 0010 group has execute permission
 *  - 0007 others have read, write, and execute permission
 *  - 0004 others have read permission
 *  - 0002 others have write permission
 *  - 0001 others have execute permission
 *
 * @returns ACE_STATUS_OK on success or an error status from ace_status_t
 * @remark thread-safe
 */
ace_status_t aceShmem_create(const char* name, uint32_t size, uint32_t mode);

/**
 * @brief Destroy the specified shared memory region.
 *
 * @param [in] name Pointer to name of the shared memory region
 *
 * @returns ACE_STATUS_OK on success or an error status from ace_status_t
 * @remark thread-safe
 */
ace_status_t aceShmem_destroy(const char* name);

/**
 * @brief Map the specified shared memory region to current process' VMA.
 *
 * @param [in] name Pointer to name of the shared memory region
 * @param [out] pointer Pointer to the virtual address of shared
 *              memory if ACE_STATUS_OK.
 *
 * @returns ACE_STATUS_OK on success or an error status from ace_status_t
 * @note Same virtual address will be returned if the specified shared memory
 *       region has already been opened in current process and not closed yet.
 *       It means we will not do memory mapping again if there is already a
 *       virtual area mapped to the shared memory region.
 *       A reference count is maintained for each shared memory region, and it
 *       is increased by 1 each time it is opened.
 * @remark thread-safe
 */
ace_status_t aceShmem_open(const char* name, void** pointer);

/**
 * @brief Unmap the specified shared memory region.
 *
 * @param [in] name Pointer to name of the shared memory region
 *
 * @returns ACE_STATUS_OK on success or an error status from ace_status_t
 * @note The reference count of the specified shared memory region is decreased
 *       by 1 each time it's closed.
 *       And we will unmap the virtual area for specified shared memory region
 *       if the reference count is 0. So it means probably you get a different
 *       virtual address if you call aceShmem_open() to open the same shared
 *       memory again.
 * @remark thread-safe
 */
ace_status_t aceShmem_close(const char* name);

/**
 * @brief Get the size of specified shared memory region.
 *
 * @param [in] name Pointer to name of the shared memory region
 * @param [out] size Pointer to the size value if ACE_STATUS_OK.
 *
 * @returns ACE_STATUS_OK on success or an error status from ace_status_t
 * @remark thread-safe
 */
ace_status_t aceShmem_getSize(const char* name, uint32_t* size);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* OSAL_SHMEM_H */
