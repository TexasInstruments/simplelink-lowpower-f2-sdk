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
 * @file hal_button_mod.h
 *
 * @brief ACE Button HAL builds on top of the device specific driver. It
 * keeps track of activation state and handles UP/DOWN changes. Any changes
 * are passed on to the button manager. The HAL is *not* responsible for timing
 * or more complex state machine-like buttons - that's the responsibility of the manager.
 * The APIs defined in this file are deprecated, and are replaced by those in hal_button_v2.h
 *
 * @addtogroup ACE_HAL_BUTTON
 *
 * USAGE
 * -----
 *
 * Steps to setup and use Button Hal interface:
 * 1) aceButtonHal_open() needs to be called first to have button Hal APIs ready for use.
 * 2) Use aceButtonHal_getNumButtons() to query the total number of buttons available on platform.
 * 3) Use number of button info from previous step to allocate enough memory for retrieving
 *    aceButtonHal_info such as button index and name via aceButtonHal_getInfo().
 * 4) Activate (enable) the button and register the callback via aceButtonHal_setParams(). By
 *    default, all buttons are disabled at beginning. aceButtonHal_getParams() can be use to
 *    retrieve current activation state.
 *
 * @{
 */

#ifndef ACE_SDK_HAL_BUTTON_MOD_H_
#define ACE_SDK_HAL_BUTTON_MOD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <ace/ace_status.h>

/**
 * @deprecated Please use the new symbols.
 * @{
 */

/** @brief maximum length of a buttonhal name */
#define ACEBUTTONHAL_MAX_LEN_BUTTON_NAME 16

/** @brief enumeration for up/down button presses */
typedef enum aceButtonHal_pressEvent {
    BUTTON_PRESSED_DOWN,
    BUTTON_RELEASED_UP
} aceButtonHal_pressEvent_t;

/** Button event information */
typedef struct aceButtonHal_event {
    uint8_t buttonIndex;                       /**< buttonIndex that this event is coming from */
    aceButtonHal_pressEvent_t value;
} aceButtonHal_event_t;

/**
 * Callback function to receive button state change
 *
 * @param event: Pointer to event information.
 * return: 0 -- successful, 1 -- failure.
 */
typedef int (*aceButtonHal_callback)(aceButtonHal_event_t *event);


/** Button activation state type */
typedef enum aceButtonHal_activationState {
    BUTTON_ACTIVATION_STATE_DISABLED, /**< Button disabled */
    BUTTON_ACTIVATION_STATE_ENABLED   /**< Button enabled */
} aceButtonHal_activationState_t;


/** Configure parameter for button */
typedef struct aceButtonHal_params {
    aceButtonHal_activationState_t activationState;    /**< Button activation state */
    aceButtonHal_callback eventCallback;               /**< Button event callback function */
} aceButtonHal_params_t;


/** Button Info */
typedef struct aceButtonHal_info {
    uint8_t buttonIndex;
    char    buttonName[ACEBUTTONHAL_MAX_LEN_BUTTON_NAME];
} aceButtonHal_info_t;


/**
 * Open must be called before using any button HAL api.
 *
 * @return ace_status_t: Return zero if Open is successful, or any non-zero error code.
 */
ace_status_t aceButtonHal_open(void);


/**
 * Close must be called when exiting button HAL api
 *
 * @return ace_status_t: Return zero if Close is successful, or any non-zero error code.
 */
ace_status_t aceButtonHal_close(void);


/**
 * Get number of buttons on this device.  The caller must
 * allocate the uint8_t pointer for numButtons.
 *
 * @param numButtons: pointer to number of button on this device.
 * @return ace_status_t: Return zero if button number retrieval is successful, or any non-zero error code.
 */
ace_status_t aceButtonHal_getNumButtons(uint8_t* numButtons);


/**
 * Get button info including button index and name.  The caller must
 * allocate enough memory for aceButtonHal_info_t pointer based on the return
 * value of aceButtonHal_getNumButtons().
 *
 * @param buttonIndex: index of the button to get Info for
 * @param buttonInfo: The pointer to store button_info.
 * @return ace_status_t: Return zero if button info retrieval is successful, or any non-zero error code.
 */
ace_status_t aceButtonHal_getInfo(uint8_t buttonIndex, aceButtonHal_info_t* buttonInfo);


/**
 * Get the parameters from a specified button. The caller must allocate
 * the memory for aceButtonHal_params_t pointer.
 *
 * @param buttonIndex: The specified button index.
 * @param params: Pointer to the button parameters.
 * @return ace_status_t: Return zero if successful, or any non-zero error code.
 */
ace_status_t aceButtonHal_getParams(uint8_t buttonIndex, aceButtonHal_params_t* params);


/**
 * Set the parameters to a specified button.
 *
 * @param buttonIndex: The specified button index.
 * @param params: Pointer to the button parameters.
 * @return ace_status_t: Return zero if successful, or any non-zero error code.
 */
ace_status_t aceButtonHal_setParams(uint8_t buttonIndex, const aceButtonHal_params_t* params);

/**
 * @}
 */    // DEPRECATED

#ifdef __cplusplus
}
#endif

#endif /* ACE_SDK_HAL_BUTTON_H_ */
/** @} */
