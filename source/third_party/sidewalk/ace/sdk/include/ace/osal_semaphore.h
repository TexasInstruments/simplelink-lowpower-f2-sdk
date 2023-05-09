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
 * @file osal_semaphore.h
 * @addtogroup ACE_OSAL_SEMAPHORE
 * @{
 */
#ifndef OSAL_SEMAPHORE_H
#define OSAL_SEMAPHORE_H

#include <stdint.h>

#include <ace/ace_status.h>
#include <ace/os_specific.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   The upper limit of the semaphore max value.
 *          For use with @ref aceSem_newEx.
 */
#define ACE_OSAL_SEMAPHORE_MAX_COUNT (0x3fffffffU)

/**
 * @brief   Defines the semaphore type, whose exact implementation typically
 *          varies depending on the underlying operating system.
 * @note    Users should treat this as opaque, and should not infer anything
 *          from the definition, nor manipulate it's internals directly.
 */
typedef struct aceSemaphore_struct aceSemaphore_t;

/**
 * @brief   Create a semaphore.
 * @details Creates a semaphore at the address pointed to by `semaphore`. The
 *          `initial_value` argument specifies the initial value for the
 *          semaphore.
 * @warning Initializing a semaphore that has already been initialized results
 *          in undefined behavior.
 *
 * @param[in]  initial_value Initial value for the semaphore.
 * @param[out] semaphore     Pointer to the semaphore to be created.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceSem_new(uint32_t initial_value, aceSemaphore_t* semaphore);

/**
 * @brief   Create a semaphore with specified max value.
 * @details Creates a semaphore at the address pointed to by `semaphore`.
 *          The `max_value` argument specifies the maximum value that
 *          the semaphore can reach. When a semaphore reaches the maximum
 *          value it can no longer be post.
 *          The `initial_value` argument specifies the initial value for the
 *          semaphore.
 * @warning Initializing a semaphore that has already been initialized
 *          results in undefined behavior.
 *
 * @param[in]  max_value     Max value that the semaphore can reach. The upper
 *                           limit is ACE_OSAL_SEMAPHORE_MAX_COUNT.
 * @param[in]  initial_value Initial value for the semaphore.
 * @param[out] semaphore     Pointer to the semaphore to be created.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 *
 * @note Certain platforms may support a `max_value` larger than
 *       ACE_OSAL_SEMAPHORE_MAX_COUNT.
 */
ace_status_t aceSem_newEx(uint32_t max_value, uint32_t initial_value,
                          aceSemaphore_t* semaphore);

/**
 * @brief   Deletes a semaphore.
 * @details Deletes a semaphore at the address pointed to by `semaphore`.
 * @warning Deleting an uninitialized semaphore results in undefined behavior.
 * @warning Deleting a semaphore that other threads are currently blocked on
 *          results in undefined behavior.
 * @warning Deleting a semaphore that has already been deleted results in
 *          undefined behavior.
 *
 * @param[in] semaphore Pointer to the semaphore to be deleted.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceSem_delete(aceSemaphore_t* semaphore);

/**
 * @brief   Decrements a semaphore.
 * @details Decrements the semaphore at the address pointed to by `semaphore`.
 *          If this would result in the semaphore having a value of zero, the
 *          calling thread will block until the semaphore is incremented.
 *          Otherwise, the function will return immediately.
 * @warning Waiting an uninitialized semaphore results in undefined behavior.
 *
 * @param[in] semaphore Pointer to the semaphore to wait on.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceSem_wait(aceSemaphore_t* semaphore);

/**
 * @brief   Decrements a semaphore, waiting until a timeout occurs.
 * @details Decrements the semaphore at the address pointed to by `semaphore`.
 *          If this would result in the semaphore having a value of zero, the
 *          calling thread will block until the semaphore is incremented or
 *          `timeout_ms` elapses. Otherwise, the function will return
 *          immediately.
 * @warning Waiting an uninitialized semaphore results in undefined behavior.
 *
 * @param[in] semaphore       Pointer to the semaphore to wait on.
 * @param[in] timeout_ms      The amount of time to wait for, in milliseconds.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 * @retval ACE_STATUS_TIMEOUT A timeout occurred while locking the semaphore.
 */
ace_status_t aceSem_waitTimeout(aceSemaphore_t* semaphore, uint32_t timeout_ms);

/**
 * @brief   Attempts to decrement a semaphore.
 * @details Decrements the semaphore at the address pointed to by `semaphore`.
 *          If this would result in the semaphore having a value of zero,
 *          @ref ACE_STATUS_TRY_AGAIN is returned immediately (without
 *          blocking).
 * @warning Waiting an uninitialized semaphore results in undefined behavior.
 *
 * @param[in] semaphore         Pointer to the semaphore to wait on.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 * @retval ACE_STATUS_TRY_AGAIN Decrement cannot be immediately performed.
 */
ace_status_t aceSem_waitTry(aceSemaphore_t* semaphore);

/**
 * @brief   Get the count of a semaphore.
 * @details Places the current value of the semaphore pointed to by `semaphore`
 *          into the buffer pointed to by `value`.
 * @warning Getting the count of an uninitialized semaphore results in undefined
 *          behavior.
 *
 * @param[in]  semaphore Pointer to the semaphore to get the count from.
 * @param[out] value     Pointer to the buffer to store the count.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceSem_getCount(aceSemaphore_t* semaphore, uint32_t* value);

/**
 * @brief   Increments a semaphore.
 * @details Increments the semaphore at the address pointed to by `semaphore`.
 *          If this would result in the semaphore having a value greater than
 *          zero, then another thread that is waiting on the semaphore will be
 *          woken up, and will proceed to increment the semaphore.
 * @warning Posting to an uninitialized semaphore results in undefined behavior.
 *
 * @param[in] semaphore Pointer to the semaphore to increment.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceSem_post(aceSemaphore_t* semaphore);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* OSAL_SEMAPHORE_H */
