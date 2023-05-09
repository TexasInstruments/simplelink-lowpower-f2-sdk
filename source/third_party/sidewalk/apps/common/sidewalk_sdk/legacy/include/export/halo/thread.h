/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef HALO_LIB_HALO_EXPORT_HALO_THREAD_H
#define HALO_LIB_HALO_EXPORT_HALO_THREAD_H

#include <stdint.h>

void halo_thread_enter_critical(void);
void halo_thread_exit_critical(void);
void halo_thread_yield(void);
void halo_thread_usleep(uint32_t usec);
void halo_thread_msleep(uint32_t msec);

#endif /* !HALO_LIB_HALO_EXPORT_HALO_THREAD_H */
