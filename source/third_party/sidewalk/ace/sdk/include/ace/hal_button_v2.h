/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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
 * @file hal_button_v2.h
 *
 * @brief ACE Button HAL builds on top of the device specific driver. It
 * keeps track of activation state and handles UP/DOWN changes. Any changes
 * are passed on to the button manager. The HAL is *not* responsible for timing
 * or more complex state machine-like buttons - that's the responsibility of the
 * manager.
 *
 * @addtogroup ACE_HAL_BUTTON
 *
 * USAGE
 * -----
 *
 * Steps to setup and use Button Hal interface:
 * 1) aceButtonHal_open() needs to be called first to have button Hal APIs ready
 * for use.
 * 2) Use aceButtonHal_getNumButtons() to query the total number of buttons
 * available on platform.
 * 3) Use button index to retrieve aceButtonHal_info such as button name and
 * attribute via
 *    aceButtonHal_getInfo(). The button index should be within the range of
 * number of buttons
 *    from previous step.
 * 4) Activate (enable) the button and register the callback via
 * aceButtonHal_setParams(). By
 *    default, all buttons are disabled at the beginning.
 * aceButtonHal_getParams() can be used to
 *    retrieve current activation state.
 *
 * @{
 */

#ifndef ACE_HAL_BUTTON_V2_H_
#define ACE_HAL_BUTTON_V2_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <ace/ace_status.h>

/** @brief Maximum length of a button name */
#define ACE_BUTTON_HAL_MAX_LEN_BUTTON_NAME 16

/**
 * @brief Button attribute (bitwise)
 * This denotes the button is stateful. This will allow upper
 * layer to persist its state and restore upon system reboot or
 * button manager service restart.
 */
#define ACE_BUTTON_HAL_ATTR_STATEFUL (1 << 0)

/** @brief Enumeration for up/down button presses */
typedef enum aceButtonHal_pressEvent {
    ACE_BUTTON_HAL_PRESSED_DOWN = 0, /**< Button pressed event */
    ACE_BUTTON_HAL_RELEASED_UP = 1   /**< Button released event */
} aceButtonHal_pressEvent_t;

/** @brief Button event information */
typedef struct aceButtonHal_event {
    uint8_t index; /**< Button index that this event is coming from */
    aceButtonHal_pressEvent_t event; /**< Button event */
} aceButtonHal_event_t;

/**
 * @brief Callback function to receive button state change
 *
 * @param[in] event Pointer to event information.
 */
typedef void (*aceButtonHal_callback)(aceButtonHal_event_t* event);

/** @brief Button activation state type */
typedef enum aceButtonHal_activationState {
    ACE_BUTTON_HAL_ACTIVATION_STATE_DISABLED = 0, /**< Button disabled */
    ACE_BUTTON_HAL_ACTIVATION_STATE_ENABLED = 1   /**< Button enabled */
} aceButtonHal_activationState_t;

/** @brief Configuration parameter for button */
typedef struct aceButtonHal_params {
    aceButtonHal_activationState_t
        activation_state;                 /**< Button activation state */
    aceButtonHal_callback event_callback; /**< Button event callback function */
} aceButtonHal_params_t;

/** @brief Button info */
typedef struct aceButtonHal_info {
    char name[ACE_BUTTON_HAL_MAX_LEN_BUTTON_NAME]; /**< Button name */
    uint32_t attr;                                 /**< Button attribute */
} aceButtonHal_info_t;

/**
 * @brief Initialize button HAL and allocate resources
 *
 * Must be called before using any button HAL API.
 *
 * @return ACE_STATUS_OK on success, and a negative value from @ref ace_status_t
 * otherwise
 */
ace_status_t aceButtonHal_open(void);

/**
 * @brief Deinitialize button HAL and release resources
 *
 * Must be called when exiting button HAL API.
 *
 * @return ACE_STATUS_OK on success, and a negative value from @ref ace_status_t
 * otherwise
 */
ace_status_t aceButtonHal_close(void);

/**
 * @brief Get number of buttons on this device
 *
 * @param[in,out] num_buttons Pointer to number of button on this device (caller
 * allocated)
 * @return ACE_STATUS_OK on success, and a negative value from @ref ace_status_t
 * otherwise
 */
ace_status_t aceButtonHal_getNumButtons(uint8_t* num_buttons);

/**
 * @brief Get button info including name and attribute
 *
 * @param[in] index Specified button index to get info for
 * @param[in,out] info Pointer to store button_info (caller allocated)
 * @return ACE_STATUS_OK on success, and a negative value from @ref ace_status_t
 * otherwise
 */
ace_status_t aceButtonHal_getInfo(uint8_t index, aceButtonHal_info_t* info);

/**
 * @brief Get the parameters of a specified button
 *
 * The caller must allocate memory for aceButtonHal_params_t pointer.
 *
 * @param[in] index Specified button index
 * @param[in,out] params Pointer to the button parameters (caller allocated)
 * @return ACE_STATUS_OK on success, and a negative value from @ref ace_status_t
 * otherwise
 */
ace_status_t aceButtonHal_getParams(uint8_t index,
                                    aceButtonHal_params_t* params);

/**
 * @brief Set the parameters for a specified button
 *
 * @param[in] index Specified button index
 * @param[in] params Pointer to the button parameters
 * @return ACE_STATUS_OK on success, and a negative value from @ref ace_status_t
 * otherwise
 */
ace_status_t aceButtonHal_setParams(uint8_t index,
                                    const aceButtonHal_params_t* params);

#ifdef __cplusplus
}
#endif

#endif /* ACE_HAL_BUTTON_V2_H_ */
/** @} */
