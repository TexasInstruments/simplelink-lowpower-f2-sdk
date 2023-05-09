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

#ifndef SID_PAL_SWI_IFC_H
#define SID_PAL_SWI_IFC_H

/** @file
 *
 * @defgroup sid_pal_lib_swi sid Software Interrupt (SWI) interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details     Provides SWI interface to be implemented by platform
 */
#include <sid_error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SWI callback
 *
 * @note The callback which will be executed in SWI context
 */
typedef void(*sid_pal_swi_cb_t)(void);

/**
 * Init the SWI handler for protocol processing
 *
 * @param[in]   event_callback      Pointer to the callback function the SWI will trigger
 *
 * @retval SID_ERROR_NONE in case of success
 *
 * Function initializes SWI for triggering events.
 */
sid_error_t sid_pal_swi_init(sid_pal_swi_cb_t event_callback);

/**
 * Trigger the SWI to run
 *
 * @retval SID_ERROR_NONE in case of success
 *
 * Function triggers SWI to run.
 */
sid_error_t sid_pal_swi_trigger(void);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
