/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_LOG_CONTROL_H
#define SID_LOG_CONTROL_H

/** @file
 *
 * @defgroup sid_log_control sid log_control implementation
 * @details Provides a common implementation for controlling the level of logging
 */

#include "sid_pal_log_ifc.h"

#include <sid_error.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sid_log_control_severity {
    sid_pal_log_severity_t level;
    bool enable;
};

void sid_log_control_set_severity(struct sid_log_control_severity *severity);
void sid_log_control_get_severity(struct sid_log_control_severity *severity);
sid_pal_log_severity_t sid_log_control_get_current_log_level();

#ifdef __cplusplus
}
#endif

#endif /* SID_LOG_CONTROL_H */
