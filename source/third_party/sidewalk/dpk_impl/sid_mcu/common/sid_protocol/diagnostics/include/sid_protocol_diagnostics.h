/*
 * Copyright 2021-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_DIAGNOSTICS_H
#define SID_DIAGNOSTICS_H

#include <sid_error.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum diagnostics_cmd {
    DIAGNOSTICS_TM_PHY_MODEM = 0,
    DIAGNOSTICS_TM_PHY_FP,
    DIAGNOSTICS_TM_PHY_MOD,
    DIAGNOSTICS_TM_PHY_PKT,
    DIAGNOSTICS_TM_PHY_PROFILE,
    DIAGNOSTICS_TM_PHY_CAD,
    DIAGNOSTICS_TM_PHY_TXTIMEOUT,
    DIAGNOSTICS_TM_PHY_TMONAIR,
    DIAGNOSTICS_TM_CW,
    DIAGNOSTICS_TM_MOD,
    DIAGNOSTICS_TM_PHY_STATE,
    DIAGNOSTICS_TM_PING,
    DIAGNOSTICS_TM_PWR,
    DIAGNOSTICS_TM_SCAN,
    DIAGNOSTICS_TM_FQHOP,
    DIAGNOSTICS_TM_RESET_RX_CNT,
    DIAGNOSTICS_TM_PRINT_RX_CNT,
    DIAGNOSTICS_TM_LAST_PKT,
    DIAGNOSTICS_TM_LAST_RSSI,
    DIAGNOSTICS_FW_VERSION,
    DIAGNOSTICS_GPIO_PIN_TEST,
    DIAGNOSTICS_CMD_MAX
};

/**
 * @brief   Process diagnostics cmd
 *
 * @return  SID_ERROR_NONE if success
 */
sid_error_t sid_diagnostics_cmd_exe(enum diagnostics_cmd cmd, int32_t argc, const char ** argv);

/*
 * @brief   Inits diagnostics
 *
 * @return  SID_ERROR_NONE if success
 */
sid_error_t sid_diagnostics_init(void);

#ifdef __cplusplus
}
#endif

#endif
