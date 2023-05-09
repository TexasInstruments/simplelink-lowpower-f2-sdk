/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef HALO_LIB_HALO_EXPORT_HALO_SYSTEM_H
#define HALO_LIB_HALO_EXPORT_HALO_SYSTEM_H

#include <stdbool.h>
#include <halo/platform/utils.h>

typedef struct {
    const char    * file_name;
    unsigned int    line_number;
    void          * pc;
} halo_system_reset_meta_info_t;

typedef enum {
    HALO_SYSTEM_RESET_TYPE_FORCED = 0x0,
    HALO_SYSTEM_RESET_TYPE_NORMAL = 0x1,
} halo_system_reset_type_t;

/**
 * Perform system shutdown and reset
 *
 * @param[in]   forced    enforce system reset right now
 *
 */
void halo_system_reset(const halo_system_reset_meta_info_t * info, halo_system_reset_type_t type);

#ifndef __FILENAME__
#define __FILENAME__ __FILE__
#endif

#define HALO_SYSTEM_RESET(forced_)                          \
    do {                                                    \
        void *pc = halo_get_pc_address();                   \
        const halo_system_reset_meta_info_t info_ = {       \
           .file_name     =  __FILENAME__,                  \
           .line_number   =  __LINE__,                      \
           .pc = pc                                         \
        };                                                  \
        halo_system_reset(&info_, forced_);                 \
    } while(0)

#endif
