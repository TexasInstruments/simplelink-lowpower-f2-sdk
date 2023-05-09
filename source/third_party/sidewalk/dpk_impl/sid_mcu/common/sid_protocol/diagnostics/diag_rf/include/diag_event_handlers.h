/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.  This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef DIAG_EVENT_HANDLERS_H
#define DIAG_EVENT_HANDLERS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief set pending IRQ for event
 */
void diag_event_process(void);

/**
 *  @brief to init SWI parameters and register SWI handler
 */
void diag_event_init(void);

/**
 *  @brief the registered SWI handler function
 */
void diag_swi_handler(void);

#ifdef __cplusplus
}
#endif

#endif