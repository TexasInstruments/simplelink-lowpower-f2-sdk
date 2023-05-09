/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef HALO_PLATFORM_INCLUDE_EXPORT_HALO_PLATFORM_DELAY_H
#define HALO_PLATFORM_INCLUDE_EXPORT_HALO_PLATFORM_DELAY_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void platform_delay_s(size_t delay_time_s);

void platform_delay_ms(size_t delay_time_ms);

void platform_delay_us(size_t delay_time_us);

#ifdef __cplusplus
}
#endif

#endif /*! HALO_PLATFORM_INCLUDE_EXPORT_HALO_PLATFORM_DELAY_H */
