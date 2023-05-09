/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_PAL_CRITICAL_REGION_IFC_H
#define SID_PAL_CRITICAL_REGION_IFC_H

/** @file
 *
 * @defgroup sid_pal_sync_ifc sid persistent synchronization API's required by Sidewalk SDK
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details     Provides API's used by components of the Sidewalk SDK to solve concurrency issues
 */


/**
 * Code executed between calls of function declared below will be protected from
 *  interruption from different thread or ISR context
 * @note Implementation is actually disabled IRQ's, so remember, amount of code
 *      and execution time of code between calls of those 2 functions should be
 *      as minimal as possible
 */

void sid_pal_enter_critical_region();
void sid_pal_exit_critical_region();


#endif /* SID_PAL_CRITICAL_REGION_IFC_H */
