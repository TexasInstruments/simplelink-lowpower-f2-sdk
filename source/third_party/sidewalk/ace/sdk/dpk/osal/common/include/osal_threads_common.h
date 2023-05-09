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
#ifndef OSAL_THREADS_COMMON_H
#define OSAL_THREADS_COMMON_H
#include <stddef.h>

#include "ace/ace_config.h"
#include "ace/osal_threads.h"

/*
 * Attempt to use the configured min stack size. If it's less than our default,
 * bump it up. If _that_ is less than the platform min, bump it up again.
 */
static inline size_t aceThread_getMinStackSize(void) {
    size_t min_stack = ACE_OSAL_THREADS_STACK_MIN;

    if (min_stack < ACE_OSAL_THREADS_STACK_DEFAULT) {
        min_stack = ACE_OSAL_THREADS_STACK_DEFAULT;
    }

    if (min_stack < ACE_OSAL_THREADS_STACK_MIN_PLATFORM) {
        min_stack = ACE_OSAL_THREADS_STACK_MIN_PLATFORM;
    }

    return min_stack;
}

#endif  // OSAL_THREADS_COMMON_H
