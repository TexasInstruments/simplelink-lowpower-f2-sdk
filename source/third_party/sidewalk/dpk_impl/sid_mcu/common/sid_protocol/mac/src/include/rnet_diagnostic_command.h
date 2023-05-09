/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNET_DIAGNOSTIC_COMMAND_H
#define RNET_DIAGNOSTIC_COMMAND_H

#include <rnet_errors.h>
#include "rnet_nw_layer.h"
#include "rnet_api.h"
#include "rnet_time.h"

#define DIAGNOSTIC_PING_INTERVAL    1000UL //ms
#define DIAGNOSTIC_PING_NUM         10UL
#define DIAGNOSTIC_LQ_TEST_INTERVAL (DIAGNOSTIC_PING_NUM * DIAGNOSTIC_PING_INTERVAL)

void            diagnostic_update(int8_t up_rssi, int8_t up_snr);
rnet_error_t    diagnostic_stop(void);
rnet_error_t    diagnostic_start(uint8_t *recv_id, uint8_t *send_id, rnet_link_t link);
void            diagnostic_get_start_tm(time_def * result);
bool            diagnostic_is_test_enable(void);

#endif
