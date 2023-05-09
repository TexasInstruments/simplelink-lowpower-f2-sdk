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
/**
 * @file hal_led.h
 * @brief ACE LED HAL builds on top of platform specific LED device driver.
 * @addtogroup ACE_HAL_LED
 *
 * USAGE
 * -----
 *
 * Steps to setup and use LED HAL interface:
 * 1) aceLedHal_open() needs to be called first to have LED HAL APIs ready
 *    for use.
 * 2) Use aceLedHal_getNumLeds() to query the total number of LEDs available
 *    on device.
 * 3) Use number of LED info from previous step to allocate enough memory for
 *    retrieving aceLedHal_info such as LED index and name.
 * 4) Activate (enable) the LED via aceLedHal_setParams(). By default, all LEDs
 *    are disabled at beginning. aceLedHal_getParams() can be used to retrieve
 *    the current activation state.
 * 5) Once the LED is activated, aceLedHal_setState() can be used to control
 *    the LED, such as setting on/off, brightness, color, etc.
 * 6) Once all LEDs are activated, aceLedHal_setMultiState() can be used to set
 *    all LED colors and other parameters at once. It is designed for using
 *    LED animation for efficiency.
 *    Note: aceLedHal_setState() and aceLedHal_getState() will only take
 *    effective when the LED is enabled. aceLedHal_setMultiState() will only
 *    take effective when all LEDs are enabled.
 *
 * @{
 */

#ifndef ACE_SDK_HAL_LED_H_
#define ACE_SDK_HAL_LED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ace/ace_status.h>
#include <stdint.h>

/** The maximum length of LED name */
#define ACELEDHAL_MAX_LEN_LED_NAME 32

/** LED activation state type */
typedef enum aceLedHal_activationState {
    ACTIVATION_STATE_LED_DISABLED, /**< LED disabled */
    ACTIVATION_STATE_LED_ENABLED   /**< LED enabled */
} aceLedHal_activationState_t;

/** RGB data format */
typedef struct aceLedHal_rgb888 {
    uint8_t r; /**< Color Red: 0-255 */
    uint8_t g; /**< Color Green: 0-255 */
    uint8_t b; /**< Color Blue: 0-255 */
} aceLedHal_rgb888_t;

/** Led operation state */
typedef struct aceLedHal_state {
    uint8_t on;             /**< On/Off flag. 0-Off, 1-On */
    uint8_t brightness;     /**< Brightness level: 0-100 */
    aceLedHal_rgb888_t rgb; /**< rgb value: in format as
                                 defined in aceLedHal_rgb888_t */
} aceLedHal_state_t;

/** Led configuration parameter */
typedef struct aceLedHal_params {
    aceLedHal_activationState_t activationState;
    /**< Enable or disable the LED */
} aceLedHal_params_t;

/** LED Info */
typedef struct aceLedHal_info {
    uint8_t ledIndex; /**< Index of LED */
    char ledName[ACELEDHAL_MAX_LEN_LED_NAME];
    /**< Name of LED */
} aceLedHal_info_t;

/**
 * Open must be called before using any LED HAL api.
 *
 * @param: None
 * @return ace_status_t: Return zero if Open is successful,
 * or any non-zero error code.
 */
ace_status_t aceLedHal_open(void);

/**
 * Close must be called when exiting LED HAL api
 *
 * @param: None
 * @return ace_status_t: Return zero if Close is successful,
 * or any non-zero error code.
 */
ace_status_t aceLedHal_close(void);

/**
 * Get number of LED on this device.  The caller must
 * allocate uint8_t pointer to hold numLeds.
 *
 * @param[out] numLeds: Number of Led on this device.
 * @return ace_status_t: Return zero if LED number retrieval is successful,
 * or any non-zero error code.
 */
ace_status_t aceLedHal_getNumLeds(uint8_t* numLeds);

/**
 * Get LED info including LED index and name.  The caller must
 * allocate enough memory for AceHalLedInfo pointer based
 * on the return value of AceHalLedGetNumLeds().
 *
 * @param[out] ledInfo: The pointer to store array of ledInfo.
 * @return ace_status_t: Return zero if LED info retrieval is successful,
 * or any non-zero error code.
 */
ace_status_t aceLedHal_getInfo(aceLedHal_info_t* ledInfo);

/**
 * Get the parameters from a specified LED. The caller must allocate
 * the memory for aceLedHal_params_t pointer.
 *
 * @param[in] ledIndex: The specified LED index.
 * @param[out] params: Pointer to the LED parameters.
 * @return ace_status_t: Return zero if successful, or any non-zero error code.
 */
ace_status_t aceLedHal_getParams(uint8_t ledIndex, aceLedHal_params_t* params);

/**
 * Set the parameters to a specified LED.
 *
 * @param[in] ledIndex: The specified LED index.
 * @param[in] params: Pointer to the LED parameters.
 * @return ace_status_t: Return zero if successful, or any non-zero error code.
 */
ace_status_t aceLedHal_setParams(uint8_t ledIndex,
                                 const aceLedHal_params_t* params);

/**
 * Get state from a specified LED. The caller must allocate
 * the memory for aceLedHal_state_t pointer.
 *
 * @param[in] ledIndex: The specified LED index.
 * @param[out] state: Pointer to a LED state.
 * @return ace_status_t: Return zero if successful, or any non-zero error code.
 */
ace_status_t aceLedHal_getState(uint8_t ledIndex, aceLedHal_state_t* state);

/**
 * Set the parameters to a specified LED.
 *
 * @param[in] ledIndex: The specified LED index.
 * @param[in] state: Pointer to a LED state.
 * @return ace_status_t: Return zero if successful, or any non-zero error code.
 */
ace_status_t aceLedHal_setState(uint8_t ledIndex,
                                const aceLedHal_state_t* state);

/**
 * Set states to multiple LEDs.
 * It will set state defined in state array to all LEDs at once.
 * The caller must allocate enough memory for aceLedHal_state_t
 * pointer based on the return value of AceHalLedGetNumLeds() to
 * hold state value for every LED.
 *
 * @param[in] frame: Pointer to array of LED state.
 * @return ace_status_t: Return zero if successful, or any non-zero error code.
 */
ace_status_t aceLedHal_setMultiState(const aceLedHal_state_t* stateArray,
                                     uint8_t stateArrayLength);

#ifdef __cplusplus
}
#endif

#endif /* ACE_SDK_HAL_LED_H_ */
/** @} */
