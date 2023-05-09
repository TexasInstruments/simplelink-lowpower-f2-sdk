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

#ifndef SID_QA_CLI_H
#define SID_QA_CLI_H

#include <sid_error.h>
#include <sid_api.h>

enum qa_proc_wait_status {
    QA_PROC_NO_WAIT = 0,
    QA_PROC_WAIT
};

typedef void (*pwr_meas_func_t)(void);
typedef void (*notify_blocked_func_t)(void);
typedef void (*notify_unblocked_func_t)(void);

typedef void (*reboot_func_t)(void);
typedef void (*set_sub_ghz_cfg_t)(struct sid_sub_ghz_links_config *sub_ghz_cfg);

struct sid_qa_callbacks {
    reboot_func_t reboot_cmd;
    set_sub_ghz_cfg_t set_sub_ghz_cfg;
};

struct sid_qa_pwr_meas_if {
    pwr_meas_func_t enter_func;
    pwr_meas_func_t exit_func;
    notify_blocked_func_t blocked_func;
    notify_unblocked_func_t unblocked_func;
};

/**
 * Initializes the QA CLI commands.
 */
sid_error_t sid_qa_init(struct sid_qa_callbacks *qa_callbacks);

/**
 * set qa config.
 */
void sid_qa_set_config(struct sid_config *config_p, struct sid_qa_pwr_meas_if *platform_ifc);

/**
 *  QA process routine.
 */
void sid_qa_process(enum qa_proc_wait_status wait_status);

#endif

