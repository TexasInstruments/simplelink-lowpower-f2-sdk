/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_GW_QA_CLI_H
#define SID_GW_QA_CLI_H

#include <sid_error.h>
#include <sid_gw_api.h>

typedef void (*pwr_meas_func_t)(void);
typedef void (*reboot_func_t)(void);

struct sid_gw_qa_pwr_meas_if {
    pwr_meas_func_t enter_func;
    pwr_meas_func_t exit_func;
};

/**
 * Initializes the QA CLI commands.
 *
 * sid_gw_qa_init() must only be called once. sid_gw_qa_init() calls sid_gw_init() for initialization Sidewalk
 *
 * @param[in] reboot_cmd    Pointer to a function to be called after the "reboot" command.
 * @param[in] config_p      The required configuration in order to properly initialize Sidewalk.
 * @param[in] platform_ifc  Pointer to a struct with callback functions for power measurement mode.
 *
 * @returns #SID_ERROR_NONE      on success.
 * @returns #SID_ERROR_GENERIC   if initialization cannot complete successfully
 */
sid_error_t
sid_gw_qa_init(reboot_func_t reboot_cmd, struct sid_gw_config *config_p, struct sid_gw_qa_pwr_meas_if *platform_ifc);

/**
 * Process power measurement events and call sid_gw_process().
 *
 */
void sid_gw_qa_process(void);

#endif
