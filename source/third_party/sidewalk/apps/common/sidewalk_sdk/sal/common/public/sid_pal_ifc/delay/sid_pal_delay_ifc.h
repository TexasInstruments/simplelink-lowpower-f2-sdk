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

#ifndef SID_PAL_DELAY_IFC_H
#define SID_PAL_DELAY_IFC_H

/** @file
 *
 * @defgroup sid_pal_lib_delay sid delay interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details     Provides a way for protocol to delay
 */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Implements a busy wait delay safe to be used in SWI context.
 * Due to busy wait implementation, should not be used for long durations.
 *
 * @param[in]   delay          Time in us to delay
 *
 * @retval none
 */
void sid_pal_delay_us(uint32_t delay);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
