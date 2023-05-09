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
/* @@@ this code is sample only and will change radically */

#ifndef OSAL_TEST_CONFIG_H
#define OSAL_TEST_CONFIG_H

#include <stdio.h>
#include <string.h>
#include "ace/osal_semaphore.h"
#include "ace/osal_threads.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_THREAD_WAIT 2000

bool create_validate_thread(aceThread_func_t func, void* arg, char* name,
                            aceThread_t* tid);
bool create_validate_thread_with_priority(aceThread_func_t func, void* arg,
                                          char* name,
                                          aceThread_priority_t priority,
                                          aceThread_t* tid);

/*---------------------------------------------------------------\
|  Sample implementation of CMSIS osSignal APIs with Semaphores  |
\_______________________________________________________________*/

/*---------\
|    API   |
\_________*/
void Signal_init(void);

/* these return the signal flags if a signal occurs, 0 for a timeout */
uint32_t Signal_wait_timeout(uint32_t wait_bits, uint32_t timeout);
uint32_t Signal_wait(uint32_t wait_bits);

/* returns 0 if the tid was deregistered, 1 if the tid was not found */
uint32_t Signal_deregister(aceThread_t* tid);

void Signal_set(aceThread_t* tid, uint32_t bits);
void Signal_clear(aceThread_t* tid, uint32_t bits);

#define SIG_MAX_THREADS 16
#define SIG_MAX_SIGNALS 32
#define NO_THREAD ((aceThread_t*)ACE_NULL_PTR)

typedef struct {
    aceThread_t* tid;
    uint32_t sig_bits;
    aceSemaphore_t sem;
} ThreadSignals_t;

/* Semaphore Test cases */
extern void test_TC_SemaphoreObtainCounting(void);
extern void test_TC_SemaphoreObtainBinary(void);
extern void test_TC_SemaphoreWaitForBinary(void);
extern void test_TC_SemaphoreWaitForCounting(void);
extern void test_TC_SemaphoreZeroCount(void);
extern void test_TC_SemParam(void);
extern void test_TC_SemaphoreCreateAndDelete(void);
extern void test_TC_SemaphoreWaitTimeout(void);

/* Thread Test cases */
extern void test_TC_ThreadCreate(void);
extern void test_TC_ThreadCreate_Priority(void);
extern void test_TC_ThreadPriority(void);
extern void test_TC_ThreadPriorityExec(void);
extern void test_TC_ThreadChainedCreate(void);
extern void test_TC_ThreadMultiInstance(void);
extern void test_TC_ThreadGetId(void);
extern void test_TC_ThreadYield(void);
extern void test_TC_ThreadParam(void);

/* Mutex Testcases */
extern void test_TC_MutexBasic(void);
extern void test_TC_MutexParam(void);
extern void test_TC_MutexNestedAcquire(void);
extern void test_TC_MutexTimeout(void);
extern void test_TC_MutexPriorityInversion(void);
extern void test_TC_MutexOwnership(void);

#ifdef __cplusplus
}
#endif

#endif /* OSAL_TEST_CONFIG_H */
