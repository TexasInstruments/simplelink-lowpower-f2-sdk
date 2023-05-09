/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <rnet_errors.h>

/**
 * Process notifier struct
 */
typedef struct {
    void (*notify)(void* data);
    void * data;
} rnet_process_notifier_t;

/**
 * Public API to register a process notifier
 *
 * @param[in] observer          Observer to use in rnet
 * @retval RNET_SUCCESS         Success
 * @retval RNET_ERROR_NULL      NULL observer
 * @retval RNET_ERROR_INTERNAL  Timer init failed
 */
rnet_error_t rnet_process_notifier_set(const rnet_process_notifier_t *notifier);

/**
 * Notify application that protocol has events to process in main task.
 *
 * WARNING: rnet_process() should not be called from this context.
 */
void rnet_process_notify();
