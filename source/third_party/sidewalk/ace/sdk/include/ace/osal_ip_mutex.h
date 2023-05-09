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
 * @file osal_ip_mutex.h
 * @addtogroup ACE_OSAL_IP_MUTEX
 * @{
 */
#ifndef OSAL_IP_MUTEX_H
#define OSAL_IP_MUTEX_H

#include <ace/ace_status.h>
#include <ace/os_specific.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Statically initializes an inter-process mutex.
 * @details Can be used to statically initialize a mutex when it is infeasible
 *          to do so at runtime, such as when the caller has no explicit
 *          initialization routine. An example is shown below:
 *          @code{.c}
 *          static aceIpMutex_t ip_lock = ACE_IPMUTEX_INITIALIZER;
 *          @endcode
 * @warning This should not be used in conjunction with @ref aceIpMutex_create.
 * @note    Users should treat this as opaque, and should not infer anything
 *          from the definition, nor manipulate it's internals directly.
 */
#define ACE_IPMUTEX_INITIALIZER OS_IPMUTEX_INITIALIZER

/**
 * @brief   Defines the inter-process mutex type, whose exact implementation
 *          typically varies depending on the underlying operating system.
 * @note    Users should treat this as opaque, and should not infer anything
 *          from the definition, nor manipulate it's internals directly.
 */
typedef struct aceIpMutex_s aceIpMutex_t;

/**
 * @brief   Creates an inter-process mutex.
 * @details Creates an inter-process mutex at the address pointed to by
 *          `ipmutex` with the specified `name`. Upon success, this function
 *          leaves the mutex in an unlocked state.
 *
 * @param[in]  name    Pointer to a C string which contains the name of the
 *                     mutex.
 * @param[out] ipmutex Pointer to the mutex to be created.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceIpMutex_create(const char* name, aceIpMutex_t* ipmutex);

/**
 * @brief   Closes an inter-process mutex.
 * @details Closes an inter-process mutex at the address pointed to by
 *          `ipmutex`.
 * @warning Closing a mutex that is currently locked results in undefined
            behavior.
 * @warning Closing a mutex that has already been closed results in undefined
 *          behavior.
 *
 * @param[in] ipmutex Pointer to the mutex to be closed.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */

ace_status_t aceIpMutex_close(aceIpMutex_t* ipmutex);

/**
 * @brief   Destroys a inter-process mutex.
 * @details Destroys a inter-process mutex at the address pointed to by
 *          `ipmutex`.
 * @warning Destroying a mutex that is currently locked results in undefined
 *          behavior.
 * @warning Destroying a mutex that has already been destroyed results in
 *          undefined behavior.
 *
 * @param[in] mutex Pointer to the mutex to be destroyed.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceIpMutex_destroy(aceIpMutex_t* ipmutex);

/**
 * @brief   Destroys a inter-process mutex with the given name.
 * @details Destroys a inter-process mutex with the name specified by `name`.
 * @warning Destroying a mutex that is currently locked results in undefined
 *          behavior.
 * @warning Destroying a mutex that has already been destroyed results in
 *          undefined behavior.
 *
 * @param[in] name Pointer to the C string which contains the name of the mutex
 *                 to be destroyed.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceIpMutex_destroyByName(const char* name);

/**
 * @brief   Acquires a inter-process mutex.
 * @details The inter-process mutex referenced by `ipmutex` shall be locked by
 *          calling this function. If the mutex is already locked, the calling
 *          thread will be blocked until the mutex becomes available.
 * @warning Attempting to recursively lock a mutex results in undefined
 *          behavior.
 *
 * @param[in] ipmutex Pointer to the mutex to be locked.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceIpMutex_acquire(aceIpMutex_t* ipmutex);

/**
 * @brief   Release an inter-process mutex.
 * @details The inter-process mutex referenced by `ipmutex` shall be unlocked by
 *          calling this function.
 * @warning Attempting to recursively unlock a mutex results in undefined
 *          behavior.
 *
 * @param[in] ipmutex Pointer to the mutex to be unlocked.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceIpMutex_release(aceIpMutex_t* ipmutex);

/**
 * @brief   Checks the validity of an inter-process mutex.
 * @details Checks whether the inter-process mutex pointed to by `ipmutex` has
 *          already been created via @ref aceIpMutex_create.
 *
 * @param ipmutex Pointer to the mutex to check the validity.
 *
 * @retval true The mutex is valid.
 * @retval false The mutex is not valid.
 */
bool aceIpMutex_valid(aceIpMutex_t* ipmutex);

#ifdef __cplusplus
}
#endif

#endif /* OSAL_IP_MUTEX_H */
/** @} */
