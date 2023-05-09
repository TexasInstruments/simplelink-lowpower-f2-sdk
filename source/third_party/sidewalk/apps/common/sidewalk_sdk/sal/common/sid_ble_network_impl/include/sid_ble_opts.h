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

#ifndef SID_BLE_OPTS_H
#define SID_BLE_OPTS_H

#if defined(SID_BLE_ENABLE_EXT_OPTS) && SID_BLE_ENABLE_EXT_OPTS
#include <sid_ble_ext_opts.h>
#endif

/*
 * @brief: Enable support for periodic timesync on BLE advertisement packet.
 */

#ifndef SID_BLE_PERIODIC_TIMESYNC_ENABLED
#define SID_BLE_PERIODIC_TIMESYNC_ENABLED 1
#endif

/*
 * @brief: BLE configuration flag to control the periodic time synchronization
 * interval with cloud on advertisement packet. Optimal value should be choosen
 * based on clock drift on the product/platform.
 */

#if defined(SID_BLE_PERIODIC_TIMESYNC_INTERVAL_SEC) && (SID_BLE_PERIODIC_TIMESYNC_INTERVAL_SEC < 7200)
#undef SID_BLE_PERIODIC_TIMESYNC_INTERVAL_SEC
#endif

#ifndef SID_BLE_PERIODIC_TIMESYNC_INTERVAL_SEC
#define SID_BLE_PERIODIC_TIMESYNC_INTERVAL_SEC 7200
#endif

/*
 * @brief: BLE configuration flag to control the duration of time synchronization
 * advertisement packet. Duration should be set to value less than 10 minutes.
 * Sidewalk cloud service determines device as lost if TX UUID based advertisement
 * is not received after 10 minutes.
 */

#if defined(SID_BLE_PERIODIC_TIMESYNC_ADV_DURATION_SEC) && (SID_BLE_PERIODIC_TIMESYNC_ADV_DURATION_SEC <= 0 || SID_BLE_PERIODIC_TIMESYNC_ADV_DURATION_SEC > 600)
#undef SID_BLE_PERIODIC_TIMESYNC_ADV_DURATION_SEC
#endif

#ifndef SID_BLE_PERIODIC_TIMESYNC_ADV_DURATION_SEC
#define SID_BLE_PERIODIC_TIMESYNC_ADV_DURATION_SEC 60
#endif

/*
 * @brief: BLE configuration flag to control how long the connection remains
 * active when there is no sidewalk tx/rx activity. Max inactivity timeout is 5
 * minutes.
 */

#if defined(SID_BLE_INACTIVITY_TIMEOUT_SEC) && (SID_BLE_INACTIVITY_TIMEOUT_SEC > 300)
#undef SID_BLE_INACTIVITY_TIMEOUT_SEC
#endif

#ifndef SID_BLE_INACTIVITY_TIMEOUT_SEC
#define SID_BLE_INACTIVITY_TIMEOUT_SEC 30
#endif

#endif
