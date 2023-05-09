/*
 * Copyright 2021-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_900_CFG_H
#define SID_900_CFG_H

/**
 * Describes the profile type of the device
 */
enum sid_device_profile_id {
    /** Device Profile ID for Synchronous Network */
    SID_LINK2_PROFILE_1 = 0x01,
    SID_LINK2_PROFILE_2 = 0x02,
    SID_LINK2_PROFILE_LAST,

    /** Device Profile ID for Asynchronous Network */
    SID_LINK3_PROFILE_A = 0x80,
    SID_LINK3_PROFILE_B = 0x81,
    SID_LINK3_PROFILE_D = 0x83,
    SID_LINK3_PROFILE_LAST,
};
#define IS_LINK3_PROFILE_ID(X) ((X == SID_LINK3_PROFILE_A) || (X == SID_LINK3_PROFILE_B) || (X == SID_LINK3_PROFILE_D))
#define IS_LINK2_PROFILE_ID(X) ((X == SID_LINK2_PROFILE_1) || (X == SID_LINK2_PROFILE_2))


/**
 * Describes the number of RX windows opened by the device
 */
enum sid_rx_window_count {
    /** Used to indicate device opens infinite RX windows */
    SID_RX_WINDOW_CNT_INFINITE = 0,
    /** Used to indicate device opens 5 RX windows */
    SID_RX_WINDOW_CNT_2 = 5,
    SID_RX_WINDOW_CNT_3 = 10,
    SID_RX_WINDOW_CNT_4 = 15,
    SID_RX_WINDOW_CNT_5 = 20,
   /** Used to indicate device is in continuous RX mode */
    SID_RX_WINDOW_CONTINUOUS = 0xFFFF,
};

/**
 * Describes the frequency of RX windows opened by the device (in ms) in synchronous mode
 */
enum sid_link2_rx_window_separation_ms {
    /** Used to indicate device opens a RX window every 63 ms */
    SID_LINK2_RX_WINDOW_SEPARATION_1 = 63,
    /** Used to indicate device opens a RX window every 315 (63*5) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_2 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 5),
    /** Used to indicate device opens a RX window every 630 (63*10) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_3 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 10),
    /** Used to indicate device opens a RX window every 945 (63*15) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_4 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 15),
    /** Used to indicate device opens a RX window every 2520 (63*40) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_5 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 40),
    /** Used to indicate device opens a RX window every 3150 (63*50) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_6 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 50),
    /** Used to indicate device opens a RX window every 5040 (63*80) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_7 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 80),
};
#define IS_VALID_SID_LINK2_RX_WINDOW_SEPERATION(X) ( \
    X <= SID_LINK2_RX_WINDOW_SEPARATION_7 && X >= SID_LINK2_RX_WINDOW_SEPARATION_1 && \
    (X == SID_LINK2_RX_WINDOW_SEPARATION_1 || X % (SID_LINK2_RX_WINDOW_SEPARATION_2) == 0))

/**
 * Describes the frequency of RX windows opened by the device (in ms) in asynchronous mode
 */
enum sid_link3_rx_window_separation_ms {
    /** Used to indicate device opens a RX window every 5000 ms */
    SID_LINK3_RX_WINDOW_SEPARATION_3 = 5000
};

/**
 * Describes the TX/RX wake up configuration of the device
 */
enum sid_unicast_wakeup_type {
    /** Used to indicate device does not participate in TX or RX events */
    SID_NO_WAKEUP = 0,
    /** Used to indicate device only participates in TX events */
    SID_TX_ONLY_WAKEUP = 1,
    /** Used to indicate device only participates in RX events */
    SID_RX_ONLY_WAKEUP = 2,
    /** Used to indicate device participates in both TX and RX events */
    SID_TX_AND_RX_WAKEUP = 3
};

/**
 * Describes unicast attributes of the device's configuration
 */
struct sid_device_profile_unicast_params {
    /** Used to indicate profile type of the device */
    enum sid_device_profile_id device_profile_id;
    /** Used to indicate the number of RX windows opened by the device */
    enum sid_rx_window_count rx_window_count;
    /** Used to indicate the frequency of RX windows opened by the device (in ms) */
    union sid_unicast_window_interval {
        /** Used to indicate the frequency of RX windows opened by the device (in ms) in synchronous mode */
        enum sid_link2_rx_window_separation_ms sync_rx_interval_ms;
        /** Used to indicate the frequency of RX windows opened by the device (in ms) in asynchronous mode */
        enum sid_link3_rx_window_separation_ms async_rx_interval_ms;
    } unicast_window_interval;
    /** Used to indicate the TX/RX wake up configuration of the device */
    enum sid_unicast_wakeup_type wakeup_type;
};

/**
 * Describes the configuration attributes of the device's profile
 */
struct sid_device_profile {
    /** Describes the unicast attributes of the device's synchronous configuration */
    struct sid_device_profile_unicast_params unicast_params;
};

/**
 * Describes the configuration attributes of registration over link2
 */
struct sid_link_type_2_registration_config {
    /** Used to enable registration over link2*/
    bool enable;
    /** Used to indicate the periodicity (in seconds) of registration process attempts */
    uint32_t periodicity_s;
};

struct sid_sub_ghz_links_config {
    /** Enable transmission of Sub-Ghz link metrics to Sidewalk cloud services */
    bool enable_link_metrics;
    struct sid_link_type_2_registration_config registration_config;
};

#endif /* SID_900_CFG_H */
