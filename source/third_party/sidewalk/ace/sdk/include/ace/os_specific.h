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
#ifndef OS_SPECIFIC_H
#define OS_SPECIFIC_H

#include <stdint.h>
#include <stdbool.h>

#include <FreeRTOS.h>
#include <semphr.h>
#include <timers.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OS_MUTEX_UNINITIALIZED 0xDEAD
#define OS_MUTEX_INITIALIZED 0x5AFE
#define OS_MUTEX_INITIALIZER \
    { .hnd = NULL, .init_mark = OS_MUTEX_UNINITIALIZED, .recursive = 0 }

#define OS_IPMUTEX_INITIALIZER \
    { NULL }

#ifndef ACE_OSAL_THREADS_STACK_MIN_PLATFORM
#define ACE_OSAL_THREADS_STACK_MIN_PLATFORM configMINIMAL_STACK_SIZE
#endif

/** @brief Timer callback information structure definition */
typedef struct {
    void (*func)(void* ctx);
    void* ctx;
} TimerCallback_t;

struct aceMutex_struct {
    StaticSemaphore_t
        mutex; /* FreeRTOS static mutex structure. Used by FreeRTOS to store
                  mutex internals. Must be first element to ensure Queue_t can
                  be reverse cast to aceMutex_t* */
    SemaphoreHandle_t
        hnd; /* FreeRTOS mutex handle passed to all FreeRTOS mutex functions */
    uint16_t init_mark; /* Indicates if the mutex is initialized */
    bool recursive;     /* Indicates whether the mutex is recursive */
};

struct aceMq_struct {
    StaticQueue_t queue; /* FreeRTOS static queue structure. Used to
                            store queue internals.  Must be first
                            element to ensure Queue_t can be reverse
                            cast to aceMq_t* */
    QueueHandle_t hnd;   /* FreeRTOS queue handle - passed to any
                            FreeRTOS queue functions */
    uint32_t queue_size; /* Used by aceQueue_getSize() */
    uint32_t msg_size;   /* Used by aceQueue_getMsgSize() */
};

/** @brief Timer structure of ACE */
struct aceTimer_struct {
    TimerHandle_t timer_id;  // Timer handle
    TimerCallback_t callb;   // Timer callback
    uint32_t period_ms;
    uint32_t time_remaining_ms;
    uint8_t state;
};

struct aceSemaphore_struct {
    StaticSemaphore_t
        sem; /* FreeRTOS static semaphore structure. Used to store semaphore
             internals.  Must be first element to ensure Queue_t can be
             reverse cast to aceSemaphore_t* */
    SemaphoreHandle_t
        hnd; /* FreeRTOS semaphore handle - passed to any FreeRTOS semaphore
             functions */
};

struct aceIpMutex_s {
    struct aceSemaphore_struct* p_sem;
    uint8_t valid : 1;
};

struct aceThread_struct {
    StaticTask_t task; /* FreeRTOS static thread structure Used to store task
                         control block - Must be first element to ensure
                         TaskHandle_t can be reverse cast to aceThread_t* */
    TaskHandle_t hnd;  /* FreeRTOS thread handle - passed to any FreeRTOS
                         semaphore functions */
    void* stack;       /* FreeRTOS thread stack */
    struct aceSemaphore_struct*
        join_sem; /* Semaphore to allow threads to be joined. */
    bool active;  /* Set true when thread starts, set false at end of thread
                    wrapper function, and in aceThread_exit() */
};

#ifdef __cplusplus
}
#endif

#endif /* OS_SPECIFIC_H */
