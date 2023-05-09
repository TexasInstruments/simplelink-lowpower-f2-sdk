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
 * @file osal_mutex.h
 * @addtogroup ACE_OSAL_MUTEX
 * @{
 */
#ifndef OSAL_MUTEX_H
#define OSAL_MUTEX_H

#include <stdint.h>

#include <ace/ace_status.h>
#include <ace/os_specific.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Statically initializes a mutex.
 * @details Can be used to statically initialize a mutex when it is infeasible
 *          to do so at runtime, such as when the caller has no explicit
 *          initialization routine. An example is shown below:
 *          @code{.c}
 *          static aceMutex_t lock = ACE_MUTEX_INITIALIZER;
 *
 *          if (aceMutex_acquire(&lock) == ACE_STATUS_OK) {
 *              // Do something while holding the lock, then release.
 *              (void)aceMutex_release(&lock);
 *          }
 *          @endcode
 * @warning This should not typically be used in conjunction with @ref
 *          aceMutex_new or @ref aceMutex_delete.
 * @note    Users should treat this as opaque, and should not infer anything
 *          from the definition, nor manipulate it's internals directly.
 */
#define ACE_MUTEX_INITIALIZER OS_MUTEX_INITIALIZER

/**
 * @name Mutex creation flags
 * @{
 */
/**
 * @brief   Enables support for multiple levels of ownership over the mutex
 *          within the same thread. For use with @ref aceMutex_new.
 */
#define ACE_MUTEX_RECURSIVE (0x00000001U)

/**
 * @brief   Enables debugging for the specified mutex. For use with @ref
 *          aceMutex_new.
 *
 * @private
 */
#define ACE_MUTEX_DEBUG (0x80000000U)
/** @} */

/**
 * @brief   Defines the mutex type, whose exact implementation typically varies
 *          depending on the underlying operating system.
 * @note    Users should treat this as opaque, and should not infer anything
 *          from the definition, nor manipulate it's internals directly.
 */
typedef struct aceMutex_struct aceMutex_t;

/**
 * @brief   Create a mutex.
 * @details Creates a mutex at the address pointed to by `mutex` with the
 *          specified `flags`. Upon success, this function leaves the mutex in
 *          the unlocked state.
 * @warning This should not typically be used in conjunction with @ref
 *          ACE_MUTEX_INITIALIZER.
 *
 * @param[in]  flags Optional flags for the mutex.
 * @param[out] mutex Pointer to the mutex to be created.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 * @retval ACE_STATUS_ALREADY_INITIALIZED If `mutex` is already initialized.
 */
ace_status_t aceMutex_new(uint32_t flags, aceMutex_t* mutex);

/**
 * @brief   Deletes a mutex.
 * @details Deletes a mutex at the address pointed to by `mutex`.
 * @warning Deleting a mutex that is currently locked results in undefined
            behavior.
 * @warning Deleting a mutex that has already been deleted results in undefined
 *          behavior.
 * @warning This should not typically be used in conjunction with @ref
 *          ACE_MUTEX_INITIALIZER.
 *
 * @param[in] mutex Pointer to the mutex to be deleted.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceMutex_delete(aceMutex_t* mutex);

/**
 * @brief   Acquires a mutex.
 * @details The mutex referenced by `mutex` shall be locked by calling this
 *          function. If the mutex is already locked, the calling thread will be
 *          blocked until the mutex becomes available.
 * @warning Acquiring a mutex that was not created with either @ref aceMutex_new
 *          or @ref ACE_MUTEX_INITIALIZER results in undefined behavior.
 * @warning Recursively locking a mutex that was not created with the @ref
 *          ACE_MUTEX_RECURSIVE flag results in undefined behavior.
 *
 * @param[in] mutex Pointer to the mutex to be locked.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceMutex_acquire(aceMutex_t* mutex);

/**
 * @brief   Acquires a mutex, waiting until a timeout occurs.
 * @details The mutex referenced by `mutex` shall be locked by calling this
 *          function. If the mutex is already locked, the calling thread will be
 *          blocked until the mutex becomes available, or until a timeout
 *          occurs, as specified by `timeout_ms`.
 * @warning Acquiring a mutex that was not created with either @ref aceMutex_new
 *          or @ref ACE_MUTEX_INITIALIZER results in undefined behavior.
 * @warning Recursively locking a mutex that was not created with the @ref
 *          ACE_MUTEX_RECURSIVE flag results in undefined behavior.
 *
 * @param[in] mutex      Pointer to the mutex to be locked.
 * @param[in] timeout_ms The amount of time to wait for, in milliseconds.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 * @retval ACE_STATUS_TIMEOUT A timeout occurred while locking the mutex.
 */
ace_status_t aceMutex_acquireTimeout(aceMutex_t* mutex, uint32_t timeout_ms);

/**
 * @brief   Attempts to acquire a mutex.
 * @details The mutex referenced by `mutex` shall be locked by calling this
 *          function, unless the mutex is already locked, in which case
 *          an error is returned immediately (without blocking).
 * @warning Acquiring a mutex that was not created with either @ref aceMutex_new
 *          or @ref ACE_MUTEX_INITIALIZER results in undefined behavior.
 * @warning Recursively locking a mutex that was not created with the @ref
 *          ACE_MUTEX_RECURSIVE flag results in undefined behavior.
 *
 * @param[in] mutex Pointer to the mutex to be locked.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceMutex_acquireTry(aceMutex_t* mutex);

/**
 * @brief   Release a mutex.
 * @details The mutex referenced by `mutex` shall be unlocked by calling this
 *          function; if the mutex was created using the @ref
 *          ACE_MUTEX_RECURSIVE flag, then the lock count is decremented. The
 *          mutex is only considered unlocked when the lock count reaches
 *          zero.
 * @warning Releasing a mutex that the caller does not own results in undefined
 *          behavior.
 * @warning Releasing a mutex that was not created with either @ref aceMutex_new
 *          or @ref ACE_MUTEX_INITIALIZER results in undefined behavior.
 * @warning Attempting to recursively unlock a mutex that was not created with
 *          the @ref ACE_MUTEX_RECURSIVE flag results in undefined behavior.
 *
 * @param[in] mutex Pointer to the mutex to be unlocked.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceMutex_release(aceMutex_t* mutex);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* OSAL_MUTEX_H */
