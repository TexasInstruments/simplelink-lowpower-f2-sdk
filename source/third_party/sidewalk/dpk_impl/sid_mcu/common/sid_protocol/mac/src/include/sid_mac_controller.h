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

#ifndef SID_MAC_CONTROLLER_H_
#define SID_MAC_CONTROLLER_H_

#include <sid_error.h>
#include <sid_900_cfg.h>
#include <stdint.h>

#define MAX_GRP_A_DEV_CAN_PARTICIPATE 5
#define MAX_UNICAST_A_DEV_CAN_PARTICIPATE 1
#define RX_WINDOW_INFINITE  0

struct sid_unicast_attr {
    uint8_t tx_en :1;
    uint8_t rx_en :1;
    uint8_t offset;
    uint8_t rx_wndw_cnt;
    /*Device Uni-cast window interval*/
    union sid_unicast_wndw_interval_t {
        /*FSK window separation*/
        uint8_t rx_periodicity; //fsk rx periodicity: a value check between 0-158
        /*LDR window interval*/
        enum sid_link3_rx_window_separation_ms rx_interval_ms;
    } unicast_rx_wndw_interval;
    /*Device Profile ID*/
    enum sid_device_profile_id sid_dev_prfl_id;
};

struct sid_grp_attr {
    uint8_t tx_en :1;
    uint8_t rx_en :1;
    uint8_t group_number;
    uint8_t rx_periodicity;
    uint8_t offset;
};

#endif /* SID_MAC_CONTROLLER_H_ */
