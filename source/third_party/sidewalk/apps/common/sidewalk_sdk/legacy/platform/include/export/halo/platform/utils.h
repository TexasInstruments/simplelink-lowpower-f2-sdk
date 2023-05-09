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

#ifndef HALO_PLATFORM_INCLUDE_EXPORT_HALO_PLATFORM_UTILS_H
#define HALO_PLATFORM_INCLUDE_EXPORT_HALO_PLATFORM_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

void halo_platform_reset(void *pc);
void halo_platform_enable_dfu_mode(void);
void *halo_get_pc_address(void);

#ifdef __cplusplus
}
#endif

#endif /*! HALO_PLATFORM_INCLUDE_EXPORT_HALO_PLATFORM_UTILS_H */
