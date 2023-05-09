/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All rights reserved.
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
#ifndef ACE_SDK_HAL_BATTERY_H_
#define ACE_SDK_HAL_BATTERY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <ace/ace_status.h>
#include <ace/battery_common.h>

/**
 * @file hal_battery.h
 *
 * @brief ACE Battery HAL provides the interfaces required for low
 * level code to update battery properties and health to middleware. This
 * layer will also serve the commands/requests from the Battery MW Service.
 * @addtogroup ACE_HAL_BATTERY
 * @{
 */

/**
 * @defgroup ACE_HAL_BATTERY_DS Data Structures and Enums
 * @{
 * @ingroup ACE_HAL_BATTERY
 */

/**
 * @brief Callback message types to notify a certain event happened.
 */
typedef enum {
    /** Update battery details when charge level, source or status change */
    ACE_BATTERYHAL_BATTERY_DETAILS = 0,
    /** An invalid charger is detected */
    ACE_BATTERYHAL_INVALID_CHARGER,
    /** Critical battery level is reached */
    ACE_BATTERYHAL_CRITICAL_BATTERY_LEVEL,
    /** Critical battery temperature is detected */
    ACE_BATTERYHAL_CRITICAL_TEMPERATURE,
    /** Critical voltage is detected */
    ACE_BATTERYHAL_CRITICAL_VOLTAGE,
    /** Critical current is detected */
    ACE_BATTERYHAL_CRITICAL_CURRENT
} aceBatteryHal_cbMsgType_t;

/**
 * @brief Data structure for the battery callback message passed from battery HAL.
 *        Type and data differ based on what events are observed, such as when
 *        critical thresholds, like temperature, are reached,
 *        or when the battery charge level, source, or status change.
 */
typedef struct {
    aceBatteryHal_cbMsgType_t type;
    union {
        /**
         * @ref ACE_BATTERYHAL_BATTERY_DETAILS
         * Update battery details when charge level, source or status change.
         */
        aceBattery_details_t battery_details;
        /**
         * @ref ACE_BATTERYHAL_INVALID_CHARGER
         * An invalid charger is detected.
         */
        bool invalid_charger;
        /**
         * @ref ACE_BATTERYHAL_CRITICAL_BATTERY_LEVEL
         * Device reached critical battery level.
         */
        uint8_t level;
        /**
         * @ref ACE_BATTERYHAL_CRITICAL_TEMPERATURE
         * Device reached critical battery temperature.
         * Tenths of a degree Centigrade, ie, 312 = 31.2C
         */
        uint16_t temperature;
        /**
         * @ref ACE_BATTERYHAL_CRITICAL_VOLTAGE
         * Critical voltage (millivolts) is detected.
         */
        uint16_t voltage_mv;
        /**
         * @ref ACE_BATTERYHAL_CRITICAL_CURRENT
         * Critical current (microamperes) is detected.
         */
        uint32_t current_ua;
    } data;
} aceBatteryHal_cbMsg_t;

/** @} */ /* ACE_HAL_BATTERY_DS */

/**
 * @brief Callback function for sending @ref aceBatteryHal_cbMsgType_t
 */
typedef void (*aceBatteryHalCallback_t)(aceBatteryHal_cbMsg_t* data);


/**
 * @brief Initialize Battery HAL to include loading default configuration.
 * Read initial battery settings and apply them.
 * Data structure and default configuration can be defined and
 * initialized to specific product requirements.
 *
 * @return ACE_STATUS_OK on success, otherwise one of the error codes @ref ace_status_t.
 */
ace_status_t aceBatteryHal_init(void);

/**
 * @brief De-initialize Battery HAL and release any associated resources.
 *
 * @return ACE_STATUS_OK on success, otherwise one of the error codes @ref ace_status_t.
 */
ace_status_t aceBatteryHal_deinit(void);

/**
 * @brief Get battery details.
 *
 * @param[out] details Allocate data structure for details to be filled with
 *                    battery details data. Caller provides memory storage.
 *
 * @return ACE_STATUS_OK on success, otherwise one of the error codes @ref ace_status_t.
 */
ace_status_t aceBatteryHal_getBatteryDetails(aceBattery_details_t* details);

/**
 * @brief Register callback function for battery details and critical events
 * described by @ref aceBatteryHal_cbMsgType_t
 *
 * @param[in] callback callback function
 *
 * @return ACE_STATUS_OK on success, otherwise one of the error codes @ref ace_status_t.
 */
ace_status_t aceBatteryHal_registerCallback(const aceBatteryHalCallback_t callback);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* ACE_SDK_HAL_BATTERY_H_ */
