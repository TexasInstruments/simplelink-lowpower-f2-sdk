/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_OPTS_H
#define SID_OPTS_H

/** @file
 *
 * @defgroup SIDEWALK_API Sidewalk options
 * @brief interface target describing static options for the Sidewalk SDK
 * @{
 * @ingroup  SIDEWALK_API
 */

/**
 * Describes whether external options are included.
 *
 * Note: external options must be passed to this target through the SID_OPTS_DIR
 * CMake variable which is passed through the sid_common.cmake build file. The user
 * must set this variable to the directory which contains sid_ext_opts.h in order
 * for the options to take effect. The file naming convention of "sid_ext_opts.h"
 * must be used. If SID_OPTS_DIR is not set by the application, the default values
 * for the options as defined in this file will apply.
 */
#if SID_OPTS_PROVIDED
#include <sid_ext_opts.h>
#endif

/**
 * Defines the keep alive period in seconds which will be sent by the internal sidewalk
 * management classes. To disable the keep alive functionality, set this value
 * to 0.
 */
#ifndef SID_OPTS_KA_INTERVAL_SEC
#define SID_OPTS_KA_INTERVAL_SEC (0)
#endif

/**
 * Defines the duty cycle which must be met in order to comply with Specific Absorption
 * Rate (SAR) requirements as set by the FCC for wearable devices.
 */
#ifndef SID_OPTS_RADIO_SAR_DCR
#define SID_OPTS_RADIO_SAR_DCR (100)
#endif

/**
 * Defines the max allowable output power in dBm allowed for each of the data rates
 * supported. The data rate mappings are as follows:
 *    ___________________________
 *   |Data Rates | Index in Table|
 *   |---------------------------|
 *   | 2 KBPS    |      0        |
 *   | 22 KBPS   |      1        |
 *   | 50 KBPS   |      2        |
 *   | 150 KBPS  |      3        |
 *   | 250 KBPS  |      4        |
 *   | 12.5 KBPS |      5        |
 *    ---------------------------
 */
#ifndef SID_OPTS_RF_POWER_MAX_TABLE
#define SID_OPTS_RF_POWER_MAX_TABLE {20, 20, 20, 20, 20, 20}
#endif

/**
 * Defines the output power in dBm allowed for each of the data rates
 * supported. The data rate mappings are as follows:
 *    ___________________________
 *   |Data Rates | Index in Table|
 *   |---------------------------|
 *   | 2 KBPS    |      0        |
 *   | 22 KBPS   |      1        |
 *   | 50 KBPS   |      2        |
 *   | 150 KBPS  |      3        |
 *   | 250 KBPS  |      4        |
 *   | 12.5 KBPS |      5        |
 *    ---------------------------
 */

#ifndef SID_OPTS_RF_POWER_TABLE
#define SID_OPTS_RF_POWER_TABLE {20, 20, 20, 20, 20, 20}
#endif

/** @} */

#endif
