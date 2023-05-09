/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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
#ifndef ACE_SDK_HAL_POWER_H_
#define ACE_SDK_HAL_POWER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <ace/ace_status.h>
#include <ace/power_types.h>

/**
 * @file hal_power.h
 *
 *@brief ACE Power HAL provides the interfaces to require low
 * level code to configure HW for different power state needs. This
 * layer will serve the commands/requests from the Power Service
 * middleware.
 * @addtogroup ACE_HAL_POWER
 * @{
 */

/**
 * @defgroup ACE_HAL_POWER_DS Data Structures and Enums
 * @{
 * @ingroup ACE_HAL_POWER
 */

/**
 * @brief Power Policy Type.
 */
typedef enum {
    /**
     * The lowest state HW support
     */
    ACE_PWRHAL_SUSPEND = 0,
    /**
     * Less busy state type, mainly for background task when uplevel don't care much of performance.
     */
    ACE_PWRHAL_STANDBY,
    /**
     * The highest policy for best performance.
     */
    ACE_PWRHAL_ACTIVE,
    /**
     * Maximium Limit
     */
    ACE_PWRHAL_MAX
} acePwrHal_policyType_t;


/**
 * @brief Specify type of message callback will handled
 * This structure can be extended to support more type.
 */
typedef enum{
    /**
     * For async operation inside HAL, error will be reported through callback if happened.
     */
    ACE_POWERHAL_ERR_STATE = 0,
    /**
     * Notify pre-set power state  before low level driver set the power state
     */
    ACE_POWERHAL_NOTIFY_PRE_SET_STATE,
    /**
     * Notify the expected power state is set.
     */
    ACE_POWERHAL_NOTIFY_POST_SET_STATE,
    /**
     * Notify device is moving to low power mode.
     */
    ACE_POWERHAL_ENTER_LOW_POWER_MODE,
    /**
     * Notify device is wakeup from low power mode.
     */
    ACE_POWERHAL_EXIT_LOW_POWER_MODE,
    ACE_POWERHAL_ERR_MAX
} acePowerHal_cb_msg_type_t;

/**
 * @brief data structure type inside callback parameter. Mainly used to indicate what is
 * pre- and post- state powerhal has complete.
 */
typedef struct {
    acePwrHal_policyType_t type;
    void * data;
} acePowerHal_cb_data_t;

/**
 * @brief callback message passed from power hal
 */
typedef struct {
    acePowerHal_cb_msg_type_t type;
    union cbData {
        ace_status_t err_state;
        acePowerHal_cb_data_t power_state;
    } data;
} acePowerHal_cb_msg_t;

/*@brief define callback function */
typedef void (*acePowerHalCallback_t)(acePowerHal_cb_msg_t * data);

/** @} */

/**
 * @brief This call can serve as HAL layer code to initialize data structure and load default
 * power configuration.
 * For example, each HW power control mechanism will be different. In Linux, this HAL layer
 * uses Linux Power Framework such as CPUFreq to configure default governor or hold
 * lock. For RTOS, it may set default cpu frequency or PMIC setting to initialize default
 * voltage on certain power domains of HW.
 * Data structure and default configuration can be defined and initialized to meet
 * the need from different OS and HW.
 *
 * @return ACE_STATUS_OK for success. Other value for failure.
 */
ace_status_t acePwrHal_init(void);

/**
 * @brief This API is to require power down of the device.
 *
 * @param[in] restart true for reboot
 *                    false for shutdown.
 * @param[in] reason  reboot or shutdown reason
 * @param[in] desc    description in string format and size is limited to @ref ACE_PWR_STR_BUFFER_SIZE
 *                    i.e., WiFi or OTA
 *
 * @return ACE_STATUS_OK for success. Other value for failure.
 */
ace_status_t acePwrHal_powerDown(bool restart, acePWR_shutdownReason_t reason, const char* desc);

/**
 * @brief This API is to require HAL layer to set a different policy based on support on each HW.
 * Most device has three power states: Active (busy), Standby(background
 * periodical active), Idle(sleep). Each state has different power requirements.
 *
 * For ACTIVE policy, it will map to  API level:  ACTIVE-HIGH, ACTIVE-MED,
 * ACTIVE-LOW parameters (see parameters) with the same policy type, but different
 * Value (use parameter - data) from 0 to 2 corresponding to LOW to HIGH active policy.
 * The same rules apply to STANDBY-HIGH, MED, LOW as well.
 * In Linux, for active high, it can adjust CpuFreq Governor to performance, and set
 * different max frequency for high, med and low.
 *
 * In RTOS, it can set different CPU frequencies to map ACTIVE-HIGH, MED, LOW.
 *
 * @param[in] type   For different power policy type.
 * @param[in] data   indicate level or special data for each type.
 *
 * @return ACE_STATUS_OK for success, other value for failure
 */
ace_status_t acePwrHal_setPolicy(const acePwrHal_policyType_t type, const void* data);

/**
 * @brief Register Callback function
 * @param[in] callback callback function
 *
 * @return ACE_STATUS_OK for success, other value for failure
 */
ace_status_t acePowerHalRegisterCallback(const acePowerHalCallback_t callback);

#ifdef __cplusplus
}
#endif

/** @} */

#endif // ACE_SDK_HAL_POWER_H_

