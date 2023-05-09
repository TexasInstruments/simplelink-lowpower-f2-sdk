/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All rights reserved.
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
 * @file hal_virtualkey.h
 *
 * @brief ACE Virtual Key HAL is designed to accept the key events coming from
 * the peripheral devices, such as Bluetooth headphone or remote control.
 * When user presses a key on such device, a key event will be generated then
 * processed by this HAL.
 * @addtogroup ACE_HAL_VIRTUAL_KEY
 *
 * USAGE
 * -----
 *
 * Steps to setup and use Virtual Key HAL interface:<br>
 * 1) aceVirtualKeyHal_open() needs to be called first to set up the detection
 *    thread ready for use.<br>
 * 2) Set up the timeout value, callback function, and enablement via
 *    aceVirtualKeyHal_setParams(). aceVirtualKeyHal_getParams() can be used
 *    to retrieve current activation state.<br>
 * 3) Use aceVirtualKeyHal_close() to exit.
 *
 * @{
 */

#ifndef ACE_SDK_HAL_VIRTUALKEY_H_
#define ACE_SDK_HAL_VIRTUALKEY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ace/ace_status.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @cond DEPRECATED
 * @deprecated Please use the new symbols.
 * @{
 */
#define ACEVIRTUALKEYHAL_MAX_LEN_DEVICE_NAME ACE_VIRTUALKEY_HAL_MAX_LEN_DEVICE_NAME
/**
 * @}
 * @endcond
 */

/** The maximum size of device name */
#define ACE_VIRTUALKEY_HAL_MAX_LEN_DEVICE_NAME 32

/** @deprecated The maxmum timeout value in milliseconds (equivalent to no timeout) */
#define ACEVIRTUALKEYHAL_MAX_TIMEOUT 0x7FFFFFFF

/** Key status information */
typedef enum aceVirtualKeyHal_eventType {
    EVENT_SYN = 0x00,      /**< @deprecated Marker to separate events */
    EVENT_KEY = 0x01,      /**< @deprecated Event from key-like devices */
    EVENT_ABS = 0x03,      /**< @deprecated Event to describe absolute axis value changes */
    EVENT_MSC = 0x04,      /**< @deprecated Event to describe miscellaneous input data */
    EVENT_KEY_DOWN = 0x10, /**< Key is pressed event */
    EVENT_KEY_UP = 0x11    /**< Key is released event */
} aceVirtualKeyHal_eventType_t;

/** Virtual Key device information */
typedef struct aceVirtualKeyHal_device {
    char deviceName[ACEVIRTUALKEYHAL_MAX_LEN_DEVICE_NAME]; /**< @deprecated device name */
    uint16_t vendorId;                                     /**< @deprecated vendor ID */
    uint16_t productId;                                    /**< @deprecated product ID */
    const char* device_name;                               /**< device name */
    uint16_t vendor_id;                                    /**< vendor ID */
    uint16_t product_id;                                   /**< product ID */
} aceVirtualKeyHal_device_t;

/** Virtual Key event information */
typedef struct aceVirtualKeyHal_event {
    uint32_t sec;                                          /**< @deprecated second part of time stamp when event happens */
    uint32_t usec;                                         /**< @deprecated microsecond part of time stamp when event happens */
    aceVirtualKeyHal_eventType_t eventType;                /**< @deprecated event type */
    int32_t value;                                         /**< @deprecated event value */
    const char* device_name;                               /**< device name */
    aceVirtualKeyHal_eventType_t event_type;               /**< event type */
    uint16_t code;                                         /**< event code */
} aceVirtualKeyHal_event_t;

/** Callback type information */
typedef enum aceVirtualKeyHal_cbType {
    DEVICE_ADD = 0x80, /**< Callback to notify when an input device is added */
    DEVICE_REMOVE,     /**< Callback to notify when an input device is removed */
    DEVICE_NAME,       /**< @deprecated Callback to notify the device name of incoming key events */
    DEVICE_KEY         /**< Callback to notify when key events are received */
} aceVirtualKeyHal_cbType_t;

/**
* Callback function to receive virtual key event
*
* @param[in] cb_type: type of callback.
* @param[in] num_events: number of events.
* @param[in] data: Pointer to either device or event information.
* @return returns zero on success, and non-zero value otherwise
*/
typedef int (*aceVirtualKeyHal_callback_t)(aceVirtualKeyHal_cbType_t cb_type, uint8_t num_events, void* data);

/** Configure parameter for virtual key */
typedef struct aceVirtualKeyHal_params {
    bool virtualKeyEnabled;                    /**< @deprecated Virtual key input enabled */
    aceVirtualKeyHal_callback_t eventCallback; /**< @deprecated Virtual key callback function */
    uint32_t timeoutInMs;                      /**< @deprecated Timeout to wait between input events in milliseconds */
    bool virtual_key_enabled;                   /**< Virtual key input enabled */
    aceVirtualKeyHal_callback_t event_callback; /**< Virtual key callback function */
} aceVirtualKeyHal_params_t;

/**
* Open must be called before using any virtual key HAL API.
*
* @return ACE_STATUS_OK on success, and a negative value from @ref ace_status_t otherwise
*/
ace_status_t aceVirtualKeyHal_open(void);

/**
 * Close must be called when exiting virtual key HAL API
 *
 * @return ACE_STATUS_OK on success, and a negative value from @ref ace_status_t otherwise
 */
ace_status_t aceVirtualKeyHal_close(void);

/**
 * Get the parameters. The caller must allocate the memory for
 * aceVirtualKeyHal_params_t pointer.
 *
 * @param[in,out] params: Pointer to the virtual key parameters.
 * @return ACE_STATUS_OK on success, and a negative value from @ref ace_status_t otherwise
 */
ace_status_t aceVirtualKeyHal_getParams(aceVirtualKeyHal_params_t* params);

/**
 * Set the parameters.
 *
 * @param[in] params: Pointer to the virtual key parameters.
 * @return ACE_STATUS_OK on success, and a negative value from @ref ace_status_t otherwise
 */
ace_status_t aceVirtualKeyHal_setParams(const aceVirtualKeyHal_params_t* params);

#ifdef __cplusplus
}
#endif

#endif /* ACE_SDK_HAL_VIRTUALKEY_H_ */
/** @} */
