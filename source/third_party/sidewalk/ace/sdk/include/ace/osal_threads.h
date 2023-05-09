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
 * @file osal_threads.h
 * @addtogroup ACE_OSAL_THREAD
 * @{
 */

#ifndef OSAL_THREADS_H
#define OSAL_THREADS_H

#include "ace/ace_config.h"
#include "ace/os_specific.h"
#include "ace/osal_common.h"
#include "ace/ace_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Defines the thread type, whose exact implementation typically varies
 *          depending on the underlying operating system.
 * @note    Users should treat this as opaque, and should not infer anything
 *          from the definition, nor manipulate it's internals directly.
 */
typedef struct aceThread_struct aceThread_t;

/** @brief Defines the aceThread entry function type. */
typedef void (*aceThread_func_t)(void* argument);

/** @brief Defines the aceThread priority type. See the ACE_PRIORITY macros. */
typedef uint32_t aceThread_priority_t;

/**
 * @brief   Defines the thread priority configuration type.
 * @details Used to configure thread priorities with @ref aceThread_init.
 */
typedef struct _aceThread_prio_config {
    aceThread_priority_t idle;
    aceThread_priority_t low;
    aceThread_priority_t below_normal;
    aceThread_priority_t normal;
    aceThread_priority_t above_normal;
    aceThread_priority_t high;
    aceThread_priority_t realtime;
} aceThread_prio_config;

extern aceThread_prio_config prio_config;

/**
 * @name Thread priorities (lowest to highest).
 * @{
 */
/** @brief Idle priority. */
#define ACE_PRIORITY_IDLE (prio_config.idle)

/** @brief Low priority. */
#define ACE_PRIORITY_LOW (prio_config.low)

/** @brief Below normal priority. */
#define ACE_PRIORITY_BELOW_NORMAL (prio_config.below_normal)

/** @brief Normal priority (default). */
#define ACE_PRIORITY_NORMAL (prio_config.normal)

/** @brief Above normal priority. */
#define ACE_PRIORITY_ABOVE_NORMAL (prio_config.above_normal)

/** @brief High priority. */
#define ACE_PRIORITY_HIGH (prio_config.high)

/** @brief Real-time priority. */
#define ACE_PRIORITY_REALTIME (prio_config.realtime)
/** @} */

/**
 * @brief The default stack size provided for threads, unless overridden via
 *        configuration, or explicitly specified with an appropriate API.
 */
#define ACE_OSAL_THREADS_STACK_DEFAULT 512

/**
 * @brief   Initializes the OSAL thread module.
 * @details Initializes the OSAL thread module using the thread priority
 *          configuration pointed to by `config`. If `config` is `NULL`, then
 *          the default priorities for the platform will be used. All
 *          subsequently created threads will uses the newly determined
 *          priorities.
 *
 *          This can be used by an application developer to fine-tune the
 *          priorities of all ACE thread's running on the system.
 * @note    This will automatically be invoked by both @ref ace_init and @ref
 *          ace_initWithConfig.
 * @note    This will not reconfigure any previously created threads.
 *
 * @param[in] config Pointer to the thread priority configuration to use for all
 *                   newly created threads.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceThread_init(aceThread_prio_config* config);

/**
 * @brief   Create a new joinable thread.
 * @details Creates a thread at the address pointed to by `thread` with the
 *          specified attributes (stack size, priority, etc). The thread will be
 *          ready to run immediately after the function returns.
 *
 *          The caller can wait (block) for this thread to terminate by
 *          passing `thread` to @ref aceThread_join.
 *
 * @param[in] func       The execution entry point for the newly created
 *                       thread.
 * @param[in] arg        The sole argument passed to `func`.
 * @param[in] name       Pointer to the C string which contains the name of the
 *                       thread. May be `NULL`.
 * @param[in] stack_mem  Pointer to the memory buffer which will be used as the
 *                       stack for `thread`. The caller is responsible for
 *                       freeing this memory after the new thread terminates.
 *                       The pointer may be `NULL`, in which case the system
 *                       will allocate the memory instead.
 * @param[in] stack_size Size of the stack for `thread`. If memory for the stack
 *                       is provided via `stack_mem`, this is used to indicate
 *                       the size of the memory. Otherwise, the system will
 *                       allocate the stack memory, and this is used to indicate
 *                       the minimum required stack size. A value of zero is
 *                       used to indicate that the platform-default should be
 *                       used.
 * @param[in] priority   The priority of `thread`.
 * @param[in] thread     Pointer to the `thread` to create. The caller is
 *                       responsible for deleting this with @ref
 *                       aceThread_delete after the new thread terminates.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceThread_new(aceThread_func_t func, void* arg, const char* name,
                           void* stack_mem, uint32_t stack_size,
                           aceThread_priority_t priority, aceThread_t* thread);

/**
 * @brief   Create a new detached thread.
 * @details Creates a new detached thread with the specified attributes (stack
 *          size, priority, etc). Detached threads have their resources
 *          automatically released back to the system when they terminate. This
 *          thread will be ready to run immediately after the function returns.
 *
 *          Detached threads are useful when the caller does not need to
 *          synchronize with the termination of the thread.
 * @note    The provided thread routine may call @ref aceThread_exit to
 *          terminate early, but is not required to do so.
 *
 * @param[in] func           The execution entry point for the newly created
 *                           thread.
 * @param[in] arg            The sole argument passed to `func`.
 * @param[in] name           Pointer to the C string which contains the name
 *                           of the thread. May be `NULL`.
 * @param[in] min_stack_size Minimum required stack size for the `thread`. A
 *                           value of zero is used to indicate that the
 *                           platform-default should be used.
 * @param[in] priority       The priority of the thread.
 *
 * @return    ACE_STATUS_OK on success, or an error otherwise.
 */
#ifdef ACE_OSAL_DETACHED_THREAD_SUPPORT
ace_status_t aceThread_detachedNew(aceThread_func_t func, void* arg,
                                   const char* name, uint32_t min_stack_size,
                                   aceThread_priority_t priority);
#endif

/**
 * @brief   Relinquishes the processor to the scheduler.
 * @details Causes the current thread to relinquish the processor to the
 *          scheduler. On linux-like systems, this will cause the thread to be
 *          moved to the end of the queue for it's static priority, allowing a
 *          new thread to run.
 * @note    This should only be called from within the execution context of an
 *          ACE thread.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceThread_yield(void);

/**
 * @brief   Waits for the specified thread to terminate.
 * @details Causes the caller to wait (block) for the thread specified by
 *          `thread` to terminate. If that thread has already terminated, then
 *          this function will return immediately.
 * @warning Attempting to join an unjoinable thread results in undefined
 *          behavior.
 *
 * @param[in] thread Pointer to the thread to join on.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceThread_join(aceThread_t* thread);

/*
 *  @brief Delete a thread.
 *
 *  This function is used to delete a thread. After that, the thread
 *  should terminate and its aceThread_t object including corresponding
 *  resources will be released also.
 *
 *  @param[in] thread Pointer to the thread
 *  @return ACE_STATUS_OK on success, else one of the @ref ace_status_t codes
 *  @ingroup ACE_OSAL_THREAD
 */

/**
 * @brief   Delete a thread.
 * @details Deletes the thread at the address pointed to by `thread`, releasing
 *          it's resources back to the system. This must only be called after
 *          calling @ref aceThread_join on the thread.
 * @warning Deleting a thread that hasn't been joined on results in undefined
 *          behavior.
 *
 * @param[in] thread Ponter to the thread to delete.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceThread_delete(aceThread_t* thread);

/**
 * @brief   Set a new priority for the specified thread.
 *
 * @param[in] thread   Pointer to the thread to set the priority.
 * @param[in] priority The new priority of the thread.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceThread_setPriority(aceThread_t* thread,
                                   aceThread_priority_t priority);

/**
 * @brief Get the priority of the specified thread.
 *
 * @param[in]  thread   Pointer to the thread to get the priority.
 * @param[out] priority Pointer to the buffer to store the priority.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
ace_status_t aceThread_getPriority(aceThread_t* thread,
                                   aceThread_priority_t* priority);

/**
 * @brief   Get a reference to the current thread.
 * @note    This should only be called from within the execution context of an
 *          ACE thread.
 *
 * @return Reference to the current thread.
 */
aceThread_t* aceThread_getCurrent(void);

/**
 * @brief Terminates the calling thread.
 * @note  This should only be called from within the execution context of an ACE
 *        thread.
 */
void aceThread_exit(void);

/**
 * @brief   Suspends execution of the current thread for an interval of time.
 * @details Causes the calling thread to sleep for the interval specified by
 *          `milliseconds`.
 * @note    The suspension time may be longer than the specified interval.
 * @note    This should only be called from within the execution context of an
 *          ACE thread.
 *
 * @param[in] millisec Amount of time to suspend the thread for, in
 *                     milliseconds.
 */
void aceThread_delay(uint32_t millisec);

/**
 * @brief   Checks whether the specified thread is valid.
 * @details Checks whether the thread at the address pointed to by `thread` is
 *          valid. A valid thread is one which is running and has not yet
 *          terminated.
 * @warning The specified thread may no longer be running by the time the caller
 *          is able to act on the return value of this function.
 *
 * @param[in] thread Pointer to the thread to check the validity of.
 *
 * @retval true  The thread is valid.
 * @retval false The thread is invalid.
 *
 * @deprecated
 */
bool aceThread_valid(aceThread_t* thread);

/**
 * @brief Get the name of the specified thread.
 *
 *  @param[in] thread Pointer to the thread to get the name of.
 *
 *  @return Pointer to a C string which contains the name of the thread.
 */
const char* aceThread_getName(aceThread_t* thread);

/**
 * @name Thread creation helper functions.
 * @{
 */
/**
 * @brief   Create a new joinable thread with default attributes.
 * @details Creates a thread at the address pointed to by `thread` with default
 *          attributes for stack size, priority, etc. The thread will be ready
 *          to run immediately after the function returns.
 *
 *          The caller can wait (block) for this thread to terminate by
 *          passing `thread` to @ref aceThread_join.
 *
 * @param[in] func   The execution entry point for the newly created thread.
 * @param[in] arg    The sole argument passed to `func`.
 * @param[in] name   Pointer to the C string which contains the name of the
 *                   thread. May be `NULL`.
 * @param[in] thread Pointer to the `thread` to create. The caller is
 *                   responsible for deleting this with @ref aceThread_delete
 *                   after the new thread terminates.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
static inline ace_status_t aceThread_create(aceThread_func_t func, void* arg,
                                            const char* name,
                                            aceThread_t* thread) {
    return aceThread_new(func, arg, name, ACE_NULL_PTR, 0, ACE_PRIORITY_NORMAL,
                         thread);
}

/**
 * @brief   Create a new joinable thread with the specified priority.
 * @details Creates a thread at the address pointed to by `thread`, with the
 *          priority specified by `priority` and default attributes otherwise.
 *          The thread will be ready to run immediately after the function
 *          returns.
 *
 *          The caller can wait (block) for this thread to terminate by
 *          passing `thread` to @ref aceThread_join.
 *
 * @param[in] func     The execution entry point for the newly created thread.
 * @param[in] arg      The sole argument passed to `func`.
 * @param[in] name     Pointer to the C string which contains the name of the
 *                     thread. May be `NULL`.
 * @param[in] priority The priority of `thread`.
 * @param[in] thread   Pointer to the `thread` to create. The caller is
 *                     responsible for deleting this with @ref aceThread_delete
 *                     after the new thread terminates.
 *
 * @return ACE_STATUS_OK on success, or an error otherwise.
 */
static inline ace_status_t aceThread_createWithPriority(aceThread_func_t func,
                                                        void* arg,
                                                        const char* name,
                                                        int priority,
                                                        aceThread_t* thread) {
    return aceThread_new(func, arg, name, ACE_NULL_PTR, 0, priority, thread);
}
/** @} */

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* OSAL_THREADS_H */
