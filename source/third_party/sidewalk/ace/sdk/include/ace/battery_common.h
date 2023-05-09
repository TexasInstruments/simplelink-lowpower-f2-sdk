/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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
#ifndef ACE_SDK_API_BATTERY_COMMON_H
#define ACE_SDK_API_BATTERY_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <ace/ace_status.h>

/**
 * @file battery_common.h
 * @brief Common interfaces for the battery service.
 *
 * @addtogroup ACE_BATTERY_API
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup ACE_BATTERY_DS Data Structures and Enums
 * @{
 * @ingroup ACE_BATTERY_API
 */

/**
 * @brief Charging power source
 */
typedef enum {
    /** No power source, running on battery */
    ACE_BATTERY_SOURCE_NONE = 0,
    /** Wall charger */
    ACE_BATTERY_SOURCE_AC,
    /** USB charger */
    ACE_BATTERY_SOURCE_USB,
    /** Wireless charger */
    ACE_BATTERY_SOURCE_WIRELESS
} aceBattery_powerSource_t;

/**
 * @brief Type of battery technology
 */
typedef enum {
    /** No battery, always connected to power */
    ACE_BATTERY_TECH_NONE = 0,
    /** Lithium Ion, chargeable battery */
    ACE_BATTERY_TECH_LION = 1,
    /** Remote, not chargeable battery, does not require monitoring */
    ACE_BATTERY_TECH_NOT_CHARGEABLE = 2
} aceBattery_technology_t;

/**
 * @brief Charge status of battery
 */
typedef enum {
    /** Not initialized, status is unknown, default at boot */
    ACE_BATTERY_STATUS_UNKNOWN = 0,
    /** Always powered, no battery (ACE_BATTERY_TECH_NONE) */
    ACE_BATTERY_STATUS_NO_BATTERY,
    /** Level is less than low power threshold */
    ACE_BATTERY_STATUS_LOW_POWER,
    /** Reached max battery level, fully charged */
    ACE_BATTERY_STATUS_FULL,
    /** Connected to power, level increasing */
    ACE_BATTERY_STATUS_CHARGING,
    /** Battery level dropping */
    ACE_BATTERY_STATUS_DISCHARGING,
    /**
     * Power is connected but level is not increasing,
     * such as, battery just reached full charge (transient state before
     * ACE_BATTERY_STATUS_FULL) or charge (ie, USB power) is not sufficient
     * to increase battery level due to activity.
     */
    ACE_BATTERY_STATUS_NOT_CHARGING
} aceBattery_chargeStatus_t;

/**
 * @brief Health of battery
 */
typedef enum {
    /** Not initialized, health is unknown, default at boot */
    ACE_BATTERY_HEALTH_UNKNOWN = 0,
    /** Battery in good condition */
    ACE_BATTERY_HEALTH_GOOD,
    /** Battery is overheating */
    ACE_BATTERY_HEALTH_OVERHEAT,
    /** Battery is dead */
    ACE_BATTERY_HEALTH_DEAD,
    /** Battery in over voltage */
    ACE_BATTERY_HEALTH_OVER_VOLTAGE,
    /** Battery health has unspecified failure */
    ACE_BATTERY_HEALTH_UNSPECIFIED_FAILURE,
    /** Battery is cold */
    ACE_BATTERY_HEALTH_COLD,
} aceBattery_health_t;

/**
 * @brief Current battery details.
 */
typedef struct {
    /** Is battery present? */
    uint8_t present;
    /** Is battery plugged in? */
    uint8_t plugged;
    /** Power source connected */
    aceBattery_powerSource_t source;
    /** Battery charging status */
    aceBattery_chargeStatus_t status;
    /** Battery technology */
    aceBattery_technology_t technology;
    /** Battery health */
    aceBattery_health_t health;
    /** Maximum battery level (integer value). Battery level can be from 0 to
     * scale
     */
    uint8_t scale;
    /** Current battery level (integer value) */
    uint8_t level;
    /** Battery temperature in tenths of a degree Centigrade, i.e., 312 = 31.2C
     */
    uint16_t temperature;
    /** Current battery capacity in microamperes-hours */
    uint32_t capacity_uah;
    /** Battery current in microamperes */
    uint32_t current_ua;
    /** Maximum charging current supported by the charger in micro amperes */
    uint32_t charging_current_max_ua;
    /** Current battery voltage in millivolts */
    uint16_t voltage_mv;
    /** Maximum charging voltage */
    uint16_t charging_voltage_max_mv;
} aceBattery_details_t;

#ifdef __cplusplus
}
#endif

/** @} */ /* ACE_BATTERY_DS */
/** @} */
#endif /** ACE_SDK_API_BATTERY_COMMON_H */
